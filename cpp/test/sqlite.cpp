#include <gtest/gtest.h>
#include <jsc/sql/sqlite.hpp>

TEST(sqlite_test, empty) {
  std::string val = jsc::sqlite::to_string({});
  EXPECT_STREQ(val.c_str(), "");
}

TEST(sqlite_test, single_condition) {
  nlohmann::json cond = nlohmann::json::parse(R"({
        "field": "name",
        "operator": "=",
        "value": "test"
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(name = 'test')");
  EXPECT_STREQ(
      ext::trim(jsc::sqlite::to_string(
                    {{"field", "name"}, {"operator", "="}, {"value", "test"}}))
          .c_str(),
      "(name = 'test')");

  cond = nlohmann::json::parse(R"({
        "field": "name",
        "operator": "eq",
        "value": "test"
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(name = 'test')");
  EXPECT_STREQ(
      ext::trim(jsc::sqlite::to_string(
                    {{"field", "name"}, {"operator", "eq"}, {"value", "test"}}))
          .c_str(),
      "(name = 'test')");
}

TEST(sqlite_test, or_condition) {
  nlohmann::json cond = nlohmann::json::parse(R"(
{
    "group0": {
        "field": "name",
        "operator": "=",
        "value": "test"
    },
    "group1": {
        "field": "age",
        "operator": ">",
        "value": 10
    }
})");

  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "( (name = 'test') ) OR  ( (age > 10) )");
  EXPECT_STREQ(
      ext::trim(
          jsc::sqlite::to_string(
              {{"group0",
                {{"field", "name"}, {"operator", "="}, {"value", "test"}}},
               {"group1", {{"field", "age"}, {"operator", ">"}, {"value", 10}}}}

              ))
          .c_str(),
      "( (name = 'test') ) OR  ( (age > 10) )");
}

TEST(sqlite_test, and_condition) {
  nlohmann::json cond = nlohmann::json::parse(R"(
[
    {
        "field": "name",
        "operator": "=",
        "value": "test"
    },
    {
        "field": "age",
        "operator": ">",
        "value": 10
    }
])");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "( (name = 'test') ) AND  ( (age > 10) )");
  EXPECT_STREQ(
      ext::trim(jsc::sqlite::to_string(
                    {{{"field", "name"}, {"operator", "="}, {"value", "test"}},
                     {{"field", "age"}, {"operator", ">"}, {"value", 10}}}))
          .c_str(),
      "( (name = 'test') ) AND  ( (age > 10) )");
}

TEST(sqlite_test, like_condition) {
  // in
  nlohmann::json cond = nlohmann::json::parse(R"({
        "field": "city",
        "operator": "in",
        "value": "dea"
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(city LIKE '%dea%')");
  EXPECT_STREQ(
      ext::trim(jsc::sqlite::to_string(
                    {{"field", "city"}, {"operator", "in"}, {"value", "dea"}}))
          .c_str(),
      "(city LIKE '%dea%')");

  // contains
  cond = nlohmann::json::parse(R"({
        "field": "city",
        "operator": "contains",
        "value": "dea"
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(city LIKE '%dea%')");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string({{"field", "city"},
                                                 {"operator", "contains"},
                                                 {"value", "dea"}}))
                   .c_str(),
               "(city LIKE '%dea%')");

  // ctns
  cond = nlohmann::json::parse(R"({
        "field": "city",
        "operator": "ctns",
        "value": "dea"
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(city LIKE '%dea%')");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string({{"field", "city"},
                                                 {"operator", "ctns"},
                                                 {"value", "dea"}}))
                   .c_str(),
               "(city LIKE '%dea%')");

  // LIKE
  cond = nlohmann::json::parse(R"({
        "field": "city",
        "operator": "like",
        "value": "%dea%"
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(city LIKE '%dea%' ESCAPE '\\')");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string({{"field", "city"},
                                                 {"operator", "like"},
                                                 {"value", "%dea%"}}))
                   .c_str(),
               "(city LIKE '%dea%' ESCAPE '\\')");

  // LIKE ESCAPE '^'
  cond = nlohmann::json::parse(R"({
        "field": "city",
        "operator": "like",
        "value": "%^_dea%",
        "escape": "^"
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(city LIKE '%^_dea%' ESCAPE '^')");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string({{"field", "city"},
                                                 {"escape", "^"},
                                                 {"operator", "like"},
                                                 {"value", "%^_dea%"}}))
                   .c_str(),
               "(city LIKE '%^_dea%' ESCAPE '^')");

  // in, contains, ctns WITH ESCAPE '\'
  cond = nlohmann::json::parse(R"({
        "field": "city",
        "operator": "in",
        "value": "_dea"
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(city LIKE '%\\_dea%' ESCAPE '\\')");
  EXPECT_STREQ(
      ext::trim(jsc::sqlite::to_string(
                    {{"field", "city"}, {"operator", "in"}, {"value", "_dea"}}))
          .c_str(),
      "(city LIKE '%\\_dea%' ESCAPE '\\')");
}

