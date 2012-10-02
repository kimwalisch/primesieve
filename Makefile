##############################################################################
# GNU Makefile for the primesieve console application (read doc/INSTALL)
#              and the primesieve C++ library         (read doc/LIBPRIMESIEVE)
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         10 July 2010
# Last modified:   2 October 2012
#
# Project home:    http://primesieve.googlecode.com
##############################################################################

TARGET    := primesieve
CXX       := g++
CXXFLAGS  := -Wall -O2
NO_STDOUT := 1> /dev/null
NO_STDERR := 2> /dev/null
NO_OUTPUT := $(NO_STDOUT) $(NO_STDERR)
BINDIR    := bin
LIBDIR    := lib
DISTDIR   := dist
EXDIR     := examples

SOE_SOURCES:= \
  src/soe/PrimeSieve.cpp \
  src/soe/ParallelPrimeSieve.cpp \
  src/soe/SieveOfEratosthenes.cpp \
  src/soe/PrimeNumberFinder.cpp \
  src/soe/PreSieve.cpp \
  src/soe/EratSmall.cpp \
  src/soe/EratMedium.cpp \
  src/soe/EratBig.cpp \
  src/soe/WheelFactorization.cpp

SOE_HEADERS := \
  src/soe/bits.h \
  src/soe/config.h \
  src/soe/EratBig.h \
  src/soe/EratMedium.h \
  src/soe/EratSmall.h \
  src/soe/GENERATE.h \
  src/soe/imath.h \
  src/soe/openmp_RAII.h \
  src/soe/ParallelPrimeSieve.h \
  src/soe/popcount.h \
  src/soe/PreSieve.h \
  src/soe/PrimeNumberFinder.h \
  src/soe/PrimeNumberGenerator.h \
  src/soe/PrimeSieve.h \
  src/soe/SieveOfEratosthenes.h \
  src/soe/SieveOfEratosthenes-inline.h \
  src/soe/toString.h \
  src/soe/WheelFactorization.h

MAIN_DEPENDENCIES := \
  src/parser/ExpressionParser.h \
  src/soe/PrimeSieve.h \
  src/soe/ParallelPrimeSieve.h \
  src/test/test.h

TEST_DEPENDENCIES := \
  src/test/test.h \
  src/soe/PrimeSieve.h \
  src/soe/ParallelPrimeSieve.h

#-----------------------------------------------------------------------------
# Add -fopenmp if GCC version >= 4.4
#-----------------------------------------------------------------------------

ifneq ($(shell $(CXX) --version $(NO_STDERR) | head -1 | grep -iE 'GCC|G\+\+'),)
  MAJOR := $(shell $(CXX) -dumpversion | cut -d'.' -f1)
  MINOR := $(shell $(CXX) -dumpversion | cut -d'.' -f2)
  GCC_VERSION := $(shell expr $(MAJOR) '*' 100 + $(MINOR) $(NO_STDERR))
  ifneq ($(shell expr $(GCC_VERSION) '>=' 404 $(NO_OUTPUT) && \
                 echo 'GCC >= 4.4'),)
    CXXFLAGS += -fopenmp
  endif
endif

#-----------------------------------------------------------------------------
# Add the CPU's L1 data cache size (in kilobytes) to CXXFLAGS
#-----------------------------------------------------------------------------

L1_DCACHE_BYTES := $(shell getconf LEVEL1_DCACHE_SIZE $(NO_STDERR))
ifeq ($(L1_DCACHE_BYTES),)
  L1_DCACHE_BYTES := $(shell sysctl hw.l1dcachesize $(NO_STDERR) | sed -e 's/^.* //')
endif

