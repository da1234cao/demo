cmake_minimum_required(VERSION 3.10.0)
 
project(test)

link_directories(${PROJECT_SOURCE_DIR}/bin)

add_executable(${PROJECT_NAME} test.cpp)
 
target_link_libraries(${PROJECT_NAME} hello)  