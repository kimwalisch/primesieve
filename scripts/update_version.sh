#!/bin/sh

if [ $# -ne 1 ]
then
    echo "Usage example:"
    echo "$ ./update_version.sh 1.2"
    echo ""
    echo "Updates the primesieve version to 1.2 in all files"

    exit 1
fi

# Run from primesieve root directory
test -e ../src && cd ..

new_version=$1
old_version=$(grep "PRIMESIEVE_VERSION " include/primesieve.hpp | cut -f2 -d'"')

new_major=$(echo $new_version | cut -f1 -d'.')
new_minor=$(echo $new_version | cut -f2 -d'.')

old_major=$(echo $old_version | cut -f1 -d'.')
old_minor=$(echo $old_version | cut -f2 -d'.')

new_year=$(date +'%Y')
old_year=$(grep "Copyright (C)" include/primesieve.hpp | cut -f4 -d' ')

echo "New version: $new_version"
echo "Old version: $old_version"
echo ""
echo "New year: $new_year"
echo "Old year: $old_year"
echo ""

# 1. Update VERSION in CMakeLists.txt
echo "Update version in CMakeLists.txt"
sed "s/VERSION $old_version/VERSION $new_version/" CMakeLists.txt > CMakeLists.txt.tmp
mv -f CMakeLists.txt.tmp CMakeLists.txt

# 2. Update version string in headers
for i in $(echo include/primesieve.hpp \
                include/primesieve.h)
do
    echo "Update version string in $i"
    sed "s/$old_major\.$old_minor/$new_version/g" $i > $i.tmp
    mv -f $i.tmp $i
done

# 3. Update version macros in headers (MAJOR/MINOR)
for i in $(echo include/primesieve.hpp \
                include/primesieve.h)
do
    echo "Update version macros in $i"
    sed "s/PRIMESIEVE_VERSION_MAJOR $old_major/PRIMESIEVE_VERSION_MAJOR $new_major/g" $i > $i.tmp
    mv -f $i.tmp $i
    sed "s/PRIMESIEVE_VERSION_MINOR $old_minor/PRIMESIEVE_VERSION_MINOR $new_minor/g" $i > $i.tmp
    mv -f $i.tmp $i
done

# 4. Update copyright year
for i in $(echo COPYING \
                src/app/help.cpp)
do
    echo "Update year in $i"
    sed "s/$old_year/$new_year/g" $i > $i.tmp
    mv -f $i.tmp $i
done

# Update version number in man page
echo ""
cmake .
cmake --build . --parallel
echo ""

echo "Version has been updated!"
