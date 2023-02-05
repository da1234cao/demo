cmake_minimum_required(VERSION 3.10.0) #cmake的 最低版本
 
project(hello)

ADD_DEFINITIONS(-D_EXPORT_DLL_)

set(SOURCE_FILE hello.cpp)

# add_library(${PROJECT_NAME} STATIC ${SOURCE_FILE})
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILE})
