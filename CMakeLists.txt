cmake_minimum_required(VERSION 2.8.2)

# 3rd party modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")
# Enable fancy colours
set(CMAKE_COLOR_MAKEFILE ON)
# Enable verbose output for 'make'
# set(CMAKE_VERBOSE_MAKEFILE ON)


##############################################################################
#                             OUTPUT CUSTOMIZATION
##############################################################################

# Uncomment to avoid dynamic linking of libraries
#set(MAKE_STATIC_VATA ON)

# The default C++ compiler
set(DEFAULT_CXX_COMPILER g++) 

# Override the default C++ compiler
set(OVERRIDE_CXX_COMPILER clang)

##############################################################################
#                               COMPILER FUN
##############################################################################

# enable C++
enable_language(CXX)

set(REQUIRED_GCC_VERSION 4.8.0)
set(GCC_ALTERNATIVES g++ g++-4.8)

# get the git revision
include(GetGitRevisionDescription)
git_describe(GIT_DESCRIBE)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

if (NOT DEFINED CMAKE_CXX_COMPILER)
	set(CMAKE_CXX_COMPILER ${DEFAULT_CXX_COMPILER})
endif()

message("-- Default C++ compiler: ${CMAKE_CXX_COMPILER}")

if (DEFINED OVERRIDE_CXX_COMPILER)
	set(CMAKE_CXX_COMPILER ${OVERRIDE_CXX_COMPILER})
	message("-- Overriding the default C++ compiler with: ${OVERRIDE_CXX_COMPILER}")
endif()

# default compiler family
set(COMPILER_FAMILY "gcc")

if ("${CMAKE_CXX_COMPILER}" MATCHES "clang")
	message("-- Assuming the Clang compiler")
	set(COMPILER_FAMILY "clang")
endif()

message("-- Testing suitability of ${CMAKE_CXX_COMPILER}")

if ("${COMPILER_FAMILY}" STREQUAL "gcc")
	# check minimum GCC version
	execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
		OUTPUT_VARIABLE GCC_VERSION)

	# check whether the version is non-empty
	if ("${GCC_VERSION}" STREQUAL "")
	message(FATAL_ERROR "-- Cannot get the GCC version")
	endif()

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
						message("-- Using ${CMAKE_CXX_COMPILER} as the C++ compiler")
						set(FOUND_COMPILER 1)
					endif()
				endif()
			endif()
		endforeach(GCC_CANDIDATE)

		if (NOT FOUND_COMPILER)
			message(FATAL_ERROR "GCC and G++ Version >= ${REQUIRED_GCC_VERSION} required!")
		endif()
	endif()
elseif ("${COMPILER_FAMILY}" STREQUAL "clang")
	message(FATAL_ERROR "Clang currently unsupported!")
else()
	message(FATAL_ERROR "Unknown compiler family \"${COMPILER_FAMILY}\"")
endif()

# You can force the GCC version here
#set(CMAKE_C_COMPILER "gcc-4.8")
#set(CMAKE_CXX_COMPILER "g++-4.8")

# Uncomment for the 32-bit memory model
# set(CMAKE_C_FLAGS_RELEASE  "${CMAKE_C_FLAGS_RELEASE} -m32")
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -m32")

# Uncomment for profiling with Callgrind
set(CMAKE_C_FLAGS_RELEASE  "${CMAKE_C_FLAGS_RELEASE} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")

##############################################################################
#                                 PROJECT
##############################################################################

project(libvata)

# Include CTest so that sophisticated testing can be done now
include(CTest)


##############################################################################
#                                DEPENDENCIES
##############################################################################

# Necessary packages
find_package(FLEX REQUIRED)
find_package(BISON 3.0.0 REQUIRED)


# Boost
set(Boost_USE_MULTITHREADED OFF)
find_package(Boost 1.54.0 COMPONENTS
  filesystem
  system
  unit_test_framework
  REQUIRED)


# Doxygen - voluntary
find_package(Doxygen)
if (DOXYGEN_FOUND)
 configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
   ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
 add_custom_target(doc
   ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif()

##############################################################################
#                                SUB-MODULES
##############################################################################

# Process subdirectories
add_subdirectory(src)
add_subdirectory(cli)
add_subdirectory(unit_tests)
add_subdirectory(tests)
