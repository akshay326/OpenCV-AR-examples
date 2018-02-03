cmake_minimum_required(VERSION 3.9)
project(iron_helmet)

# Required packages
find_package(OpenCV REQUIRED)
find_package(dlib REQUIRED)

set(CMAKE_CXX_STANDARD 11)

add_executable(iron_helmet main.cpp render.cpp render.h)

target_link_libraries(iron_helmet ${OpenCV_LIBS} dlib::dlib)
