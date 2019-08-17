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

# Store the directory where we're building things, then shift it out
# so that our remaining arguments get forwarded on to make.
BUILD_DIR=$1
shift

if (cd "$BUILD_DIR"; make $@ 2> .autograder.error.log); then
  exit 0
else
  # For internal purposes, display the error that was generated.
  ERROR_MESSAGE=`cat "$BUILD_DIR/.autograder.error.log"`
  printf -v RENDERED_ERROR_MESSAGE "The code you submitted did not compile. Compiler error log:\n%s" "$ERROR_MESSAGE"

  tools/error.sh "$RENDERED_ERROR_MESSAGE"
  exit 1
fi
  
