cmake_minimum_required(VERSION 3.8)
project(opengl_examples)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp models.cpp)
add_executable(opengl_examples ${SOURCE_FILES})

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

target_link_libraries(opengl_examples ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})