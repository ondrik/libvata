cmake_minimum_required(VERSION 2.8.2)


project(unit_tests)

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
find_package(Boost 1.42.0 COMPONENTS unit_test_framework REQUIRED)

include_directories(../include)

add_library(tests
  log_fixture.cc
  formula_parser.cc
)

set(TESTS
  "ondriks_mtbdd_c_test"
)
foreach (TEST ${TESTS})
  add_executable(${TEST} ${TEST}.cc)

  target_link_libraries(${TEST} vatalib)
  target_link_libraries(${TEST} tests)
  target_link_libraries(${TEST} ${LOG4CPP_LIBRARIES})
  target_link_libraries(${TEST} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
	target_link_libraries(${TEST} ${LOKI_LIBRARY})

  add_test(${TEST} ${CMAKE_CURRENT_BINARY_DIR}/${TEST})
endforeach(TEST)
