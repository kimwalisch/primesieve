AC_INIT([primesieve], [5.0], [kim.walisch@gmail.com])
AC_SUBST([primesieve_lib_version], [1:0:0])
PACKAGE_YEAR=2013

AC_PREREQ([2.62])
AC_LANG([C++])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([subdir-objects foreign -Wall -Werror])
AC_PROG_CC
AC_PROG_CXX
AM_PROG_AR
# needed for automake <= 1.13:
AM_PROG_CC_C_O
LT_INIT

AC_ARG_VAR([L1_DCACHE_SIZE], [CPU L1 data cache size in kilobytes])

AS_IF([test "x$L1_DCACHE_SIZE" = x], [[
    L1_DCACHE_SIZE=32
    command -v getconf >/dev/null 2>/dev/null
    if [ $? -eq 0 ]; then
        L1_DCACHE_BYTES=$(getconf LEVEL1_DCACHE_SIZE 2>/dev/null)
    fi
    if [ "$L1_DCACHE_BYTES" == "" ]; then
        command -v sysctl >/dev/null 2>/dev/null
        if [ $? -eq 0 ]; then
            L1_DCACHE_BYTES=$(sysctl hw.l1dcachesize 2>/dev/null | sed -e 's/^.* //')
        fi
    fi
    if [ "$L1_DCACHE_BYTES" != "" ]; then
        L1_DCACHE_SIZE=$(expr $L1_DCACHE_BYTES '/' 1024)
    fi]])

AC_DEFINE_UNQUOTED([L1_DCACHE_SIZE],
    [$L1_DCACHE_SIZE],
    [CPU L1 data cache size in kilobytes])

AC_ARG_ENABLE(examples,
    [--enable-examples Compile the examples programs],
    ENABLE_EXAMPLES="yes")
AM_CONDITIONAL(MAKE_EXAMPLES, test "$ENABLE_EXAMPLES" = "yes")

AC_CANONICAL_HOST
AM_CONDITIONAL([SOLARIS], [AS_CASE([$host_os], [solaris*], [true], [false])])
AC_OPENMP
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
