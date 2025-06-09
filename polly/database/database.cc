// Copyright RCT Power 2025
// Author: ivan.yu (ivan.yu@rct-power.com.cn)

#include "rctems/database/database.h"

#include <assert.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <memory>
#include <map>
#include <utility>

#include "boost/url.hpp"
#include "rctems/common/logging.h"
#include "boost/lexical_cast.hpp"

namespace rctems {

using std::chrono::steady_clock;
using std::chrono::duration_cast;

namespace {
inline const std::string
GetUrlParam(const boost::urls::params_ref& params, const std::string& key) {
    auto it = params.find(key);
    if (it != params.end() && (*it).has_value) {
        return (*it).value;
    }
    return {};
}

}  // anonymous namespace

Connection::Connection() {}
Connection::~Connection() {}

// Connection builder
#ifdef RCTEMS_USE_MYSQL
ConnectionPtr MysqlBuilder(const DataSource& ds);
#endif

DataSource::DataSource(): DataSource::DataSource("") {}

DataSource::DataSource(const std::string& str)
        : scheme_(), host_(), port_(0),
          dbname_(), user_(), password_(),
          connect_timeout_(kConnectTimeout),
          read_timeout_(kReadTimeout),
          write_timeout_(kWriteTimeout),
          builder_() {
#ifdef RCTEMS_USE_MYSQL
    builder_["mysql"] = MysqlBuilder;
#endif
    if (!str.empty())
        SetUrl(str);
}

DataSource::~DataSource() {}

void DataSource::SetUrl(const std::string& str) {
    auto res = boost::urls::parse_uri(str);
    if (res.has_error()) {
        RCTEMS_FATAL() << "SetUrl(): " << res.error().message();
        std::abort();
    }

    boost::urls::url url = res.value();
    scheme_ = url.scheme();
    auto const& params = url.params();

    if (url.has_port())
        port_ = url.port_number();

    if (url.host().empty()) {
        host_ = url.path();
        dbname_ = GetUrlParam(params, "dbname");
    } else {
        host_ = url.host();
        dbname_ = url.path();
        if (dbname_.empty())
            dbname_ = GetUrlParam(params, "dbname");
        if (!dbname_.empty() && dbname_.front() == '/')
            dbname_ = dbname_.substr(1);
    }

    user_ = url.user();
    if (user_.empty())
        user_ = GetUrlParam(params, "username");

    password_ = url.password();
    if (password_.empty())
        password_ = GetUrlParam(params, "password");

    auto connect_timeout = GetUrlParam(params, "connect_timeout");
    if (!connect_timeout.empty()) {
        try {
            connect_timeout_ = boost::lexical_cast<int>(connect_timeout);
        } catch (const std::exception& exc) {
            RCTEMS_FATAL() << "lexical_cast(): " << exc.what();
            std::abort();
        }
    }

    auto read_timeout = GetUrlParam(params, "read_timeout");
    if (!read_timeout.empty()) {
        try {
            read_timeout_ = boost::lexical_cast<int>(read_timeout);
        } catch (const std::exception& exc) {
            RCTEMS_FATAL() << "lexical_cast(): " << exc.what();
            std::abort();
        }
    }

    auto write_timeout = GetUrlParam(params, "write_timeout");
    if (!write_timeout.empty()) {
        try {
            write_timeout_ = boost::lexical_cast<int>(write_timeout);
        } catch (const std::exception& exc) {
            RCTEMS_FATAL() << "lexical_cast(): " << exc.what();
            std::abort();
        }
    }
}

std::unique_ptr<Connection> DataSource::GetConnection() {
    auto const it = builder_.find(Scheme());
    if (it != builder_.cend()) {
        return it->second(*this);
    }
    RCTEMS_FATAL() << "Unimplemented scheme: " << Scheme();
    std::abort();
}

const std::string& DataSource::Scheme() const {
    return scheme_;
}

const std::string& DataSource::Host() const {
    return host_;
}

int DataSource::Port() const {
    return port_;
}

const std::string& DataSource::DatabaseName() const {
    return dbname_;
}

const std::string& DataSource::User() const {
    return user_;
}

const std::string& DataSource::Password() const {
    return password_;
}

int DataSource::GetConnectTimeout() const {
    return connect_timeout_;
}

int DataSource::GetReadTimeout() const {
    return read_timeout_;
}

int DataSource::GetWriteTimeout() const {
    return write_timeout_;
}

}  // namespace rctems
