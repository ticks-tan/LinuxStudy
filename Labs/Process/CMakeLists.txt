cmake_minimum_required(VERSION 3.14)

project(Process LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(
        Process
        src/main.cc
        src/process.cc
        )

target_include_directories(Process PUBLIC
        include
        )