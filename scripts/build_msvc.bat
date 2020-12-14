@ECHO OFF

:: === Usage ===
:: This script builds a primesieve binary that is suitable for
:: distribution on the internet. The built primesieve binary
:: will only depend on kernel32.dll.
::
:: 1) Open a Visual Studio Command Prompt
:: 2) cd into primesieve repo
:: 3) Run: scripts/build_msvc.bat

rmdir /s /q build-msvc
mkdir build-msvc
cd build-msvc

cl /O2 /EHsc /W3 /D NDEBUG /I ../include ../src/*.cpp ../src/app/*.cpp /Feprimesieve.exe

echo ""
primesieve.exe --version

echo ""
primesieve.exe --test
