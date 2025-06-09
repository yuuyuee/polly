// Copyright (C) 2025 Ivan Yu (yuyue2200@hotmail.com)
//
// This file is placed under the LGPL.  Please see the file
// COPYING for more details.
//
// SPDX-License-Identifier: LGPL-2.1

#include "polly/communication/modbus.h"

#include <unistd.h>

#include <cassert>
#include <memory>
#include <cerrno>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <utility>

#include "polly/format.h"
#include "polly/logging.h"
#include "polly/strings.h"
#include "modbus/modbus.h"
#include "boost/lexical_cast.hpp"

namespace polly {
boost::urls::url ParseUrl(const std::string& url) {
    auto res = boost::urls::parse_uri(url);
    if (res.has_error()) {
        RCTEMS_FATAL() << "ParseUrl: " << res.error().message();
        std::abort();
    }
    return res.value();
}

// ModbusClient
ModbusClient::ModbusClient(const std::string& str) {
    boost::urls::url url = ParseUrl(str);
    Init(url);
    pretty_host_ = format("{}:{}", host_, port_);
}

ModbusClient::ModbusClient(const boost::urls::url& url) {
    Init(url);
    pretty_host_ = format("{}:{}", host_, port_);
}

ModbusClient::~ModbusClient() {
    Close();
}

ModbusClient::ModbusClient(ModbusClient&& other) noexcept {
    std::swap(scheme_, other.scheme_);
    std::swap(host_, other.host_);
    std::swap(pretty_host_, other.pretty_host_);
    std::swap(port_, other.port_);
    std::swap(res_timeout_ms_, other.res_timeout_ms_);
    std::swap(context_, other.context_);
}

ModbusClient& ModbusClient::operator=(ModbusClient&& other) noexcept {
    if (this != &other) {
        Close();
        std::swap(scheme_, other.scheme_);
        std::swap(host_, other.host_);
        std::swap(pretty_host_, other.pretty_host_);
        std::swap(port_, other.port_);
        std::swap(res_timeout_ms_, other.res_timeout_ms_);
        std::swap(context_, other.context_);
    }
    return *this;
}

const std::string& ModbusClient::PrettyHost() const {
    return pretty_host_;
}

bool ModbusClient::ReadRegisters(int addr, int num, uint16_t* dest) {
    assert(addr >= 0 && "Invalid addr");
    assert(num >= 0 && "Invalid num");
    assert(dest != nullptr && "Invalid dest");

    if (context_ == nullptr && !Connect())
        return false;

    while (num > 0) {
        // Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 3 page 15)
        // Quantity of Registers to read (2 bytes): 1 to 125 (0x7D)
        // (chapter 6 section 12 page 31)
        // Quantity of Registers to write (2 bytes) 1 to 123 (0x7B)
        // (chapter 6 section 17 page 38)
        // Quantity of Registers to write in R/W registers (2 bytes) 1 to 121 (0x79)
        int nb = std::min(num, 0x79);
        if (modbus_read_registers(context_, addr, nb, dest) != nb) {
            RCTEMS_ERROR() << "modbus_read_registers(): "
                    << modbus_strerror(errno);
            Close();
            return false;
        }
        addr += nb;
        dest += nb;
        num -= nb;
    }
    return true;
}

bool ModbusClient::WriteRegister(int addr, uint16_t value) {
    assert(addr >= 0 && "Invalid addr");

    if (context_ == nullptr && !Connect())
        return false;

    if (modbus_write_register(context_, addr, value) != 1) {
        RCTEMS_ERROR() << "modbus_write_register(): " << modbus_strerror(errno);
        Close();
        return false;
    }
    return true;
}

void ModbusClient::Init(const boost::urls::url& url) {
    scheme_ = url.scheme();
    host_ = url.host();
    if (url.has_port())
    port_ = url.port_number();
    auto it = url.params().find("res_timeout_ms");
    if (it != url.params().end() && (*it).has_value) {
        try {
            res_timeout_ms_ = boost::lexical_cast<int>((*it).value);
        } catch (const std::exception& exc) {
            RCTEMS_FATAL() << "lexical_cast(): " << exc.what();
            std::abort();
        }
    }
}

bool ModbusClient::Connect() {
    Close();

    if (scheme_ == "modbus" || scheme_ == "modbus-tcp") {
        const char* host = !host_.empty() ? host_.c_str() : nullptr;
        context_ = modbus_new_tcp(host, port_);
        if (!context_) {
            RCTEMS_ERROR() << "modbus_new_tcp(): " << modbus_strerror(errno);
            return false;
        }
    } else {
        RCTEMS_ERROR() << "Unimplemented scheme: " << scheme_;
        return false;
    }

#ifndef NDEBUG
    // modbus_set_debug(context_, 1);
#endif

    int res_timeout_s = res_timeout_ms_ / 1000;
    int res_timeout_us = res_timeout_ms_ % 1000 * 1000;
    modbus_set_response_timeout(context_, res_timeout_s, res_timeout_us);
    if (modbus_connect(context_) != 0) {
        Close();
        RCTEMS_ERROR() << "modbus_connect(): " << modbus_strerror(errno);
        return false;
    }
    return true;
}

void ModbusClient::Close() {
    if (context_) {
        modbus_close(context_);
        modbus_free(context_);
        context_ = nullptr;
    }
}

// ModbusServer::Session
class ModbusServer::Session
        : public std::enable_shared_from_this<Session> {
 public:
    Session(asio::ip::tcp::socket socket, ModbusServer* server);

    void Start();

 private:
    void Response();

 private:
    asio::ip::tcp::socket socket_;
    ModbusServer* server_;
};

ModbusServer::Session::Session(asio::ip::tcp::socket socket,
                               ModbusServer* server)
        : socket_(std::move(socket)), server_(server) {
    // socket_.non_blocking(true);
}

void ModbusServer::Session::Start() {
    auto self = shared_from_this();
    socket_.async_wait(
        asio::ip::tcp::socket::wait_read,
        [this, self](auto ec) {
            if (ec) {
                RCTEMS_ERROR() << "async_wait_read: " << ec;
                socket_.shutdown(asio::socket_base::shutdown_both);
                socket_.close();
                return;
            }

            Response();
        });
}

void ModbusServer::Session::Response() {
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    modbus_set_socket(server_->context_, socket_.native_handle());
    int res = modbus_receive(server_->context_, query);
    if (res >= 0) {
        if (res != 0) {
            res = modbus_reply(server_->context_, query, res, server_->map_);
        }  // else indication request is a query for another slave in RTU mode

        if (res != -1) {
            server_->changged_ = !!res;
            Start();
            return;
        }
    }

    RCTEMS_INFO() << "Connection closed: " << socket_.native_handle()
            << ": " << modbus_strerror(errno);
    socket_.shutdown(asio::socket_base::shutdown_both);
    socket_.close();
}

// ModbusServer
ModbusServer::ModbusServer(const std::string& str)
        : ModbusServer(ParseUrl(str)) {}

ModbusServer::ModbusServer(const boost::urls::url& url)
        : io_context_(BOOST_ASIO_CONCURRENCY_HINT_UNSAFE),
          acceptor_(io_context_) {
    Init(url);
    Accept();
}

ModbusServer::~ModbusServer() {
    if (!stopped_) {
        Stop();
        stopped_ = true;
    }
    modbus_free(context_);
    modbus_mapping_free(map_);
}

void ModbusServer::ReadRegisters(int addr, int num, uint16_t* dest) const {
    assert(addr >= 0 && "Invalid addr");
    assert(num >= 0 && "Invalid num");
    assert(addr + num <= map_->nb_registers && "Out of range");
    assert(dest != nullptr && "Invalid dest");

    for (int i = 0; i < num; ++i) {
        dest[i] = map_->tab_registers[addr + i];
    }
}

void ModbusServer::WriteRegisters(int addr,
                                  int num,
                                  const uint16_t* dest) {
    assert(addr >= 0 && "Invalid addr");
    assert(num >= 0 && "Invalid num");
    assert(addr + num <= map_->nb_registers && "Out of range");
    assert(dest != nullptr && "Invalid dest");

    for (int i = 0; i < num; ++i) {
        // TODO(ivan.yu): Consider the big-endian
        map_->tab_registers[addr + i] = dest[i];
    }
}

bool ModbusServer::StartOnce() {
    io_context_.run_one();
    return changged_;
}

void ModbusServer::Start() {
    io_context_.run();
}

void ModbusServer::Stop() {
    acceptor_.cancel();
    io_context_.stop();
    stopped_ = true;
}

void ModbusServer::Init(const boost::urls::url& url) {
    scheme_ = url.scheme();
    host_ = url.host();

    if (url.has_port())
        port_ = url.port_number();

    auto it = url.params().find("req_timeout_ms");
    if (it != url.params().end() && (*it).has_value) {
        try {
            req_timeout_ms_ = boost::lexical_cast<int>((*it).value);
        } catch (const std::exception& exc) {
            RCTEMS_FATAL() << "lexical_cast(): " << exc.what();
            std::abort();
        }
    }

    it = url.params().find("size");
    if (it != url.params().end() && (*it).has_value) {
        try {
            size_ = boost::lexical_cast<int>((*it).value);
        } catch (const std::exception& exc) {
            RCTEMS_FATAL() << "lexical_cast(): " << exc.what();
            std::abort();
        }
    }

    if (scheme_ == "modbus" || scheme_ == "modbus-tcp") {
        const char* host = SafeCstr(host_);
        context_ = modbus_new_tcp(host, port_);
        if (!context_) {
            RCTEMS_FATAL() << "modbus_new_tcp(): " << modbus_strerror(errno);
            std::abort();
        }
    } else {
        RCTEMS_FATAL() << "Unimplemented scheme: " << scheme_;
        std::abort();
    }

    int req_timeout_s = req_timeout_ms_ / 1000;
    int req_timeout_us = req_timeout_ms_ % 1000 * 1000;
    modbus_set_indication_timeout(context_, req_timeout_s, req_timeout_us);

    map_ = modbus_mapping_new(0, 0, size_, 0);
    if (map_ == nullptr) {
        RCTEMS_FATAL() << "modbus_mapping_new(): alloc memory failed";
        std::abort();
    }

    int fd = modbus_tcp_listen(context_, 1024);
    if (fd < 0) {
        RCTEMS_FATAL() << "modbus_tcp_listen(): " << modbus_strerror(errno);
        std::abort();
    }

    acceptor_.assign(asio::ip::tcp::v4(), fd);
}

void ModbusServer::Accept() {
    acceptor_.async_accept([this] (auto ec, auto socket) {
        if (ec) {
            if (ec != asio::error::operation_aborted)
                RCTEMS_ERROR() << "async_accept: " << ec;
        } else {
            RCTEMS_INFO() << "New connection: " << socket.remote_endpoint();
            std::make_shared<ModbusServer::Session>(std::move(socket), this)
                    ->Start();
            Accept();
        }
    });
}

}  // namespace polly
