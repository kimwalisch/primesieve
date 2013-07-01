##############################################################################
# GNU Makefile for the primesieve console application (read INSTALL)
#              and the primesieve C++ library         (read doc/LIBPRIMESIEVE)
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         10 July 2010
# Last modified:   01 July 2013
#
# Project home:    http://primesieve.googlecode.com
##############################################################################

TARGET   := primesieve
CXX      := c++
CXXFLAGS := -Wall -O2
BINDIR   := bin
EXDIR    := examples
INCDIR   := include
LIBDIR   := lib
OBJDIR   := obj
SOEDIR   := src/soe

SOE_OBJECTS:= \
  $(SOEDIR)/EratBig.o \
  $(SOEDIR)/EratMedium.o \
  $(SOEDIR)/EratSmall.o \
  $(SOEDIR)/ParallelPrimeSieve.o \
  $(SOEDIR)/popcount.o \
  $(SOEDIR)/PreSieve.o \
  $(SOEDIR)/PrimeFinder.o \
  $(SOEDIR)/PrimeGenerator.o \
  $(SOEDIR)/PrimeSieve-nthPrime.o \
  $(SOEDIR)/PrimeSieve.o \
  $(SOEDIR)/SieveOfEratosthenes.o \
  $(SOEDIR)/WheelFactorization.o

SOE_HEADERS := \
  $(SOEDIR)/bits.h \
  $(SOEDIR)/config.h \
  $(SOEDIR)/endiansafe_cast.h \
  $(SOEDIR)/EratBig.h \
  $(SOEDIR)/EratMedium.h \
  $(SOEDIR)/EratSmall.h \
  $(SOEDIR)/imath.h \
  $(SOEDIR)/ParallelPrimeSieve.h \
  $(SOEDIR)/ParallelPrimeSieve-lock.h \
  $(SOEDIR)/PreSieve.h \
  $(SOEDIR)/PrimeFinder.h \
  $(SOEDIR)/PrimeGenerator.h \
  $(SOEDIR)/PrimeSieve.h \
  $(SOEDIR)/primesieve_error.h \
  $(SOEDIR)/PrimeSieveCallback.h \
  $(SOEDIR)/PrimeSieve-lock.h \
  $(SOEDIR)/SieveOfEratosthenes.h \
  $(SOEDIR)/SieveOfEratosthenes-GENERATE.h \
  $(SOEDIR)/SieveOfEratosthenes-inline.h \
  $(SOEDIR)/stop_primesieve.h \
  $(SOEDIR)/toString.h \
  $(SOEDIR)/WheelFactorization.h

#-----------------------------------------------------------------------------
# Needed to suppress output while checking system features
#-----------------------------------------------------------------------------

NO_STDOUT := 1> /dev/null
NO_STDERR := 2> /dev/null
NO_OUTPUT := $(NO_STDOUT) $(NO_STDERR)

#-----------------------------------------------------------------------------
# Find the compiler's OpenMP flag
#-----------------------------------------------------------------------------

OPENMP_PROGRAM := '\#include <omp.h>\n int main() { return _OPENMP; }'

is-openmp = $(shell command -v $(CXX) $(NO_OUTPUT) && \
                    printf $(OPENMP_PROGRAM) | \
                    $(CXX) $(CXXFLAGS) $1 -xc++ -c -o /dev/null - $(NO_STDERR) && \
                    echo successfully compiled!)

ifeq ($(call is-openmp,),)
  ifneq ($(call is-openmp,-openmp),)
    CXXFLAGS += -openmp
  else
    ifneq ($(call is-openmp,-fopenmp),)
      CXXFLAGS += -fopenmp
    endif
  endif
endif

#-----------------------------------------------------------------------------
# Add the CPU's L1 data cache size (in kilobytes) to CXXFLAGS
#-----------------------------------------------------------------------------

L1_DCACHE_BYTES := $(shell getconf LEVEL1_DCACHE_SIZE $(NO_STDERR) || \
                           command -v sysctl $(NO_OUTPUT) && \
                           sysctl hw.l1dcachesize $(NO_STDERR) | \
                           sed -e 's/^.* //')

