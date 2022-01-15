/**
 * @file sqlite.hpp
 * @author Jung-kang Lee (ntoskrnl7@gmail.com)
 * @brief This module implements sqlite features.
 *
 * @copyright Copyright (c) 2020 C++ Json search condition library Authors
 *
 */
#pragma once

#ifndef _JSC_SQLITE_HPP_
#define _JSC_SQLITE_HPP_

#include <algorithm>
#include <string>

#include <ext/string>
#include <nlohmann/json.hpp>

namespace jsc {
namespace sqlite {
inline std::string to_string(const nlohmann::json &condition) {
  std::string where;
  if (condition.is_object()) {
    if (condition.contains("field") && condition["field"].is_string()) {
      auto &field = condition["field"];
      if (condition.contains("operator") && condition["operator"].is_string()) {
        auto to_str = [](const nlohmann::json &value,
                         std::string &str) -> bool {
          if (value.is_number_unsigned())
            str = std::to_string(value.get<uintmax_t>());
          else if (value.is_number_integer())
            str = std::to_string(value.get<intmax_t>());
          else if (value.is_number_float())
            str = std::to_string(value.get<double>());
          else if (value.is_string())
            str = "'" + value.get<std::string>() + "'";
          else if (value.is_boolean())
            str = value ? "1" : "0";
          else
            return false;
          return true;
        };

        std::string op = condition["operator"];
        bool is_not = op.rfind("NOT ") == 0 || op.rfind("not ") == 0;
        if (is_not)
          op = op.substr(sizeof("NOT ") - 1);

        auto sql_comp_op = [&]() -> const char * {
          if (op == "eq" || op == "=")
            return "=";
          if (op == "neq" || op == "!=" || op == "<>")
            return "!=";
          if (op == "lt" || op == "<")
            return "<";
          if (op == "le" || op == "<=")
            return "<=";
          if (op == "gt" || op == ">")
            return ">";
          if (op == "ge" || op == ">=")
            return ">=";
          return nullptr;
        }();
        if (sql_comp_op) {
          if (condition.contains("value")) {
            std::string str;
            if (to_str(condition["value"], str)) {
              where += " (";
              if (is_not)
                where += "NOT ";
              where += field.get<std::string>() + " ";
              where += sql_comp_op;
              where += " " + str + ") ";
            }
          }
        } else {
          std::transform(op.begin(), op.end(), op.begin(), ::toupper);
          enum search_op_code {
            sop_between,
            sop_like,
            sop_glob,
            sop_in,
            sop_range,
            sop_contains,
            sop_unknown,
          };

          auto code = [&]() -> search_op_code {
            if (op == "BETWEEN")
              return sop_between;
            if (op == "LIKE")
              return sop_like;
            if (op == "GLOB")
              return sop_glob;
            if (op == "IN")
              return sop_in;
            if (op == "RANGE")
              return sop_range;
            if (op == "CTNS" || op == "CONTAINS")
              return sop_contains;
            return sop_unknown;
          }();

          if (condition.contains("value")) {
            auto &value = condition["value"];

            //
            // in, range, contains (ctns)
            //
            // [ x, y, z, ...]
            //
            // [NOT] IN ( x, y, z, ... )
            //
            if ((code == sop_range || code == sop_in || code == sop_contains) &&
                (value.is_array() && !value.empty())) {
              where += " (" + field.get<std::string>() + " ";
              if (is_not)
                where += "NOT ";
              where += "IN (";

              std::string values;
              for (auto &v : value) {
                std::string val;
                if (to_str(v, val))
                  values += " " + val + ", ";
              }
              if (!values.empty()) {
                values.resize(values.size() - sizeof(", ") + 1);
                where += values;
              }
              where += " )) ";
            }
            //
            // glob, in, ctns, contains
            //
            // <caseSensitive is true>
            //
            // "text"
            //
            // [NOT] GLOB '*text*'
            //
            else if ((((code == sop_in || code == sop_contains) &&
                       condition.contains("caseSensitive") &&
                       condition["caseSensitive"].is_boolean() &&
                       condition["caseSensitive"]) ||
                      code == sop_glob) &&
                     (value.is_string())) {
              std::string val = value;
              if (code != sop_glob)
                val = "*" + val + "*";

              where += " (" + field.get<std::string>() + " ";
              if (is_not)
                where += "NOT ";
              where += "GLOB '" + val + "') ";
            }

            //
            // like, in, ctns, contains
            //
            // <case insensitive>
            //
            // "text"
            //
            // [NOT] LIKE '%text%' ESCAPE '^_^'
            //
            else if ((code == sop_in || code == sop_contains ||
                      code == sop_like) &&
                     (value.is_string())) {
              std::string val = value;
              if (code == sop_like) {
                std::string escape;
                if (condition.contains("escape") &&
                    condition["escape"].is_string())
                  escape = condition["escape"];
                if (escape.length() != 1) {
                  escape = "\\";
                }
                val = "'" + val + "' ESCAPE '" + escape + "'";
              } else {
                if (ext::search(val, "_") || ext::search(val, "%")) {
                  ext::replace_all(val, "_", "\\_");
                  ext::replace_all(val, "%", "\\%");
                  val = "'%" + val + "%' ESCAPE '\\'";
                } else {
                  val = "'%" + val + "%'";
                }
              }

              where += " (" + field.get<std::string>() + " ";
              if (is_not)
                where += "NOT ";
              where += "LIKE ";
              where += val + ") ";
            }
          } else {
            //
            // between, range
            //
            // begin ~ end
            //
            // [NOT] BETWEEN begin AND end
            //
            if ((code == sop_range || code == sop_between) &&
                (condition.contains("begin") && condition.contains("end") &&
                 (condition["begin"].is_string() ||
                  condition["begin"].is_number()) &&
                 (condition["end"].is_string() ||
                  condition["end"].is_number()))) {
              std::string begin, end;
              if (to_str(condition["begin"], begin) &&
                  to_str(condition["end"], end)) {
                where += " (" + field.get<std::string>() + " ";
                if (is_not)
                  where += "NOT ";
                where += "BETWEEN " + begin + " AND " + end + ") ";
              }
            }
          }
        }
      }
      return where;
    }

    bool processed = false;
    for (auto &item : condition.items()) {
      auto query = to_string(item.value());
      if (!query.empty()) {
        where += " (";
        where += query;
        where += ") OR ";
        processed = true;
      }
    }
    if (processed)
      where.resize(where.size() - sizeof(" OR ") + 1);
  } else if (condition.is_array()) {
    bool processed = false;
    for (auto &item : condition) {
      auto query = to_string(item);
      if (!query.empty()) {
        where += " (";
        where += query;
        where += ") AND ";
        processed = true;
      }
    }
    if (processed)
      where.resize(where.size() - sizeof(" AND ") + 1);
  }
  return where;
}
} // namespace sqlite
} // namespace jsc

#endif // _JSC_SQLITE_HPP_