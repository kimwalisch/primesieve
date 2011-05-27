##############################################################################
# Makefile for primesieve (console version)
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         10 July 2010 
# Last modified:   27 May 2011
#
# Project home:    http://primesieve.googlecode.com
##############################################################################

TARGET = primesieve
SRCDIR = soe
MAINDIR = console
OUTDIR = out
CXX = g++

# sunCC : Oracle Solaris Studio
ifneq ($(shell $(CXX) -V 2>&1 | head -1 | grep -iE 'sun'),)
  $(warning $(CXX): you might need to set OMP_NUM_THREADS for OpenMP)
  CXXFLAGS += -xopenmp +w -fast
  CXXFLAGS += -xipo -xrestrict -xalias_level=compatible

# icpc : Intel C++ Compiler
else ifeq ($(CXX),icpc)
  CXXFLAGS += -openmp -Wall -fast

# g++ : GNU Compiler Collection
else ifneq ($(shell $(CXX) --version 2>&1 | head -1 | grep -iE 'GCC|G\+\+'),)
  # Apple - Mac OS
  ifneq ($(shell $(CXX) --version 2>&1 | head -1 | grep -i apple),)
    CXXFLAGS += -fopenmp -Wall -fast
  else
    CXXFLAGS += -fopenmp -Wall -O2
    # Add SSE4.2 POPCNT flag if using GCC >= 4.4
    GCC_MAJ := $(shell $(CXX) -dumpversion 2>&1 | cut -d'.' -f1)
    GCC_MIN := $(shell $(CXX) -dumpversion 2>&1 | cut -d'.' -f2)
    GCC_VER := $(shell echo $$(($(GCC_MAJ)*10+$(GCC_MIN))))
    ifneq ($(shell if [ $(GCC_VER) -ge 44 ]; then echo yes; fi),)
      CXXFLAGS  += -mpopcnt
    endif
  endif

# Other compilers
else
  $(warning $(CXX): add OpenMP and SSE4.2 flags if supported)
  CXXFLAGS += -O2
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
