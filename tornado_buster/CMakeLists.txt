cmake_minimum_required(VERSION 4.3)

project(tornado_buster)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)


add_executable(tornado_buster main.cpp sensors.cpp
        sensors.h)

target_include_directories(tornado_buster PRIVATE
        ext)

target_link_libraries(tornado_buster PRIVATE raylib)