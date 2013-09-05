##############################################################################
# GNU Makefile for the primesieve console application (read INSTALL)
#              and the primesieve C++ library         (read doc/LIBPRIMESIEVE)
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         10 July 2010
# Last modified:   05 September 2013
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
FPICDIR  := obj/fpic
SOEDIR   := src/soe

SOE_OBJECTS:= \
  $(OBJDIR)/EratBig.o \
  $(OBJDIR)/EratMedium.o \
  $(OBJDIR)/EratSmall.o \
  $(OBJDIR)/ParallelPrimeSieve.o \
  $(OBJDIR)/popcount.o \
  $(OBJDIR)/PreSieve.o \
  $(OBJDIR)/PrimeFinder.o \
  $(OBJDIR)/PrimeGenerator.o \
  $(OBJDIR)/PrimeSieve-nthPrime.o \
  $(OBJDIR)/PrimeSieve.o \
  $(OBJDIR)/SieveOfEratosthenes.o \
  $(OBJDIR)/WheelFactorization.o

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

INC_HEADERS := \
  $(SOEDIR)/ParallelPrimeSieve.h \
  $(SOEDIR)/PrimeSieve.h \
  $(SOEDIR)/primesieve_error.h \
  $(SOEDIR)/PrimeSieveCallback.h \
  $(SOEDIR)/stop_primesieve.h

#-----------------------------------------------------------------------------
# Needed to suppress output while checking system features
#-----------------------------------------------------------------------------

NO_STDOUT := 1> /dev/null
NO_STDERR := 2> /dev/null
NO_OUTPUT := $(NO_STDOUT) $(NO_STDERR)

#-----------------------------------------------------------------------------
# Find the compiler's OpenMP flag
#-----------------------------------------------------------------------------

ifneq ($(OPENMP),no)
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
# make        -> libprimesieve.a
# make shared -> libprimesieve.(so|dylib)
#-----------------------------------------------------------------------------

ifeq ($(SHARED),)
  STATIC := yes
endif

ifneq ($(shell uname | grep -i darwin),)
  SOFLAG := -dynamiclib
  SHARED_LIBRARY := lib$(TARGET).dylib
else
  SOFLAG := -shared
  SHARED_LIBRARY := lib$(TARGET).so
  ifeq ($(shell uname | egrep -i 'mingw|cygwin'),)
    FPIC := -fPIC
  endif
endif

#-----------------------------------------------------------------------------
# Top level targets
#-----------------------------------------------------------------------------

.PHONY: default all lib

default: bin static

all: bin static shared

lib: $(if $(STATIC),static,) $(if $(SHARED),shared,)

#-----------------------------------------------------------------------------
# Create and clean output directories
#-----------------------------------------------------------------------------

.PHONY: make_dir clean

make_dir:
	@mkdir -p $(BINDIR) $(LIBDIR) $(OBJDIR) $(FPICDIR)
	@mkdir -p  $(INCDIR)/$(TARGET)/soe

clean:
	rm -rf $(BINDIR) $(LIBDIR) $(OBJDIR) $(INCDIR)

#-----------------------------------------------------------------------------
# Compilation rules
#-----------------------------------------------------------------------------

$(OBJDIR)/%.o: $(SOEDIR)/%.cpp $(SOE_HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(FPICDIR)/%.o: $(SOEDIR)/%.cpp $(SOE_HEADERS)
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
  $(SOE_OBJECTS)

.PHONY: bin bin_obj

bin: make_dir bin_obj

bin_obj: $(BIN_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(BINDIR)/$(TARGET) $^

#-----------------------------------------------------------------------------
# Build libprimesieve
#-----------------------------------------------------------------------------

SHARED_OBJECTS = $(if $(FPIC), \
                   $(subst $(OBJDIR),$(FPICDIR),$(SOE_OBJECTS)), \
                     $(SOE_OBJECTS))

.PHONY: static shared static_obj shared_obj

static: make_dir static_obj
	cp -f $(INC_HEADERS) $(INCDIR)/$(TARGET)/soe

shared: make_dir shared_obj
	cp -f $(INC_HEADERS) $(INCDIR)/$(TARGET)/soe

static_obj: $(SOE_OBJECTS)
	$(AR) rcs $(LIBDIR)/lib$(TARGET).a $^

shared_obj: $(SHARED_OBJECTS)
	$(CXX) $(strip $(CXXFLAGS) $(FPIC) $(SOFLAG)) -o $(LIBDIR)/$(SHARED_LIBRARY) $^

#-----------------------------------------------------------------------------
# Build the example programs
#-----------------------------------------------------------------------------

.PHONY: examples

examples: make_dir $(subst $(EXDIR),$(BINDIR), \
                     $(basename \
                       $(wildcard $(EXDIR)/*.cpp)))

#-----------------------------------------------------------------------------
# Run integration tests
#-----------------------------------------------------------------------------

.PHONY: check test

check test: bin
	$(BINDIR)/./$(TARGET) --test

#-----------------------------------------------------------------------------
# Install & uninstall targets
#-----------------------------------------------------------------------------

.PHONY: install uninstall

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
      ifeq ($(firstword $(subst /, ,$(PREFIX))),usr)
			ldconfig $(PREFIX)/lib
      endif
    endif
  endif
endif

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
		rm -f $(wildcard $(PREFIX)/lib/lib$(TARGET).*)
    ifneq ($(shell command -v ldconfig $(NO_STDERR)),)
      ifeq ($(firstword $(subst /, ,$(PREFIX))),usr)
			ldconfig $(PREFIX)/lib
      endif
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
	@echo "make                                     Build primesieve and static libprimesieve"
	@echo "make all                                 Build primesieve and static & shared libprimesieve"
	@echo "make CXX=icpc CXXFLAGS=\"-O2 -openmp\"     Specify a custom C++ compiler, here icpc"
	@echo "make L1_DCACHE_SIZE=32                   Specify the CPU's L1 data cache size, here 32 kilobytes"
	@echo "make static                              Build only static libprimesieve"
	@echo "make shared                              Build only shared libprimesieve"
	@echo "make examples                            Build the example programs in ./examples"
	@echo "make check                               Test primesieve for correctness"
	@echo "sudo make install                        Install primesieve and libprimesieve to /usr[/local]"
	@echo "sudo make install PREFIX=/path           Specify a custom installation path"
	@echo "sudo make uninstall                      Completely remove primesieve and libprimesieve"
	@echo "make clean                               Clean the output directories (bin, lib, ...)"
	@echo "make help                                Print this help menu"
