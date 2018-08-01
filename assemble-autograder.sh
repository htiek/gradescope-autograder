#!/bin/bash
#
# Run this script before uploading the autograder! It goes through and cleans everything out.

echo "We're going to do an end-to-end dry run of the autograder to make sure"
echo "that we can build everything appropriately. This means that you'll need"
echo "to have a submission/ directory containing a minimally-compilable version"
echo "of the files you'd expect a student to submit."
echo
echo "If that isn't the case, this process won't work."
echo
echo "Cleaning up all intermediary files..."
echo

# Clean the build directory and the tests directory, just in case.
tools/build.sh build-directory clean
tools/build.sh test-driver -f Makefile.tests clean

echo
echo "End-to-end dry run..."
echo

tools/assemble.sh assembly && TOTAL_POINTS=$(assembly/run-tests --count-points) || exit 1

echo
echo "Assembling ZIP archive..."
echo

TARGET_ZIP=Autograder.zip
zip -r "$TARGET_ZIP" build-directory MANIFEST run_autograder setup.sh test-driver tests tools || exit 1

echo
echo "Autograder is ready to upload!"
echo "  Autograder file: $TARGET_ZIP"
echo "  Points possible: $TOTAL_POINTS"
echo
