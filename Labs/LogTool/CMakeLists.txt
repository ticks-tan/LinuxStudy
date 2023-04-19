cmake_minimum_required(VERSION 3.14)

project(log_tool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 11)

add_executable(LogTool
        src/log_file.cc
        src/log_tool.cc
        src/main.cc
        )

target_include_directories(LogTool PUBLIC
        include
        )