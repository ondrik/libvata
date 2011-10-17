cmake_minimum_required(VERSION 2.8.2)


project(unit_tests)

set(compiler_flags_list
  -std=c++0x
  -pedantic-errors
  -Wextra
  -Wall
  -Wfloat-equal
  -Wctor-dtor-privacy
  -Weffc++
  -Woverloaded-virtual
  -Wold-style-cast
  -fdiagnostics-show-option
  -march=native
)

set(compiler_flags "")
foreach(param ${compiler_flags_list})
  set(compiler_flags "${compiler_flags} ${param}")
endforeach(param)

# uncomment for profiling
#add_definitions(-pg)
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")

include_directories(../include)

add_library(tests
	log_fixture.cc
	formula_parser.cc
)

get_target_property(libtests_sources tests SOURCES)

foreach(src ${libtests_sources})

  set_source_files_properties(
    ${src} PROPERTIES COMPILE_FLAGS ${compiler_flags})

endforeach()

set(TESTS
	"ondriks_mtbdd_c_test"
  "timbuk_parser_test"
	"bdd_bu_tree_aut_test"
	"bdd_td_tree_aut_test"
  "explicit_tree_aut_test"
)

foreach (TEST ${TESTS})
	add_executable(${TEST} ${TEST}.cc)

  set_source_files_properties(
    ${TEST} PROPERTIES COMPILE_FLAGS ${compiler_flags})

	target_link_libraries(${TEST} tests)
	target_link_libraries(${TEST} libvata)
	target_link_libraries(${TEST} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
	target_link_libraries(${TEST} ${Boost_FILESYSTEM_LIBRARY})
	target_link_libraries(${TEST} ${Boost_SYSTEM_LIBRARY})

	add_test(${TEST} ${CMAKE_CURRENT_BINARY_DIR}/${TEST})
endforeach(TEST)