ifneq ($(shell expr $(L1_DCACHE_BYTES) '-' $(L1_DCACHE_BYTES) '+' 1 $(NO_OUTPUT) && \
               echo is a number),)
  L1_DCACHE_SIZE := $(shell expr $(L1_DCACHE_BYTES) '/' 1024)
  ifeq ($(shell expr $(L1_DCACHE_SIZE) '>=' 8    $(NO_OUTPUT) && \
                expr $(L1_DCACHE_SIZE) '<=' 2048 $(NO_OUTPUT) && \
                echo valid),)
    L1_DCACHE_SIZE :=
  endif
endif

ifneq ($(L1_DCACHE_SIZE),)
  override CXXFLAGS += -DL1_DCACHE_SIZE=$(L1_DCACHE_SIZE)
endif

#-----------------------------------------------------------------------------
# Default installation path
#-----------------------------------------------------------------------------

PREFIX := /usr

ifneq ($(shell uname | grep -i linux),)
  PREFIX := /usr/local
endif
ifneq ($(shell uname | grep -i mingw),)
  PREFIX := /mingw
endif

#-----------------------------------------------------------------------------
# make            -> libprimesieve.a
# make SHARED=yes -> libprimesieve.(so|dylib)
#-----------------------------------------------------------------------------

ifeq ($(SHARED),)
  LIBRARY := lib$(TARGET).a
else
  ifneq ($(shell uname | grep -i darwin),)
    SOFLAG := -dynamiclib
    LIBRARY := lib$(TARGET).dylib
  else
    SOFLAG := -shared
    LIBRARY := lib$(TARGET).so
    ifeq ($(shell uname | egrep -i 'mingw|cygwin'),)
      FPIC := -fPIC
    endif
  endif
endif

#-----------------------------------------------------------------------------
# Default targets
#-----------------------------------------------------------------------------

.PHONY: all

all: bin lib

#-----------------------------------------------------------------------------
# Create and clean output directories
#-----------------------------------------------------------------------------

.PHONY: make_dir clean

make_dir:
	@mkdir -p $(BINDIR) $(LIBDIR) $(OBJDIR) $(INCDIR)/$(TARGET)/soe

clean:
	rm -rf $(BINDIR) $(LIBDIR) $(OBJDIR) $(INCDIR)

#-----------------------------------------------------------------------------
# Compilation rules
#-----------------------------------------------------------------------------

