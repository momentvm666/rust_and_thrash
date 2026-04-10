# Set the base path
export DJDIR=/opt/djgpp

# 1. Update the PATH so you can call the compiler directly
export PATH=$DJDIR/bin:$PATH

# 2. THE CRITICAL ONE: Tell DJGPP where its config is
# This usually points to the djgpp.env file in the root of the installation
export DJGPP=$DJDIR/djgpp.env

echo "DJGPP set up!"
echo "Compiler: $(which i586-pc-msdosdjgpp-g++)"