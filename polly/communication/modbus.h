// Copyright RCT Power 2025
// Author: ivan.yu (ivan.yu@rct-power.com.cn)

#ifndef RCTEMS_COMMUNICATION_MODBUS_H_
#define RCTEMS_COMMUNICATION_MODBUS_H_

#include <cstdint>
#include <string>
#include <type_traits>

#include "rctems/common/stubs.h"
#include "rctems/common/asio.h"

#include "boost/url.hpp"
#include "modbus/modbus.h"

namespace rctems {

class ModbusClient {
 public:
    explicit ModbusClient(const std::string& str);
    explicit ModbusClient(const boost::urls::url& url);
    ~ModbusClient();

    ModbusClient(ModbusClient&& other) noexcept;
    ModbusClient& operator=(ModbusClient&& other) noexcept;

    const std::string& PrettyHost() const;

    // function code: 0x03
    bool ReadRegisters(int addr, int num, uint16_t* dest);
    bool ReadRegisters(int addr, int num, int16_t* dest);

    // function code: 0x06
    bool WriteRegister(int addr, uint16_t value);
    bool WriteRegister(int addr, int16_t value);

    void Close();

 private:
    void Init(const boost::urls::url& url);
    bool Connect();

 private:
    std::string scheme_;
    std::string host_;
    std::string pretty_host_;
    int port_ = 502;
    int res_timeout_ms_ = 500;
    modbus_t* context_ = nullptr;

    RCTEMS_DISALLOW_COPY_AND_ASSIGN(ModbusClient);
};

static_assert(std::is_move_assignable_v<rctems::ModbusClient>);
static_assert(std::is_move_constructible_v<rctems::ModbusClient>);

inline bool ModbusClient::ReadRegisters(int addr, int num, int16_t* dest) {
    return ReadRegisters(addr, num, reinterpret_cast<uint16_t*>(dest));
}

inline bool ModbusClient::WriteRegister(int addr, int16_t value) {
    union {
        int16_t i16;
        uint16_t u16;
    } v;
    v.i16 = value;
    return WriteRegister(addr, v.u16);
}

class ModbusServer {
 public:
    struct Layout {
        int nb_registers;
        uint16_t *tab_registers;
    };

    explicit ModbusServer(const std::string& str);
    explicit ModbusServer(const boost::urls::url& url);
    ~ModbusServer();

    // function code: 0x03
    void ReadRegisters(int addr, int num, uint16_t* dest) const;
    void ReadRegisters(int addr, int num, int16_t* dest) const;

    // function code: 0x06
    void WriteRegisters(int addr, int num, const uint16_t* dest);
    void WriteRegisters(int addr, int num, const int16_t* dest);

    void Start();
    bool StartOnce();
    void Stop();

 private:
    class Session;
    friend class Session;

    void Init(const boost::urls::url& url);
    void Accept();

 private:
    std::string scheme_;
    std::string host_;
    int port_ = 502;
    int req_timeout_ms_ = 500;
    modbus_t* context_ = nullptr;
    modbus_mapping_t* map_ = nullptr;
    size_t size_ = 0x1000u;
    bool changged_ = false;
    bool stopped_ = false;
    asio::io_context io_context_;
    asio::ip::tcp::acceptor acceptor_;

    RCTEMS_DISALLOW_COPY_AND_ASSIGN(ModbusServer);
    RCTEMS_DISALLOW_MOVE_COPY_AND_MOVE_ASSIGN(ModbusServer);
};

inline void ModbusServer::ReadRegisters(int addr,
                                        int num,
                                        int16_t* dest) const {
    ReadRegisters(addr, num, reinterpret_cast<uint16_t*>(dest));
}

inline void ModbusServer::WriteRegisters(int addr,
                                         int num,
                                         const int16_t* dest) {
    WriteRegisters(addr, num, reinterpret_cast<const uint16_t*>(dest));
}

}  // namespace rctems

#endif  // RCTEMS_COMMUNICATION_MODBUS_H_
