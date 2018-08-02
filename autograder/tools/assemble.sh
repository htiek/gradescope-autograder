#!/bin/bash
#
# Builds the test harness into the specified directory. If that directory exists, it will
# be overwritten and replaced with the result
#
#   Usage: assemble.sh where-to

# Ensure we have the right number of arguments.
if [ $# -ne 1 ]
then
    echo "Internal error: Too few arguments to assemble.sh."
    echo "Number of arguments: $#"
    exit 1
fi

rm -rf "$1"                                       &&  # Ensure there's no assembly directory lying around
([ -d results ] || mkdir results)                 &&  # Ensure there's a results directory
cp -r build-directory "$1"                        &&  # Create a spot to build everything
tools/copy-submission.sh MANIFEST "$1"            &&  # Copy student submissions
tools/build.sh "$1"                               &&  # Build student submission
cp -r tests/* "$1"/                               &&  # Copy over test cases
cp -r test-driver/* "$1"/                         &&  # Copy over test driver
tools/build.sh "$1" -f Makefile.tests                 # Build the testing harness
