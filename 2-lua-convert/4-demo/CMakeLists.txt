cmake_minimum_required(VERSION 3.11)

project(convert)

ADD_DEFINITIONS(-DLUA_BUILD_AS_DLL)

set(LUA_HEADER_PAHT ${CMAKE_CURRENT_SOURCE_DIR}/../lua64/src)
set(LUA_LIB_PAHT ${CMAKE_CURRENT_SOURCE_DIR}/../lua64)
set(LUA_LIB_NAME "lua53")
include_directories(${LUA_HEADER_PAHT})
link_directories(${LUA_LIB_PAHT})

# boost
set(Boost_USE_STATIC_LIBS
    ON
    CACHE BOOL "Set using static boost library")
find_package(
  Boost REQUIRED
  COMPONENTS iostreams
             locale)
if(WIN32)
  link_directories(${PROJECT_SOURCE_DIR} $ENV{BOOST_ROOT}/lib)
endif(WIN32)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/convert.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${LUA_LIB_NAME})