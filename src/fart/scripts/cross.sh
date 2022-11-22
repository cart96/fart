# cross-compile for fart with zig cc

export CC="zig cc"
export FLAGS="-Os -s -Wall -fstack-protector -std=c99 -pedantic-errors -o fart"

cd ..

if [ -d "./build" ]
then
    rm -rf ./build
fi

mkdir build

for target in "x86_64-linux-musl" "i386-linux-musl"  "aarch64-linux-musl"  "riscv64-linux-musl" "x86_64-windows-gnu";
do
    mkdir ./build/$target
    $CC $FLAGS -target $target main.c

    mv ./fart ./build/$target/fart
    sha256sum ./build/$target/fart > ./build/$target/sha256.checksum.txt

    if [ "$target" = "x86_64-windows-gnu" ];
    then
        mv ./build/$target/fart ./build/$target/fart.exe
    fi

    cd ./build
    zip -r "fart-$target.zip" $target
    cd ..
done
