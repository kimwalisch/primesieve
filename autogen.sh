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

# This code patches configure.ac (replaces AC_CONFIG_MACRO_DIRS
# by AC_CONFIG_MACRO_DIR) if autoconf version < 2.69
command -v autoconf >/dev/null 2>/dev/null
if [ $? -eq 0 ]
then
    autoconf_version=$(autoconf --version 2>/dev/null | head -n1 2>/dev/null | sed 's/[^0-9.]*\([0-9.]*\).*/\1/' 2>/dev/null)
    if [ "x$autoconf_version" != "x" ];
    then
        is_patch=$(echo $autoconf_version 2.69 | awk '{print ($1 < $2)}' 2>/dev/null)
        if [ "$is_patch" = "1" ];
        then
            sed 's/AC_CONFIG_MACRO_DIRS/AC_CONFIG_MACRO_DIR/' < configure.ac > configure2.ac
            rm -f configure.ac
            mv configure2.ac configure.ac
        fi
    fi
fi
