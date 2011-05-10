##############################################################################
# Makefile for primesieve (console version)
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         10 July 2010 
# Last modified:   10 May 2011
#
# Project home:    http://primesieve.googlecode.com
##############################################################################

TARGET = primesieve
SRCDIR = soe
MAINDIR = console
OUTDIR = out
CXX = g++

# Oracle Solaris Studio (former Sun Studio)
ifeq ($(CXX),sunCC)
  CXXFLAGS += +w -xopenmp -O5 -xarch=sse4_2
  CXXFLAGS += -xipo -xrestrict -xalias_level=compatible

# Intel C++ Compiler
else ifeq ($(CXX),icpc)
  CXXFLAGS += -Wall -openmp -fast

# GCC, the GNU Compiler Collection
else ifneq ($(shell $(CXX) --version 2>&1 | head -1 | grep -iE 'GCC|G\+\+'),)
  CXXFLAGS += -fopenmp
  # Mac OS X
  ifneq ($(shell $(CXX) --version 2>&1 | head -1 | grep -i apple),)
    CXXFLAGS += -fast
  else
    CXXFLAGS += -O2 -Wall
  endif
  GCC_MAJOR := $(shell $(CXX) -dumpversion 2>&1 | cut -d'.' -f1)
  GCC_MINOR := $(shell $(CXX) -dumpversion 2>&1 | cut -d'.' -f2)
  GCC_VERSION := $(shell echo $$(($(GCC_MAJOR)*10+$(GCC_MINOR))))
  # Add POPCNT (SSE 4.2) support if using GCC >= 4.4
  CXXFLAGS += $(shell if [ $(GCC_VERSION) -ge 44 ]; then echo -mpopcnt; fi)

# Unkown compiler, add OpenMP flag if supported
else
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
