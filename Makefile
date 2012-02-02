##############################################################################
# Makefile for the primesieve console application (read INSTALL) and the
# primesieve C++ library (read docs/LIBPRIMESIEVE)
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         10 July 2010
# Last modified:   2 February 2012
#
# Project home:    http://primesieve.googlecode.com
##############################################################################

TARGET       = primesieve
CXX          = g++
CXXFLAGS     = -Wall -O2
CXXFLAGS_LIB = -Wall -O2
SOEDIR       = src/soe
CONDIR       = src/console
BINDIR       = bin
LIBDIR       = lib

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

OBJECTS_LIB = $(LIBDIR)/WheelFactorization.o \
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
# Add -fopenmp for GCC 4.4 or later (supports OpenMP 3.0)
#-----------------------------------------------------------------------------

ifneq ($(shell $(CXX) --version 2>&1 | head -1 | grep -iE 'GCC|G\+\+'),)
  GCC_MAJOR = $(shell $(CXX) -dumpversion 2>&1 | cut -d'.' -f1)
  GCC_MINOR = $(shell $(CXX) -dumpversion 2>&1 | cut -d'.' -f2)
  ifneq ($(shell if [ $$(($(GCC_MAJOR)*100+$(GCC_MINOR))) -ge 404 ]; \
      then echo GCC 4.4 or later; fi),)
    CXXFLAGS += -fopenmp
  endif
endif

#-----------------------------------------------------------------------------
# check if the user indicated his CPU's L1 data cache size per core
# e.g. `make L1_DCACHE_SIZE=32`
#-----------------------------------------------------------------------------

ifneq ($(L1_DCACHE_SIZE),)
  CPU_CACHE_SIZE += -DL1_DCACHE_SIZE=$(L1_DCACHE_SIZE)
endif

#-----------------------------------------------------------------------------
# build the primesieve console application (read INSTALL)
#-----------------------------------------------------------------------------

.PHONY: bin dir_bin

bin: dir_bin $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(CPU_CACHE_SIZE) -o $(BINDIR)/$(TARGET) $(OBJECTS)

$(BINDIR)/%.o: $(SOEDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(CPU_CACHE_SIZE) -o $@ -c $<

$(BINDIR)/%.o: $(CONDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(CPU_CACHE_SIZE) -o $@ -c $<

dir_bin:
	@mkdir -p $(BINDIR)

#-----------------------------------------------------------------------------
# build the libprimesieve library (read ./docs/LIBPRIMESIEVE)
#-----------------------------------------------------------------------------

.PHONY: lib dir_lib

lib: dir_lib $(OBJECTS_LIB)
	ar rcs $(LIBDIR)/lib$(TARGET).a $(OBJECTS_LIB)

$(LIBDIR)/%.o: $(SOEDIR)/%.cpp
	$(CXX) $(CXXFLAGS_LIB) $(CPU_CACHE_SIZE) -o $@ -c $<

dir_lib:
	@mkdir -p $(LIBDIR)

#-----------------------------------------------------------------------------
# Common primesieve & libprimesieve targets
#-----------------------------------------------------------------------------

.PHONY: all clean install uninstall

all: bin lib

clean:
ifneq ($(shell [ -d $(BINDIR) ] && echo exists),)
	rm -f $(BINDIR)/$(TARGET) $(BINDIR)/$(TARGET).exe $(BINDIR)/*.o
endif
ifneq ($(shell [ -f $(BINDIR)/$(TARGET).exe ] && echo exists),)
	rm -f $(BINDIR)/$(TARGET).exe
endif
ifneq ($(shell [ -d $(LIBDIR) ] && echo exists),)
	rm -f $(LIBDIR)/lib$(TARGET).a $(LIBDIR)/*.o
endif

# needs root privileges (sudo make install)
# installation directories: /usr/bin, /usr/lib, /usr/include/soe
install:
ifneq ($(shell [ -f $(BINDIR)/$(TARGET) ] && echo exists),)
	cp -f $(BINDIR)/$(TARGET) /usr/bin
endif
ifneq ($(shell [ -f $(LIBDIR)/lib$(TARGET).a ] && echo exists),)
	cp -f $(LIBDIR)/lib$(TARGET).a /usr/lib
	mkdir -p /usr/include/soe
	cp -f src/soe/*.h /usr/include/soe
endif

# needs root privileges (sudo make uninstall)
uninstall:
ifneq ($(shell [ -f /usr/bin/$(TARGET) ] && echo exists),)
	rm -f /usr/bin/$(TARGET)
endif
ifneq ($(shell [ -f /usr/lib/lib$(TARGET).a ] && echo exists),)
	rm -f /usr/lib/lib$(TARGET).a
endif
ifneq ($(shell [ -d /usr/include/soe ] && echo exists),)
	rm -rf /usr/include/soe
endif

#-----------------------------------------------------------------------------
# `make check` runs various sieving tests to assure that the compiled
# primesieve binary produces correct results
#-----------------------------------------------------------------------------

.PHONY: check test

check test: bin
	$(BINDIR)/./$(TARGET) -test

#-----------------------------------------------------------------------------
# Makefile help menu
#-----------------------------------------------------------------------------

.PHONY: help

help:
	@echo ---------------------------------------------------
	@echo --------------- primesieve Makefile ---------------
	@echo ---------------------------------------------------
	@echo "make                                   Builds the primesieve console application using g++ (DEFAULT)"
	@echo "make lib                               Builds the primesieve C++ library (read docs/LIBPRIMESIEVE)"
	@echo "make check                             Tests the compiled primesieve binary"
	@echo "sudo make install                      Installs primesieve and libprimesieve (to /usr/bin, /usr/lib)"
	@echo "sudo make uninstall                    Completely removes primesieve and libprimesieve"
	@echo "make clean                             Cleans the output directories (./bin, ./lib)"
	@echo "make CXX=compiler CXXFLAGS=\"options\"   Specify a custom C++ compiler"
	@echo "make L1_DCACHE_SIZE=KB                 Specify the CPU's L1 data cache size (read INSTALL)"
	@echo "make help                              Prints this help menu"
