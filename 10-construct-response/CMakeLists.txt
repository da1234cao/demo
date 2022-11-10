cmake_minimum_required (VERSION 3.8)

project("main")

set(CMAKE_BUILD_TYPE "Debug")

find_package( 
 Boost 1.78 REQUIRED 
 COMPONENTS json 
)

# compile josn-test.cpp
add_executable("json-test" "json-test.cpp")
target_link_libraries("json-test" Boost::json)

# compile main.cpp
add_executable(${PROJECT_NAME} "main.cpp")
target_link_libraries(${PROJECT_NAME} Boost::json)