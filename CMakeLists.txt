cmake_minimum_required(VERSION 3.16)

project(hotspot VERSION 0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(hproto)
add_subdirectory(chat)
add_subdirectory(relay)
