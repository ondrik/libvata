#!/bin/bash

# Absolute path to this script, e.g. /home/user/bin/foo.sh
SCRIPT=`readlink -f $0`

# Absolute path this script is in, thus /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

# SFTA executable
SFTA="${SCRIPTPATH}/sfta"

while read  inputline; do
  what+="${inputline} "
done

if [[ "$1" == "symsu" || "$1" == "symsd" || "$1" == "symsds" || "$1" == "symsdsw" || "$1" == "symsdsb" || "$1" == "symsdsbw" ]] ; then
	tmp[1]=`mktemp`
	tmp[2]=`mktemp`
	tmpproc[1]=`mktemp`
	tmpproc[2]=`mktemp`
	echo ${what} | sed 's/^\(..*\)\<Ops\>/\1\nOps/g' | sed '2d' > ${tmp[1]}
	echo ${what} | sed 's/^\(..*\)\<Ops\>/\1\nOps/g' | sed '1d' > ${tmp[2]}

	for i in 1 2; do
		cat ${tmp[${i}]} \
			| sed 's/Automaton/\n\nAutomaton/' \
			| sed 's/Final States/FinalStates/' \
			| sed 's/\<States\>/\n\nStates/' \
			| sed 's/FinalStates/\n\nFinal States/' \
			| sed 's/Transitions/\n\nTransitions/' \
			| sed 's/()//g' \
			| awk '/^States/ { printf("States"); for (i = 2; i <= NF; ++i) {printf(" %s:0", $i)} printf("\n") ; next}  { print }' \
			| awk '/^Transitions/ { print "Transitions"; for (i = 2; i <= NF; ++i) { print $i } ; next}  { print }' \
			| sed 's/->/ -> /g' \
			> ${tmpproc[$i]}
		rm ${tmp[$i]}
	done;

	if [[ "$1" == "symsd" ]] ; then
		${SFTA} -o ${tmpproc[1]} ${tmpproc[2]}
		retval=$?
	else
		if [[ "$1" == "symsu" ]] ; then
			${SFTA} -p ${tmpproc[1]} ${tmpproc[2]}
			retval=$?
		else
			if [[ "$1" == "symsds" ]] ; then
				${SFTA} -n ${tmpproc[1]} ${tmpproc[2]}
				retval=$?
			else
				if [[ "$1" == "symsdsw" ]] ; then
					${SFTA} -w ${tmpproc[1]} ${tmpproc[2]}
					retval=$?
				else
					if [[ "$1" == "symsdsb" ]] ; then
						${SFTA} -m ${tmpproc[1]} ${tmpproc[2]}
						retval=$?
					else
						if [[ "$1" == "symsdsbw" ]] ; then
							${SFTA} -a ${tmpproc[1]} ${tmpproc[2]}
							retval=$?
						else
							echo "ERROR"
							exit 130
						fi
					fi
				fi
			fi
		fi
	fi

	rm ${tmpproc[1]} ${tmpproc[2]}
else
  ./main $1 <<< $(echo -e ${what})
  retval=$?
fi

exit ${retval}
