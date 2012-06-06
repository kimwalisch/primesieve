# -------------------------------------------------
# primesieve icon & stdint.h
# -------------------------------------------------
win* {
    RC_FILE = icons/win/primesieve.rc
    INCLUDEPATH += ../thirdparty
}
macx {
    RC_FILE = icons/osx/primesieve.icns
}
# -------------------------------------------------
# Compiler options for MSVC, GCC, ICC, LLVM
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
*llvm* {
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS   += -fopenmp
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
    ../soe/PreSieve.cpp \
    ../soe/WheelFactorization.cpp
HEADERS += ../soe/EratSmall.h \
    ../soe/EratBig.h \
    ../soe/PrimeNumberFinder.h \
    ../soe/PrimeSieve.h \
    ../soe/ParallelPrimeSieve.h \
    ../soe/SieveOfEratosthenes.h \
    ../soe/SieveOfEratosthenes-inline.h \
    ../soe/EratMedium.h \
    ../soe/PrimeNumberGenerator.h \
    ../soe/PreSieve.h \
    ../soe/WheelFactorization.h \
    ../soe/GENERATE.h \
    ../soe/imath.h \
    ../soe/popcount.h \
    ../soe/bits.h \
    ../soe/config.h
