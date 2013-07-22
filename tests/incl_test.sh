#!/bin/bash

################################# CONSTANTS ##################################

# default timeout (can be overriden from command line)
timeout=300;

# Absolute path to this script, e.g. /home/user/bin/foo.sh
SCRIPT=`readlink -f $0`

# Absolute path this script is in, thus /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

#METHODS=("symdown" "symdownX" "downT")
#METHODS=("symdown" "symdown-ul" "olddown" "olddown-ul" "downT" "downSimT" "upT")
#METHODS=("symdown" "symdown-ul" "expldown" "downT" "downSimT" "upT")
#METHODS=("explup" "explup-sim" "expldown" "expldown-sim" "expldown-optC" "expldown-sim-optC" "symup" "symdown" "symdown-sim" "symdown-optC" "symdown-sim-optC")

METHODS=( 
#  "expldown-rec"
#  "expldown-rec-optC"
#  "expldown-nonrec"

#  "expldown-rec-sim"
#  "expldown-rec-sim-optC"
#  "expldown-nonrec-sim"

#  "expldown-rec-sim-nosimtime"
#  "expldown-rec-sim-optC-nosimtime"
#  "expldown-nonrec-sim-nosimtime"

  "explup"
  "explup-sim"
#  "explup-sim-nosimtime"

#  "old-expldown"
#  "old-expldown-sim"
#  "old-expldown-sim-nosimtime"
#  "old-expldown-optC"
#  "old-expldown-sim-optC"
#  "old-expldown-sim-optC-nosimtime"
#  "symdown-rec"
#  "symdown-rec-sim"
#  "symdown-rec-sim-nosimtime"
#  "symdown-rec-optC"
#  "symdown-rec-sim-optC"
#  "symdown-rec-sim-optC-nosimtime"
#  "symup"

#  "explfa-ac"
#  "explfa-ac-sim"
  "explfa-congr"
  "explfa-congr-sim"
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
  ${SCRIPTPATH}/incl_wrapper.sh "$1" "$2" "$3" > "$4" 2> "$5"
  return $?
}

# Function that runs all tests for one pair of automata
function runall {

  # Set trap for terminating the script
  trap killscripts INT TERM EXIT

  local res="?";
  local i;

  # run all methods
  for (( i=0; i < ${#METHODS[*]}; i++ )); do
    runone "${METHODS[$i]}" "$1" "$2" "${tmp[$i]}" "${tmpTime[$i]}" 2> /dev/null &
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
      local v=(`<"${tmp[$i]}"` "`<"${tmpTime[$i]}"`");
      [ "$ret" == "0" ] || die "inclusion test failed! ($ret)"; 
      if [ "$res" == "?" ]; then
        res=${v[0]};
      else
        [ "$res" == ${v[0]} ] || die "inconsistent results! ($res != ${v[0]})";
      fi;
      num=$(printf "%.4f" "${v[1]}")
      printcolumn "${num}";
    fi;
  done;
  printcolumn "$res"
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

echo "======== Checking inclusion of automata ======="
echo "Automata directory:   ${1}"
echo "Timeout:              ${timeout} s"

# Create temporary files for storing the output
for (( i=0; i < ${#METHODS[*]}; i++ )); do
  tmp[$i]=`mktemp`;
  tmpTime[$i]=`mktemp`;
done;

echo "reading files ..."
cnt=0;
for x in ${AUT_DIR}/*; do
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

printcolumn "'<='"
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
