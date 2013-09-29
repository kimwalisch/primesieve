ACLOCAL_AMFLAGS = -Im4
AM_CPPFLAGS = -Iinclude
EXTRA_DIST = Makefile.msvc src/msvc_compat src/apps/gui

lib_LTLIBRARIES = libprimesieve.la
libprimesieve_la_CXXFLAGS = $(OPENMP_CXXFLAGS)

bin_PROGRAMS = primesieve
primesieve_LDADD = libprimesieve.la
primesieve_LDFLAGS = $(OPENMP_CXXFLAGS)

check: primesieve
	libtool execute primesieve --test

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
