cmake_minimum_required(VERSION 3.16)

project(hApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_CLIENT "Build the Qt6 client (requires Qt 6.8+)" ON)
option(BUILD_TESTS  "Build the test suite" ON)

add_subdirectory(common)
add_subdirectory(server)

if(BUILD_CLIENT)
    add_subdirectory(client)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
