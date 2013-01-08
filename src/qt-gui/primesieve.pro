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
    ../soe/EratSmall.cpp \
    ../soe/PrimeNumberFinder.cpp \
    ../soe/PrimeSieve.cpp \
    ../soe/ParallelPrimeSieve.cpp \
    ../soe/SieveOfEratosthenes.cpp \
    ../soe/EratMedium.cpp \
    ../soe/PreSieve.cpp \
    ../soe/WheelFactorization.cpp \
    ../soe/popcount.cpp

HEADERS += ../soe/EratSmall.h \
    ../soe/EratBig.h \
    ../soe/PrimeNumberFinder.h \
    ../soe/PrimeSieve.h \
    ../soe/PrimeSieveCallback.h \
    ../soe/primesieve_error.h \
    ../soe/ParallelPrimeSieve.h \
    ../soe/SieveOfEratosthenes.h \
    ../soe/SieveOfEratosthenes-GENERATE.h \
    ../soe/SieveOfEratosthenes-inline.h \
    ../soe/EratMedium.h \
    ../soe/PrimeNumberGenerator.h \
    ../soe/PreSieve.h \
    ../soe/WheelFactorization.h \
    ../soe/bits.h \
    ../soe/config.h \
    ../soe/imath.h \
    ../soe/endiansafe_cast.h \
    ../soe/openmp_lock.h \
    ../soe/popcount.h \
    ../soe/toString.h \
    ../soe/SynchronizeThreads.h

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