ifneq ($(shell expr $(L1_DCACHE_BYTES) '-' $(L1_DCACHE_BYTES) '+' 1 $(NO_OUTPUT) && \
               echo is a number),)
  L1_DCACHE_SIZE := $(shell expr $(L1_DCACHE_BYTES) '/' 1024)
  ifeq ($(shell expr $(L1_DCACHE_SIZE) '>=' 8 && \
                expr $(L1_DCACHE_SIZE) '<=' 4096 && \
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
# `make lib`            -> libprimesieve.a
# `make lib SHARED=yes` -> libprimesieve.(so|dylib)
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
    ifeq ($(shell uname | grep -iE 'mingw|cygwin'),)
      FPIC := -fPIC
    endif
  endif
endif

#-----------------------------------------------------------------------------
# Build the primesieve console application
#-----------------------------------------------------------------------------

BIN_OBJECTS := \
  $(addprefix $(BINDIR)/, \
    $(subst .cpp,.o, \
      $(notdir $(SOE_SOURCES)))) \
  $(BINDIR)/main.o \
  $(BINDIR)/test.o

.PHONY: bin bin_dir bin_obj

bin: bin_dir bin_obj

bin_dir:
	@mkdir -p $(BINDIR)

bin_obj: $(BIN_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(BINDIR)/$(TARGET) $^

$(BINDIR)/main.o: src/application/main.cpp $(MAIN_DEPENDENCIES)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINDIR)/test.o: src/test/test.cpp $(TEST_DEPENDENCIES)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINDIR)/%.o: src/soe/%.cpp $(SOE_HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

#-----------------------------------------------------------------------------
# Build libprimesieve
#-----------------------------------------------------------------------------

LIB_CXXFLAGS := $(strip $(CXXFLAGS) $(FPIC))
LIB_OBJECTS  := \
  $(addprefix $(LIBDIR)/, \
    $(subst .cpp,.o, \
      $(notdir $(SOE_SOURCES))))

.PHONY: lib lib_dir lib_obj

lib: lib_dir lib_obj

lib_dir:
	@mkdir -p $(LIBDIR)

lib_obj: $(LIB_OBJECTS)
ifneq ($(SHARED),)
	$(CXX) $(LIB_CXXFLAGS) $(SOFLAG) -o $(LIBDIR)/$(LIBRARY) $^
else
	ar rcs $(LIBDIR)/$(LIBRARY) $^
endif

$(LIBDIR)/%.o: src/soe/%.cpp $(SOE_HEADERS)
	$(CXX) $(LIB_CXXFLAGS) -c $< -o $@

#-----------------------------------------------------------------------------
# Compile the example programs (./examples)
#-----------------------------------------------------------------------------

.PHONY: examples

examples: $(basename $(wildcard $(EXDIR)/*.cpp))

$(EXDIR)/%: $(EXDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ -l$(TARGET)

#-----------------------------------------------------------------------------
# Create a libprimesieve distribution archive (./dist)
#-----------------------------------------------------------------------------

.PHONY: dist

dist:
	@mkdir -p $(DISTDIR)/$(TARGET)/soe
	cp -f $(LIBDIR)/lib$(TARGET).* $(DISTDIR)
	cp -f src/soe/*PrimeSieve.h $(DISTDIR)/$(TARGET)/soe

#-----------------------------------------------------------------------------
# `make check` runs correctness tests
#-----------------------------------------------------------------------------

.PHONY: check test

check test: bin
	$(BINDIR)/./$(TARGET) -test

#-----------------------------------------------------------------------------
# Common targets (all, clean, install, uninstall)
#-----------------------------------------------------------------------------

.PHONY: all clean install uninstall

all: bin lib

EXAMPLE_PROGRAMS = $(shell find $(EXDIR) -mindepth 1 -maxdepth 1 \
  ! -name '*.cpp' ! -name INSTALL $(NO_STDERR))

clean:
	rm -rf $(BINDIR) $(LIBDIR) $(DISTDIR) $(EXAMPLE_PROGRAMS)

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
ifneq ($(wildcard $(PREFIX)/include/primesieve),)
	rm -rf $(PREFIX)/include/primesieve
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
	@echo "make                                     Build the primesieve console application using g++ (DEFAULT)"
	@echo "make CXX=icpc CXXFLAGS=\"-fast -openmp\"   Specify a custom C++ compiler, here icpc"
	@echo "make L1_DCACHE_SIZE=32                   Specify the CPU's L1 data cache size, here 32 kilobytes"
	@echo "make check                               Test primesieve for correctness"
	@echo "make clean                               Clean the output directories (bin, lib)"
	@echo "make lib                                 Build a static libprimesieve library (using g++)"
	@echo "make lib SHARED=yes                      Build a shared libprimesieve library (using g++)"
	@echo "make dist                                Create a libprimesieve distribution archive (./dist)"
	@echo "make examples                            Build the example programs in ./examples"
	@echo "sudo make install                        Install primesieve and libprimesieve to /usr/local (Linux) or /usr (Unix)"
	@echo "sudo make install PREFIX=/path           Specify a custom installation path"
	@echo "sudo make uninstall                      Completely remove primesieve and libprimesieve"
	@echo "make help                                Print this help menu"
