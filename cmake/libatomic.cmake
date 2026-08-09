# This file first tries to find out whether it is necessary to
# add libatomic to the linker flags. Usually this is required
# on old 32-bit CPUs. Then it tries to find libatomic, ideally
# we would have liked to use an offical CMake module to find
# libatomic, however there is none.
# See discussion: https://github.com/kimwalisch/primesieve/issues/141

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

cmake_push_check_state()

set(atomic64_TEST_SOURCE "
    #include <atomic>
    #include <stdint.h>
    int main() {
        std::atomic<int64_t> x;
        x = 1;
        x--;
        return (int) x;
    }")

if(CMAKE_CXX11_STANDARD_COMPILE_OPTION)
    set(CMAKE_REQUIRED_FLAGS ${CMAKE_CXX11_STANDARD_COMPILE_OPTION})
endif()

# Check if code compiles without libatomic.
# Should always work on CPUs >= 64-bits
check_cxx_source_compiles("${atomic64_TEST_SOURCE}" atomic64)

# Our code requires libatomic to compile
if(NOT atomic64)
    # First try -latomic as the compiler may find libraries
    # in directories which CMake does not search.
    set(CMAKE_REQUIRED_LIBRARIES "-latomic")
    check_cxx_source_compiles("${atomic64_TEST_SOURCE}" atomic64_with_latomic)

    if(atomic64_with_latomic)
        set(LIBATOMIC "-latomic")
    else()
        find_library(LIBATOMIC NAMES atomic atomic.so.1 libatomic.so.1)

        if(LIBATOMIC)
            set(CMAKE_REQUIRED_LIBRARIES "${LIBATOMIC}")
            check_cxx_source_compiles("${atomic64_TEST_SOURCE}" atomic64_with_libatomic_path)
        endif()
    endif()

    if(atomic64_with_latomic OR
       atomic64_with_libatomic_path)
        list(APPEND PRIMESIEVE_LINK_LIBRARIES "${LIBATOMIC}")
    else()
        message(FATAL_ERROR "Failed to compile std::atomic, libatomic likely not found!")
    endif()

    if(atomic64_with_latomic)
        string(APPEND PRIMESIEVE_PKGCONFIG_LIBS_PRIVATE "-latomic ")
    endif()
endif()

cmake_pop_check_state()
