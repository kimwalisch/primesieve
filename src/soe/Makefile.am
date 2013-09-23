lib_LTLIBRARIES = libprimesieve.la

libprimesieve_la_SOURCES = \
	EratBig.cpp \
	EratMedium.cpp \
	EratSmall.cpp \
	ParallelPrimeSieve.cpp \
	popcount.cpp \
	PreSieve.cpp \
	PrimeFinder.cpp \
	PrimeGenerator.cpp \
	PrimeSieve-nthPrime.cpp \
	PrimeSieve.cpp \
	SieveOfEratosthenes.cpp \
	WheelFactorization.cpp

libprimesieve_la_CXXFLAGS = $(OPENMP_CXXFLAGS)

pkginclude_HEADERS = \
	ParallelPrimeSieve.h \
	PrimeSieve.h \
	PrimeSieveCallback.h

noinst_HEADERS = \
	EratBig.h \
	EratMedium.h \
	EratSmall.h \
	ParallelPrimeSieve-lock.h \
	PreSieve.h \
	PrimeFinder.h \
	PrimeGenerator.h \
	PrimeSieve-lock.h \
	SieveOfEratosthenes-GENERATE.h \
	SieveOfEratosthenes-inline.h \
	SieveOfEratosthenes.h \
	WheelFactorization.h \
	bits.h \
	config.h \
	imath.h \
	littleendian_cast.h \
	primesieve_error.h \
	stop_primesieve.h \
	toString.h
