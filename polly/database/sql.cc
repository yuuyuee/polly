// Copyright (C) 2025 Ivan Yu (yuyue2200@hotmail.com)
//
// This file is placed under the LGPL.  Please see the file
// COPYING for more details.
//
// SPDX-License-Identifier: LGPL-2.1

#include "polly/database/sql.h"

#include <cassert>
#include <set>
#include "polly/strings.h"

namespace polly {
namespace sql_internal {

// Simple SQL escape
std::string Escape(const std::string_view& value) {
    std::string escaped;
    for (char c : value) {
        if (c == '\'')
            escaped += '\'';
        escaped += c;
    }
    return "'" + escaped + "'";
}

std::set<std::string> g_sql_func;
}  // namespace sql_internal

void AddSqlFuncion(std::string_view func) {
    sql_internal::g_sql_func.insert(ToUpper(func));
}

bool IsSqlFunction(std::string_view func) {
    auto pos = func.find('(');
    if (pos != std::string_view::npos)
        func.remove_suffix(func.size() - pos);
    auto const it = sql_internal::g_sql_func.find(ToUpper(func));
    return it != sql_internal::g_sql_func.cend();
}

// Build SELECT statements
std::string Select::Build() const {
    std::string sql;
    sql.reserve(1024);

    sql.append("SELECT ");

    if (columns_.empty()) {
        sql.append("*");
    } else {
        sql.append(Join(columns_, ", "));
    }

    sql.append(" FROM ");
    sql.append(table_name_);

    if (!where_condition_.empty()) {
        sql.append(" WHERE ");
        sql.append(where_condition_);
    }

    if (!orderby_columns_.empty()) {
        sql.append(" ORDER BY ");
        sql.append(orderby_columns_);
    }

    if (limit_ >= 0) {
        sql.append(" LIMIT ");
        sql.append(std::to_string(limit_));
    }

    if (offset_ > 0) {
        sql.append(" OFFSET ");
        sql.append(std::to_string(offset_));
    }

    return sql;
}

// Build INSERT statements
std::string InsertInto::Build() const {
    std::string sql;
    sql.reserve(1024);

    sql.append("INSERT INTO ");
    sql.append(table_name_);

    if (!columns_.empty()) {
        sql.push_back('(');
        sql.append(Join(columns_, ", "));
        sql.push_back(')');
    }

    sql.append(" VALUES ");
    for (size_t i = 0; i < value_list_.size(); ++i) {
        if (i > 0)
            sql.append(", ");
        sql.push_back('(');
        assert(columns_.size() == value_list_[i].size());
        sql.append(Join(value_list_[i], ", "));
        sql.push_back(')');
    }
    return sql;
}

// Build UPDATE statements
std::string Update::Build() const {
    std::string sql;
    sql.reserve(1024);

    sql.append("UPDATE ");
    sql.append(table_name_);
    sql.append(" SET ");
    sql.append(Join(set_assignments_, ", "));
    if (!where_condition_.empty()) {
        sql.append(" WHERE ");
        sql.append(where_condition_);
    }

    return sql;
}

// Build DELETE statements
std::string DeleteFrom::Build() const {
    std::string sql;
    sql.reserve(1024);

    sql.append("DELETE FROM ");
    sql.append(table_name_);
    if (!where_condition_.empty()) {
        sql.append(" WHERE ");
        sql.append(where_condition_);
    }
    return sql;
}

}  // namespace polly
