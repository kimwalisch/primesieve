#!/bin/sh
# This shell script downloads, compiles and installs the
# latest primesieve and libprimesieve version.
# Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
# This file is distributed under the BSD License.
# Usage: $ sh install_primesieve.sh [GNU make arguments]

# Check prerequisites (SVN, GNU make, c++ compiler)
command -v svn > /dev/null 2> /dev/null
if [ $? -ne 0 ]; then
  echo "Error: Subversion is not installed!";
  exit 1;
fi
command -v make > /dev/null 2> /dev/null
if [ $? -ne 0 ]; then
  echo "Error: GNU make is not installed!";
  exit 1;
fi
command -v c++ > /dev/null 2> /dev/null
if [ $? -ne 0 ]; then
  echo "Error: There is no c++ compiler installed!";
  exit 1;
fi

# Download latest source code
svn checkout http://primesieve.googlecode.com/svn/trunk/ primesieve
if [ $? -ne 0 ]; then
  echo "Error: Failed to download primesieve!";
  exit 1;
fi

# Compile primesieve and libprimesieve
cd primesieve
make all "$@"
if [ $? -ne 0 ]; then
  echo "Error: Failed to build primesieve!";
  exit 1;
fi

# Install
command -v sudo > /dev/null 2> /dev/null
if [ $? -eq 0 ]; then
  echo 
  echo "Installing primesieve requires root privileges."
  sudo make install;
else
  make install;
fi
if [ $? -eq 0 ]; then
  echo "primesieve and libprimesieve successfully installed!";
else
  echo "Error: Failed to install primesieve!";
  exit 1;
fi
