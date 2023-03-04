cmake_minimum_required(VERSION 3.11)

project(server)

# linux下Boost.Asio依赖于Boost.Thread库，这是一个必需的依赖项
# 而在Windows上，Boost.Asio可能能够使用内置的Windows线程API而不是Boost.Thread
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  find_package(Boost REQUIRED thread)
else()
  find_package(Boost REQUIRED)
endif()
if(NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found")
endif()
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(${Boost_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} main.cpp server.cpp connection.cpp)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})