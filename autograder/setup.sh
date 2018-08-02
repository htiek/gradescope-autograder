# Install dependencies.
add-apt-repository ppa:ubuntu-toolchain-r/test -y   &&
apt-get update                                      &&
apt-get install build-essential g++-6 -y

# Move everything into the same directory arrangment as in the local version
mv autograder/source/* autograder/
