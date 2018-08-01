# Install dependencies.
add-apt-repository ppa:ubuntu-toolchain-r/test -y   &&
apt-get update                                      &&
apt-get upgrade -y                                  &&
apt-get install g++-6 -y                            &&

# Move everything from the source/ directory up a level. That extra indirection is just a
# wee bit confusing. :-)
mv source/* .
