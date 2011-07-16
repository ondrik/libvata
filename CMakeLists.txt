cmake_minimum_required(VERSION 2.8.2)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_COLOR_MAKEFILE ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)

project(vatalib)

# Include CTest so that sophisticated testing can be done now
include(CTest)

add_subdirectory(src)
add_subdirectory(unit_tests)
add_subdirectory(tests)

