// Copyright 2022 The Oak Authors.

#ifndef POLLY_DATABASE_H_
#define POLLY_DATABASE_H_

#include <vector>
#include <string>
#include <any>
#include <memory>
#include <chrono>

namespace polly {
class ConnectionPool;

class Connection {
 public:
    using Record = std::vector<std::any>;
    using RecordSet = std::vector<Record>;

    virtual ~Connection();

    virtual bool Execute(const std::string& sql, RecordSet* res) = 0;

    bool IsAlive() const;
    void SetIsAlive(bool is_alive);
    std::chrono::seconds Lifetime() const;
    void ResetLifetime();

 protected:
    explicit Connection(ConnectionPool* pool);

 private:
    ConnectionPool* pool_;
    std::chrono::steady_clock::time_point start_;
    bool is_alive_;
};

class ConnectionPool {
 public:
    ConnectionPool(const std::string& url, size_t max_pool_size);

    void SetInitialSize(size_t initial_size);
    size_t GetInitialSize() const;

    void SetMinIdleSize(size_t min_idle_size);
    size_t GetMinIdleSize() const;

    const std::string& GetUrl() const;

    Connection GetConnection();
    void Recovery(Connection* conn);

 private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// internal classes

class DataSource {
 public:
    DataSource();

    void SetUrl();
    void SetUser();
    void SetPassword();

    Connection GetConnection(/*connection_timeout*/);
    ConnectionPool GetConnectionPool();

    // void SetInitialSize();
    // void SetMinIdle();

    // void SetMaxLifetime();
    // void SetMaxPoolSize();


    // int GetMaxConnections() const;
    // int GetIdleTimeout() const;
    // int GetConnectionTimeout() const;
    // const std::string& GetUrl() const;

};

// class DataSource;
// class Connection;
// class ConnectionPool;

// internal class
// class ConnectionFactory;
// class MysqlConnectionFactory;
// class PostgresqlConnectionFactory;

// DataSource ds;
// ds.SetPoolSize(100);
// Connection conn = ds.GetConnection();
// ...
// conn.Close();

// ConnectionPool pool(ds);


}  // namespace polly

#endif  // POLLY_DATABASE_H_
