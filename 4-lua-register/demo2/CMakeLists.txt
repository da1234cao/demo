cmake_minimum_required(VERSION 3.11)

project(complex C)

# lua path
set(LUA_HEADER_PAHT C:/lua64/include)
set(LUA_LIB_PAHT C:/lua64)
set(LUA_LIB_NAME "lua53")
include_directories(${LUA_HEADER_PAHT})
link_directories(${LUA_LIB_PAHT})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/complex.c)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${LUA_LIB_NAME})