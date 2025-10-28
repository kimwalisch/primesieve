# primesieve build instructions

# Contents

* [Prerequisites](#prerequisites)
* [Unix-like OSes](#unix-like-oses)
* [MinGW/MSYS2 (Windows)](#mingwmsys2-windows)
* [Microsoft Visual C++](#microsoft-visual-c)
* [Emscripten/WebAssembly](#emscriptenwebassembly)
* [CMake configure options](#cmake-configure-options)
* [Run the tests](#run-the-tests)
* [API documentation](#api-documentation)
* [Man page regeneration](#man-page-regeneration)

# Prerequisites

You need to have installed a C++ compiler which supports C++11 (or later) and CMake ≥ 3.4.

<table>
    <tr>
        <td><b>macOS</b></td>
        <td><code>xcode-select --install && brew install cmake</code></td>
    </tr>
    <tr>
        <td><b>Debian/Ubuntu:</b></td>
        <td><code>sudo apt install g++ cmake</code></td>
    </tr>
    <tr>
        <td><b>Fedora:</b></td>
        <td><code>sudo dnf install gcc-c++ cmake</code></td>
    </tr>
    <tr>
        <td><b>openSUSE:</b></td>
        <td><code>sudo zypper install gcc-c++ cmake</code></td>
    </tr>
    <tr>
        <td><b>Arch Linux:</b></td>
        <td><code>sudo pacman -S gcc cmake</code></td>
    </tr>
</table>

# Unix-like OSes

Open a terminal, cd into the primesieve directory and run:

```bash
cmake .
cmake --build . --parallel
sudo cmake --install .
sudo ldconfig
```

# MinGW/MSYS2 (Windows)

Open a terminal, cd into the primesieve directory and run:

```bash
cmake -G "Unix Makefiles" .
cmake --build . --parallel
```

# Microsoft Visual C++

First install [Visual Studio](https://visualstudio.microsoft.com/downloads/)
(includes CMake) on your Windows PC. Then go to the start menu, select Visual
Studio and open a **x64 Command Prompt**. Now cd into the primesieve directory
and run the commands below:

```bash
# Use 'cmake -G' to find your Visual Studio version
cmake -G "Visual Studio 17 2022" .
cmake --build . --config Release

# Optionally install using Admin shell
cmake --install . --config Release
```

# Emscripten/WebAssembly

Using the Emscripten compiler you can compile the primesieve C/C++ library to WebAssembly:

```bash
# Install the Emscripten compiler
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source emsdk_env.sh

# Compile primesieve to WebAssembly
git clone https://github.com/kimwalisch/primesieve.git
cd primesieve
emcmake cmake .
emmake make -j4

# Run the primesieve WebAssembly binary
node ./primesieve.js 1e10
```

# CMake configure options

By default the primesieve binary and the static/shared libprimesieve will be
built. The build options can be modified at the configure step using e.g.
```cmake . -DBUILD_TESTS=ON```.

```CMake
option(BUILD_PRIMESIEVE  "Build primesieve binary"       ON)
option(BUILD_SHARED_LIBS "Build shared libprimesieve"    ON)
option(BUILD_STATIC_LIBS "Build static libprimesieve"    ON)
option(BUILD_DOC         "Build C/C++ API documentation" OFF)
option(BUILD_MANPAGE     "Regenerate man page using a2x" OFF)
option(BUILD_EXAMPLES    "Build example programs"        OFF)
option(BUILD_TESTS       "Build test programs"           OFF)

option(WITH_MULTIARCH       "Enable runtime dispatching to fastest supported CPU instruction set" ON)
option(WITH_MSVC_CRT_STATIC "Link primesieve.lib with /MT instead of the default /MD" OFF)
```

# Run the tests

Open a terminal, cd into the primesieve directory and run:

```bash
cmake -DBUILD_TESTS=ON .
cmake --build . --parallel
ctest
```

For developers hacking on primesieve's source code the
[test/README.md](../test/README.md) document contains more information
about primesieve testing such as testing in debug mode and testing
using GCC/Clang sanitizers.

# API documentation

To build the primesieve C/C++ API documentation in html/PDF format
you need to have installed the ```doxygen```, ```doxygen-latex``` and
```graphviz (dot)``` packages.

```bash
cmake -DBUILD_DOC=ON .
cmake --build . --target doc
```

# Man page regeneration

primesieve includes an up to date man page at ```doc/primesieve.1```.
That man page has been generated from ```doc/primesieve.txt``` using
the ```a2x``` program from the ```asciidoc``` package. However when
packaging primesieve for e.g. a Linux distro it is recommended to
regenerate the man page.

```bash
# Build man page using a2x program (asciidoc package)
cmake -DBUILD_MANPAGE=ON .
cmake --build . --parallel
```
