cmake_minimum_required(VERSION 3.9)
project(face_detect)

# Required packages
find_package(OpenCV REQUIRED)
find_package(dlib REQUIRED)

set(CMAKE_CXX_STANDARD 11)

add_executable(face_detect main.cpp)

target_link_libraries(face_detect ${OpenCV_LIBS} dlib::dlib)