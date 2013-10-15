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
  src/ExpressionParser.hpp \
  src/PrimeSieveGUI.hpp \
  src/PrimeSieveGUI_const.hpp \
  src/PrimeSieveProcess.hpp

# ---------------------------------------------------------
# Sieve of Eratosthenes sources (src/soe)
# ---------------------------------------------------------

INCLUDEPATH += ../../../include

SOURCES += \
  ../../soe/EratBig.cpp \
  ../../soe/EratMedium.cpp \
  ../../soe/EratSmall.cpp \
  ../../soe/ParallelPrimeSieve.cpp \
  ../../soe/popcount.cpp \
  ../../soe/PreSieve.cpp \
  ../../soe/PrimeFinder.cpp \
  ../../soe/PrimeGenerator.cpp \
  ../../soe/PrimeSieve-nthPrime.cpp \
  ../../soe/PrimeSieve.cpp \
  ../../soe/SieveOfEratosthenes.cpp \
  ../../soe/WheelFactorization.cpp

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
# Add OpenMP compiler flag
# ---------------------------------------------------------

*msvc* {
  QMAKE_CXXFLAGS += /openmp /EHsc
}

*g++* {
  QMAKE_CXXFLAGS += -fopenmp
  QMAKE_LFLAGS   += -fopenmp
}

*clang* {
  QMAKE_CXXFLAGS += -fopenmp
  QMAKE_LFLAGS   += -fopenmp
}

*icc* {
  win* {
    QMAKE_CXXFLAGS += /Qopenmp /EHsc
  }
  unix {
    QMAKE_CXXFLAGS += -openmp
    QMAKE_LFLAGS   += -openmp
  }
}
