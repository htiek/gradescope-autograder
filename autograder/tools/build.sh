#!/bin/bash
#
# File: build.sh
# Author: Keith Schwarz
#
# Utility to build all of the files in a given directory. The usage is
#
#   ./build.sh directory [make-flags]
#
# That last argument can be specified to pass extra flags to make.
if [ $# -lt 1 ]
then
    echo "Internal error: Too few arguments to build.sh."
    echo "Number of arguments: $#"
    exit 1
fi

if (cd "$1"; shift; make $@); then
  exit 0
else
  # TODO: Better error reporting?
  tools/error.sh "The code you submitted did not compile."
  exit 1
fi
  
