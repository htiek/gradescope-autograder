#!/bin/bash
#
# File: error.sh
# Author: Keith Schwarz (htiek@cs.stanford.edu)
#
# Bash script used by the autograder to write a JSON result file consisting of a piece of error text.
# This is used by the autograder to report some sort of problem that occurs during setup.
echo "ERROR: $1"

cat - > results/results.json << EOM
{
  "tests":
  [
    {
        "name": "Autograder Error",
        "score": 0.0,
        "output": "$1"
    }
  ]
}
EOM
