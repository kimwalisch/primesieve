##############################################################################
# Makefile for the primesieve console application (read INSTALL) and the
# primesieve C++ library (read docs/LIBPRIMESIEVE)
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         10 July 2010
# Last modified:   3 February 2012
#
# Project home:    http://primesieve.googlecode.com
##############################################################################

TARGET   = primesieve
CXX      = g++
CXXFLAGS = -Wall -O2
SOEDIR   = src/soe
CONDIR   = src/console
BINDIR   = bin
LIBDIR   = lib

BIN_OBJECTS = $(BINDIR)/WheelFactorization.o \
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

LIB_OBJECTS = $(LIBDIR)/WheelFactorization.o \
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
# set the installation directory (read docs/LIBPRIMESIEVE)
#-----------------------------------------------------------------------------

ifeq ($(shell uname | grep -i darwin),)
  # default installation directory is /usr/local
  PREFIX = /usr/local
else
  # on Mac OS X it is /usr
  PREFIX = /usr
endif

#-----------------------------------------------------------------------------
# check if the user wants to build a shared library instead of a
# static one e.g. `make SHARED=yes`
#-----------------------------------------------------------------------------

ifeq ($(SHARED),)
  LIBPRIMESIEVE = lib$(TARGET).a
else
  ifeq ($(shell uname | grep -i darwin),)
    SOFLAG = -shared
    LIBPRIMESIEVE = lib$(TARGET).so
    FPIC = -fPIC
  else
    SOFLAG = -dynamiclib
    LIBPRIMESIEVE = lib$(TARGET).dylib
  endif
endif

#-----------------------------------------------------------------------------
# use -fopenmp with GCC 4.4 or later (supports OpenMP >= 3.0)
#-----------------------------------------------------------------------------

ifneq ($(shell $(CXX) --version 2>&1 | head -1 | grep -iE 'GCC|G\+\+'),)
  GCC_MAJOR = $(shell $(CXX) -dumpversion 2>&1 | cut -d'.' -f1)
  GCC_MINOR = $(shell $(CXX) -dumpversion 2>&1 | cut -d'.' -f2)
  ifneq ($(shell if [ $$(($(GCC_MAJOR)*100+$(GCC_MINOR))) -ge 404 ]; \
      then echo GCC 4.4 or later; fi),)
    OPENMP = -fopenmp
  endif
endif

#-----------------------------------------------------------------------------
# build the primesieve console application (read INSTALL)
#-----------------------------------------------------------------------------

.PHONY: bin dir_bin

BIN_CXXFLAGS = $(CXXFLAGS)
ifneq ($(OPENMP),)
  BIN_CXXFLAGS += $(OPENMP)
endif
ifneq ($(L1_DCACHE_SIZE),)
  BIN_CXXFLAGS += -DL1_DCACHE_SIZE=$(L1_DCACHE_SIZE)
endif

bin: dir_bin $(BIN_OBJECTS)
	$(CXX) $(BIN_CXXFLAGS) -o $(BINDIR)/$(TARGET) $(BIN_OBJECTS)

$(BINDIR)/%.o: $(SOEDIR)/%.cpp
	$(CXX) $(BIN_CXXFLAGS) -o $@ -c $<

$(BINDIR)/%.o: $(CONDIR)/%.cpp
	$(CXX) $(BIN_CXXFLAGS) -o $@ -c $<

dir_bin:
	@mkdir -p $(BINDIR)

#-----------------------------------------------------------------------------
# build libprimesieve (read docs/LIBPRIMESIEVE)
#-----------------------------------------------------------------------------

.PHONY: lib dir_lib

LIB_CXXFLAGS = $(CXXFLAGS)
ifneq ($(FPIC),)
  LIB_CXXFLAGS += $(FPIC)
endif
ifneq ($(L1_DCACHE_SIZE),)
  LIB_CXXFLAGS += -DL1_DCACHE_SIZE=$(L1_DCACHE_SIZE)
endif

lib: dir_lib $(LIB_OBJECTS)
ifeq ($(SHARED),)
	ar rcs $(LIBDIR)/$(LIBPRIMESIEVE) $(LIB_OBJECTS)
else
	$(CXX) $(LIB_CXXFLAGS) $(SOFLAG) -o $(LIBDIR)/$(LIBPRIMESIEVE) $(LIB_OBJECTS)
endif

$(LIBDIR)/%.o: $(SOEDIR)/%.cpp
	$(CXX) $(LIB_CXXFLAGS) -o $@ -c $<

dir_lib:
	@mkdir -p $(LIBDIR)

#-----------------------------------------------------------------------------
# Common targets (clean, install, uninstall)
#-----------------------------------------------------------------------------

.PHONY: all clean install uninstall

all: bin lib

clean:
ifneq ($(shell [ -d $(BINDIR) ] && echo exists),)
	rm -f $(BINDIR)/$(TARGET) $(BINDIR)/*.o
	@rm -f $(BINDIR)/$(TARGET).exe
endif
ifneq ($(shell [ -d $(LIBDIR) ] && echo exists),)
	rm -f $(LIBDIR)/lib$(TARGET).* $(LIBDIR)/*.o
endif

# might need root privileges (sudo make install)
install:
ifneq ($(shell [ -f $(BINDIR)/$(TARGET) ] && echo exists),)
	cp -f $(BINDIR)/$(TARGET) $(PREFIX)/bin
endif
ifneq ($(shell [ -f $(LIBDIR)/lib$(TARGET).* ] && echo exists),)
	cp -f $(LIBDIR)/lib$(TARGET).* $(PREFIX)/lib
	mkdir -p $(PREFIX)/include/primesieve
	cp -f src/soe/*.h $(PREFIX)/include/primesieve
endif

# might need root privileges (sudo make uninstall)
uninstall:
ifneq ($(shell [ -f $(PREFIX)/bin/$(TARGET) ] && echo exists),)
	rm -f $(PREFIX)/bin/$(TARGET)
	@rm -f $(PREFIX)/bin/$(TARGET).exe
endif
ifneq ($(shell [ -f $(PREFIX)/lib/lib$(TARGET).* ] && echo exists),)
	rm -f $(PREFIX)/lib/lib$(TARGET).*
endif
ifneq ($(shell [ -d $(PREFIX)/include/primesieve ] && echo exists),)
	rm -rf $(PREFIX)/include/primesieve
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
	@echo "sudo make install                      Installs primesieve and libprimesieve (to /usr/local or /usr)"
	@echo "sudo make uninstall                    Completely removes primesieve and libprimesieve"
	@echo "make clean                             Cleans the output directories (./bin, ./lib)"
	@echo "make CXX=compiler CXXFLAGS=\"options\"   Specify a custom C++ compiler"
	@echo "make L1_DCACHE_SIZE=KB                 Specify the CPU's L1 data cache size (read INSTALL)"
	@echo "make help                              Prints this help menu"
