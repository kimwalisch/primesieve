##############################################################################
# Makefile for primesieve (console version)
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         10 July 2010 
# Last modified:   11 January 2011
#
# Project home:    http://primesieve.googlecode.com
##############################################################################

TARGET = primesieve
SRCDIR = src
MAINDIR = console
OUTDIR = out
STDINT_MACROS = -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS
CXX = g++

# GNU GCC compiler
ifeq ($(CXX),g++)
  IS_OSX_GCC := $(shell $(CXX) --version 2>&1 | head -1 | grep -i apple)
  # Mac OS X
  ifneq ($(IS_OSX_GCC),)
    CXXFLAGS += -fast
  # Linux & Unix
  else
    CXXFLAGS += -O3
  endif
  GCC_MAJOR := $(shell $(CXX) -dumpversion 2>&1 | cut -d'.' -f1)
  GCC_MINOR := $(shell $(CXX) -dumpversion 2>&1 | cut -d'.' -f2)
  GCC_VERSION := $(shell echo $$(($(GCC_MAJOR)*10+$(GCC_MINOR))))
  # Add POPCNT (SSE 4.2) support if using GCC >= 4.4
  CXXFLAGS += $(shell if [ $(GCC_VERSION) -ge 44 ]; then echo -mpopcnt; fi)
  # trusted compiler diable assertions
  CXXFLAGS += -DNDEBUG
# Intel C++ Compiler
else ifeq ($(CXX),icpc)
  # trusted compiler diable assertions
  CXXFLAGS += -fast -DNDEBUG
# Oracle Solaris Studio (former Sun Studio)
else ifeq ($(CXX),sunCC)
  # trusted compiler diable assertions
  CXXFLAGS += -O5 -xarch=sse4_2 -xipo -xrestrict -xalias_level=compatible -DNDEBUG
else
  # Unkown compiler keep assertions enabled!
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
	$(CXX) $(OBJS) -o $(TARGET)

$(OUTDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(STDINT_MACROS) -c -o $@ $<

$(OUTDIR)/%.o: $(MAINDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(STDINT_MACROS) -c -o $@ $<

.PHONY: clean
clean:
	rm $(OBJS)
	rm $(TARGET)
