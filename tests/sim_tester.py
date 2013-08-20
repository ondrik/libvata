#! /bin/python

# @param1 the first file with result of a simulation over an automaton
# @param2 the second file with result of a simulation over an automaton

import sys

def getSim(fileToLoad):
	mappingStates=0
	simulationResult=1
	state=mappingStates

	dic1 = {}
	sim1 = {}
	line=0

	for i in fileToLoad:
		if i == "\n": # end of mapping part of file
			if state == mappingStates:
				dic1[int(max(dic1.keys()))+1]="x" # add start state
			state = state+1 # change states
			continue
		if state == mappingStates:
			ml = i.split()
			dic1[ml[2]] = ml[0]
		if state == simulationResult:
			inds = [i for i,x in enumerate(i) if x=="1"]
			for ind in inds:
				try:
					sim1[dic1[str(line)]].extend([dic1[str(ind)]])
				except KeyError:
					sim1[dic1[str(line)]]=[dic1[str(ind)]]
			sim1[dic1[str(line)]].sort()
			line=line+1

	return sim1


if len (sys.argv) < 3:
	sys.stderr.write("Not enough parameters")
	exit(1)

firstFile = open(sys.argv[1],'r')
try:
	sim1 = getSim(firstFile)

	firstFile.close()

	secondFile = open(sys.argv[2],'r')
	sim2 = getSim(secondFile)
	if sim1 == sim2:
		print "Simulations "+sys.argv[1]+" "+sys.argv[2]+" are identical"
	else:
		print "Simulations "+sys.argv[1]+" "+sys.argv[2]+" are not identical"
except KeyError:
	sys.stderr.write("Bad format of a simulation output\n")
