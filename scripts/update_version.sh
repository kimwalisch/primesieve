#!/bin/sh

if [ $# -ne 2 ]
then
    echo "Usage example:"
    echo "$ ./update_version.sh 1.2.3 7:0:0"
    echo ""
    echo "Updates the primesieve version to 1.2.3 in all files and"
    echo "updates the libtool library version to 7:0:0"

    exit 1
fi

# Run from primesieve root directory
test -e ../src && cd ..
test -f configure || ./autogen.sh

new_version=$1
old_version=$(grep "PRIMESIEVE_VERSION " include/primesieve.hpp | cut -f2 -d'"')

new_major=$(echo $new_version | cut -f1 -d'.')
new_minor=$(echo $new_version | cut -f2 -d'.')
new_patch=$(echo $new_version | cut -f3 -d'.')

old_major=$(echo $old_version | cut -f1 -d'.')
old_minor=$(echo $old_version | cut -f2 -d'.')
old_patch=$(echo $old_version | cut -f3 -d'.')

new_libtool_version=$2
old_libtool_version=$(grep primesieve_lib_version configure.ac | \
                      cut -f2 -d' ' | \
                      cut -f2 -d'[' | \
                      cut -f1 -d']')

new_year=$(date +'%Y')
old_year=$(grep "Copyright (C)" src/apps/gui/src/PrimeSieveGUI_const.hpp | cut -f5 -d' ')

echo "New version: $new_version"
echo "Old version: $old_version"
echo ""
echo "New libtool version: $new_libtool_version"
echo "Old libtool version: $old_libtool_version"
echo ""
echo "New year: $new_year"
echo "Old year: $old_year"
echo ""

# Update version
for i in $(echo README.md \
                configure.ac \
                include/primesieve.hpp \
                include/primesieve.h)
do
    echo "Update version in $i"
    sed "s/$old_major\.$old_minor\.$old_patch/$new_version/g" $i > $i.tmp
    mv -f $i.tmp $i
done

# Update version
for i in $(echo include/primesieve.hpp \
                include/primesieve.h)
do
    sed "s/PRIMESIEVE_VERSION_MAJOR $old_major/PRIMESIEVE_VERSION_MAJOR $new_major/g" $i > $i.tmp
    mv -f $i.tmp $i
    sed "s/PRIMESIEVE_VERSION_MINOR $old_minor/PRIMESIEVE_VERSION_MINOR $new_minor/g" $i > $i.tmp
    mv -f $i.tmp $i
    sed "s/PRIMESIEVE_VERSION_PATCH $old_patch/PRIMESIEVE_VERSION_PATCH $new_patch/g" $i > $i.tmp
    mv -f $i.tmp $i
done

# Update libtool version
echo ""
echo "Update libtool version in configure.ac $new_libtool_version"
sed "s/$old_libtool_version/$new_libtool_version/g" configure.ac > configure.ac.tmp
mv -f configure.ac.tmp configure.ac
echo ""

# Update year
for i in $(echo COPYING \
                src/apps/console/help.cpp \
                src/apps/gui/src/PrimeSieveGUI_const.hpp)
do
    echo "Update year in $i"
    sed "s/$old_year/$new_year/g" $i > $i.tmp
    mv -f $i.tmp $i
done

# Update version number in man page
echo ""
./configure --with-help2man
make man -j8
echo ""

echo "Version has been updated!"
