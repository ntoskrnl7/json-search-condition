cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cpp/cmake/")
include(pkg-utils)
include(CPM)

pkg_get_version(JSC ${CMAKE_CURRENT_SOURCE_DIR}/cpp/include/jsc/_version VERSION)

project(jsc VERSION "${VERSION}" LANGUAGES C CXX)

option(JSC_BUILD_TESTS "Set to ON to build tests" OFF)

add_library(jsc INTERFACE)

target_include_directories(jsc INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cpp/include>
)

CPMAddPackage("gh:nlohmann/json@3.10.5")
CPMAddPackage("gh:ntoskrnl7/ext@0.5.6")
target_link_libraries(jsc INTERFACE ext nlohmann_json)

if (JSC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cpp/test)
endif()
