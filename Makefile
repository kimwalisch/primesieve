##############################################################################
# Makefile for primesieve (console version)
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         10 July 2010 
# Last modified:   7 February 2011
#
# Project home:    http://primesieve.googlecode.com
##############################################################################

TARGET = primesieve
SRCDIR = soe
MAINDIR = console
OUTDIR = out
STDINT_MACROS = -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS
CXX = g++

# GNU GCC compiler
ifneq ($(shell $(CXX) --version 2>&1 | head -1 | grep -iE 'GCC|G++'),)
  CXXFLAGS += -fopenmp
  # Mac OS X
  ifneq ($(shell $(CXX) --version 2>&1 | head -1 | grep -i apple),)
    CXXFLAGS += -fast
  else
    CXXFLAGS += -O3
  endif
  GCC_MAJOR := $(shell $(CXX) -dumpversion 2>&1 | cut -d'.' -f1)
  GCC_MINOR := $(shell $(CXX) -dumpversion 2>&1 | cut -d'.' -f2)
  GCC_VERSION := $(shell echo $$(($(GCC_MAJOR)*10+$(GCC_MINOR))))
  # Add POPCNT (SSE 4.2) support if using GCC >= 4.4
  CXXFLAGS += $(shell if [ $(GCC_VERSION) -ge 44 ]; then echo -mpopcnt; fi)
  CXXFLAGS += -DNDEBUG

# Intel C++ Compiler
else ifeq ($(CXX),icpc)
  CXXFLAGS += -openmp -fast -DNDEBUG

# Oracle Solaris Studio (former Sun Studio)
else ifeq ($(CXX),sunCC)
  CXXFLAGS += -xopenmp -O5 -xarch=sse4_2 -xipo -xrestrict -xalias_level=compatible -DNDEBUG

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
	$(CXX) $(CXXFLAGS) $(STDINT_MACROS) -c -o $@ $<

$(OUTDIR)/%.o: $(MAINDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(STDINT_MACROS) -c -o $@ $<

.PHONY: clean
clean:
	rm $(OBJS)
	rm $(TARGET)

