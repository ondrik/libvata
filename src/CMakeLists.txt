cmake_minimum_required(VERSION 2.8.2)

set(CMAKE_COLOR_MAKEFILE ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)

project(vata)

set(flex_bison_compiler_flags_list
  -std=c++0x
  -pedantic-errors
  -Wextra
  -Wall
  -Wfloat-equal
  -Wctor-dtor-privacy
  -Weffc++
  -Woverloaded-virtual
  -fdiagnostics-show-option
  -march=native
)

set(vata_compiler_add_flags_list
  -Wold-style-cast
)

set(flex_bison_compiler_flags "")
foreach(param ${flex_bison_compiler_flags_list})
  set(flex_bison_compiler_flags "${flex_bison_compiler_flags} ${param}")
endforeach(param)

set(vata_compiler_flags ${flex_bison_compiler_flags})
foreach(param ${vata_compiler_add_flags_list})
  set(vata_compiler_flags "${vata_compiler_flags} ${param}")
endforeach(param)

# uncomment for profiling
#add_definitions(-pg)
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")

include_directories(../include)

flex_target(timbuk_scanner timbuk.l ${CMAKE_CURRENT_BINARY_DIR}/timbuk.l.cc)
bison_target(timbuk_parser timbuk.y ${CMAKE_CURRENT_BINARY_DIR}/timbuk.y.cc)
add_flex_bison_dependency(timbuk_scanner timbuk_parser)

add_library(libvata STATIC
	aut_base.cc
	bdd_bu_tree_aut.cc
  bdd_bu_tree_aut_sim.cc
  bdd_bu_tree_aut_incl.cc
  bdd_bu_tree_aut_isect.cc
  bdd_bu_tree_aut_union.cc
  bdd_bu_tree_aut_union_disj.cc
  bdd_bu_tree_aut_unreach.cc
  bdd_bu_tree_aut_useless.cc
  bdd_td_tree_aut_sim.cc
  bdd_td_tree_aut_incl.cc
  bdd_td_tree_aut_isect.cc
  bdd_td_tree_aut_union.cc
  bdd_td_tree_aut_union_disj.cc
  bdd_td_tree_aut_unreach.cc
  bdd_td_tree_aut_useless.cc
  explicit_lts_sim.cc
  explicit_tree_aut.cc
  convert.cc
  fake_file.cc
	symbolic_aut_base.cc
  timbuk_parser.cc
  timbuk_serializer.cc
  util.cc
  var_asgn.cc
  ${FLEX_timbuk_scanner_OUTPUTS}
  ${BISON_timbuk_parser_OUTPUTS}
)
set_target_properties(libvata PROPERTIES
  OUTPUT_NAME vata
  CLEAN_DIRECT_OUTPUT 1
)


set(scanner_parser_files
  ${FLEX_timbuk_scanner_OUTPUTS}
  ${BISON_timbuk_parser_OUTPUTS}
)

get_target_property(libvata_sources libvata SOURCES)

foreach(src ${libvata_sources})

  unset(is_scanner_parser_file)
  foreach (checked_file ${scanner_parser_files})
    if (${src} STREQUAL ${checked_file})
      set(is_scanner_parser_file 1)
    endif()
  endforeach()

  if (is_scanner_parser_file)
    set_source_files_properties(
      ${src} PROPERTIES COMPILE_FLAGS ${flex_bison_compiler_flags})
  else()
    set_source_files_properties(
      ${src} PROPERTIES COMPILE_FLAGS ${vata_compiler_flags})
  endif()
endforeach()

add_executable(vata
  vata.cc
  parse_args.cc
)

get_target_property(vata_sources vata SOURCES)

foreach(src ${vata_sources})

  set_source_files_properties(
    ${src} PROPERTIES COMPILE_FLAGS ${vata_compiler_flags})

endforeach()

target_link_libraries(vata libvata)
target_link_libraries(vata ${LOG4CPP_LIBRARIES})
target_link_libraries(vata rt)
