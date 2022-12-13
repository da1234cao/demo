cmake_minimum_required(VERSION 3.11)

project(thread_test)

include(FetchContent)

# pthread-win32
FetchContent_Declare(
    pthread-win32
    GIT_REPOSITORY https://gitee.com/da1234cao/pthread-win32.git
    # GIT_REPOSITORY https://github.com/GerHobbelt/pthread-win32.git
)
FetchContent_MakeAvailable(pthread-win32)
include_directories(${pthread-win32_SOURCE_DIR})

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} pthreadVC3)