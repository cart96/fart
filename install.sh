# fart installation script for linux

export SRC="https://github.com/cart96/fart/archive/refs/heads/main.zip"
export SRCOUT="fart-main.zip"

export CC="gcc"
export FLAGS="-Os -s -Wall -Werror -fstack-protector -std=c99 -pedantic-errors"

# download zip
echo "downloading source..."
curl -sL -o $SRCOUT $SRC

# extract zip
echo "extracting zip file..."
unzip -q $SRCOUT

# build with gcc
echo "compiling from source code..."
$CC $FLAGS -o ./fart ./fart-main/src/fart/main.c

# clean up source code files
echo "cleaning source code files..."
rm -rf ./fart-main
rm ./fart-main.zip

# move binary to the /usr/bin
echo "moving binary to /usr/bin"
sudo mv ./fart /usr/bin/fart
