# json-search-condition

## Contents

- [json-search-condition](#json-search-condition)
  - [Contents](#contents)
  - [Dependencies](#dependencies)
  - [Usage](#usage)
    - [CMake](#cmake)
      - [CMakeLists.txt](#cmakeliststxt)
    - [Examples](#examples)
      - [Scheme](#scheme)
      - [SQLite](#sqlite)

## Dependencies

- [ext](https://github.com/ntoskrnl7/ext)
- [nlohmann::json](https://github.com/nlohmann/json)

## Usage

### CMake

#### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# create project
project(MyProject)

# add executable
add_executable(tests tests.cpp)

# add dependencies
include(cmake/CPM.cmake)
CPMAddPackage("gh:ntoskrnl7/json-search-condition@0.1.0")

# link dependencies
target_link_libraries(tests jsc)
```

### Examples

#### Scheme

- The search condition can be expressed as follows.

    ```JSON
    {
        "sample0": // name = "test"
        {
            "field": "name",
            "operator": "=", // eq
            "value": "test"
        },
        "sample1": // name = "test" && age > 10
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
        ],
        "sample2": // name = "test" || age > 10
        {
            "group0": {
                "field": "name",
                "operator": "=", // eq
                "value": "test"
            },
            "group1": {
                "field": "age",
                "operator": ">", // gt
                "value": 10
            }
        },
        "sample4": // name in "dea"
        {
            "field": "city",
            "operator": "in", // contains, ctns
            "value": "dea"
        },
        "sample5": // age 20 ~ 30
        {
            "field": "age",
            "operator": "range", // BEETWEN
            "begin": 20,
            "end": 30
        },
        "sample3": // name in ("seoul", "daejeon", "daegu", "busan")
        {
            "field": "city",
            "operator": "range", // IN, contains, ctns
            "value": [
                "seoul",
                "daejeon",
                "daegu",
                "busan"
            ]
        }
    }

    ```

#### SQLite

```C++
#include <jsc/sql/sqlite.hpp>

...

sqlite3_exec(
    db, "SELECT * FROM user WHERE " +
            jsc::sqlite::to_string(
                {{"field", "name"}, {"operator", "="}, {"value", "test"}})
                .c_str());
```
