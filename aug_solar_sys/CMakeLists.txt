cmake_minimum_required(VERSION 2.8)
project(drawSolar)

SET(CMAKE_MODULE_PATH ${CMAKE_INSTALL_PREFIX}/lib/cmake/ )
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Required packages
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

# Adding local ARUco Library
include_directories(/home/akshay/Projects/vision/aruco_src/include/)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )
link_directories(/home/akshay/Projects/vision/aruco_src/lib/)

add_executable(drawSolar sample.cpp)
target_link_libraries(drawSolar ${OpenCV_LIBS} aruco ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
