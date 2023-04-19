cmake_minimum_required(VERSION 3.14)

project(ParseArgs)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable( ParseArgs
        src/main.cc
        )

target_include_directories( ParseArgs PUBLIC
        include
        )