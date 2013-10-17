#!/bin/sh
mkdir -p m4 || exit $?
autoreconf --install || exit $?
# This code patches ltmain.sh if GNU Libtool version < 2.4.2
# so that it correctly links in OpenMP if required
command -v libtool >/dev/null 2>/dev/null
if [ $? -eq 0 ]
then
    libtool_version=$(libtool --version 2>/dev/null | head -n1 | cut -d' ' -f4)
    libtool_f1=$(echo "$libtool_version" | cut -d'.' -f1)
    libtool_f2=$(echo "$libtool_version" | cut -d'.' -f2)
    libtool_f3=$(echo "$libtool_version" | cut -d'.' -f3)
    if [ "$libtool_f1" != "" ]; then libtool_nr=$(expr "$libtool_f1" '*' 1000000 2>/dev/null); fi
    if [ "$libtool_f2" != "" ]; then libtool_nr=$(expr "$libtool_nr" '+' "$libtool_f2" '*' 1000 2>/dev/null); fi
    if [ "$libtool_f3" != "" ]; then libtool_nr=$(expr "$libtool_nr" '+' "$libtool_f3" 2>/dev/null); fi
    if [ "$libtool_nr" -lt 2004002 2>/dev/null ]
    then
        sed 's/|-threads)/-threads|-fopenmp|-openmp|-mp|-xopenmp|-omp)/g' ltmain.sh > ltmain2.sh
        mv -f ltmain2.sh ltmain.sh
    fi
fi
