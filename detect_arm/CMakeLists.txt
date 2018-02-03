cmake_minimum_required(VERSION 2.8)
project(findArm)

SET(CMAKE_MODULE_PATH ${CMAKE_INSTALL_PREFIX}/lib/cmake/ )
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Required packages
find_package(OpenCV REQUIRED)

# Adding local ARUco Library
include_directories(/home/akshay/Projects/vision/aruco_src/include/)
link_directories(/home/akshay/Projects/vision/aruco_src/lib/)

add_executable(findArm main.cpp)
target_link_libraries(findArm ${OpenCV_LIBS} aruco)
