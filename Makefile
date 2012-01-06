##############################################################################
# Makefile for primesieve (console app) and libprimesieve
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         10 July 2010
# Last modified:   6 January 2012
#
# Project home:    http://primesieve.googlecode.com
##############################################################################

SOEDIR = ./src/soe
CONDIR = ./src/console
BINDIR = ./bin
LIBDIR = ./lib
BINARY = primesieve
LIBPRIMESIEVE = libprimesieve.a
CXX = g++
CXXFLAGS = -Wall -O2 -fopenmp

OBJECTS = $(BINDIR)/WheelFactorization.o \
  $(BINDIR)/PreSieve.o \
  $(BINDIR)/EratSmall.o \
  $(BINDIR)/EratMedium.o \
  $(BINDIR)/EratBig.o \
  $(BINDIR)/SieveOfEratosthenes.o \
  $(BINDIR)/PrimeNumberGenerator.o \
  $(BINDIR)/PrimeNumberFinder.o \
  $(BINDIR)/PrimeSieve.o \
  $(BINDIR)/ParallelPrimeSieve.o \
  $(BINDIR)/test.o \
  $(BINDIR)/main.o

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
# check if the user indicated his CPU's L1/L2 cache sizes per core
# e.g. make L1_DCACHE_SIZE=32 L2_CACHE_SIZE=256
#-----------------------------------------------------------------------------

ifneq ($(L1_DCACHE_SIZE),)
  CXXFLAGS += -DL1_DCACHE_SIZE=$(L1_DCACHE_SIZE)
endif
ifneq ($(L2_CACHE_SIZE),)
  CXXFLAGS += -DL2_CACHE_SIZE=$(L2_CACHE_SIZE)
endif

#-----------------------------------------------------------------------------
# build primesieve console application (read INSTALL)
#-----------------------------------------------------------------------------

.PHONY: all dir_bin

all: dir_bin $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(BINDIR)/$(BINARY) $(OBJECTS)
ifneq ($(REMARK),)
	@echo Remark: $(REMARK)
endif

$(BINDIR)/%.o: $(SOEDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(BINDIR)/%.o: $(CONDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

dir_bin:
	mkdir -p $(BINDIR)

#-----------------------------------------------------------------------------
# build libprimesieve (static library, read ./docs/LIBPRIMESIEVE)
#-----------------------------------------------------------------------------

.PHONY: lib set_cxxflags dir_lib

lib: set_cxxflags dir_lib $(OBJECTS_LIBPRIMESIEVE)
	ar rcs $(LIBDIR)/$(LIBPRIMESIEVE) $(OBJECTS_LIBPRIMESIEVE)

$(LIBDIR)/%.o: $(SOEDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

# by default libprimesieve is built without OpenMP
set_cxxflags:
DEFAULT = -Wall -O2 -fopenmp
ifeq ($(CXXFLAGS),$(DEFAULT))
CXXFLAGS = -Wall -O2
endif

dir_lib:
	mkdir -p $(LIBDIR)

#-----------------------------------------------------------------------------
# primesieve & libprimesieve installation
# copies to directories: /usr/bin, /usr/lib, /usr/include/soe
#-----------------------------------------------------------------------------

.PHONY: install uninstall clean

# needs root privileges (sudo make install)
install:
ifneq ($(shell [ -f $(BINDIR)/$(BINARY) ] && echo exists),)
	cp -f $(BINDIR)/$(BINARY) /usr/bin
endif
ifneq ($(shell [ -f $(LIBDIR)/$(LIBPRIMESIEVE) ] && echo exists),)
	cp -f $(LIBDIR)/$(LIBPRIMESIEVE) /usr/lib
	mkdir -p /usr/include/soe
	cp -f src/soe/*.h /usr/include/soe
endif

# needs root privileges (sudo make uninstall)
uninstall:
ifneq ($(shell [ -f /usr/bin/$(BINARY) ] && echo exists),)
	rm -f /usr/bin/$(BINARY)
endif
ifneq ($(shell [ -f /usr/lib/$(LIBPRIMESIEVE) ] && echo exists),)
	rm -f /usr/lib/$(LIBPRIMESIEVE)
endif
ifneq ($(shell [ -d /usr/include/soe ] && echo exists),)
	rm -rf /usr/include/soe
endif

clean:
ifneq ($(shell [ -d $(BINDIR) ] && echo exists),)
	rm -f $(BINDIR)/$(BINARY) $(BINDIR)/*.o
endif
ifneq ($(shell [ -d $(LIBDIR) ] && echo exists),)
	rm -f $(LIBDIR)/$(LIBPRIMESIEVE) $(LIBDIR)/*.o
endif

#-----------------------------------------------------------------------------
# help menu (make help)
#-----------------------------------------------------------------------------

.PHONY: help

help:
	@echo -----------------------------------------------------------
	@echo ------------------- primesieve Makefile -------------------
	@echo -----------------------------------------------------------
	@echo "make                                      Builds the primesieve console application using g++ (DEFAULT)"
	@echo "make lib                                  Builds the primesieve C++ library (read docs/LIBPRIMESIEVE)"
	@echo "sudo make install                         Installs primesieve and libprimesieve (to /usr/bin, /usr/lib)"
	@echo "sudo make uninstall                       Completely removes primesieve and libprimesieve"
	@echo "make clean                                Cleans the output directories (./bin, ./lib)"
	@echo "make CXX=compiler CXXFLAGS=\"options\"      Specify a custom C++ compiler"
	@echo "make L1_DCACHE_SIZE=KB L2_CACHE_SIZE=KB   Specify the CPU's L1/L2 cache sizes (read INSTALL)"
	@echo "make help                                 Prints this help menu"
