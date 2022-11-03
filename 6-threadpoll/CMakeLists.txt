cmake_minimum_required (VERSION 3.8)

project("main")

set(CMAKE_BUILD_TYPE "Debug")

link_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_FILE "main.cpp" "thread_pool.hpp")

add_executable(${PROJECT_NAME} ${SOURCE_FILE})

target_link_libraries(${PROJECT_NAME} pthread)