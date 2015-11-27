cmake_minimum_required(VERSION 2.8.2)

set(CMAKE_COLOR_MAKEFILE ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)

project(vata)

# Flags for basic C files
set(basic_c_compiler_flags_list
  -pedantic-errors
  -Wextra
  -Wall
  -Wfloat-equal
  -fdiagnostics-show-option
  -march=native
)

# Additional flags for basic C++ files
set(basic_cxx_compiler_add_flags_list
  -std=c++11
  -Wctor-dtor-privacy
  -Weffc++
  -fPIC
  -Woverloaded-virtual
)

# Additional flags for VATA C++ files
set(vata_cxx_compiler_add_flags_list
  -Wold-style-cast
)

# Setting GCC flags for C++ compilation of Bison & Flex outputs
set(flex_bison_compiler_flags "")
foreach(param ${basic_c_compiler_flags_list})
  set(flex_bison_compiler_flags "${flex_bison_compiler_flags} ${param}")
endforeach(param)
foreach(param ${basic_cxx_compiler_add_flags_list})
  set(flex_bison_compiler_flags "${flex_bison_compiler_flags} ${param}")
endforeach(param)

# Setting GCC flags for compilation of VATA C files
set(vata_c_compiler_flags "")
foreach(param ${basic_c_compiler_flags_list})
  set(vata_c_compiler_flags "${vata_c_compiler_flags} ${param}")
endforeach(param)

# Setting GCC flags for compilation of VATA C++ files
set(vata_cxx_compiler_flags ${vata_c_compiler_flags})
foreach(param ${basic_cxx_compiler_add_flags_list})
  set(vata_cxx_compiler_flags "${vata_cxx_compiler_flags} ${param}")
endforeach(param)
foreach(param ${vata_cxx_compiler_add_flags_list})
  set(vata_cxx_compiler_flags "${vata_cxx_compiler_flags} ${param}")
endforeach(param)

# uncomment for profiling
#add_definitions(-pg)
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")

include_directories(../include)

flex_target(timbuk_scanner timbuk.l ${CMAKE_CURRENT_BINARY_DIR}/timbuk.l.cc COMPILE_FLAGS "--header-file=${CMAKE_CURRENT_BINARY_DIR}/timbuk.l.hh")
bison_target(timbuk_parser timbuk.y ${CMAKE_CURRENT_BINARY_DIR}/timbuk.y.cc)
add_flex_bison_dependency(timbuk_scanner timbuk_parser)

add_library(libvata STATIC
	aut_base.cc
	bdd_bu_tree_aut.cc
	bdd_bu_tree_aut_core.cc
	bdd_bu_tree_aut_sim.cc
	bdd_bu_tree_aut_incl.cc
	bdd_bu_tree_aut_isect.cc
	bdd_bu_tree_aut_union.cc
	bdd_bu_tree_aut_union_disj.cc
	bdd_bu_tree_aut_unreach.cc
	bdd_bu_tree_aut_useless.cc
	bdd_td_tree_aut.cc
	bdd_td_tree_aut_core.cc
	bdd_td_tree_aut_sim.cc
	bdd_td_tree_aut_incl.cc
	bdd_td_tree_aut_isect.cc
	bdd_td_tree_aut_union.cc
	bdd_td_tree_aut_union_disj.cc
	bdd_td_tree_aut_unreach.cc
	bdd_td_tree_aut_useless.cc
	explicit_finite_aut.cc
	explicit_finite_aut_core.cc
	explicit_finite_union.cc
	explicit_finite_isect.cc
	explicit_finite_incl.cc
	explicit_finite_reverse.cc
	explicit_finite_useless.cc
	explicit_finite_unreach.cc
	explicit_finite_candidate.cc
	explicit_finite_compl.cc
	explicit_finite_sim.cc
	explicit_tree_aut_core.cc
	explicit_tree_comp_down.cc
	explicit_tree_incl_down.cc
	explicit_tree_incl_up.cc
	explicit_lts_sim.cc
	explicit_tree_aut.cc
	explicit_tree_candidate.cc
	explicit_tree_union.cc
	explicit_tree_isect.cc
	explicit_tree_isect_bu.cc
	explicit_tree_incl.cc
	explicit_tree_unreach.cc
	explicit_tree_useless.cc
	explicit_tree_sim.cc
	convert.cc
	incl_param.cc
	fake_file.cc
	fmemopen.c
	symbolic.cc
	memstream.c
	timbuk_parser.cc
	timbuk_serializer.cc
	util.cc
	sym_var_asgn.cc
	symbolic_tree_aut_base_core.cc
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

set(c_files
	fmemopen.c
	memstream.c
)

get_target_property(libvata_sources libvata SOURCES)

foreach(src ${libvata_sources})

  unset(is_scanner_parser_file)
  foreach (checked_file ${scanner_parser_files})
    if (${src} STREQUAL ${checked_file})
      set(is_scanner_parser_file 1)
    endif()
  endforeach()

	unset(is_c_file)
  foreach (checked_file ${c_files})
    if (${src} STREQUAL ${checked_file})
      set(is_c_file 1)
    endif()
  endforeach()

  if (is_scanner_parser_file)
    set_source_files_properties(
      ${src} PROPERTIES COMPILE_FLAGS ${flex_bison_compiler_flags})
  else()
		if (is_c_file)
			set_source_files_properties(
				${src} PROPERTIES COMPILE_FLAGS ${vata_c_compiler_flags})
		else()
			set_source_files_properties(
				${src} PROPERTIES COMPILE_FLAGS ${vata_cxx_compiler_flags})
		endif()
  endif()
endforeach()
