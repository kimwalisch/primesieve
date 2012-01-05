##############################################################################
# Makefile for primesieve (console app) and libprimesieve
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         10 July 2010
# Last modified:   5 January 2012
#
# Project home:    http://primesieve.googlecode.com
##############################################################################

SOEDIR = src/soe
CONDIR = src/console
OUTDIR = out
LIBDIR = lib
BINARY = $(OUTDIR)/primesieve
LIBPRIMESIEVE = $(LIBDIR)/libprimesieve.a
CXX = g++
CXXFLAGS = -O2
CXXFLAGS_LIBPRIMESIEVE = -O2

OBJECTS = $(OUTDIR)/WheelFactorization.o \
  $(OUTDIR)/PreSieve.o \
  $(OUTDIR)/EratSmall.o \
  $(OUTDIR)/EratMedium.o \
  $(OUTDIR)/EratBig.o \
  $(OUTDIR)/SieveOfEratosthenes.o \
  $(OUTDIR)/PrimeNumberGenerator.o \
  $(OUTDIR)/PrimeNumberFinder.o \
  $(OUTDIR)/PrimeSieve.o \
  $(OUTDIR)/ParallelPrimeSieve.o \
  $(OUTDIR)/test.o \
  $(OUTDIR)/main.o

OBJECTS_LIBPRIMESIEVE = $(LIBDIR)/WheelFactorization.o \
  $(LIBDIR)/PreSieve.o \
  $(LIBDIR)/EratSmall.o \
  $(LIBDIR)/EratMedium.o \
  $(LIBDIR)/EratBig.o \
  $(LIBDIR)/SieveOfEratosthenes.o \
  $(LIBDIR)/PrimeNumberGenerator.o \
  $(LIBDIR)/PrimeNumberFinder.o \
  $(LIBDIR)/PrimeSieve.o \
  $(LIBDIR)/ParallelPrimeSieve.o

#-----------------------------------------------------------------------------
# set CXXFLAGS for various C++ compilers (sunCC, icpc, g++, ...)
# e.g. make CXX=sunCC; sets "CXXFLAGS = +w -fast -xopenmp -xrestrict"
#-----------------------------------------------------------------------------

# sunCC, CC : Oracle Solaris Studio
# Optimization flags: http://dsc.sun.com/solaris/articles/amdopt.html
ifneq ($(shell $(CXX) -V 2>&1 | head -1 | grep -iE "sun|oracle"),)
  CXXFLAGS = +w -fast -xopenmp -xrestrict
  REMARK = you might need to export OMP_NUM_THREADS for OpenMP multi-threading.

# icpc : Intel C++ Compiler
# == Profile-guided optimization (5 percent faster, icpc 12.0) ==
# make CXX=icpc "CXXFLAGS= -openmp -O2 -prof-gen"
# out/./primesieve 1E18 -o1E10 -t1
# make clean
# make CXX=icpc "CXXFLAGS= -openmp -O2 -ipo -prof-use"
else ifeq ($(CXX),icpc)
  CXXFLAGS = -Wall -O2 -openmp
  REMARK = read the Makefile for instructions on profile-guided optimization.

# g++ : GNU Compiler Collection
else ifneq ($(shell $(CXX) --version 2>&1 | head -1 | grep -iE "GCC|G\+\+"),)
  ifneq ($(shell $(CXX) --version 2>&1 | head -1 | grep -i apple),)
    # Apple g++ flags
    CXXFLAGS = -Wall -fast -fopenmp
  else
    # GNU g++ flags
    CXXFLAGS = -Wall -O2 -fopenmp
  endif

else
  REMARK = unkown compiler, add OpenMP flag if supported \(edit Makefile\).
endif

#-----------------------------------------------------------------------------
# check if the user indicated his CPU's L1/L2 cache sizes per core
# e.g. make L1_DCACHE_SIZE=32 L2_CACHE_SIZE=256
#-----------------------------------------------------------------------------

ifneq ($(L1_DCACHE_SIZE),)
  CXXFLAGS += -DL1_DCACHE_SIZE=$(L1_DCACHE_SIZE)
  CXXFLAGS_LIBPRIMESIEVE += -DL1_DCACHE_SIZE=$(L1_DCACHE_SIZE)
endif
ifneq ($(L2_CACHE_SIZE),)
  CXXFLAGS += -DL2_CACHE_SIZE=$(L2_CACHE_SIZE)
  CXXFLAGS_LIBPRIMESIEVE += -DL2_CACHE_SIZE=$(L2_CACHE_SIZE)
endif

#-----------------------------------------------------------------------------
# build primesieve console application (read INSTALL)
#-----------------------------------------------------------------------------

all: create-out-dir build remark

create-out-dir:
	mkdir -p $(OUTDIR)

build: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(BINARY) $(OBJECTS)

$(OUTDIR)/%.o: $(SOEDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OUTDIR)/%.o: $(CONDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

remark:
ifneq ($(REMARK),)
	@echo Remark: $(REMARK)
endif

.PHONY: clean
clean:
	rm -f $(BINARY) $(OUTDIR)/*.o

#-----------------------------------------------------------------------------
# build libprimesieve (static library, read docs/LIBPRIMESIEVE)
# installation directories: /usr/local/lib, /usr/local/include/soe
#-----------------------------------------------------------------------------

create-lib-dir:
	mkdir -p $(LIBDIR)

lib: create-lib-dir $(OBJECTS_LIBPRIMESIEVE)
	ar rcs $(LIBPRIMESIEVE) $(OBJECTS_LIBPRIMESIEVE)

$(LIBDIR)/%.o: $(SOEDIR)/%.cpp
	$(CXX) $(CXXFLAGS_LIBPRIMESIEVE) -o $@ -c $<

# needs root privileges ($ sudo make install-lib)
install-lib:
	cp -f $(LIBPRIMESIEVE) /usr/local/lib
	mkdir -p /usr/local/include/soe
	cp -f src/soe/*.h /usr/local/include/soe

# needs root privileges ($ sudo make uninstall-lib)
uninstall-lib:
	rm -rf /usr/local/lib/$(LIBPRIMESIEVE) /usr/local/include/soe

clean-lib:
	rm -f $(LIBPRIMESIEVE) $(LIBDIR)/*.o
