cmake_minimum_required (VERSION 3.8)

project("main")

link_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT_NAME} "main.c")

target_link_libraries(${PROJECT_NAME} PRIVATE llhttp)