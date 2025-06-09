// Copyright (C) 2025 Ivan Yu (yuyue2200@hotmail.com)
//
// This file is placed under the LGPL.  Please see the file
// COPYING for more details.
//
// SPDX-License-Identifier: LGPL-2.1

#ifndef POLLY_DATABASE_DATABASE_H_
#define POLLY_DATABASE_DATABASE_H_

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <chrono>  // NOLINT

#include "polly/stubs.h"

namespace polly {
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
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
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

}  // namespace polly
#endif  // POLLY_DATABASE_DATABASE_H_
