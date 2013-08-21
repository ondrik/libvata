# CMakeLists.txt for tests

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/vata-tester.sh.in     ${CMAKE_CURRENT_BINARY_DIR}/vata-tester.sh     @ONLY)

# program for reading a file
set(cat_prog cat)

# the tester
set(tester ${CMAKE_CURRENT_BINARY_DIR}/vata-tester.sh)

# read the content of the file with automata and results of inclusion test
# The format is:
#
#     aut1    aut2   0
#     aut3    aut4   1
#
# where 0 means that the inclusion L(aut1) <= L(aut2) DOES NOT HOLD and
# 1 means that the inclusion L(aut3) <= L(aut4) HOLDS
exec_program(${cat_prog} ARGS ${CMAKE_CURRENT_SOURCE_DIR}/aut_timbuk_smaller_incl.txt OUTPUT_VARIABLE file_content)

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

foreach(triple ${file_lines})
  string(REGEX REPLACE "\ +" ";" test_spec ${triple})
  list(GET test_spec 0 aut1)
  list(GET test_spec 1 aut2)
  list(GET test_spec 2 result)

  set(test "value")
  set(test "${test} 'incl ")
  set(test "${test} ../../tests/aut_timbuk_smaller/${aut1}")
  set(test "${test} ../../tests/aut_timbuk_smaller/${aut2}'")
  set(test "${test} ${result}")

  set(cmd "${tester}")
  set(cmd "${cmd} ${test}")

  add_test("test-${aut1}-${aut2}" bash -o pipefail -c "${cmd}")
endforeach()
