# -------------------------------------------------
# primesieve GUI project settings
# -------------------------------------------------

TARGET = primesieve
TEMPLATE = app
FORMS += forms/PrimeSieveGUI.ui

# -------------------------------------------------
# Qt modules: core, gui and widgets (Qt > 4)
# -------------------------------------------------

QT_VER = $$QT_VERSION
QT_VER = $$split(QT_VER, ".")
QT_MAJ = $$member(QT_VER, 0)
!contains(QT_MAJ, 4) {
  QT += core gui widgets
}

# -------------------------------------------------
# primesieve GUI application sources (src/qt-gui)
# -------------------------------------------------

SOURCES += \
  src/main.cpp \
  src/PrimeSieveGUI.cpp \
  src/PrimeSieveGUI_menu.cpp \
  src/PrimeSieveProcess.cpp

HEADERS += \
  src/PrimeSieveGUI.h \
  src/PrimeSieveGUI_const.h \
  src/PrimeSieveProcess.h

# -------------------------------------------------
# Arithmetic Expression parser (src/parser)
# -------------------------------------------------    

HEADERS += ../parser/ExpressionParser.h

# -------------------------------------------------
# Sieve of Eratosthenes sources (src/soe)
# -------------------------------------------------

SOURCES += \
  ../soe/EratBig.cpp \
  ../soe/EratMedium.cpp \
  ../soe/EratSmall.cpp \
  ../soe/ParallelPrimeSieve.cpp \
  ../soe/popcount.cpp \
  ../soe/PreSieve.cpp \
  ../soe/PrimeFinder.cpp \
  ../soe/PrimeNumberGenerator.cpp \
  ../soe/PrimeSieve.cpp \
  ../soe/SieveOfEratosthenes.cpp \
  ../soe/WheelFactorization.cpp

HEADERS += \
  ../soe/bits.h \
  ../soe/config.h \
  ../soe/endiansafe_cast.h \
  ../soe/EratBig.h \
  ../soe/EratMedium.h \
  ../soe/EratSmall.h \
  ../soe/imath.h \
  ../soe/ParallelPrimeSieve.h \
  ../soe/ParallelPrimeSieve-lock.h \
  ../soe/popcount.h \
  ../soe/PreSieve.h \
  ../soe/PrimeFinder.h \
  ../soe/PrimeNumberGenerator.h \
  ../soe/PrimeSieve.h \
  ../soe/primesieve_error.h \
  ../soe/PrimeSieveCallback.h \
  ../soe/PrimeSieve-lock.h \
  ../soe/SieveOfEratosthenes.h \
  ../soe/SieveOfEratosthenes-GENERATE.h \
  ../soe/SieveOfEratosthenes-inline.h \
  ../soe/toString.h \
  ../soe/WheelFactorization.h

# -------------------------------------------------
# primesieve icon file
# -------------------------------------------------

win* {
  RC_FILE = icons/win/primesieve.rc
}
macx {
  RC_FILE = icons/osx/primesieve.icns
}

# -------------------------------------------------
# Add OpenMP compiler flag
# -------------------------------------------------

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
