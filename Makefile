##############################################################################
# GNU Makefile for the primesieve console application (read doc/INSTALL)
#              and the primesieve C++ library         (read doc/LIBPRIMESIEVE)
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         10 July 2010
# Last modified:   23 September 2012
#
# Project home:    http://primesieve.googlecode.com
##############################################################################

TARGET   := primesieve
CXX      := g++
CXXFLAGS := -Wall -O2
BINDIR   := bin
LIBDIR   := lib
DISTDIR  := dist

BIN_OBJECTS := \
  $(BINDIR)/WheelFactorization.o \
  $(BINDIR)/PreSieve.o \
  $(BINDIR)/EratSmall.o \
  $(BINDIR)/EratMedium.o \
  $(BINDIR)/EratBig.o \
  $(BINDIR)/SieveOfEratosthenes.o \
  $(BINDIR)/PrimeNumberFinder.o \
  $(BINDIR)/PrimeSieve.o \
  $(BINDIR)/ParallelPrimeSieve.o \
  $(BINDIR)/test.o \
  $(BINDIR)/main.o

LIB_OBJECTS := \
  $(LIBDIR)/WheelFactorization.o \
  $(LIBDIR)/PreSieve.o \
  $(LIBDIR)/EratSmall.o \
  $(LIBDIR)/EratMedium.o \
  $(LIBDIR)/EratBig.o \
  $(LIBDIR)/SieveOfEratosthenes.o \
  $(LIBDIR)/PrimeNumberFinder.o \
  $(LIBDIR)/PrimeSieve.o \
  $(LIBDIR)/ParallelPrimeSieve.o

#-----------------------------------------------------------------------------
# Use the Bash shell
#-----------------------------------------------------------------------------

BASH := $(shell command -v bash 2> /dev/null)

ifneq ($(BASH),)
  SHELL := $(BASH)
endif

#-----------------------------------------------------------------------------
# Add -fopenmp to CXXFLAGS if GCC supports OpenMP >= 3.0
#-----------------------------------------------------------------------------

ifneq ($(shell $(CXX) --version 2> /dev/null | head -1 | grep -iE 'GCC|G\+\+'),)
  MAJOR := $(shell $(CXX) -dumpversion | cut -d'.' -f1)
  MINOR := $(shell $(CXX) -dumpversion | cut -d'.' -f2)
  GCC_VERSION := $(shell echo $$(( $(MAJOR) * 100 + $(MINOR) )) )
  ifneq ($(shell if (( $(GCC_VERSION) >= 404 )); then echo 'OpenMP >= 3.0'; fi),)
    CXXFLAGS += -fopenmp
  endif
endif

#-----------------------------------------------------------------------------
# Add the CPU's L1 data cache size (in kilobytes) to CXXFLAGS
#-----------------------------------------------------------------------------

L1_DCACHE_BYTES := $(shell getconf LEVEL1_DCACHE_SIZE 2> /dev/null)
ifeq ($(L1_DCACHE_BYTES),)
  L1_DCACHE_BYTES := $(shell sysctl hw.l1dcachesize 2> /dev/null | sed -e 's/^.* //')
endif

ifneq ($(shell if (( $(L1_DCACHE_BYTES) > 0 )) 2> /dev/null; then echo is a number; fi),)
  L1_DCACHE_SIZE := $(shell echo $$(( $(L1_DCACHE_BYTES) / 1024 )) )
  ifneq ($(shell if (( $(L1_DCACHE_SIZE) < 8 )) || \
                    (( $(L1_DCACHE_SIZE) > 4096 )); then echo no; fi),)
    L1_DCACHE_SIZE :=
  endif
endif

ifneq ($(L1_DCACHE_SIZE),)
  override CXXFLAGS += -DL1_DCACHE_SIZE=$(L1_DCACHE_SIZE)
endif

#-----------------------------------------------------------------------------
# Installation path: Linux = /usr/local, Unix = /usr
#-----------------------------------------------------------------------------

PREFIX := $(if $(shell uname | grep -i linux),/usr/local,/usr)

#-----------------------------------------------------------------------------
# `make lib`            -> libprimesieve.a
# `make lib SHARED=yes` -> libprimesieve.(so|dylib)
#-----------------------------------------------------------------------------

ifeq ($(SHARED),)
  LIBPRIMESIEVE := lib$(TARGET).a
else
  ifneq ($(shell uname | grep -i darwin),)
    SOFLAG := -dynamiclib
    LIBPRIMESIEVE := lib$(TARGET).dylib
  else
    SOFLAG := -shared
    FPIC := -fPIC
    LIBPRIMESIEVE := lib$(TARGET).so
  endif
endif

#-----------------------------------------------------------------------------
# Build the primesieve console application
#-----------------------------------------------------------------------------

.PHONY: bin dir_bin

bin: dir_bin $(BIN_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(BINDIR)/$(TARGET) $(BIN_OBJECTS)

$(BINDIR)/%.o: src/soe/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINDIR)/%.o: src/test/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINDIR)/%.o: src/console/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

dir_bin:
	@mkdir -p $(BINDIR)

