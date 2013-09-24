AC_INIT([primesieve], [4.4], [kim.walisch@gmail.com])
PACKAGE_YEAR=2013

AC_PREREQ([2.62])

AC_LANG([C++])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AC_PROG_CXX
AM_PROG_AR 

LT_INIT

PACKAGE_VERSION_MAJOR=patsubst(AC_PACKAGE_VERSION, [\..*$], [])
PACKAGE_VERSION_MINOR=patsubst(AC_PACKAGE_VERSION, [^.*\.], [])
AC_SUBST(PACKAGE_VERSION_MAJOR)
AC_SUBST(PACKAGE_VERSION_MINOR)
AC_SUBST(PACKAGE_YEAR)

######################################################################
# Find the CPU's L1 data cache size in kilobytes

[

command -v getconf >/dev/null 2>/dev/null
if [ $? -eq 0 ]
then
    L1_DCACHE_BYTES=`getconf LEVEL1_DCACHE_SIZE 2>/dev/null`
fi

if [ "$L1_DCACHE_BYTES" == "" ]
then
    command -v sysctl >/dev/null 2>/dev/null
    if [ $? -eq 0 ]
    then
        L1_DCACHE_BYTES=`sysctl hw.l1dcachesize 2>/dev/null | sed -e 's/^.* //'`
    fi
fi

if [ "$L1_DCACHE_BYTES" != "" ]
then
    L1_DCACHE_SIZE=`expr $L1_DCACHE_BYTES '/' 1024`
fi

]

######################################################################

AC_ARG_VAR([L1_DCACHE_SIZE], [CPU L1 data cache size in kilobytes])
AS_IF([test "x$L1_DCACHE_SIZE" = x], [L1_DCACHE_SIZE=32])
AC_DEFINE_UNQUOTED([L1_DCACHE_SIZE], [$L1_DCACHE_SIZE], [CPU L1 data cache size in kilobytes])

AC_OPENMP

AC_CONFIG_HEADERS([src/soe/autoconfig.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/apps/Makefile
	src/apps/console/Makefile
	src/soe/Makefile

	src/soe/PrimeSieve.h
	])

AC_OUTPUT
