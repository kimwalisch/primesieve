#!/bin/bash

# Usage: scripts/build_mingw64_clang.sh ARCH
# @ARCH: either x64 or arm64
# Builds a primesieve release binary that is statically linked
# and ready for distribution.

# Prerequisites arm64:
# 1) Install a trial version of both Parallels & Windows on a MacBook ARM64.
# 2) No need to purchase/register Parallels & Windows, keep using the trial version.
# 3) Install MSYS2 x64 (or arm64 if available)
# 4) Open C:/msys64/clangarm64.exe
# 5) pacman -Syu (exit then run it again)
# 6) pacman -S mingw-w64-clang-aarch64-clang mingw-w64-clang-aarch64-openmp make cmake git zip unzip
# 7) cd primesieve
# 8) scripts/build_mingw64_clang.sh arm64

if [ $# -ne 1 ]
then
    echo "Usage examples:"
    echo "$ scripts/build_mingw64_clang.sh x64"
    echo "Build primesieve for the x64 CPU architecture."
    echo "$ scripts/build_mingw64_clang.sh arm64"
    echo "Build primesieve for the arm64 CPU architecture."
    exit 1
fi

arch=$1

# Exit if any error occurs
set -e

rm -rf build*

####################################################################

FULL_DATE=$(date +'%B %d, %Y')
YEAR=$(date +'%Y')

cd include
VERSION=$(grep "PRIMESIEVE_VERSION " primesieve.hpp | cut -f2 -d'"')
cd ..

####################################################################

handle_error() {
    echo ""
    echo "Error: $1"
    exit 1
}

####################################################################

# The repo must no have any uncommited changes as we
# switch to another branch during the script.
git diff --exit-code > /dev/null || handle_error "repo must not have any uncommitted changes"

# Build primesieve binary ##########################################

git pull
mkdir build-release
cd build-release

clang++ -static -O3 -DNDEBUG -D_WIN32_WINNT=0x0A00 -Wall -Wextra -pedantic -I ../include ../src/*.cpp ../src/app/*.cpp -o primesieve.exe
strip primesieve.exe

# Create a release zip archive
wget https://github.com/kimwalisch/primesieve/releases/download/v11.0/primesieve-11.0-win-$arch.zip
unzip primesieve-11.0-win-$arch.zip -d primesieve-$VERSION-win-$arch
rm primesieve-11.0-win-$arch.zip

echo ""
echo ""
echo "Old file size: $(ls -l --block-size=K primesieve-$VERSION-win-$arch/primesieve.exe)"
echo "New file size: $(ls -l --block-size=K primesieve.exe)"
echo ""
echo ""

mv -f primesieve.exe primesieve-$VERSION-win-$arch
cd primesieve-$VERSION-win-$arch
sed -i "1 s/.*/primesieve $VERSION/" README.txt
sed -i "2 s/.*/$FULL_DATE/" README.txt
sed -i "3 s/.*/Copyright \(c\) 2010 - $YEAR, Kim Walisch\./" COPYING

# Verify sed has worked correctly
[ "$(sed -n '1p' < README.txt)" = "primesieve $VERSION" ] || handle_error "failed updating README.txt"
[ "$(sed -n '2p' < README.txt)" = "$FULL_DATE" ] || handle_error "failed updating README.txt"
[ "$(sed -n '3p' < COPYING)" = "Copyright (c) 2010 - $YEAR, Kim Walisch." ] || handle_error "failed updating COPYING"

zip primesieve-$VERSION-win-$arch.zip primesieve.exe README.txt COPYING
cp primesieve-$VERSION-win-$arch.zip ..

./primesieve -v
echo ""
echo ""

./primesieve --cpu-info
echo ""
echo ""

./primesieve --test
echo ""
echo ""

./primesieve 1e11
echo ""
echo ""

####################################################################

echo "Release binary built successfully!"
cd ..
