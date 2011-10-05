##############################################################################
# Makefile for primesieve (console version)
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         10 July 2010
# Last modified:   05 October 2011
#
# Project home:    http://primesieve.googlecode.com
##############################################################################

TARGET = primesieve
SRCDIR = src/soe
MAINDIR = src/console
OUTDIR = out
CXX = g++

# sunCC : Oracle Solaris Studio
# Sun Studio optimization flags: http://dsc.sun.com/solaris/articles/amdopt.html
#
# == Profile guided optimization (3 percent speed up, sunCC 12.2) ==
# CXXFLAGS = +w -xopenmp -fast -xalias_level=compatible -xrestrict -xipo=2 -xprofile=collect:./feedback
# make CXX=sunCC
# out/./primesieve 1e18 1e18+1e10 -t1
# make clean
# CXXFLAGS = +w -xopenmp -fast -xalias_level=compatible -xrestrict -xipo=2 -xprofile=use:./feedback
# make CXX=sunCC
ifneq ($(shell $(CXX) -V 2>&1 | head -1 | grep -iE 'sun'),)
  $(warning primesieve: You might need to export OMP_NUM_THREADS for OpenMP multi-threading)
  $(warning )
  CXXFLAGS = +w -xopenmp -fast -xalias_level=compatible -xrestrict
 
# icpc : Intel C++ Compiler
#
# == Profile guided optimization (5 percent speed up, icpc 12.0) ==
# CXXFLAGS = -openmp -Wall -fast -prof-gen
# make CXX=icpc
# out/./primesieve 1e18 1e18+1e10 -t1
# make clean
# CXXFLAGS = -openmp -Wall -fast -prof-use
# make CXX=icpc
else ifeq ($(CXX),icpc)
  CXXFLAGS = -openmp -Wall -O2

# g++ : GNU Compiler Collection
# Compilers: GNU g++, MinGW g++, Apple g++, llvm-g++, ...
else ifneq ($(shell $(CXX) --version 2>&1 | head -1 | grep -iE 'GCC|G\+\+'),)
  ifneq ($(shell $(CXX) --version 2>&1 | head -1 | grep -i apple),)
    # Apple g++ produces the fastest executable using the -fast option
    CXXFLAGS += -fopenmp -Wall -fast
  else
    # GNU g++ produces the fastest executable using the -O2 option
    # Profile guided optimization (-fprofile-generate, -fprofile-use) does
    # not speed things up
    CXXFLAGS += -fopenmp -Wall -O2
  endif

# Unkown compilers
else
  $(warning primesieve: Unkown compiler, add OpenMP flag if supported)
  $(warning )
  CXXFLAGS = -O2
endif

# Check if the user has given his CPU L1/L2 cache sizes
ifneq ($(L1_DCACHE_SIZE),)
  CXXFLAGS += -DL1_DCACHE_SIZE=$(L1_DCACHE_SIZE)
endif
ifneq ($(L2_CACHE_SIZE),)
  CXXFLAGS += -DL2_CACHE_SIZE=$(L2_CACHE_SIZE)
endif

# Generate list of object files
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OUTDIR)/%.o,$(wildcard $(SRCDIR)/*.cpp))
OBJS += $(patsubst $(MAINDIR)/%.cpp,$(OUTDIR)/%.o,$(wildcard $(MAINDIR)/*.cpp))

TARGET := $(OUTDIR)/$(TARGET)

# Create output directory if it does not exist
ifeq ($(wildcard $(OUTDIR)/),)
$(shell mkdir -p $(OUTDIR))
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

$(OUTDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUTDIR)/%.o: $(MAINDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm $(OBJS)
	rm $(TARGET)
