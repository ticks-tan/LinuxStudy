cmake_minimum_required(VERSION 3.14)

project(process LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(
        process_test
        src/main.cc
        src/process.cc
        )

target_include_directories(process_test PUBLIC
        include
        )