# This file first tries to find out whether it is necessary to
# add libatomic to the linker flags. Usually this is required
# on old 32-bit CPUs. Then it tries to find libatomic, ideally
# we would have liked to use an offical CMake module to find
# libatomic, however there is none.
# See discussion: https://github.com/kimwalisch/primesieve/issues/141

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

cmake_push_check_state()

if(CMAKE_CXX11_STANDARD_COMPILE_OPTION)
    set(CMAKE_REQUIRED_FLAGS ${CMAKE_CXX11_STANDARD_COMPILE_OPTION})
endif()

# Check if code compiles without libatomic.
# Should always work on CPUs >= 64-bits
check_cxx_source_compiles("
    #include <atomic>
    #include <stdint.h>
    int main() {
        std::atomic<int64_t> x;
        x = 1;
        x--;
        return (int) x;
    }"
    atomic64)

# Our code requires libatomic to compile
if(NOT atomic64)
    find_library(LIBATOMIC NAMES atomic atomic.so.1 libatomic.so.1)

    if(LIBATOMIC)
        message(STATUS "Found libatomic: ${LIBATOMIC}")
    else()
        # Some package managers like homebrew and macports store the compiler's
        # libraries in a subdirectory of the library directory. E.g. GCC
        # installed via homebrew stores libatomic at lib/gcc/13/libatomic.dylib
        # instead of lib/libatomic.dylib. CMake's find_library() cannot easily
        # be used to recursively find libraries. Therefore we use this workaround
        # here (try adding -latomic to linker options) for this use case.
        set(LIBATOMIC "-latomic")
        message(STATUS "Add linker flag: ${LIBATOMIC}")
    endif()

    set(CMAKE_REQUIRED_LIBRARIES "${LIBATOMIC}")

    check_cxx_source_compiles("
        #include <atomic>
        #include <stdint.h>
        int main() {
            std::atomic<int64_t> x;
            x = 1;
            x--;
            return (int) x;
        }"
        atomic64_with_libatomic)

    if(atomic64_with_libatomic)
        list(APPEND PRIMESIEVE_LINK_LIBRARIES "${LIBATOMIC}")
    else()
        set(LIBATOMIC "")
        message(FATAL_ERROR "Failed to compile std::atomic, libatomic likely not found!")
    endif()
endif()

cmake_pop_check_state()
