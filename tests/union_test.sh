#!/bin/bash

################################# CONSTANTS ##################################

# default timeout (can be overriden from command line)
timeout=300;

# Absolute path to this script, e.g. /home/user/bin/foo.sh
SCRIPT=`readlink -f $0`

# Absolute path this script is in, thus /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

METHODS=( 
  "expl"
  "symdown"
  "symup"
  "timbuk"
)

################################# FUNCTIONS ##################################

# Function that terminates the script with a message
function die {
  echo "$1";
  exit -1;
}

# Function that runs a single test
# takes:   <LHS automaton>   <RHS automaton>  <method>   <file to write result>   <file to write time>
function runone {
  ulimit -St "${timeout}";
  ${SCRIPTPATH}/union_wrapper.sh "$1" "$2" "$3" 2> "$4"
  return $?
}

# Function that runs all tests for one pair of automata
function runall {

  # Set trap for terminating the script
  trap killscripts INT TERM EXIT

  local i;

  # run all methods
  for (( i=0; i < ${#METHODS[*]}; i++ )); do
    runone "${METHODS[$i]}" "$1" "$2" "${tmpTime[$i]}" 2> /dev/null &
    pid[$i]=$!;
  done;

  # evaluate all methods
  for (( i=0; i < ${#METHODS[*]}; i++ )); do
    wait ${pid[$i]};
    local ret=$?;
    pid[$i]=0;
    if [ "$ret" == "152" ]; then
      printcolumn "-"
    else
      local v=`<"${tmpTime[$i]}"`;
      [ "$ret" == "0" ] || die "union failed! ($ret)"; 
      num=$(printf "%.4f" "${v}")
      printcolumn "${num}";
    fi;
  done;
  printf "\n"
}

# Prints a column into the result table
function printcolumn {
  printf "%10s;" "$1"
}

function killscripts {
  local i

  for i in $(ps | grep "\(vata\|sfta\)" | cut -d' ' -f1); do
    kill ${i}
  done
}


################################## PROGRAM ###################################

# Check the number of command-line arguments
if [ \( "$#" -eq 0 \) -o \( "$#" -gt 2 \) ] ; then
  die "usage: $0 <dir> [timeout]"
fi 

# If timeout is specified, use it instead of the default
if [ "$2" != "" ]; then
  timeout="$2";
fi;

AUT_DIR="$1"

# If the directory doesn't exist, terminate
if [ ! -d "${AUT_DIR}" ]; then
  die "directory '${AUT_DIR}' does not exist!"
fi

echo "======== Computing union of automata ======="
echo "Automata directory:   ${1}"
echo "Timeout:              ${timeout} s"

# Create temporary files for storing the output
for (( i=0; i < ${#METHODS[*]}; i++ )); do
  tmpTime[$i]=`mktemp`;
done;

echo "reading files ..."
cnt=0;
for x in $1/*; do
  key[$cnt]=`basename $x`;
  val[$cnt]="${x}";
  #[ "$?" == "0" ] || die "unable to preprocess $x";
  #printf "$x (%s)\n" "`./main i <<< ${val[$cnt]}`";
  cnt=$(($cnt+1));
done;

echo "-----------------------------------------------"
for i in "aut1" "aut2" ; do
  printcolumn ${i}
done

for (( i=0; i < ${#METHODS[*]}; i++ )); do
  printcolumn "${METHODS[$i]}"
done;

printf "\n"

for (( i=0; i < $cnt; i++ )); do
  for (( j=0; j < $cnt; j++ )); do
    printcolumn ${key[$i]}
    printcolumn ${key[$j]}
    runall "${val[$i]}" "${val[$j]}";
  done;
done;

# Remove temporary files
for (( i=0; i < ${#METHODS[*]}; i++ )); do
  rm ${tmp[$i]}
  rm ${tmpTime[$i]}
done;
