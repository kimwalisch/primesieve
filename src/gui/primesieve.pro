# ---------------------------------------------------------
# primesieve GUI project settings
# ---------------------------------------------------------

TARGET = primesieve
TEMPLATE = app
FORMS += forms/PrimeSieveGUI.ui

# ---------------------------------------------------------
# Qt modules: core, gui and widgets (Qt > 4)
# ---------------------------------------------------------

QT_VER = $$QT_VERSION
QT_VER = $$split(QT_VER, ".")
QT_MAJ = $$member(QT_VER, 0)
!contains(QT_MAJ, 4) {
  QT += core gui widgets
}

# ---------------------------------------------------------
# primesieve GUI application sources
# ---------------------------------------------------------

SOURCES += \
  src/main.cpp \
  src/PrimeSieveGUI.cpp \
  src/PrimeSieveGUI_menu.cpp \
  src/PrimeSieveProcess.cpp

HEADERS += \
  ../../include/primesieve/calculator.hpp \
  src/PrimeSieveGUI.hpp \
  src/PrimeSieveProcess.hpp

# ---------------------------------------------------------
# Sieve of Eratosthenes source code
# ---------------------------------------------------------

INCLUDEPATH += ../../include

SOURCES += \
  ../api.cpp \
  ../CpuInfo.cpp \
  ../EratBig.cpp \
  ../EratMedium.cpp \
  ../EratSmall.cpp \
  ../iterator.cpp \
  ../IteratorHelper.cpp \
  ../MemoryPool.cpp \
  ../PrimeGenerator.cpp \
  ../nthPrime.cpp \
  ../ParallelSieve.cpp \
  ../popcount.cpp \
  ../PreSieve.cpp \
  ../PrintPrimes.cpp \
  ../SievingPrimes.cpp \
  ../PrimeSieve.cpp \
  ../Erat.cpp \
  ../LookupTables.cpp

# ---------------------------------------------------------
# primesieve icon file
# ---------------------------------------------------------

win* {
  RC_FILE = icons/win/primesieve.rc
}
macx {
  RC_FILE = icons/osx/primesieve.icns
}

# ---------------------------------------------------------
# Add compiler flags
# ---------------------------------------------------------

*msvc* {
  QMAKE_CXXFLAGS += /EHsc
}

*g++* {
  QMAKE_CXXFLAGS += -std=c++14 -Wno-implicit-fallthrough
}

*clang* {
  QMAKE_CXXFLAGS += -std=c++14 -Wno-implicit-fallthrough
}

*icc* {
  win* {
    QMAKE_CXXFLAGS += /EHsc
  }
}
