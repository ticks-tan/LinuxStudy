cmake_minimum_required(VERSION 3.10)

project(FileTool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(FileTool
        src/file_tool.cc
        src/main.cc
        )

target_include_directories(FileTool PRIVATE
        include
        )