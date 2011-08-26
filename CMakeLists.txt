cmake_minimum_required(VERSION 2.8.2)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_COLOR_MAKEFILE ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)

set(REQUIRED_GCC_VERSION 4.6.0)

set(GCC_ALTERNATIVES g++ g++-4.6)

# check minimum GCC version
execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
	OUTPUT_VARIABLE GCC_VERSION)
if (NOT(GCC_VERSION VERSION_GREATER ${REQUIRED_GCC_VERSION}
	OR GCC_VERSION VERSION_EQUAL ${REQUIRED_GCC_VERSION}))

  # in case standard (let's hope) GCC is too old
  foreach (GCC_CANDIDATE ${GCC_ALTERNATIVES})
    if (NOT FOUND_COMPILER)
      execute_process(COMMAND ${GCC_CANDIDATE} -dumpversion RESULT_VARIABLE RES
        OUTPUT_VARIABLE GCC_VERSION)
      if (NOT RES)
        # in case the process ended correctly (i.e., returns 0)
        if (GCC_VERSION VERSION_GREATER ${REQUIRED_GCC_VERSION}
          OR GCC_VERSION VERSION_EQUAL ${REQUIRED_GCC_VERSION})
          # in case the GCC version is usable
          set(CMAKE_CXX_COMPILER ${GCC_CANDIDATE})
          message("-- Using ${CMAKE_CXX_COMPILER} as C++ compiler")
          set(FOUND_COMPILER 1)
        endif()
      endif()
    endif()
  endforeach(GCC_CANDIDATE)

  if (NOT FOUND_COMPILER)
    message(FATAL_ERROR "GCC Version >= ${REQUIRED_GCC_VERSION} required!")
  endif()
endif()


project(libvata)

# Include CTest so that sophisticated testing can be done now
include(CTest)

# Necessary packages
find_package(Log4CPP REQUIRED)
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
find_package(Boost 1.42.0 COMPONENTS unit_test_framework REQUIRED)
find_package(Doxygen REQUIRED)

if (DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif()



# Process subdirectories
add_subdirectory(src)
add_subdirectory(unit_tests)
add_subdirectory(tests)
