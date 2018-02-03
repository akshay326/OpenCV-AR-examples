cmake_minimum_required(VERSION 3.8)
project(aug-skull)

set(CMAKE_CXX_STANDARD 11)

# Required packages
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(aug-skull main.cpp)

# Adding local ARUco Library
include_directories(/home/akshay/Projects/vision/aruco_src/include/)
link_directories(/home/akshay/Projects/vision/aruco_src/lib/)

include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

target_link_libraries(aug-skull ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} aruco)