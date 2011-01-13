# -------------------------------------------------
# primesieve icon & stdint.h
# -------------------------------------------------
win32 {
  RC_FILE = icons/win/primesieve.rc
  # MSVC (prior 2010!?) does not support ISO c99 stdint.h, thus I 
  # provide my own version
  INCLUDEPATH += ../thirdparty
}
# might be necessary in the future
win64 {
  RC_FILE = icons/win/primesieve.rc
  INCLUDEPATH += ../thirdparty
}
macx {
  RC_FILE = icons/osx/primesieve.icns
}
# -------------------------------------------------
# Project created by QtCreator
# -------------------------------------------------
TARGET = primesieve
TEMPLATE = app
SOURCES += ../thirdparty/eval11/ArithmeticExpression.cpp \
    ../thirdparty/eval11/evalwrap.c \
    ../thirdparty/eval11/evalkern.c
HEADERS += ../thirdparty/eval11/ArithmeticExpression.h \
    ../thirdparty/eval11/evaldefs.h \
    ../thirdparty/eval11/evalkern.h
SOURCES += src/main.cpp \
    src/PrimeSieveGUI.cpp \
    src/PrimeSieveGUI_sieve.cpp \
    src/PrimeSieveGUI_menu.cpp \
    src/PrimeSieveProcess.cpp
HEADERS += src/PrimeSieveGUI.h \
    src/PrimeSieveProcess.h \
    src/PrimeSieveGUI_const.h
FORMS += forms/PrimeSieveGUI.ui
DEFINES += NDEBUG \
    __STDC_CONSTANT_MACROS \
    __STDC_LIMIT_MACROS
SOURCES += ../src/EratBig.cpp \
    ../src/EratSmall.cpp \
    ../src/PrimeNumberFinder.cpp \
    ../src/PrimeSieve.cpp \
    ../src/SieveOfEratosthenes.cpp \
    ../src/EratMedium.cpp \
    ../src/PrimeNumberGenerator.cpp \
    ../src/ResetSieve.cpp \
    ../src/WheelFactorization.cpp
HEADERS += ../src/EratBig.h \
    ../src/EratSmall.h \
    ../src/PrimeNumberFinder.h \
    ../src/PrimeSieve.h \
    ../src/SieveOfEratosthenes.h \
    ../src/EratMedium.h \
    ../src/PrimeNumberGenerator.h \
    ../src/ResetSieve.h \
    ../src/WheelFactorization.h \
    ../src/pmath.h \
    ../src/bits.h \
    ../src/settings.h \
    ../src/utils/cpuid.h \
    ../src/utils/cpuid_gcc451.h \
    ../src/utils/strtoull.h
