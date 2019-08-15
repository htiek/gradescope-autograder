# Install dependencies.
apt-get update                                      &&
apt-get install build-essential -y

# Move everything into the same directory arrangment as in the local version
mv autograder/source/* autograder/
