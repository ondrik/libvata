# CMakeLists.txt for tests

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/vata-tester.sh.in     ${CMAKE_CURRENT_BINARY_DIR}/vata-tester.sh     @ONLY)

# program for reading a file
set(cat_prog cat)

# the tester
set(tester ${CMAKE_CURRENT_BINARY_DIR}/vata-tester.sh)

# timeout for each test (in seconds)
set(test_timeout 30)

set(incl_test_file "${CMAKE_CURRENT_SOURCE_DIR}/aut_timbuk_smaller_incl_td.txt")

# read the content of the file with automata and results of inclusion test
# The format is:
#
#     aut1    aut2   0
#     aut3    aut4   1
#
# where 0 means that the inclusion L(aut1) <= L(aut2) DOES NOT HOLD and
# 1 means that the inclusion L(aut3) <= L(aut4) HOLDS, where aut{1,2,3,4} are
# files containing respective automata
exec_program(${cat_prog} ARGS "${incl_test_file}" OUTPUT_VARIABLE file_content)

string(REPLACE "\n" ";" file_lines ${file_content})

# inclusion tests
#set(tests
#  "value '-r expl incl ../../tests/aut_timbuk_smaller/A0053 ../../tests/aut_timbuk_smaller/A0053' 1"
#)

#set(cnt 1)
#
#foreach(test ${tests})
#  set(cmd "${tester}")
#  set(cmd "${cmd} ${test}")
#
#  add_test("test-${cnt}" bash -o pipefail -c "${cmd}")
#
#  MATH(EXPR cnt "${cnt} + 1")
#endforeach()

set(incl_tests
  "-r expl -o dir=down,sim=no,rec=yes,optC=no"
  "-r expl -o dir=down,sim=no,rec=yes,optC=yes"
  "-r expl -o dir=down,sim=no,rec=no"
  "-r expl -o dir=down,sim=yes,rec=yes,optC=no"
  "-r expl -o dir=down,sim=yes,rec=yes,optC=yes"
  "-r expl -o dir=down,sim=yes,rec=no"
  "-r expl -o dir=up,sim=no"
  "-r expl -o dir=up,sim=yes"
  "-r bdd-td -o dir=down,sim=no,rec=yes,optC=no"
  "-r bdd-td -o dir=down,sim=no,rec=yes,optC=yes"
  "-r bdd-bu -o dir=down,sim=no,rec=yes,optC=no"
  "-r bdd-bu -o dir=down,sim=no,rec=yes,optC=yes"
  "-r bdd-bu -o dir=down,sim=yes,rec=yes,optC=no"
  "-r bdd-bu -o dir=down,sim=yes,rec=yes,optC=yes"
  "-r bdd-bu -o dir=up,sim=no"
)

foreach(incl_test_parm ${incl_tests})
  foreach(triple ${file_lines})
    string(REGEX REPLACE "\ +" ";" test_spec ${triple})
    list(GET test_spec 0 aut1)
    list(GET test_spec 1 aut2)
    list(GET test_spec 2 result)

    set(test "value")
    set(test "${test} 'incl ")
    set(test "${test} ${incl_test_parm}")
    set(test "${test} ../../tests/aut_timbuk_smaller/${aut1}")
    set(test "${test} ../../tests/aut_timbuk_smaller/${aut2}'")
    set(test "${test} ${result}")

    set(cmd "${tester}")
    set(cmd "${cmd} ${test}")

    string(REGEX REPLACE "\ +" "_" test_parm_joined "${incl_test_parm}")
    set(test_name "test_${aut1}_${aut2}_${test_parm_joined}")

    add_test("${test_name}" bash -o pipefail -c "${cmd}")
    set_tests_properties("${test_name}" PROPERTIES TIMEOUT "${test_timeout}")

  endforeach()
endforeach()
