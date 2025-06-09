// Copyright RCT Power 2025
// Author: ivan.yu (ivan.yu@rct-power.com.cn)

#ifndef RCTEMS_DATABASE_DATABASE_H_
#define RCTEMS_DATABASE_DATABASE_H_

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <chrono>  // NOLINT

#include "rctems/common/stubs.h"

namespace rctems {
class DataSource;

class Connection {
 public:
    using Record = std::vector<std::string>;
    using RecordSet = std::vector<Record>;

    virtual ~Connection();
    virtual bool Execute(const std::string& sql, RecordSet* res) = 0;

 protected:
    Connection();

 private:
    RCTEMS_DISALLOW_COPY_AND_ASSIGN(Connection);
};

using ConnectionPtr = std::unique_ptr<Connection>;

class DataSource {
 public:
    static constexpr const int kConnectTimeout = 30;   // seconds
    static constexpr const int kReadTimeout = 60;      // seconds
    static constexpr const int kWriteTimeout = 60;     // seconds

 public:
    using ConnectionBuilder = ConnectionPtr (*)(const DataSource&);

 public:
    DataSource();
    explicit DataSource(const std::string& str);
    ~DataSource();

    void SetUrl(const std::string& str);
    ConnectionPtr GetConnection();

    const std::string& Scheme() const;
    const std::string& Host() const;
    int Port() const;
    const std::string& DatabaseName() const;
    const std::string& User() const;
    const std::string& Password() const;
    int GetConnectTimeout() const;
    int GetReadTimeout() const;
    int GetWriteTimeout() const;

 private:
    std::string scheme_;
    std::string host_;
    int port_;
    std::string dbname_;
    std::string user_;
    std::string password_;
    int connect_timeout_;
    int read_timeout_;
    int write_timeout_;
    std::map<std::string, ConnectionBuilder> builder_;
};

}  // namespace rctems
#endif  // RCTEMS_DATABASE_DATABASE_H_
