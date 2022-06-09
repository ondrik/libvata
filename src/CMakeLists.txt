cmake_minimum_required(VERSION 2.8.2)

set(CMAKE_COLOR_MAKEFILE ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)

project(vata)

# Export compile commands to be used with YouCompleteMe
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Flags for VATA
set(vata_cxx_compiler_flags
  -pedantic-errors
  -Wextra
  -Wall
  -Wfloat-equal
  -fdiagnostics-show-option
  -std=c++11
  -Wctor-dtor-privacy
  -Weffc++
  -fPIC
	-fno-strict-aliasing
  -Woverloaded-virtual
  -Wold-style-cast
)

foreach(flag ${vata_cxx_compiler_flags})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
endforeach()

# uncomment for profiling
#add_definitions(-pg)
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")

include_directories(../include)

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
	symbolic.cc
	timbuk_parser-nobison.cc
	timbuk_serializer.cc
	util.cc
	sym_var_asgn.cc
	symbolic_tree_aut_base_core.cc
)

set_target_properties(libvata PROPERTIES
  OUTPUT_NAME vata
  CLEAN_DIRECT_OUTPUT 1
)
