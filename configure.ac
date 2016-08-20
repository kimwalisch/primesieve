AC_INIT([primesieve], [5.7.2], [kim.walisch@gmail.com])
AC_SUBST([primesieve_lib_version], [7:2:0])

AC_PREREQ([2.62])
AC_LANG([C++])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([subdir-objects foreign -Wall -Werror])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_PROG_CC
AC_PROG_CXX
AC_OPENMP
AC_CANONICAL_HOST
# Silence warning: ar: 'u' modifier ignored since 'D' is the default
AC_SUBST(AR_FLAGS, [cr])
AM_PROG_AR
# Needed for automake <= 1.13
AM_PROG_CC_C_O
LT_INIT

# doxygen is used to generate C/C++ API documentation
AC_CHECK_PROG([DOXYGEN], [doxygen], [doxygen])

AM_CONDITIONAL([HAVE_DOXYGEN], [test x$DOXYGEN = xdoxygen])

# Check if graphviz is installed
AC_CHECK_PROG([HAVE_DOT], [dot], [YES], [NO])

AC_ARG_WITH([help2man],
    [AS_HELP_STRING([--with-help2man], [Use help2man to generate man pages])],
    WITH_HELP2MAN="yes")

AS_IF([test "x$WITH_HELP2MAN" = "xyes"], [
    AC_CHECK_PROG([HELP2MAN], [help2man], [help2man])
    test -z "$HELP2MAN" && AC_MSG_ERROR([help2man not found])
])

AM_CONDITIONAL([HAVE_HELP2MAN], [test x$HELP2MAN = xhelp2man])

AC_ARG_ENABLE(examples,
    [AS_HELP_STRING([--enable-examples], [Compile the examples programs])],
    ENABLE_EXAMPLES="yes")

AM_CONDITIONAL(MAKE_EXAMPLES, test "x$ENABLE_EXAMPLES" = "xyes")

# Force building shared library on Windows
# Reference: <http://article.gmane.org/gmane.comp.gnu.mingw.user/18727>
case "$host_os" in
    cygwin*) AM_CONDITIONAL(FORCE_SHARED_LIBRARY, true);;
    mingw*)  AM_CONDITIONAL(FORCE_SHARED_LIBRARY, true);;
    *)       AM_CONDITIONAL(FORCE_SHARED_LIBRARY, false);;
esac

AC_ARG_VAR([L1_DCACHE_SIZE], [CPU L1 data cache size in kilobytes])

AS_IF([test "x$L1_DCACHE_SIZE" != "x"],
    [L1_DETECTED="yes"])

# This is a very portable shell script that finds the CPU's L1
# cache size (in kilobytes) on most Unix-like systems
AS_IF([test "x$L1_DCACHE_SIZE" = "x"],
    [[
    # Default L1 cache size
    L1_DCACHE_SIZE=32

    command -v getconf >/dev/null 2>/dev/null
    if [ $? -eq 0 ];
    then
        # Returns L1 cache size in bytes
        L1_DCACHE_BYTES=$(getconf LEVEL1_DCACHE_SIZE 2>/dev/null)
    fi

    if test "x$L1_DCACHE_BYTES" = "x" || test "$L1_DCACHE_BYTES" = "0";
    then
        # Returns L1 cache size like e.g. 32K, 1M
        L1_DCACHE_BYTES=$(cat /sys/devices/system/cpu/cpu0/cache/index0/size 2>/dev/null)

        if test "x$L1_DCACHE_BYTES" != "x";
        then
            is_kilobytes=$(echo $L1_DCACHE_BYTES | grep K)
            if test "x$is_kilobytes" != "x";
            then
                L1_DCACHE_BYTES=$(expr $(echo $L1_DCACHE_BYTES | sed -e s'/K$//') '*' 1024)
            fi
            is_megabytes=$(echo $L1_DCACHE_BYTES | grep M)
            if test "x$is_megabytes" != "x";
            then
                L1_DCACHE_BYTES=$(expr $(echo $L1_DCACHE_BYTES | sed -e s'/M$//') '*' 1024 '*' 1024)
            fi
        else
            command -v sysctl >/dev/null 2>/dev/null
            if [ $? -eq 0 ];
            then
                # Returns L1 cache size in bytes
                L1_DCACHE_BYTES=$(sysctl hw.l1dcachesize 2>/dev/null | sed -e 's/^.* //')
            fi
        fi
    fi

    if test "x$L1_DCACHE_BYTES" != "x";
    then
        if [ $L1_DCACHE_BYTES -ge 2048 2>/dev/null ];
        then
            # Convert L1 cache size to kilobytes
            L1_DCACHE_SIZE=$(expr $L1_DCACHE_BYTES '/' 1024)
            L1_DETECTED="yes"
        fi
    fi]])

AC_DEFINE_UNQUOTED([L1_DCACHE_SIZE],
    [$L1_DCACHE_SIZE],
    [CPU L1 data cache size in kilobytes])

AC_ARG_VAR([SIEVESIZE], [Default sieve size in kilobytes])

AS_IF([test "x$SIEVESIZE" = "x"], [SIEVESIZE=$L1_DCACHE_SIZE])

AC_DEFINE_UNQUOTED([SIEVESIZE],
    [$SIEVESIZE],
    [Default sieve size in kilobytes])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
    Makefile
    primesieve.pc
    doc/Doxyfile])
AC_OUTPUT

AS_IF([test "x$L1_DETECTED" != "x"],
    [AC_MSG_NOTICE([NOTICE: using L1_DCACHE_SIZE=$L1_DCACHE_SIZE (kilobytes)])])

AS_IF([test "x$L1_DETECTED" = "x"],
    [AC_MSG_WARN([CPU L1 data cache size not detected, using L1_DCACHE_SIZE=$L1_DCACHE_SIZE (kilobytes)])])

AC_MSG_NOTICE([NOTICE: using SIEVESIZE=$SIEVESIZE (kilobytes)])
