// Copyright 2022 The Oak Authors.

#ifndef POLLY_DATABASE_POOL_H_
#define POLLY_DATABASE_POOL_H_

#include <string>

namespace polly {


class Connection {};

class ConnectionPool {};

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

 private:
    int max_conn_;
    int idle_timeout_;
    int conn_timeout_;
    std::string url_;
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

#endif  // POLLY_DATABASE_POOL_H_
