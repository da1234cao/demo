cmake_minimum_required (VERSION 3.8)

project("main")

set(CMAKE_BUILD_TYPE "Debug")

link_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_FILE "main.cpp"
                "server.hpp"
                "resolve.hpp"
                "thread_pool.hpp"
                "util.hpp")

add_executable(${PROJECT_NAME} ${SOURCE_FILE})

target_link_libraries(${PROJECT_NAME} pthread)