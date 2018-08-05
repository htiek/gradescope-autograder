#!/bin/bash
#
# Run this script before uploading the autograder! It goes through and cleans everything out.

# By default, we will try updating the autograder to use the most recent version. You can
# override this by passing in --no-update as an argument to the script.
if [ "$1" != "--no-update" ]; then
    echo "We're going to start off by updating the autograder framework to use the"
    echo "most recent version available on GitHub."
    echo
    
    # Stash the zip and extract it in a temporary place.
    UPDATE_ARCHIVE=$(mktemp)
    UPDATE_DIRECTORY=$(mktemp -d)
    
    # Base path to append to the extracted directory to get where we want.
    FILE_PATH="gradescope-autograder-master/autograder"
    
    curl -sS "https://codeload.github.com/htiek/gradescope-autograder/zip/master" > $UPDATE_ARCHIVE && # Download
    unzip -q $UPDATE_ARCHIVE -d $UPDATE_DIRECTORY || exit 1                                            # Extract
    
    # List of files to copy over
    UPDATE_FILES="test-driver tools Instructions run_autograder setup.sh assemble-autograder.sh"
    
    # Directory to stash all the old versions of these files.
    OLD_FILES_DIRECTORY=$(mktemp -d)
    
    # Copy over all the files that need to be updated.
    for file in $UPDATE_FILES; do
      # Move the old files to a temporary directory. This is important, since this script is currently
      # running and we don't want the update to break things!
      mv $file $OLD_FILES_DIRECTORY/               || exit 1
      cp -r $UPDATE_DIRECTORY/$FILE_PATH/$file .   || exit 1
    done
    
    echo "Update complete!"
    echo
    
    # Run the new update script, passing in --no-update to ensure that this doesn't form an infinite
    # recursive loop!
    ./assemble-autograder.sh --no-update || exit 1
    exit 0
fi

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

tools/assemble.sh assembly && TOTAL_POINTS=$(cd assembly && ./run-tests --count-points) || exit 1

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
