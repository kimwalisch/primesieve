#!/bin/sh

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
            sed 's/AC_CONFIG_MACRO_DIRS/AC_CONFIG_MACRO_DIR/g' configure.ac > configure2.ac
            mv -f configure2.ac configure.ac
        fi
    fi
fi

mkdir -p m4 || exit $?
autoreconf --install || exit $?

# This code patches ltmain.sh if GNU Libtool version < 2.4.2
# so that it correctly links in OpenMP if required
command -v libtool >/dev/null 2>/dev/null
if [ $? -eq 0 ]
then
    libtool_version=$(libtool --version 2>/dev/null | head -n1 2>/dev/null | sed 's/[^0-9.]*\([0-9.]*\).*/\1/' 2>/dev/null)
    if [ "x$libtool_version" != "x" ];
    then
        is_patch=$(echo $libtool_version 2.4.2 | awk '{print ($1 < $2)}' 2>/dev/null)
        if [ "$is_patch" = "1" ];
        then
            sed 's/|-threads)/-threads|-fopenmp|-openmp|-mp|-xopenmp|-omp)/g' ltmain.sh > ltmain2.sh
            mv -f ltmain2.sh ltmain.sh
        fi
    fi
fi
