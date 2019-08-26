# copy.sh
#
# Script to copy a file with a given name to a given destination.
#
# Usage: copy.sh filename dest-directory missing-files-list

# Ensure we have the right number of arguments.
if [ $# -ne 3 ]; then
    echo "Internal error: Too few arguments to copy.sh."
    echo "Number of arguments: $#"
    exit 1
fi

# Find all submitted files matching the given name.
readarray -t STUDENT_FILE << DONE
`find submission -name $1`
DONE

# If too many files are found, we have to panic and give up.
if [ ${#STUDENT_FILE[@]} -ge 2 ]; then
    tools/error.sh "Multiple copies of $1 were submitted; not sure which to use."
    exit 1
fi


# If no files are found, see if there's a fallback.
if [ ${#STUDENT_FILE[@]} -eq 0 ] || [ -z "$STUDENT_FILE" ]; then
    # Is there a fallback?
    if [ -f "default-files/$1" ]; then
      STUDENT_FILE="default-files/$1"
      echo "  NOT SUBMITTED: $1; using fallback."
      
      # Write this file to the list of missing files.
      echo "$1" >> $3
    else
      echo "No submission for $1; no fallback exists."
      tools/error.sh "You need to submit a source file named $1."
      exit 1
    fi
else
  echo "      SUBMITTED: $1"
fi

# Copy over the student's submission. If no submission was found, report an error.
if !(cp "$STUDENT_FILE" "$2/"); then
    tools/error.sh "An internal error occurred trying to copy $1. Please contact the course staff."
    exit 1
fi
