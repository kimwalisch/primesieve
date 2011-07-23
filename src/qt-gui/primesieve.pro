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
win* {
    *msvc* {
        QMAKE_CXXFLAGS += /openmp /EHsc
    }
    *icc* {
        QMAKE_CXXFLAGS += /Qopenmp /EHsc
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
    macx {
        # Apple GCC performs best with -fast
        QMAKE_CXXFLAGS += -fast
    }
}
# -------------------------------------------------
# Project created by QtCreator
# -------------------------------------------------
TARGET = primesieve
TEMPLATE = app
FORMS += forms/PrimeSieveGUI.ui
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
    ../soe/PreSieve.cpp \
    ../soe/WheelFactorization.cpp
HEADERS += ../soe/EratBig.h \
    ../soe/EratSmall.h \
    ../soe/PrimeNumberFinder.h \
    ../soe/PrimeSieve.h \
    ../soe/ParallelPrimeSieve.h \
    ../soe/SieveOfEratosthenes.h \
    ../soe/EratMedium.h \
    ../soe/PrimeNumberGenerator.h \
    ../soe/PreSieve.h \
    ../soe/WheelFactorization.h \
    ../soe/imath.h \
    ../soe/bithacks.h \
    ../soe/defs.h \
