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
