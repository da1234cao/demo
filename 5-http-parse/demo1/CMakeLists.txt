cmake_minimum_required (VERSION 3.8)

project("main")

set(CMAKE_BUILD_TYPE "Debug")

link_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_FILE "main.cpp" "http_parser.c" "request.hpp")

add_executable(${PROJECT_NAME} ${SOURCE_FILE})