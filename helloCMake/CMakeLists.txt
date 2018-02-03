# Necessary for all builds
cmake_minimum_required(VERSION 2.6)

# Name the project/source dir
project(Tutorial)

# The Version number
set (Tutorial_VERSION_MAJOR 1)
set (Tutorial_VERSION_MINOR 0)
set (CMAKE_BUILD_TYPE debug)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

# configure a header file to pass some cmake settings
configure_file(
	"${PROJECT_SOURCE_DIR}/TutorialConfig.h.in"
	"${PROJECT_BINARY_DIR}/TutorialConfig.h"
)

# add the binary tree to the search path for include files
include_directories("${PROJECT_BINARY_DIR}")

message("Gonna create my first hello_world cmake file")

# add the executable
add_executable(Tutorial tutorial.cxx)

install(TARGETS Tutorial DESTINATION bin)
install(FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h" DESTINATION include)
