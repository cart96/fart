# build script for linux/macos

export CC="clang"
export FLAGS="-Os -s -Wall -Werror -fstack-protector -std=c99 -pedantic-errors"

$CC $FLAGS -o ../fart ../main.c
