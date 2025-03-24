// Copyright 2022 The Oak Authors.

#include "polly/database.h"

#include <assert.h>
#include <stdexcept>

#define POLLY_USE_MYSQL_DATASOURCE
#ifdef POLLY_USE_MYSQL_DATASOURCE
#   include <mysql/mysql.h>
#endif

namespace polly {
using std::chrono::steady_clock;
using std::chrono::duration_cast;

Connection::Connection(ConnectionPool* pool)    \
        : pool_(pool), start_(), is_alive_(false) {}

Connection::~Connection() {}

bool Connection::IsAlive() const {
    return is_alive_;
}

void Connection::SetIsAlive(bool is_alive) {
    is_alive_ = is_alive;
}

std::chrono::seconds Connection::Lifetime() const {
    assert(start_ > steady_clock::time_point::min());
    auto lifetime = steady_clock::now() - start_;
    return duration_cast<std::chrono::seconds>(lifetime);
}

void Connection::ResetLifetime() {
    start_ = steady_clock::now();
}

#ifdef POLLY_USE_MYSQL_DATASOURCE
class MySQLConnection: public Connection {
 public:
    explicit MySQLConnection(ConnectionPool* pool);
    virtual ~MySQLConnection();

    virtual bool Execute(const std::string& sql, RecordSet* res);

 private:


 private:
    MYSQL* conn_;
};

MySQLConnection::MySQLConnection(ConnectionPool* pool)  \
        : Connection(pool), conn_(nullptr) {}

MySQLConnection::~MySQLConnection() {
    if (conn_)
        mysql_close(conn_);
}

bool MySQLConnection::Execute(const std::string& sql, RecordSet* res) {

}

#endif  // POLLY_USE_MYSQL_DATASOURCE

Connection::Impl::Impl(ConnectionPool* pool)
        : pool_(pool), is_alive_(false), life_time_(0) {}

class ConnectionPool::Impl {
 public:
    Impl(const std::string& url, size_t max_pool_size);

    void SetInitialSize(size_t initial_size);
    size_t GetInitialSize() const;

    void SetMinIdleSize(size_t min_idle_size);
    size_t GetMinIdleSize() const;

    const std::string& GetUrl() const;

    Connection GetConnection();
    void Recovery(Connection* conn);

 private:
    size_t max_pool_size_;
    size_t min_idle_;
    int idle_timeout_;
    int conn_timeout_;
    std::string url_;
    std::vector<Connection
};


}  // namespace polly
