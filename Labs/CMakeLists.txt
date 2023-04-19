cmake_minimum_required(VERSION 3.14)

project(LinuxStudy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_subdirectory(FileTool)
add_subdirectory(ParseArg)
add_subdirectory(Process)
add_subdirectory(LogTool)
