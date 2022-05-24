include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

cmake_push_check_state()

if(CMAKE_CXX11_STANDARD_COMPILE_OPTION)
    set(CMAKE_REQUIRED_FLAGS ${CMAKE_CXX11_STANDARD_COMPILE_OPTION})
endif()

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

if(NOT atomic64)
    find_library(ATOMIC NAMES atomic libatomic.so.1)

    if(ATOMIC)
        set(LIBATOMIC ${ATOMIC})
        message(STATUS "Found libatomic: ${LIBATOMIC}")
    else()
        check_cxx_source_compiles("
            #include <atomic>
            #include <stdint.h>
            int main() {
                std::atomic<int32_t> x;
                x = 1;
                x--;
                return (int) x;
            }"
            atomic32)

        if(atomic32)
            message(FATAL_ERROR "Failed to find libatomic!")
        endif()
    endif()
endif()

cmake_pop_check_state()
