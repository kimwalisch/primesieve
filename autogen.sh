#!/bin/sh

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
