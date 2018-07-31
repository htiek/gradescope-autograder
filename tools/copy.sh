# copy.sh
#
# Script to copy a file with a given name to a given destination.
#
# Usage: copy.sh filename dest-directory

# Ensure we have the right number of arguments.
if [ $# -ne 2 ]
then
    echo "Internal error: Too few arguments to copy.sh."
    echo "Number of arguments: $#"
    exit 1
fi

# Find all submitted files matching the given name.
readarray -t STUDENT_FILE << DONE
`find submission -name $1`
DONE

# If too many files are found, we have to panic and give up.
if [ ${#STUDENT_FILE[@]} -ge 2 ]
then
    tools/error.sh "Multiple copies of $1 were submitted; not sure which to use."
    exit 1
fi


# If no files are found, give up.
if [ ${#STUDENT_FILE[@]} -eq 0 ] || [ -z "$STUDENT_FILE" ]
then
    echo "Couldn't find file $1 in submission."
    tools/error.sh "You need to submit a source file named $1."
    exit 1
fi

echo "Found student submission: [$STUDENT_FILE]"

# Copy over the student's submission. If no submission was found, report an error.
if cp "$STUDENT_FILE" "$2/"
then
    echo "Copied file $1."
else
    tools/error.sh "An internal error occurred trying to copy $1. Please contact the course staff."
    exit 1
fi
