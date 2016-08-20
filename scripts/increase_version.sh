#!/bin/sh

if [ $# -ne 2 ]
then
    echo "Usage example: ./increase_version.sh 1.2.3 7:0:0"
    echo ""
    echo "Increases the primesieve version number to 1.2.3 in all source files"
    echo "and sets the libtool library version number to 7:0:0"

    exit 1
fi

# Run from primesieve root directory
test -e ../src && cd ..
test -f configure || ./autogen.sh

old_version=$(grep "PRIMESIEVE_VERSION " include/primesieve.hpp | cut -f2 -d'"')

old_major_version=$(echo $old_version | cut -f1 -d'.')
old_minor_version=$(echo $old_version | cut -f2 -d'.')
old_patch_version=$(echo $old_version | cut -f3 -d'.')

new_major_version=$(echo $1 | cut -f1 -d'.')
new_minor_version=$(echo $1 | cut -f2 -d'.')
new_patch_version=$(echo $1 | cut -f3 -d'.')

old_year=$(grep "Copyright (C)" src/apps/gui/src/PrimeSieveGUI_const.hpp | cut -f5 -d' ')
new_year=$(date +'%Y')

old_libtool_version=$(grep primesieve_lib_version configure.ac | cut -f2 -d' ' | cut -f2 -d'[' | cut -f1 -d']')

echo "Old version: $old_version"
echo "New version: $1"
echo ""
echo "Old libtool version: $old_libtool_version"
echo "New libtool version: $2"
echo ""
echo "Old year: $old_year"
echo "New year: $new_year"
echo ""

# Increase version number
for i in $(echo README.md configure.ac include/primesieve.hpp include/primesieve.h)
do
    echo "Increase version in $i"
    sed "s/$old_version/$1/g" $i > $i.tmp
    mv -f $i.tmp $i
done

# Increase version number
for i in $(echo include/primesieve.hpp include/primesieve.h)
do
    sed "s/PRIMESIEVE_VERSION_MAJOR $old_major_version/PRIMESIEVE_VERSION_MAJOR $new_major_version/g" $i > $i.tmp
    mv -f $i.tmp $i
    sed "s/PRIMESIEVE_VERSION_MINOR $old_minor_version/PRIMESIEVE_VERSION_MINOR $new_minor_version/g" $i > $i.tmp
    mv -f $i.tmp $i
    sed "s/PRIMESIEVE_VERSION_PATCH $old_patch_version/PRIMESIEVE_VERSION_PATCH $new_patch_version/g" $i > $i.tmp
    mv -f $i.tmp $i
done

# Update year
for i in $(echo src/apps/console/help.cpp src/apps/gui/src/PrimeSieveGUI_const.hpp)
do
    echo "Update year in $i"
    sed "s/$old_year/$new_year/g" $i > $i.tmp
    mv -f $i.tmp $i
done

echo "Update libtool version in configure.ac"
sed "s/$old_libtool_version/$2/g" configure.ac > configure.ac.tmp
mv -f configure.ac.tmp configure.ac
echo ""

# Update version number in man page
./configure --with-help2man
make man -j8
