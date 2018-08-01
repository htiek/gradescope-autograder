#!/bin/bash
#
# Run this script before uploading the autograder! It goes through and cleans everything out.

# Nuke the assembly directory; it shouldn't be there.
rm -rf assembly/

# Clean the build directory and the tests directory, just in case.
tools/build.sh build-directory clean
tools/build.sh tests -f Makefile.tests clean

# Remove the results/ directory so there's no risk of uploading a results.json file.
rm -rf results/

# Remove the submission/ directory so there's no risk of uploading a default submission.
rm -rf submission/
