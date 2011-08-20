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
find_package(Doxygen REQUIRED)


# Process subdirectories
add_subdirectory(src)
add_subdirectory(unit_tests)
add_subdirectory(tests)

SET(REQUIRED_GCC_VERSION 4.6.0)

# check minimum GCC version
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
	OUTPUT_VARIABLE GCC_VERSION)
if (NOT(GCC_VERSION VERSION_GREATER ${REQUIRED_GCC_VERSION}
	OR GCC_VERSION VERSION_EQUAL ${REQUIRED_GCC_VERSION}))
	message(FATAL_ERROR "GCC Version >= ${REQUIRED_GCC_VERSION} required!")
endif()


if (DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif()
