#!/bin/bash

# Usage: scripts/build_mingw64_x64.sh
# Builds a primesieve release binary that is statically linked
# and ready for distribution.

# === Prerequisites x64 ===
# 1) Install MSYS2 x64
# 2) pacman -Syu (exit then run it again)
# 3) pacman -S --needed base-devel mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake zip unzip git
# 4) git clone https://github.com/kimwalisch/primesieve.git
# 5) scripts/build_mingw64_x64.sh

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

g++ -static -O3 -mpopcnt -flto -DNDEBUG -D_WIN32_WINNT=0x0A00 -Wall -Wextra -pedantic -DENABLE_MULTIARCH_AVX512_BW -DENABLE_MULTIARCH_AVX512_VBMI2 -I../include -I../src ../src/*.cpp ../src/arch/x86/*.cpp ../src/app/*.cpp -o primesieve.exe
strip primesieve.exe

# Create a release zip archive
wget https://github.com/kimwalisch/primesieve/releases/download/v12.0/primesieve-12.0-win-x64.zip
unzip primesieve-12.0-win-x64.zip -d primesieve-$VERSION-win-x64
rm primesieve-12.0-win-x64.zip

echo ""
echo ""
echo "Old file size: $(ls -l --block-size=K primesieve-$VERSION-win-x64/primesieve.exe)"
echo "New file size: $(ls -l --block-size=K primesieve.exe)"
echo ""
echo ""

mv -f primesieve.exe primesieve-$VERSION-win-x64
cd primesieve-$VERSION-win-x64
sed -i "1 s/.*/primesieve $VERSION/" README.txt
sed -i "2 s/.*/$FULL_DATE/" README.txt
sed -i "3 s/.*/Copyright \(c\) 2010 - $YEAR, Kim Walisch\./" COPYING

# Verify sed has worked correctly
[ "$(sed -n '1p' < README.txt)" = "primesieve $VERSION" ] || handle_error "failed updating README.txt"
[ "$(sed -n '2p' < README.txt)" = "$FULL_DATE" ] || handle_error "failed updating README.txt"
[ "$(sed -n '3p' < COPYING)" = "Copyright (c) 2010 - $YEAR, Kim Walisch." ] || handle_error "failed updating COPYING"

zip primesieve-$VERSION-win-x64.zip primesieve.exe README.txt COPYING
cp primesieve-$VERSION-win-x64.zip ..

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
