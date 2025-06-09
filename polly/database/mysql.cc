// Copyright RCT Power 2025
// Author: ivan.yu (ivan.yu@rct-power.com.cn)

#ifdef RCTEMS_USE_MYSQL
#include <mysql/mysql.h>

#include <string>
#include <memory>
#include <iostream>

#include "rctems/database/database.h"
#include "rctems/common/logging.h"
#include "rctems/common/strings.h"

namespace rctems {
namespace {
const struct DatabaseGlobalInitInternal {
    DatabaseGlobalInitInternal() {
        mysql_library_init(0, nullptr, nullptr);
        // RCTEMS_TRACE() << "mysql_library_init()";
    }
    ~DatabaseGlobalInitInternal() {
        mysql_library_end();
        // RCTEMS_TRACE() << "mysql_library_end()";
    }
} database_global_init_internal;

}  // anonymous namespace

class MySQLConnection: public Connection {
 public:
    explicit MySQLConnection(const DataSource& ds);
    virtual ~MySQLConnection();

    bool Execute(const std::string& sql, RecordSet* res) override;
    void Close();

 private:
    bool Connect();

 private:
    const DataSource& ds_;
    MYSQL* conn_;

    MySQLConnection(MySQLConnection const&) = delete;
    void operator=(MySQLConnection const&) = delete;
};

MySQLConnection::MySQLConnection(const DataSource& ds)
        : ds_(ds), conn_(nullptr) {}

MySQLConnection::~MySQLConnection() {
    Close();
}

bool MySQLConnection::Execute(const std::string& sql, RecordSet* res) {
    if (conn_ == nullptr && !Connect())
        return false;

    if (mysql_real_query(conn_, sql.c_str(), sql.size()) != 0) {
        RCTEMS_ERROR() << "mysql_real_query(): " << mysql_error(conn_);
        Close();
        return false;
    }

    MYSQL_RES* result = mysql_store_result(conn_);
    if (!result) {
        auto num_fields = mysql_field_count(conn_);
        if (num_fields != 0) {
            RCTEMS_ERROR() << "mysql_store_result(): " << mysql_error(conn_);
            Close();
            return false;
        }
        return true;
    }

    if (res) {
        auto num_fields = mysql_num_fields(result);
        auto fields = mysql_fetch_fields(result);
        auto lengths = mysql_fetch_lengths(result);
        MYSQL_ROW row;
        RecordSet record_set;
        Record record;
        while ((row = mysql_fetch_row(result))) {
            record.clear();
            for (unsigned int i = 0; i < num_fields; ++i) {
                if (!row[i]) {
                    record.push_back("");
                } else {
                    if (fields[i].type != MYSQL_TYPE_BLOB) {
                        record.push_back(row[i]);
                    } else {
                        record.emplace_back(row[i], lengths[i]);
                    }
                }
            }
            record_set.push_back(record);
        }
        res->swap(record_set);
    }
    mysql_free_result(result);
    return true;
}

bool MySQLConnection::Connect() {
    conn_ = mysql_init(nullptr);
    if (conn_ == nullptr) {
        RCTEMS_ERROR() << "mysql_init(): no enough memory";
        return false;
    }

    int connect_timeout = ds_.GetConnectTimeout();
    mysql_options(conn_, MYSQL_OPT_CONNECT_TIMEOUT, &connect_timeout);
    int read_timeout = ds_.GetConnectTimeout();
    mysql_options(conn_, MYSQL_OPT_READ_TIMEOUT, &read_timeout);
    int write_timeout = ds_.GetConnectTimeout();
    mysql_options(conn_, MYSQL_OPT_WRITE_TIMEOUT, &write_timeout);
    mysql_options(conn_, MYSQL_SET_CHARSET_DIR , "utf8mb4");

    const char* host = nullptr;
    const char* unix_socket = nullptr;
    if (ds_.Host()[0] != '/') {
        host = ds_.Host().c_str();
    } else {
        unix_socket = ds_.Host().c_str();
    }
    int port = ds_.Port() == 0 ? 3306 : ds_.Port();
    const char* user = SafeCstr(ds_.User());
    const char* pass = SafeCstr(ds_.Password());
    const char* dbname = SafeCstr(ds_.DatabaseName());
    if (!mysql_real_connect(conn_, host, user, pass,
            dbname, port, unix_socket, CLIENT_IGNORE_SIGPIPE)) {
        RCTEMS_ERROR() << "mysql_real_connect(): " << mysql_error(conn_);
        Close();
        return false;
    }
    return true;
}

void MySQLConnection::Close() {
    if (conn_) {
        mysql_close(conn_);
        conn_ = nullptr;
    }
}

ConnectionPtr MysqlBuilder(const DataSource& ds) {
    return std::make_unique<MySQLConnection>(ds);
}

}  // namespace rctems

#endif  // RCTEMS_USE_MYSQL
