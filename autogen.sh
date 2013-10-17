#!/bin/sh
mkdir -p m4 || exit $?
autoreconf --install || exit $?
# This code patches ltmain.sh if GNU Libtool version < 2.4.2
# so that it correctly links in OpenMP if required
command -v libtool >/dev/null 2>/dev/null
if [ $? -eq 0 ]
then
    libtool_version=$(libtool --version 2>/dev/null | head -n1 | cut -d' ' -f4)
    libtool1=$(echo $libtool_version | cut -d'.' -f1)
    libtool2=$(echo $libtool_version | cut -d'.' -f2)
    libtool3=$(echo $libtool_version | cut -d'.' -f3)
    if [ "$libtool1" != "" ]; then libtool_version=$(expr $libtool1 '*' 1000000 2>/dev/null); fi
    if [ "$libtool2" != "" ]; then libtool_version=$(expr $libtool_version '+' $libtool2 '*' 1000 2>/dev/null); fi
    if [ "$libtool3" != "" ]; then libtool_version=$(expr $libtool_version '+' $libtool3 2>/dev/null); fi
    if [ "$libtool_version" != "" ] && [ $libtool_version -lt 2004002 ]
    then
        sed 's/|-threads)/-threads|-fopenmp|-openmp|-mp|-xopenmp|-omp|-qsmp=*)/g' ltmain.sh > ltmain2.sh
        mv -f ltmain2.sh ltmain.sh
        find . -exec touch {} \;
    fi
fi
