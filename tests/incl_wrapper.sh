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
#SFTA="${SCRIPTPATH}/sfta"
SFTA="${SCRIPTPATH}/sfta_wrapper.sh"

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
  expldown)
    ${VATA} -r expl -t incl -o dir=down,sim=no,optC=no "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  expldown-sim)
    ${VATA} -r expl -t incl -o dir=down,sim=yes,optC=no,timeS=yes "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  expldown-sim-nosimtime)
    ${VATA} -r expl -t incl -o dir=down,sim=yes,optC=no,timeS=no "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  expldown-optC)
    ${VATA} -r expl -t incl -o dir=down,sim=no,optC=yes "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  expldown-sim-optC)
    ${VATA} -r expl -t incl -o dir=down,sim=yes,optC=yes,timeS=yes "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  expldown-sim-optC-nosimtime)
    ${VATA} -r expl -t incl -o dir=down,sim=yes,optC=yes,timeS=no "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  explup)
    ${VATA} -r expl -t incl -o dir=up,sim=no "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  explup-sim)
    ${VATA} -r expl -t incl -o dir=up,sim=yes,timeS=yes "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  explup-sim-nosimtime)
    ${VATA} -r expl -t incl -o dir=up,sim=yes,timeS=no "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  symdown)
    ${VATA} -r bdd-td -t incl -o dir=down,sim=no,optC=no "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  symdown-sim)
    ${VATA} -r bdd-bu -t incl -o dir=down,sim=yes,optC=no,timeS=yes "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  symdown-sim-nosimtime)
    ${VATA} -r bdd-bu -t incl -o dir=down,sim=yes,optC=no,timeS=no "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  symdown-optC)
    ${VATA} -r bdd-td -t incl -o dir=down,sim=no,optC=yes "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  symdown-sim-optC)
    ${VATA} -r bdd-bu -t incl -o dir=down,sim=yes,optC=yes,timeS=yes "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  symdown-sim-optC-nosimtime)
    ${VATA} -r bdd-bu -t incl -o dir=down,sim=yes,optC=yes,timeS=no "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  symup)
    ${VATA} -r bdd-bu -t incl -o dir=up,sim=no "${FILE_LHS}" "${FILE_RHS}"
    RETVAL="$?"
    ;;
  symdownX)
    ${SFTA} symsd <<< $(echo "${LHS}" "${RHS}")
    RETVAL="$?"
    ;;
  symdown-simX)
    ${SFTA} symsdsb <<< $(echo "${LHS}" "${RHS}")
    RETVAL="$?"
    ;;
  symdown-sim-nosimtimeX)
    ${SFTA} symsdsbw <<< $(echo "${LHS}" "${RHS}")
    RETVAL="$?"
    ;;
  symupX)
    ${SFTA} symsu <<< $(echo "${LHS}" "${RHS}")
    RETVAL="$?"
    ;;
  expldownT)
    ${TALIB} sdif <<< $(echo "${LHS}" "${RHS}")
    RETVAL="$?"
    ;;
  expldown-simT)
    ${TALIB} sddf <<< $(echo "${LHS}" "${RHS}")
    RETVAL="$?"
    ;;
  expldown-sim-nosimtimeT)
    ${TALIB} sdds <<< $(echo "${LHS}" "${RHS}")
    RETVAL="$?"
    ;;
  explupT)
    ${TALIB} suif <<< $(echo "${LHS}" "${RHS}")
    RETVAL="$?"
    ;;
  explup-simT)
    ${TALIB} suuf <<< $(echo "${LHS}" "${RHS}")
    RETVAL="$?"
    ;;
  explup-sim-nosimtimeT)
    ${TALIB} suus <<< $(echo "${LHS}" "${RHS}")
    RETVAL="$?"
    ;;
  *) die "Invalid option ${OPERATION}"
    ;;
esac

exit ${RETVAL}

