#!/bin/bash

################################# CONSTANTS ##################################

OPERATION=$1
FILE_LHS=`readlink -f $2`
FILE_RHS=`readlink -f $3`

# path to the Timbuk directory
TIMBUK_PATH=/home/ondra/timbuk

# TAML executable
TAML=./taml


################################# FUNCTIONS ##################################

# Function that terminates the script with a message
function die {
  echo "$1";
  exit -1;
}


################################## PROGRAM ###################################

if [ "$#" -ne 3 ]; then
  die "usage: $0 <method> <file1> <file2>"
fi

case "${OPERATION}" in
  isect)
    command=inter
    ;;
  union)
    command=union
    ;;
  *) die "Invalid command ${OPERATION}"
    ;;
esac

cd ${TIMBUK_PATH}
${TAML} > /dev/null << END
let _time = ref 0.;;
let reset (x : unit) = (
  _time := Sys.time ()
);;

let eval (x : unit) = (
  Printf.eprintf "%.9f\n" (Sys.time () -. !_time);
  flush_all ()
);;

let a1 = List.hd (read_automaton_list "${FILE_LHS}") ;;
let a2 = List.hd (read_automaton_list "${FILE_RHS}") ;;

reset () ;;
${command} a1 a2 ;;
eval () ;;
END
