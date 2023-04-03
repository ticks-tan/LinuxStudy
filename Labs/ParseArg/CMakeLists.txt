cmake_minimum_required(VERSION 3.10)

project(parse_args)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(
        args_parse_test
        src/main.cc
)

target_include_directories(
        args_parse_test PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)