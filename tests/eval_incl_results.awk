#!/usr/bin/awk -f

#
# This AWK script compares 2 columns in the output of incl_test.sh
#

# Prints an error message and exits
function printErrorExit(msg) {
  print msg > "/dev/stderr"
  exit 1
}

# Checks whether tha parameter 's' is a string representing an index
function checkIsIndex(s) {
  if ((strtonum(s) != s) || (s < 0) || (int(s) != s))
  {
    printErrorExit("The string \"" s "\" is not an index")
  }
}

# Checks whether the current line is valid
function checkCurrentLineValid() {
  if ((NF <= first) || (NF <= second))
  {
    printErrorExit("Insufficient number of fields: " NF " on line " FNR " of the input file")
  }
}

# Returns the absolute value of a number
function abs(value) {
  return ((value < 0)? -value : value);
}

BEGIN {
# TODO: remove
#  # get the name of the program
#  getline tmp1 < "/proc/self/cmdline";
#  split(tmp1, tmp2, "\0")
#  awkprog=tmp2[3]

#  if (4 != ARGC) {
#    print "need to specify the results file and indices of 2 columns to compare" > "/dev/stderr"
#    print "Example: " awkprog " <file> 1 2" > "/dev/stderr"
#    exit 1
#  }

  # 'first' and 'second' are to be set using the command line, e.g.
  # ./eval_incl_results.awk -v first=0 -v second=1 <file>

  checkIsIndex(first)
  checkIsIndex(second)

  # there are 2 initial columns and the numbering starts from 1
  first+=2+1
  second+=2+1

  # initial timeout
  timeout=0

  # epsilon: the sensitivity of the comparison
  epsilon=0.0005

  # initial values of statistical variables
  totalCases=0
  firstWon=0
  secondWon=0
  nobodyWon=0
  firstBetterMax=0
  secondBetterMax=0
  firstTotal=0
  secondTotal=0
  firstTimeouts=0
  secondTimeouts=0
  sumOfDiffs=0

  state=0
}

# timeout
/Timeout:/ {
  if (0 != timeout)
  {
    printErrorExit("Timeout specified multiple times")
  }

  timeout=$2
  if ((strtonum(timeout) != timeout) || (timeout <= 0) || (int(timeout) != timeout))
  {
    printErrorExit("Invalid timeout value:" timeout)
  }

  print "Timeout value: " $2
  next
}

# the delimiter
/^------/ {
  if (0 != state)
  {
    print "Invalid input format (delimiter twice)" > "/dev/stderr"
    exit 1
  }

  # change the field separator
  FS=";"

  # start processing header
  state=1
  next
}

# the header
1 == state {
  checkCurrentLineValid()

  method1=$first
  method2=$second

  print "Method 1 = " method1 ", Method 2 = " method2

  # start processing data
  state=2
  next
}

# data
2 == state {
  checkCurrentLineValid()

  firstVal=$first
  secondVal=$second

  if (match(firstVal, / *- */))
  {
    ++firstTimeouts
    firstVal=timeout
  }

  if (match(secondVal, / *- */))
  {
    ++secondTimeouts
    secondVal=timeout
  }

  ++totalCases
  firstTotal += firstVal
  secondTotal += secondVal

  diff = secondVal - firstVal
  sumOfDiffs += diff
  print "First = " firstVal " Second = " secondVal " Diff = " diff

  if (abs(diff) <= epsilon)
  { # negligible difference
    ++nobodyWon
  }
  else if (diff > 0)
  {
    ++firstWon

    if (diff > firstBetterMax)
    {
      firstBetterMax = diff
    }
  }
  else if (diff < 0)
  {
    diff = -diff   # invert for uniform work

    ++secondWon

    if (diff > secondBetterMax)
    {
      secondBetterMax = diff
    }
  }
  else
  {
    printErrorExit("Internal error");
  }

  next
}

END {
  firstTotalSpeedupPrc=secondTotal/firstTotal * 100
  secondTotalSpeedupPrc=firstTotal/secondTotal * 100

  printf("Comparing       %20s      vs. %20s\n", method1, method2)
  printf("Total cases     %12s%8s          %12s\n", totalCases, "", totalCases)
  printf("Time-out time   %12s s%6s          %12s s\n", timeout, "", timeout)
  printf("Won             %12s%8s          %12s\n", firstWon, "", secondWon) 
  printf("Nobody won      %12s%8s          %12s\n", nobodyWon, "", nobodyWon) 
  printf("Total time      %12s s%6s          %12s s\n", firstTotal, "", secondTotal)
  printf("Total speedup   %12s %%%6s          %12s %%\n", firstTotalSpeedupPrc, "", secondTotalSpeedupPrc) 
  printf("Best speedup    %12s s%6s          %12s s\n", firstBetterMax, "", secondBetterMax)
  printf("Time-outs       %12s%8s          %12s\n", firstTimeouts, "", secondTimeouts)
  printf("Sum of diffs    %12s s%6s          %12s s\n", sumOfDiffs, "", -sumOfDiffs)
}
