AM_CPPFLAGS = -I$(top_srcdir)/include

lib_LTLIBRARIES = libprimesieve.la
libprimesieve_la_CXXFLAGS = $(OPENMP_CXXFLAGS)
libprimesieve_la_LDFLAGS = -version-info @primesieve_lib_version@

if FORCE_SHARED_LIBRARY
libprimesieve_la_LDFLAGS += -no-undefined 
endif

bin_PROGRAMS = primesieve
primesieve_LDADD = libprimesieve.la

include_HEADERS = \
	include/primesieve.hpp \
	include/primesieve.h

otherincludedir = $(includedir)/primesieve
otherinclude_HEADERS = \
	include/primesieve/Callback.hpp \
	include/primesieve/cancel_callback.hpp \
	include/primesieve/iterator.hpp \
	include/primesieve/ParallelPrimeSieve.hpp \
	include/primesieve/primesieve_error.hpp \
	include/primesieve/PrimeSieve.hpp \
	include/primesieve/primesieve_iterator.h \
	include/primesieve/PushBackPrimes.hpp

EXTRA_DIST = \
	autogen.sh \
	README.md \
	RELEASE.md \
	Makefile.msvc \
	doc/Doxyfile.in \
	doc/primesieve.1 \
	doc/README.md \
	doc/mainpage.dox \
	examples/README.md \
	scripts/update_version.sh \
	src/apps/gui \
	src/msvc_compat \
	src/primesieve/README

DISTCLEANFILES = \
	doc/Doxyfile

man_MANS = \
	doc/primesieve.1

libprimesieve_la_SOURCES = \
	src/primesieve/EratBig.cpp \
	src/primesieve/EratMedium.cpp \
	src/primesieve/EratSmall.cpp \
	src/primesieve/ParallelPrimeSieve.cpp \
	src/primesieve/popcount.cpp \
	src/primesieve/PreSieve.cpp \
	src/primesieve/PrimeFinder.cpp \
	src/primesieve/PrimeGenerator.cpp \
	src/primesieve/iterator.cpp \
	src/primesieve/primesieve_iterator.cpp \
	src/primesieve/primesieve_test.cpp \
	src/primesieve/PrimeSieve-nthPrime.cpp \
	src/primesieve/PrimeSieve.cpp \
	src/primesieve/primesieve-api.cpp \
	src/primesieve/primesieve-api-c.cpp \
	src/primesieve/SieveOfEratosthenes.cpp \
	src/primesieve/WheelFactorization.cpp \
	include/primesieve/bits.hpp \
	include/primesieve/callback_t.hpp \
	include/primesieve/config.hpp \
	include/primesieve/EratBig.hpp \
	include/primesieve/EratMedium.hpp \
	include/primesieve/EratSmall.hpp \
	include/primesieve/malloc_vector.hpp \
	include/primesieve/pmath.hpp \
	include/primesieve/littleendian_cast.hpp \
	include/primesieve/ParallelPrimeSieve-lock.hpp \
	include/primesieve/PreSieve.hpp \
	include/primesieve/PrimeFinder.hpp \
	include/primesieve/PrimeGenerator.hpp \
	include/primesieve/PrimeSieve-lock.hpp \
	include/primesieve/SieveOfEratosthenes.hpp \
	include/primesieve/SieveOfEratosthenes-inline.hpp \
	include/primesieve/toString.hpp \
	include/primesieve/WheelFactorization.hpp

primesieve_SOURCES = \
	src/apps/console/main.cpp \
	src/apps/console/help.cpp \
	src/apps/console/cmdoptions.cpp \
	src/apps/console/cmdoptions.hpp \
	src/apps/console/calculator.hpp

check: all
	./primesieve --test

# If installation directory is /usr/local/lib
# execute ldconfig after installation
install-exec-hook:
	test "$(prefix)" = "/usr/local" && command -v ldconfig 2>/dev/null && ldconfig /usr/local/lib 2>/dev/null || true

if MAKE_EXAMPLES
noinst_PROGRAMS = callback_cancel
callback_cancel_LDADD = libprimesieve.la
callback_cancel_SOURCES = examples/cpp/callback_cancel.cpp

noinst_PROGRAMS += callback_primes
callback_primes_LDADD = libprimesieve.la
callback_primes_SOURCES = examples/cpp/callback_primes.cpp

noinst_PROGRAMS += callback_primes_oop
callback_primes_oop_LDADD = libprimesieve.la
callback_primes_oop_SOURCES = examples/cpp/callback_primes_oop.cpp

noinst_PROGRAMS += count_primes
count_primes_LDADD = libprimesieve.la
count_primes_SOURCES = examples/cpp/count_primes.cpp

noinst_PROGRAMS += primesieve_iterator
primesieve_iterator_LDADD = libprimesieve.la
primesieve_iterator_SOURCES = examples/cpp/primesieve_iterator.cpp

noinst_PROGRAMS += nth_prime
nth_prime_LDADD = libprimesieve.la
nth_prime_SOURCES = examples/cpp/nth_prime.cpp

noinst_PROGRAMS += previous_prime
previous_prime_LDADD = libprimesieve.la
previous_prime_SOURCES = examples/cpp/previous_prime.cpp

noinst_PROGRAMS += store_primes_in_vector
store_primes_in_vector_LDADD = libprimesieve.la
store_primes_in_vector_SOURCES = examples/cpp/store_primes_in_vector.cpp

noinst_PROGRAMS += count_primes_c
count_primes_c_LDADD = libprimesieve.la
count_primes_c_SOURCES = examples/c/count_primes.c

noinst_PROGRAMS += previous_prime_c
previous_prime_c_LDADD = libprimesieve.la
previous_prime_c_SOURCES = examples/c/previous_prime.c

noinst_PROGRAMS += primesieve_iterator_c
primesieve_iterator_c_LDADD = libprimesieve.la
primesieve_iterator_c_SOURCES = examples/c/primesieve_iterator.c

noinst_PROGRAMS += nth_prime_c
nth_prime_c_LDADD = libprimesieve.la
nth_prime_c_SOURCES = examples/c/nth_prime.c

noinst_PROGRAMS += store_primes_in_array_c
store_primes_in_array_c_LDADD = libprimesieve.la
store_primes_in_array_c_SOURCES = examples/c/store_primes_in_array.c
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = primesieve.pc

if HAVE_DOXYGEN
doxygen:
	cd doc && $(DOXYGEN)
endif

if HAVE_HELP2MAN
help2man: doc/primesieve.1

doc/primesieve.1: primesieve
	$(HELP2MAN) --manual="primesieve" \
         --source="primesieve @PACKAGE_VERSION@" \
         -n "efficient prime number generator" \
         -s 1 --no-info \
         -o doc/primesieve.1 primesieve
endif

man: $(HELP2MAN)
doc: man $(DOXYGEN)
