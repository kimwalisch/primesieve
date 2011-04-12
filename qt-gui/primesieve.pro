# -------------------------------------------------
# primesieve icon & stdint.h
# -------------------------------------------------
win* {
    RC_FILE = icons/win/primesieve.rc
    # MSVC (prior 2010!?) does not support ISO c99 stdint.h, thus I
    # provide my own version
    INCLUDEPATH += ../thirdparty
}
macx {
    RC_FILE = icons/osx/primesieve.icns
}
# -------------------------------------------------
# Compiler options & OpenMP for MSVC, GCC and ICC
# -------------------------------------------------
DEFINES += NDEBUG \
    __STDC_CONSTANT_MACROS \
    __STDC_LIMIT_MACROS
win* {
    *msvc* {
        QMAKE_CXXFLAGS += /openmp
    }
    *icc* {
        QMAKE_CXXFLAGS += /Qopenmp
    }
}
*icc* {
    unix {
        QMAKE_CXXFLAGS += -openmp
        QMAKE_LFLAGS   += -openmp
    }
}
*g++* {
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS   += -fopenmp
    # -mpopcnt requires GCC 4.4 (from 2009) or above, Apple GCC is
    # still version 4.2.1 in 2011
    !macx {
        QMAKE_CXXFLAGS += -mpopcnt
    }
    macx {
        # Apple GCC performs better with -fast than -O2
        QMAKE_CXXFLAGS += -fast
    }
}
# -------------------------------------------------
# Project created by QtCreator
# -------------------------------------------------
TARGET = primesieve
TEMPLATE = app
HEADERS += ../expr/ExpressionParser.h
SOURCES += src/main.cpp \
    src/PrimeSieveGUI.cpp \
    src/PrimeSieveGUI_menu.cpp \
    src/PrimeSieveProcess.cpp
HEADERS += src/PrimeSieveGUI.h \
    src/PrimeSieveProcess.h \
    src/PrimeSieveGUI_const.h
SOURCES += ../soe/EratBig.cpp \
    ../soe/EratSmall.cpp \
    ../soe/PrimeNumberFinder.cpp \
    ../soe/PrimeSieve.cpp \
    ../soe/ParallelPrimeSieve.cpp \
    ../soe/SieveOfEratosthenes.cpp \
    ../soe/EratMedium.cpp \
    ../soe/PrimeNumberGenerator.cpp \
    ../soe/ResetSieve.cpp \
    ../soe/WheelFactorization.cpp
HEADERS += ../soe/EratBig.h \
    ../soe/EratSmall.h \
    ../soe/PrimeNumberFinder.h \
    ../soe/PrimeSieve.h \
    ../soe/ParallelPrimeSieve.h \
    ../soe/SieveOfEratosthenes.h \
    ../soe/EratMedium.h \
    ../soe/PrimeNumberGenerator.h \
    ../soe/ResetSieve.h \
    ../soe/WheelFactorization.h \
    ../soe/pmath.h \
    ../soe/bits.h \
    ../soe/settings.h \
    ../soe/cpuid.h \
    ../soe/cpuid_gcc451.h
FORMS += forms/PrimeSieveGUI.ui
