#!/bin/bash
#
# File: error.sh
# Author: Keith Schwarz (htiek@cs.stanford.edu)
#
# Bash script used by the autograder to write a JSON result file consisting of a piece of error text.
# This is used by the autograder to report some sort of problem that occurs during setup.
echo "ERROR: $1"

# Escape the string that needs to be printed. Thanks to
# https://stackoverflow.com/questions/10053678/escaping-characters-in-bash-for-json
# for this one.
CLEAN_ERROR=`python -c 'import json,sys; print(json.dumps(sys.stdin.read()))' <<< "$1"`

cat - > results/results.json << EOM
{
  "tests":
  [
    {
        "name": "Autograder Error",
        "score": 0.0,
        "output": $CLEAN_ERROR
    }
  ]
}
EOM
