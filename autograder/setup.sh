# Install dependencies.
apt-get install python3 build-essential -y

# Move everything into the same directory arrangment as in the local version
mv autograder/source/* autograder/

# If there is a custom setup script, run it.
if [ -f "autograder/my-setup.sh" ]; then
  autograder/my-setup.sh || exit 1
fi

# Build everything in the build-directory once as a clean build so that we
# don't have to recompile things later.
cd autograder
tools/build.sh "build-directory"
