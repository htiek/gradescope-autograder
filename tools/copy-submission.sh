# copy-submission.sh
#
# Script to copy the relevant files from the student submission to a target destination.
# The first argument should be the name of a file containing a list of all the files we
# expect the student to submit, and the second argument should be the target directory
# where they need to end up.
#
# Usage: copy-submission.sh manifest destination

# Ensure we have the right number of arguments.
if [ $# -ne 2 ]
then
  echo "Internal error: Too few arguments to copy-submission.sh."
  echo "Number of arguments: $#"
  exit 1
fi

manifestFile=$1
destination=$2

# Read the manifest file.
if [ ! -f "$manifestFile" ]; then
  echo "Internal error: Manifest file $manifestFile not found."
  exit 1
fi

# Copy all files from the manifest into the destination directory
cat "$manifestFile" | while read -r line; do
  # Skip empty lines or lines starting with #.
  ([[ "$line" =~ ^#.*$ ]] || [ -z "$line" ]) && continue
  
  tools/copy.sh "$line" "$destination" || exit 1
done
