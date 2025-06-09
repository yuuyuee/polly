// Copyright RCT Power 2025
// Author: ivan.yu (ivan.yu@rct-power.com.cn)

#ifndef RCTEMS_DATABASE_SQL_H_
#define RCTEMS_DATABASE_SQL_H_

#include <string>
#include <vector>
#include <utility>
#include <iterator>
#include <type_traits>
#include <string_view>
#include <initializer_list>

#include "rctems/common/format.h"

namespace rctems {
void AddSqlFuncion(std::string_view func);
bool IsSqlFunction(std::string_view func);

namespace sql_internal {
// Simple SQL escape
std::string Escape(const std::string_view& value);

// Value serialize
template <typename T>
std::string SerializeValue(const T& value) {
    if constexpr (std::is_same_v<T, std::nullptr_t>) {
        return "NULL";
    } else if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(value);
    } else if constexpr (std::is_convertible_v<T, std::string_view>) {
        return IsSqlFunction(value) ? std::string(value) : Escape(value);
    } else {
        static_assert(sizeof(T) == 0, "Unsupported type");
    }
}

}  // namespace sql_internal

// Examples:
// std::string sql = Select({"field1", "field2"})
//                  .From("table_name")
//                  .Where("field > {} AND field2 < {}", 0, 100)
//                  .OrderBy("field")
//                  .Offset(10)
//                  .Limit(10)
//                  .Build();
class Select {
 public:
    Select(std::initializer_list<std::string_view> columns = {}) {
        columns_ = columns;
    }
    ~Select() = default;

    // FROM statements
    Select& From(const std::string_view& table) {
        table_name_ = table;
        return *this;
    }

    // Indicate the columns
    Select& Columns(std::initializer_list<std::string_view> columns) {
        columns_ = columns;
        return *this;
    }

    // WHERE statements
    template <typename... Args>
    Select& Where(format_string<Args...> fmt, Args&&... args) {
        where_condition_ =
            format(std::move(fmt),
                sql_internal::SerializeValue(std::forward<Args>(args))...);
        return *this;
    }

    // ORDER BY statements
    Select& OrderBy(std::string_view column, std::string_view direction = "ASC") {
        orderby_columns_ = column;
        if (!direction.empty()) {
            orderby_columns_.append(" ");
            orderby_columns_.append(direction);
        }
        return *this;
    }

    // LIMIT statements
    Select& Limit(int count) {
        limit_ = count;
        return *this;
    }

    // OFFSET statements
    Select& Offset(int count) {
        offset_ = count;
        return *this;
    }

    // Build object as an strings
    std::string Build() const;

 private:
    std::string_view table_name_;
    std::vector<std::string_view> columns_;
    std::string where_condition_;
    std::string orderby_columns_;
    int limit_ = -1;
    int offset_ = -1;
};

// Examples:
// std::string sql = InsertInto("table_name")
//                  .Columns({"field1", "field2"})
//                  .Values("value1", value2)
//                  .Values("value3", value3)
//                  .Build();
class InsertInto {
 public:
    explicit InsertInto(const std::string_view& table): table_name_(table) {}
    ~InsertInto() = default;

    // Insert fields
    InsertInto& Columns(std::initializer_list<std::string_view> columns) {
        columns_ = columns;
        return *this;
    }

    //  VALUES statement (maybe multi-line)
    template <typename... Args>
    InsertInto& Values(const Args&... args) {
        std::vector<std::string> value;
        (value.push_back(sql_internal::SerializeValue(args)), ...);
        value_list_.push_back(std::move(value));
        return *this;
    }

    // Build object as an strings
    std::string Build() const;

 private:
    std::string_view table_name_;
    std::vector<std::string_view> columns_;
    std::vector<std::vector<std::string>> value_list_;
};

// Examples:
// std::string sql = Update("table_name")
//                  .Set("field", value)
//                  .Set("field1", value1)
//                  .Where("field > {}", 0)
//                  .Build();
class Update {
 public:
    explicit Update(const std::string_view& table): table_name_(table) {}

    // SET statements
    template <typename T>
    Update& Set(const std::string_view& column, T&& value) {
        std::string assign;
        assign.append(column);
        assign.append(" = ");
        assign.append(sql_internal::SerializeValue(std::forward<T>(value)));
        set_assignments_.push_back(std::move(assign));
        return *this;
    }

    // WHERE statements
    template <typename... Args>
    Update& Where(format_string<Args...> fmt, Args&&... args) {
        where_condition_ =
            format(std::move(fmt),
                sql_internal::SerializeValue(std::forward<Args>(args))...);
        return *this;
    }

    // Build object as an strings
    std::string Build() const;

 private:
    std::string_view table_name_;
    std::string where_condition_;
    std::vector<std::string> set_assignments_;
};

// Examples:
// std::string sql = DeleteFrom("table_name")
//                  .Where("field > {}", 0)
//                  .Build();
class DeleteFrom {
 public:
    explicit DeleteFrom(const std::string_view& table): table_name_(table) {}

    // WHERE statements
    template <typename... Args>
    DeleteFrom& Where(format_string<Args...> fmt, Args&&... args) {
        where_condition_ =
            format(std::move(fmt),
                sql_internal::SerializeValue(std::forward<Args>(args))...);
        return *this;
    }

    // Build object as an strings
    std::string Build() const;

 private:
    std::string_view table_name_;
    std::string where_condition_;
};

}  // namespace rctems

#endif  // RCTEMS_DATABASE_SQL_H_
