AC_INIT([primesieve], [5.2], [kim.walisch@gmail.com])
AC_SUBST([primesieve_lib_version], [3:0:0])

AC_PREREQ([2.62])
AC_LANG([C++])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([subdir-objects foreign -Wall -Werror])
AC_PROG_CC
AC_PROG_CXX
AC_OPENMP
AC_CANONICAL_HOST
AM_PROG_AR
# needed for automake <= 1.13:
AM_PROG_CC_C_O
LT_INIT

AC_ARG_ENABLE(examples,
    [--enable-examples Compile the examples programs],
    ENABLE_EXAMPLES="yes")
AM_CONDITIONAL(MAKE_EXAMPLES, test "x$ENABLE_EXAMPLES" = "xyes")

# Force building shared library on Windows
# Reference: <http://article.gmane.org/gmane.comp.gnu.mingw.user/18727>
case "$host_os" in
    cygwin*) AM_CONDITIONAL(FORCE_SHARED_LIBRARY, true);;
    mingw*)  AM_CONDITIONAL(FORCE_SHARED_LIBRARY, true);;
    *)       AM_CONDITIONAL(FORCE_SHARED_LIBRARY, false);;
esac

AC_ARG_VAR([L1_DCACHE_SIZE],
    [CPU L1 data cache size in kilobytes])

AS_IF([test "x$L1_DCACHE_SIZE" != "x"], [L1_DETECTED="yes"])
AS_IF([test "x$L1_DCACHE_SIZE" = "x"],
    [[L1_DCACHE_SIZE=32
    command -v getconf >/dev/null 2>/dev/null
    if [ $? -eq 0 ]; then
        L1_DCACHE_BYTES=$(getconf LEVEL1_DCACHE_SIZE 2>/dev/null)
    fi
    if test "x$L1_DCACHE_BYTES" = "x"; then
        command -v sysctl >/dev/null 2>/dev/null
        if [ $? -eq 0 ]; then
            L1_DCACHE_BYTES=$(sysctl hw.l1dcachesize 2>/dev/null | sed -e 's/^.* //')
        fi
    fi
    if test "x$L1_DCACHE_BYTES" != "x"; then
        L1_DETECTED="yes"
        L1_DCACHE_SIZE=$(expr $L1_DCACHE_BYTES '/' 1024)
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
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

AS_IF([test "x$L1_DETECTED" != "x"],
    [AC_MSG_NOTICE([NOTICE: using L1_DCACHE_SIZE=$L1_DCACHE_SIZE (kilobytes)])])

AS_IF([test "x$L1_DETECTED" = "x"],
    [AC_MSG_WARN([CPU L1 data cache size not detected, using L1_DCACHE_SIZE=$L1_DCACHE_SIZE (kilobytes)])])

AC_MSG_NOTICE([NOTICE: using SIEVESIZE=$SIEVESIZE (kilobytes)])
