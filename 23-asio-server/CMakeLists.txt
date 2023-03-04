cmake_minimum_required(VERSION 3.11)

project(server)

find_package(Boost REQUIRED thread)
if(NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found")
endif()
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(${Boost_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} main.cpp server.cpp connection.cpp)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})