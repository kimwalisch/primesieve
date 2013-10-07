ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -I$(top_srcdir)/include
EXTRA_DIST = \
	Doxyfile \
	mainpage.dox \
	README.md \
	Makefile.msvc \
	examples/README \
	src/apps/gui \
	src/msvc_compat \
	src/soe/README

lib_LTLIBRARIES = libprimesieve.la
libprimesieve_la_CXXFLAGS = $(OPENMP_CXXFLAGS)

bin_PROGRAMS = primesieve
primesieve_LDADD = libprimesieve.la
primesieve_LDFLAGS = $(OPENMP_CXXFLAGS)

# If installation directory is /usr/local/lib
# execute ldconfig after installation
install-exec-hook:
	test "$(prefix)" = "/usr/local" && command -v ldconfig 2>/dev/null && ldconfig /usr/local/lib 2>/dev/null || true

check: all
	./primesieve --test

if MAKE_EXAMPLES

noinst_PROGRAMS += callback_cancel
callback_cancel_LDADD = libprimesieve.la
callback_cancel_LDFLAGS = $(OPENMP_CXXFLAGS)
callback_cancel_SOURCES = examples/callback_cancel.cpp

noinst_PROGRAMS += callback_primes
callback_primes_LDADD = libprimesieve.la
callback_primes_LDFLAGS = $(OPENMP_CXXFLAGS)
callback_primes_SOURCES = examples/callback_primes.cpp

noinst_PROGRAMS += callback_primes_oop
callback_primes_oop_LDADD = libprimesieve.la
callback_primes_oop_LDFLAGS = $(OPENMP_CXXFLAGS)
callback_primes_oop_SOURCES = examples/callback_primes.cpp

noinst_PROGRAMS += count_primes
count_primes_LDADD = libprimesieve.la
count_primes_LDFLAGS = $(OPENMP_CXXFLAGS)
count_primes_SOURCES = examples/count_primes.cpp

noinst_PROGRAMS += next_prime
next_prime_LDADD = libprimesieve.la
next_prime_LDFLAGS = $(OPENMP_CXXFLAGS)
next_prime_SOURCES = examples/next_prime.cpp

noinst_PROGRAMS += nth_prime
nth_prime_LDADD = libprimesieve.la
nth_prime_LDFLAGS = $(OPENMP_CXXFLAGS)
nth_prime_SOURCES = examples/nth_prime.cpp

noinst_PROGRAMS += previous_prime
previous_prime_LDADD = libprimesieve.la
previous_prime_LDFLAGS = $(OPENMP_CXXFLAGS)
previous_prime_SOURCES = examples/previous_prime.cpp

noinst_PROGRAMS += store_primes_in_vector
store_primes_in_vector_LDADD = libprimesieve.la
store_primes_in_vector_LDFLAGS = $(OPENMP_CXXFLAGS)
store_primes_in_vector_SOURCES = examples/store_primes_in_vector.cpp

endif

include_HEADERS = \
	include/primesieve.h

otherincludedir = $(includedir)/primesieve/soe
otherinclude_HEADERS = \
	include/primesieve/soe/ParallelPrimeSieve.h \
	include/primesieve/soe/primesieve_error.h \
	include/primesieve/soe/PrimeSieve.h \
	include/primesieve/soe/prime_iterator.h \
	include/primesieve/soe/PrimeSieveCallback.h \
	include/primesieve/soe/stop_primesieve.h

libprimesieve_la_SOURCES = \
	src/soe/EratBig.cpp \
	src/soe/EratMedium.cpp \
	src/soe/EratSmall.cpp \
	src/soe/ParallelPrimeSieve.cpp \
	src/soe/popcount.cpp \
	src/soe/PreSieve.cpp \
	src/soe/PrimeFinder.cpp \
	src/soe/PrimeGenerator.cpp \
	src/soe/prime_iterator.cpp \
	src/soe/PrimeSieve-nthPrime.cpp \
	src/soe/PrimeSieve.cpp \
	src/soe/primesieve-api.cpp \
	src/soe/SieveOfEratosthenes.cpp \
	src/soe/WheelFactorization.cpp \
	src/soe/test.cpp \
	include/primesieve/soe/bits.h \
	include/primesieve/soe/config.h \
	include/primesieve/soe/EratBig.h \
	include/primesieve/soe/EratMedium.h \
	include/primesieve/soe/EratSmall.h \
	include/primesieve/soe/imath.h \
	include/primesieve/soe/littleendian_cast.h \
	include/primesieve/soe/ParallelPrimeSieve-lock.h \
	include/primesieve/soe/PreSieve.h \
	include/primesieve/soe/PrimeFinder.h \
	include/primesieve/soe/PrimeGenerator.h \
	include/primesieve/soe/PrimeSieve-lock.h \
	include/primesieve/soe/SieveOfEratosthenes.h \
	include/primesieve/soe/SieveOfEratosthenes-GENERATE.h \
	include/primesieve/soe/SieveOfEratosthenes-inline.h \
	include/primesieve/soe/toString.h \
	include/primesieve/soe/WheelFactorization.h

primesieve_SOURCES = \
	src/apps/console/main.cpp \
	src/apps/console/help.cpp \
	src/apps/console/cmdoptions.cpp \
	src/apps/console/cmdoptions.h \
	src/apps/console/ExpressionParser.h