#-----------------------------------------------------------------------------
# Build libprimesieve
#-----------------------------------------------------------------------------

.PHONY: lib dir_lib

LIB_CXXFLAGS := $(if $(FPIC),$(CXXFLAGS) $(FPIC),$(CXXFLAGS))

lib: dir_lib $(LIB_OBJECTS)
ifeq ($(SHARED),)
	ar rcs $(LIBDIR)/$(LIBPRIMESIEVE) $(LIB_OBJECTS)
else
	$(CXX) $(LIB_CXXFLAGS) $(SOFLAG) -o $(LIBDIR)/$(LIBPRIMESIEVE) $(LIB_OBJECTS)
endif

$(LIBDIR)/%.o: src/soe/%.cpp
	$(CXX) $(LIB_CXXFLAGS) -c $< -o $@

dir_lib:
	@mkdir -p $(LIBDIR)

#-----------------------------------------------------------------------------
# Create a libprimesieve distribution archive (./dist)
#-----------------------------------------------------------------------------

.PHONY: dist check_lib

dist: check_lib
	@mkdir -p $(DISTDIR)/$(TARGET)/soe
	cp -f $(wildcard $(LIBDIR)/lib$(TARGET).*) $(DISTDIR)
	cp -f src/soe/*PrimeSieve.h $(DISTDIR)/$(TARGET)/soe

check_lib:
ifeq ($(wildcard $(LIBDIR)/*),)
	$(error Error: Please use `make lib` first)
endif

#-----------------------------------------------------------------------------
# Common targets (all, clean, install, uninstall)
#-----------------------------------------------------------------------------

.PHONY: all clean install uninstall

all: bin lib

clean:
ifneq ($(wildcard $(BINDIR)/$(TARGET) $(BINDIR)/$(TARGET).exe $(BINDIR)/*.o),)
	rm -f $(BINDIR)/$(TARGET) $(BINDIR)/*.o
	@rm -f $(BINDIR)/$(TARGET).exe
endif
ifneq ($(wildcard $(LIBDIR)/lib$(TARGET).* $(LIBDIR)/*.o),)
	rm -f $(wildcard $(LIBDIR)/lib$(TARGET).*) $(LIBDIR)/*.o
endif

# requires sudo privileges
install:
ifneq ($(wildcard $(BINDIR)/$(TARGET)*),)
	@mkdir -p $(PREFIX)/bin
	cp -f $(BINDIR)/$(TARGET) $(PREFIX)/bin
endif
ifneq ($(wildcard $(LIBDIR)/lib$(TARGET).*),)
	@mkdir -p $(PREFIX)/include/primesieve/soe
	@mkdir -p $(PREFIX)/lib
	cp -f $(wildcard $(LIBDIR)/lib$(TARGET).*) $(PREFIX)/lib
	cp -f src/soe/*PrimeSieve.h $(PREFIX)/include/primesieve/soe
  ifneq ($(wildcard $(LIBDIR)/lib$(TARGET).so),)
    ifneq ($(shell command -v ldconfig 2> /dev/null),)
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
ifneq ($(wildcard $(PREFIX)/include/primesieve),)
	rm -rf $(PREFIX)/include/primesieve
endif
ifneq ($(wildcard $(PREFIX)/lib/lib$(TARGET).*),)
  ifneq ($(wildcard $(PREFIX)/lib/lib$(TARGET).so),)
		rm -f $(wildcard $(PREFIX)/lib/lib$(TARGET).so)
    ifneq ($(shell command -v ldconfig 2> /dev/null),)
		ldconfig $(PREFIX)/lib
    endif
  else
	rm -f $(wildcard $(PREFIX)/lib/lib$(TARGET).*)
  endif
endif

#-----------------------------------------------------------------------------
# `make check` runs correctness tests
#-----------------------------------------------------------------------------

.PHONY: check test

check test: bin
	$(BINDIR)/./$(TARGET) -test

#-----------------------------------------------------------------------------
# Makefile help menu
#-----------------------------------------------------------------------------

.PHONY: help

help:
	@echo ----------------------------------------------
	@echo ---------- primesieve build options ----------
	@echo ----------------------------------------------
	@echo "make                                     Build the primesieve console application using g++ (DEFAULT)"
	@echo "make CXX=icpc CXXFLAGS=\"-fast -openmp\"   Specify a custom C++ compiler, here icpc"
	@echo "make L1_DCACHE_SIZE=32                   Specify the CPU's L1 data cache size, here 32 kilobytes"
	@echo "make check                               Test primesieve for correctness"
	@echo "make clean                               Clean the output directories (bin, lib)"
	@echo "make lib                                 Build a static libprimesieve library (using g++)"
	@echo "make lib SHARED=yes                      Build a shared libprimesieve library (using g++)"
	@echo "make dist                                Create a libprimesieve distribution archive (./dist)"
	@echo "sudo make install                        Install primesieve and libprimesieve to /usr/local (Linux) or /usr (Unix)"
	@echo "sudo make install PREFIX=/path           Specify a custom installation path"
	@echo "sudo make uninstall                      Completely remove primesieve and libprimesieve"
	@echo "make help                                Print this help menu"
