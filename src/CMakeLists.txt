cmake_minimum_required(VERSION 2.8.2)

set(CMAKE_COLOR_MAKEFILE ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)

project(vata)

add_definitions(-std=c++98)
add_definitions(-pedantic-errors)
add_definitions(-Wextra)
add_definitions(-Wall)
add_definitions(-Wfloat-equal)
add_definitions(-Wctor-dtor-privacy)
add_definitions(-Weffc++)
add_definitions(-Wold-style-cast)
add_definitions(-Woverloaded-virtual)
add_definitions(-finline-functions)
add_definitions(-fdiagnostics-show-option)
add_definitions(-O2)
add_definitions(-march=core2)
add_definitions(-mtune=core2)
add_definitions(-msse2)
add_definitions(-g)

# uncomment for profiling
#add_definitions(-pg)
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")

find_package(Log4CPP REQUIRED)
find_package(Loki REQUIRED)

include_directories(../include)

add_library(vatalib STATIC
  convert.cc
)
set_target_properties(vatalib PROPERTIES
   OUTPUT_NAME vata
   CLEAN_DIRECT_OUTPUT 1
)

#add_executable(vata vata.cc)

#target_link_libraries(vata vatalib)
#target_link_libraries(vata ${LOG4CPP_LIBRARIES})
#target_link_libraries(vata ${LOKI_LIBRARY})
#target_link_libraries(vata rt)
