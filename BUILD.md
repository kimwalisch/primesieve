# primesieve build instructions

## Prerequisites

You need to have installed a C++ compiler which supports C++11 (or later) and CMake ≥ 3.4.

```bash
# macOS
xcode-select --install
brew install cmake

# Debian, Ubuntu
sudo apt install g++ cmake

# Fedora, Red Hat
sudo dnf install gcc-c++ cmake

# Arch Linux
sudo pacman -S gcc cmake
```

## Unix-like OSes

Open a terminal, cd into the primesieve directory and run:

```bash
cmake .
make -j
sudo make install
```

## Microsoft Visual C++

First install [Visual Studio](https://visualstudio.microsoft.com/downloads/)
(includes CMake) on your Windows PC. Then go to the start menu, select Visual
Studio and open a **x64 Command Prompt**. Now cd into the primesieve directory
and run the commands below:

```bash
# Use 'cmake -G' to find your Visual Studio version
cmake -G "Visual Studio 15 2017 Win64" .
cmake --build . --config Release

# Optionally install using Admin shell
cmake --build . --config Release --target install
```

## MinGW/MSYS2 (Windows)

Open a terminal, cd into the primesieve directory and run:

```bash
cmake -G "Unix Makefiles" .
make -j
sudo make install
```

## MinGW cross compilation

Open a terminal, cd into the primesieve directory and run:

```bash
CC=x86_64-w64-mingw32-gcc-posix CXX=x86_64-w64-mingw32-g++-posix \
cmake -DCMAKE_SYSTEM_NAME=Windows .

make -j
```

## CMake configure options

By default the primesieve binary and the static/shared libprimesieve will be
built. The build options can be modified at the configure step using e.g.
```cmake . -DBUILD_TESTS=ON```.

```CMake
option(BUILD_PRIMESIEVE  "Build primesieve binary"    ON)
option(BUILD_SHARED_LIBS "Build shared libprimesieve" ON)
option(BUILD_STATIC_LIBS "Build static libprimesieve" ON)
option(BUILD_DOC         "Build documentation"        OFF)
option(BUILD_EXAMPLES    "Build example programs"     OFF)
option(BUILD_TESTS       "Build test programs"        OFF)
```

## Run the tests

Open a terminal, cd into the primesieve directory and run:

```bash
cmake -DBUILD_TESTS=ON .
make -j
ctest
```

## C/C++ examples

Open a terminal, cd into the primesieve directory and run:

```bash
cmake -DBUILD_EXAMPLES=ON .
make -j
```

## API documentation

To build the primesieve C/C++ API documentation in html/PDF format
you need to have installed the ```doxygen```, ```doxygen-latex``` and
```graphviz (dot)``` packages.

```bash
cmake -DBUILD_DOC=ON .
make doc
```
