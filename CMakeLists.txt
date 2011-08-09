cmake_minimum_required(VERSION 2.8.2)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_COLOR_MAKEFILE ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)

project(libvata)

# Include CTest so that sophisticated testing can be done now
include(CTest)

# Necessary packages
find_package(Log4CPP REQUIRED)
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
find_package(Boost 1.42.0 COMPONENTS unit_test_framework REQUIRED)


# Process subdirectories
add_subdirectory(src)
add_subdirectory(unit_tests)
add_subdirectory(tests)

