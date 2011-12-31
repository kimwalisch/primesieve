##############################################################################
# Makefile for primesieve (console version)
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         10 July 2010
# Last modified:   31 December 2011
#
# Project home:    http://primesieve.googlecode.com
##############################################################################

TARGET = primesieve
SRCDIR = src/soe
MAINDIR = src/console
OUTDIR = out
CXX = g++
CXXFLAGS = -O2

# sunCC : Oracle Solaris Studio
# Sun Studio optimization flags: http://dsc.sun.com/solaris/articles/amdopt.html
ifneq ($(shell $(CXX) -V 2>&1 | head -1 | grep -iE 'sun'),)
  REMARK = You might need to export OMP_NUM_THREADS for OpenMP multi-threading.
  CXXFLAGS = +w -xopenmp -fast -xrestrict

# icpc : Intel C++ Compiler
# == Profile-guided optimization (5 percent speed up, icpc 12.0) ==
# $ make CXX=icpc "CXXFLAGS= -openmp -Wall -O2 -prof-gen"
# $ out/./primesieve 1E18 -o1E10 -t1
# $ make clean
# $ make CXX=icpc "CXXFLAGS= -openmp -Wall -O2 -ipo -prof-use"
else ifeq ($(CXX),icpc)
  REMARK = Read Makefile for instructions on profile-guided optimization.
  CXXFLAGS = -openmp -Wall -O2

# g++ : GNU Compiler Collection
else ifneq ($(shell $(CXX) --version 2>&1 | head -1 | grep -iE 'GCC|G\+\+'),)
  ifneq ($(shell $(CXX) --version 2>&1 | head -1 | grep -i apple),)
    # Apple g++, fastest executable using -fast
    CXXFLAGS = -fopenmp -Wall -fast
  else
    # GNU g++, fastest executable using -O2
    CXXFLAGS = -fopenmp -Wall -O2
  endif

# Unkown compilers (use default flags)
else
  REMARK = unkown compiler, add OpenMP flag if supported (make help).
endif

# Check if the user indicated his CPU L1/L2 cache sizes
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

ifneq ($(REMARK),)
  all: $(TARGET) remark
else
  all: $(TARGET)
endif

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

$(OUTDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUTDIR)/%.o: $(MAINDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

remark:
	@echo Remark: $(REMARK)

help:
	@echo -------------------------
	@echo -- primesieve Makefile --
	@echo -------------------------
	@echo make                                       build primesieve using g++ (DEFAULT)
	@echo make CXX=my_compiler "CXXFLAGS=options"    specify a custom C++ compiler
	@echo make L1_DCACHE_SIZE=32 L2_CACHE_SIZE=256   specify L1/L2 cache sizes per core
	@echo make clean                                 remove binary and object files
	@echo make help                                  show this help menu

.PHONY: clean
clean:
	rm $(OBJS)
	rm $(TARGET)
