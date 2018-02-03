cmake_minimum_required(VERSION 3.8)
project(aug-skull)

set(CMAKE_CXX_STANDARD 11)

# Required packages
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)

if (GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
    link_libraries(${GLEW_LIBRARIES})
endif()

add_executable(aug-skull
        main.cpp
        render.cpp
        render.h)

# Adding local ARUco Library
include_directories(/home/akshay/Projects/vision/aruco_src/include/)
link_directories(/home/akshay/Projects/vision/aruco_src/lib/)

include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

target_link_libraries(aug-skull ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} aruco ${GLEW_LIBRARIES})