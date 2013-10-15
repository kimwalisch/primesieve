ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_LDFLAGS = $(OPENMP_CXXFLAGS)

lib_LTLIBRARIES = libprimesieve.la
libprimesieve_la_CXXFLAGS = $(OPENMP_CXXFLAGS)
libprimesieve_la_LDFLAGS = -no-undefined -version-info @primesieve_lib_version@

bin_PROGRAMS = primesieve
primesieve_LDADD = libprimesieve.la

# If installation directory is /usr/local/lib
# execute ldconfig after installation
install-exec-hook:
	test "$(prefix)" = "/usr/local" && command -v ldconfig 2>/dev/null && ldconfig /usr/local/lib 2>/dev/null || true

check: all
	./primesieve --test

if MAKE_EXAMPLES

noinst_PROGRAMS = callback_cancel
callback_cancel_LDADD = libprimesieve.la
callback_cancel_SOURCES = examples/callback_cancel.cpp

noinst_PROGRAMS += callback_primes
callback_primes_LDADD = libprimesieve.la
callback_primes_SOURCES = examples/callback_primes.cpp

noinst_PROGRAMS += callback_primes_oop
callback_primes_oop_LDADD = libprimesieve.la
callback_primes_oop_SOURCES = examples/callback_primes_oop.cpp

noinst_PROGRAMS += count_primes
count_primes_LDADD = libprimesieve.la
count_primes_SOURCES = examples/count_primes.cpp

noinst_PROGRAMS += count_primes_c
count_primes_c_LDADD = libprimesieve.la
count_primes_c_SOURCES = examples/count_primes-c.c

noinst_PROGRAMS += next_prime
next_prime_LDADD = libprimesieve.la
next_prime_SOURCES = examples/next_prime.cpp

noinst_PROGRAMS += next_prime_c
next_prime_c_LDADD = libprimesieve.la
next_prime_c_SOURCES = examples/next_prime-c.c

noinst_PROGRAMS += nth_prime
nth_prime_LDADD = libprimesieve.la
nth_prime_SOURCES = examples/nth_prime.cpp

noinst_PROGRAMS += nth_prime_c
nth_prime_c_LDADD = libprimesieve.la
nth_prime_c_SOURCES = examples/nth_prime-c.c

noinst_PROGRAMS += previous_prime
previous_prime_LDADD = libprimesieve.la
previous_prime_SOURCES = examples/previous_prime.cpp

noinst_PROGRAMS += store_primes_in_vector
store_primes_in_vector_LDADD = libprimesieve.la
store_primes_in_vector_SOURCES = examples/store_primes_in_vector.cpp

endif

include_HEADERS = \
	include/primesieve.hpp \
	include/primesieve-c.h

otherincludedir = $(includedir)/primesieve/soe
otherinclude_HEADERS = \
	include/primesieve/soe/ParallelPrimeSieve.hpp \
	include/primesieve/soe/primesieve_error.hpp \
	include/primesieve/soe/PrimeSieve.hpp \
	include/primesieve/soe/iterator.hpp \
	include/primesieve/soe/primesieve_iterator-c.h \
	include/primesieve/soe/PrimeSieveCallback.hpp \
	include/primesieve/soe/PushBackPrimes.hpp \
	include/primesieve/soe/cancel_callback.hpp

EXTRA_DIST = \
	Doxyfile \
	mainpage.dox \
	README.md \
	Makefile.msvc \
	examples/README \
	src/apps/gui \
	src/msvc_compat \
	src/soe/README

libprimesieve_la_SOURCES = \
	src/soe/EratBig.cpp \
	src/soe/EratMedium.cpp \
	src/soe/EratSmall.cpp \
	src/soe/ParallelPrimeSieve.cpp \
	src/soe/popcount.cpp \
	src/soe/PreSieve.cpp \
	src/soe/PrimeFinder.cpp \
	src/soe/PrimeGenerator.cpp \
	src/soe/iterator.cpp \
	src/soe/primesieve_iterator-c.cpp \
	src/soe/PrimeSieve-nthPrime.cpp \
	src/soe/PrimeSieve.cpp \
	src/soe/primesieve-api.cpp \
	src/soe/primesieve-api-c.cpp \
	src/soe/SieveOfEratosthenes.cpp \
	src/soe/WheelFactorization.cpp \
	src/soe/test.cpp \
	include/primesieve/soe/bits.hpp \
	include/primesieve/soe/config.hpp \
	include/primesieve/soe/EratBig.hpp \
	include/primesieve/soe/EratMedium.hpp \
	include/primesieve/soe/EratSmall.hpp \
	include/primesieve/soe/imath.hpp \
	include/primesieve/soe/littleendian_cast.hpp \
	include/primesieve/soe/ParallelPrimeSieve-lock.hpp \
	include/primesieve/soe/PreSieve.hpp \
	include/primesieve/soe/PrimeFinder.hpp \
	include/primesieve/soe/PrimeGenerator.hpp \
	include/primesieve/soe/PrimeSieve-lock.hpp \
	include/primesieve/soe/SieveOfEratosthenes.hpp \
	include/primesieve/soe/SieveOfEratosthenes-CALLBACK.hpp \
	include/primesieve/soe/SieveOfEratosthenes-inline.hpp \
	include/primesieve/soe/toString.hpp \
	include/primesieve/soe/WheelFactorization.hpp

primesieve_SOURCES = \
	src/apps/console/main.cpp \
	src/apps/console/help.cpp \
	src/apps/console/cmdoptions.cpp \
	src/apps/console/cmdoptions.hpp \
	src/apps/console/ExpressionParser.hpp
