##############################################################################
# Makefile for primesieve (console version)
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         10 July 2010
# Last modified:   1 January 2012
#
# Project home:    http://primesieve.googlecode.com
##############################################################################

SRCDIR = src/soe
MAINDIR = src/console
OUTDIR = out
BINARY = $(OUTDIR)/primesieve
CXX = g++
CXXFLAGS = -O2
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OUTDIR)/%.o,$(wildcard $(SRCDIR)/*.cpp))
OBJS += $(patsubst $(MAINDIR)/%.cpp,$(OUTDIR)/%.o,$(wildcard $(MAINDIR)/*.cpp))

#-----------------------------------------------------------------------------
# set CXXFLAGS for various C++ compilers (sunCC, icpc, g++, ...)
# e.g. make CXX=sunCC; sets "CXXFLAGS = +w -fast -xopenmp -xrestrict"
#-----------------------------------------------------------------------------

# sunCC : Oracle Solaris Studio
# Sun Studio optimization flags: http://dsc.sun.com/solaris/articles/amdopt.html
ifneq ($(shell $(CXX) -V 2>&1 | head -1 | grep -iE sun),)
  CXXFLAGS = +w -fast -xopenmp -xrestrict
  REMARK = you might need to export OMP_NUM_THREADS for OpenMP multi-threading.

# icpc : Intel C++ Compiler
# == Profile-guided optimization (5 percent speed up, icpc 12.0) ==
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
  REMARK = unkown compiler, add OpenMP flag if supported \(make help\).
endif

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

# create output directory
ifeq ($(shell [ -d $(OUTDIR) ] && echo exists),)
  $(shell mkdir $(OUTDIR))
endif

all: build remark

help:
	@echo                  -----------------------------------------
	@echo                  ---------- primesieve Makefile ----------
	@echo                  -----------------------------------------
	@echo make                                       build primesieve using g++ (DEFAULT)
	@echo make CXX=my_compiler "CXXFLAGS=options"    specify a custom C++ compiler
	@echo make L1_DCACHE_SIZE=32 L2_CACHE_SIZE=256   specify L1/L2 cache sizes per core
	@echo make clean                                 remove binary and object files
	@echo make help                                  show this help menu

build: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(BINARY) $(OBJS)

$(OUTDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OUTDIR)/%.o: $(MAINDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

remark:
ifneq ($(REMARK),)
	@echo Remark: $(REMARK)
endif

.PHONY: clean
clean:
	rm -f $(BINARY) $(OUTDIR)/*.o