$(OBJDIR)/%.o: $(SOEDIR)/%.cpp $(SOE_HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(LIBDIR)/%.o: $(SOEDIR)/%.cpp $(SOE_HEADERS)
	$(CXX) $(CXXFLAGS) $(FPIC) -c $< -o $@

$(OBJDIR)/%.o: src/apps/console/%.cpp $(SOE_HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: src/test/%.cpp $(SOE_HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINDIR)/%: $(EXDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $< -o $(BINDIR)/$(notdir $@) -l$(TARGET)

#-----------------------------------------------------------------------------
# Build the console application
#-----------------------------------------------------------------------------

BIN_OBJECTS = \
  $(OBJDIR)/cmdoptions.o \
  $(OBJDIR)/help.o \
  $(OBJDIR)/main.o \
  $(OBJDIR)/test.o \
  $(subst $(SOEDIR),$(OBJDIR),$(SOE_OBJECTS))

.PHONY: bin bin_obj

bin: make_dir bin_obj

bin_obj: $(BIN_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(BINDIR)/$(TARGET) $^

#-----------------------------------------------------------------------------
# Build libprimesieve
#-----------------------------------------------------------------------------

LIB_OBJECTS = $(addprefix \
                $(if $(FPIC),$(LIBDIR)/,$(OBJDIR)/), \
                  $(notdir $(SOE_OBJECTS)))

.PHONY: lib lib_obj

lib: make_dir lib_obj
	cp -f $(SOEDIR)/PrimeSieve.h $(INCDIR)/$(TARGET)/soe
	cp -f $(SOEDIR)/ParallelPrimeSieve.h $(INCDIR)/$(TARGET)/soe
	cp -f $(SOEDIR)/primesieve_error.h $(INCDIR)/$(TARGET)/soe
	cp -f $(SOEDIR)/PrimeSieveCallback.h $(INCDIR)/$(TARGET)/soe
	cp -f $(SOEDIR)/stop_primesieve.h $(INCDIR)/$(TARGET)/soe

lib_obj: $(LIB_OBJECTS)
ifneq ($(SHARED),)
	$(CXX) $(strip $(CXXFLAGS) $(FPIC) $(SOFLAG)) -o $(LIBDIR)/$(LIBRARY) $^
else
	ar rcs $(LIBDIR)/$(LIBRARY) $^
endif

#-----------------------------------------------------------------------------
# Compile the example programs
#-----------------------------------------------------------------------------

.PHONY: examples

examples: make_dir $(subst $(EXDIR),$(BINDIR), \
                     $(basename \
                       $(wildcard $(EXDIR)/*.cpp)))

#-----------------------------------------------------------------------------
# `make check` runs correctness tests
#-----------------------------------------------------------------------------

.PHONY: check test

check test: bin
	$(BINDIR)/./$(TARGET) --test

#-----------------------------------------------------------------------------
# Install & uninstall targets
#-----------------------------------------------------------------------------

.PHONY: install uninstall

# requires sudo privileges
install:
ifneq ($(wildcard $(BINDIR)/$(TARGET)*),)
	@mkdir -p $(PREFIX)/bin
	cp -f $(BINDIR)/$(TARGET) $(PREFIX)/bin
endif
ifneq ($(wildcard $(LIBDIR)/lib$(TARGET).*),)
	@mkdir -p $(PREFIX)/lib
	cp -f $(wildcard $(LIBDIR)/lib$(TARGET).*) $(PREFIX)/lib
	cp -Rf $(INCDIR) $(PREFIX)
  ifneq ($(wildcard $(LIBDIR)/lib$(TARGET).so),)
    ifneq ($(shell command -v ldconfig $(NO_STDERR)),)
		ldconfig $(PREFIX)/lib
    endif
  endif
endif

# requires sudo privileges
uninstall:
ifneq ($(wildcard $(PREFIX)/bin/$(TARGET)*),)
	rm -f $(PREFIX)/bin/$(TARGET)
	@rm -f $(PREFIX)/bin/$(TARGET).exe
endif
ifneq ($(wildcard $(PREFIX)/include/$(TARGET)),)
	rm -rf $(PREFIX)/include/$(TARGET)
endif
ifneq ($(wildcard $(PREFIX)/lib/lib$(TARGET).*),)
  ifneq ($(wildcard $(PREFIX)/lib/lib$(TARGET).so),)
		rm -f $(wildcard $(PREFIX)/lib/lib$(TARGET).so)
    ifneq ($(shell command -v ldconfig $(NO_STDERR)),)
		ldconfig $(PREFIX)/lib
    endif
  else
	rm -f $(wildcard $(PREFIX)/lib/lib$(TARGET).*)
  endif
endif

#-----------------------------------------------------------------------------
# Makefile help menu
#-----------------------------------------------------------------------------

.PHONY: help

help:
	@echo ----------------------------------------------
	@echo ---------- primesieve build options ----------
	@echo ----------------------------------------------
	@echo "make                                     Build primesieve using the default c++ compiler"
	@echo "make CXX=icpc CXXFLAGS=\"-O2 -openmp\"     Specify a custom C++ compiler, here icpc"
	@echo "make L1_DCACHE_SIZE=32                   Specify the CPU's L1 data cache size, here 32 kilobytes"
	@echo "make check                               Test primesieve for correctness"
	@echo "make clean                               Clean the output directories (bin, lib, ...)"
	@echo "make SHARED=yes                          Build a shared libprimesieve library"
	@echo "make examples                            Build the example programs in ./examples"
	@echo "sudo make install                        Install primesieve and libprimesieve to /usr/local or /usr"
	@echo "sudo make install PREFIX=/path           Specify a custom installation path"
	@echo "sudo make uninstall                      Completely remove primesieve and libprimesieve"
	@echo "make help                                Print this help menu"
