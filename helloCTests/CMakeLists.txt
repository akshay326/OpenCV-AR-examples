# Necessary for all builds
cmake_minimum_required(VERSION 2.6)

# Name the project/source dir
project(CTest)

# The Version number
set (CTest_VERSION_MAJOR 1)
set (CTest_VERSION_MINOR 0)
set (CMAKE_BUILD_TYPE debug)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

# To run
# mkdir build; cd build; cmake ..
# Else it'll shit everything in the same folder

message("Gonna create my first test file")

# add the executable
add_executable(CTest ctest.cxx)


##
# Though u can create a special subdirectory for testing
# Yet using CTests is simple and easy, only 2 line per test
# Hit `ctest` command line tool to run
##
include(CTest)

# T1: Does it work
add_test(NAME CTestFind25 COMMAND CTest 25)
set_tests_properties(CTestFind25 PROPERTIES PASS_REGULAR_EXPRESSION "25 is 5")

# T2: It should fail
add_test(NAME CTestFindNegative COMMAND CTest 4)
set_tests_properties(CTestFindNegative PROPERTIES FAIL_REGULAR_EXPRESSION "4 is -2")

# T3: Inadequate number of args
add_test(NAME CTestLessArgs COMMAND CTest)
set_tests_properties(CTestLessArgs PROPERTIES PASS_REGULAR_EXPRESSION "Usage: .* number")