TEST(sqlite_test, glob_condition) {
  // in
  nlohmann::json cond = nlohmann::json::parse(R"({
        "field": "city",
        "caseSensitive": true,
        "operator": "in",
        "value": "dea"
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(city GLOB '*dea*')");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string({{"field", "city"},
                                                 {"caseSensitive", true},
                                                 {"operator", "in"},
                                                 {"value", "dea"}}))
                   .c_str(),
               "(city GLOB '*dea*')");

  // contains
  cond = nlohmann::json::parse(R"({
        "field": "city",
        "caseSensitive": true,
        "operator": "contains",
        "value": "dea"
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(city GLOB '*dea*')");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string({{"field", "city"},
                                                 {"caseSensitive", true},
                                                 {"operator", "contains"},
                                                 {"value", "dea"}}))
                   .c_str(),
               "(city GLOB '*dea*')");

  // ctns
  cond = nlohmann::json::parse(R"({
        "field": "city",
        "caseSensitive": true,
        "operator": "ctns",
        "value": "dea"
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(city GLOB '*dea*')");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string({{"field", "city"},
                                                 {"caseSensitive", true},
                                                 {"operator", "ctns"},
                                                 {"value", "dea"}}))
                   .c_str(),
               "(city GLOB '*dea*')");

  // GLOB
  cond = nlohmann::json::parse(R"({
        "field": "city",
        "operator": "glob",
        "value": "*dea*"
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(city GLOB '*dea*')");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string({{"field", "city"},
                                                 {"operator", "glob"},
                                                 {"value", "*dea*"}}))
                   .c_str(),
               "(city GLOB '*dea*')");
}

TEST(sqlite_test, between_condition) {
  // range
  nlohmann::json cond = nlohmann::json::parse(R"({
        "field": "age",
        "operator": "range",
        "begin": 20,
        "end": 30
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(age BETWEEN 20 AND 30)");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string({{"field", "age"},
                                                 {"operator", "range"},
                                                 {"begin", 20},
                                                 {"end", 30}}))
                   .c_str(),
               "(age BETWEEN 20 AND 30)");

  // BETWEEN
  cond = nlohmann::json::parse(R"({
        "field": "age",
        "operator": "BETWEEN",
        "begin": 20,
        "end": 30
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(age BETWEEN 20 AND 30)");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string({{"field", "age"},
                                                 {"operator", "range"},
                                                 {"begin", 20},
                                                 {"end", 30}}))
                   .c_str(),
               "(age BETWEEN 20 AND 30)");
}

TEST(sqlite_test, in_condition) {
  // IN
  nlohmann::json cond = nlohmann::json::parse(R"({
        "field": "city",
        "operator": "in",
        "value": [
            "seoul",
            "daejeon",
            "daegu",
            "busan"
        ]
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(city IN ( 'seoul',  'daejeon',  'daegu',  'busan' ))");
  EXPECT_STREQ(
      ext::trim(jsc::sqlite::to_string(
                    {{"field", "city"},
                     {"operator", "in"},
                     {"value", {"seoul", "daejeon", "daegu", "busan"}}}))
          .c_str(),
      "(city IN ( 'seoul',  'daejeon',  'daegu',  'busan' ))");

  // range
  cond = nlohmann::json::parse(R"({
        "field": "city",
        "operator": "range",
        "value": [
            "seoul",
            "daejeon",
            "daegu",
            "busan"
        ]
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(city IN ( 'seoul',  'daejeon',  'daegu',  'busan' ))");
  EXPECT_STREQ(
      ext::trim(jsc::sqlite::to_string(
                    {{"field", "city"},
                     {"operator", "range"},
                     {"value", {"seoul", "daejeon", "daegu", "busan"}}}))
          .c_str(),
      "(city IN ( 'seoul',  'daejeon',  'daegu',  'busan' ))");

  // contains
  cond = nlohmann::json::parse(R"({
        "field": "city",
        "operator": "contains",
        "value": [
            "seoul",
            "daejeon",
            "daegu",
            "busan"
        ]
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(city IN ( 'seoul',  'daejeon',  'daegu',  'busan' ))");
  EXPECT_STREQ(
      ext::trim(jsc::sqlite::to_string(
                    {{"field", "city"},
                     {"operator", "contains"},
                     {"value", {"seoul", "daejeon", "daegu", "busan"}}}))
          .c_str(),
      "(city IN ( 'seoul',  'daejeon',  'daegu',  'busan' ))");

  // ctns
  cond = nlohmann::json::parse(R"({
        "field": "city",
        "operator": "ctns",
        "value": [
            "seoul",
            "daejeon",
            "daegu",
            "busan"
        ]
    })");
  EXPECT_STREQ(ext::trim(jsc::sqlite::to_string(cond)).c_str(),
               "(city IN ( 'seoul',  'daejeon',  'daegu',  'busan' ))");
  EXPECT_STREQ(
      ext::trim(jsc::sqlite::to_string(
                    {{"field", "city"},
                     {"operator", "ctns"},
                     {"value", {"seoul", "daejeon", "daegu", "busan"}}}))
          .c_str(),
      "(city IN ( 'seoul',  'daejeon',  'daegu',  'busan' ))");
}
