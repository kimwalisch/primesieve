##############################################################################
# Makefile for primesieve (console version)
#
# Author:          Kim Walisch
# Contact:         kim.walisch@gmail.com
# Created:         16 November 2010
# Last modified:   16 November 2010
#
# Project home:    http://code.google.com/p/primesieve
##############################################################################

TARGET = primesieve
SRCDIR = src
OUTDIR = out
STDINT_MACROS = -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS
CXX = g++

# Set the compiler flags
ifeq ($(CXX),g++)
    IS_OSX_GCC := $(shell $(CXX) --version 2>&1 | head -1 | grep -i apple)
    # Mac OS X operating system
    ifneq ($(IS_OSX_GCC),)
        CXXFLAGS += -fast
    # Linux & Unix operating systems
    else
        CXXFLAGS += -O3
    endif
    GCC_MAJOR := $(shell $(CXX) -dumpversion 2>&1 | cut -d'.' -f1)
    GCC_MINOR := $(shell $(CXX) -dumpversion 2>&1 | cut -d'.' -f2)
    GCC_VERSION := $(shell echo $$(($(GCC_MAJOR)*10+$(GCC_MINOR))))
    # Add SSE 4.2 support if using GCC >= 4.4
    CXXFLAGS += $(shell if [ $(GCC_VERSION) -ge 44 ]; then echo -msse4.2; fi)
    # Common GCC flags
    CXXFLAGS += -funroll-all-loops -DNDEBUG
else ifeq ($(CXX),icpc)
    CXXFLAGS += -fast -DNDEBUG
else ifeq ($(CXX),sunCC)
    CXXFLAGS += -O5 -xarch=sse4_2 -xipo -xrestrict -xalias_level=compatible -DNDEBUG
else
    # Default flags for other compilers
    CXXFLAGS += -O2
endif

OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OUTDIR)/%.o,$(wildcard $(SRCDIR)/*.cpp))
TARGET := $(OUTDIR)/$(TARGET)

# Create the output directory if it does not exist
ifeq ($(wildcard $(OUTDIR)/),)
$(shell mkdir -p $(OUTDIR))
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

$(OUTDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(STDINT_MACROS) -c -o $@ $<

.PHONY: clean
clean:
	rm $(OBJS)
	rm $(TARGET)
