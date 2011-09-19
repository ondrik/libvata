#!/bin/bash

################################# CONSTANTS ##################################

OPERATION=$1
FILE_LHS=$2
FILE_RHS=$3

# Absolute path to this script, e.g. /home/user/bin/foo.sh
SCRIPT=`readlink -f $0`

# Absolute path this script is in, thus /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

# VATA executable
VATA="${SCRIPTPATH}/../build/src/vata"

# VATA executable
OLDVATA="${SCRIPTPATH}/old-vata"

# SFTA executable
SFTA="${SCRIPTPATH}/sfta"

# TAlib executable
TALIB="${SCRIPTPATH}/talib"

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

LHS=$(${TALIB} n < "${FILE_LHS}")
RHS=$(${TALIB} n < "${FILE_RHS}")

RETVAL="?"

case "${OPERATION}" in
  symdown)
    ${VATA} -r bdd-td -t incl "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  symdown-ul)
    ${VATA} -r bdd-td -t incl -s "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  symup)
    ${VATA} -r bdd-bu -t incl "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  symup-ul)
    ${VATA} -r bdd-bu -t incl -s "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  expldown)
    ${VATA} -r expl -t incl "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  expldown-ul)
    ${VATA} -r expl -t incl -s "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  olddown)
    ${OLDVATA} -t incl "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  olddown-ul)
    ${OLDVATA} -t incl -s "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  symdownX)
    ${SFTA} -o "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  downT)
    ${TALIB} sdif <<< $(echo "${LHS}" "${RHS}")
    RETVAL="$?"
    ;;
  downSimT)
    ${TALIB} sddf <<< $(echo "${LHS}" "${RHS}")
    RETVAL="$?"
    ;;
  upT)
    ${TALIB} suif <<< $(echo "${LHS}" "${RHS}")
    RETVAL="$?"
    ;;
  *) die "Invalid option ${OPERATION}"
    ;;
esac

exit ${RETVAL}

