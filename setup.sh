# Install dependencies.
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt update
sudo apt upgrade -y
sudo apt install g++-6 -y

# Move everything from the source/ directory up a level. That extra indirection is just a
# wee bit confusing. :-)
mv source/* .
