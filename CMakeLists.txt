cmake_minimum_required(VERSION 3.1)
project(primesieve)
set(PRIMESIEVE_VERSION "6.4")
set(PRIMESIEVE_SOVERSION "8.4.0")

# Build options ######################################################

option(BUILD_PRIMESIEVE  "Build primesieve binary"    ON)
option(BUILD_SHARED_LIBS "Build shared libprimesieve" ON)
option(BUILD_STATIC_LIBS "Build static libprimesieve" ON)
option(BUILD_DOC         "Build documentation"        OFF)
option(BUILD_EXAMPLES    "Build example programs"     OFF)
option(BUILD_TESTS       "Build test programs"        OFF)

if(WIN32)
    set(BUILD_SHARED_LIBS OFF)
endif()

# primesieve binary source files #####################################

set(BIN_SRC src/console/cmdoptions.cpp
            src/console/help.cpp
            src/console/main.cpp)

# primesieve library source files ####################################

set(LIB_SRC src/api-c.cpp
            src/api.cpp
            src/CpuInfo.cpp
            src/EratBig.cpp
            src/EratMedium.cpp
            src/EratSmall.cpp
            src/iterator-c.cpp
            src/iterator.cpp
            src/nthPrime.cpp
            src/ParallelPrimeSieve.cpp
            src/popcount.cpp
            src/PreSieve.cpp
            src/PrimeGenerator.cpp
            src/PrimeSieve.cpp
            src/SieveOfEratosthenes.cpp
            src/SievingPrimes.cpp
            src/Wheel.cpp)

# Compiler must support C++11 or later ###############################

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Release ##################################

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

# These variables are needed for rpm builds ##########################

if(NOT INCLUDE_INSTALL_DIR)
    set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include")
endif()

if(NOT LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

if(NOT SHARE_INSTALL_PREFIX)
    set(SHARE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/share")
endif()

# Multiarch support for Debian/Ubuntu ################################

if(CMAKE_LIBRARY_ARCHITECTURE)
    set(LIB_INSTALL_DIR "${LIB_INSTALL_DIR}/${CMAKE_LIBRARY_ARCHITECTURE}")
endif()

# Silence GCC switch fall through warnings ###########################

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wno-implicit-fallthrough Wno_fallthrough)

if(Wno_fallthrough)
    set_source_files_properties(src/EratSmall.cpp PROPERTIES COMPILE_FLAGS -Wno-implicit-fallthrough)
endif()

# Check if libatomic is needed #######################################

include(CMakePushCheckState)
include(CheckCXXSourceCompiles)

cmake_push_check_state()
set(CMAKE_REQUIRED_FLAGS ${CMAKE_CXX11_STANDARD_COMPILE_OPTION})

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

# libprimesieve ######################################################

add_library(libprimesieve ${LIB_SRC})
set_target_properties(libprimesieve PROPERTIES OUTPUT_NAME primesieve)
find_package(Threads REQUIRED)
target_link_libraries(libprimesieve Threads::Threads ${LIBATOMIC})

target_include_directories(libprimesieve PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

if(BUILD_SHARED_LIBS)
    string(REPLACE "." ";" SOVERSION_LIST ${PRIMESIEVE_SOVERSION})
    list(GET SOVERSION_LIST 0 PRIMESIEVE_SOVERSION_MAJOR)
    set_target_properties(libprimesieve PROPERTIES SOVERSION ${PRIMESIEVE_SOVERSION_MAJOR})
    set_target_properties(libprimesieve PROPERTIES VERSION ${PRIMESIEVE_SOVERSION})
endif()

install(TARGETS libprimesieve DESTINATION ${LIB_INSTALL_DIR})

# static_libprimesieve ###############################################

if(BUILD_STATIC_LIBS AND BUILD_SHARED_LIBS)
    add_library(static_libprimesieve STATIC ${LIB_SRC})
    set_target_properties(static_libprimesieve PROPERTIES OUTPUT_NAME primesieve)
    target_link_libraries(static_libprimesieve Threads::Threads ${LIBATOMIC})
    add_dependencies(static_libprimesieve libprimesieve)

    target_include_directories(static_libprimesieve PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

    install(TARGETS static_libprimesieve DESTINATION ${LIB_INSTALL_DIR})
endif()

# primesieve binary ##################################################

if(BUILD_PRIMESIEVE)
    add_executable(primesieve ${BIN_SRC})
    target_link_libraries(primesieve libprimesieve)
    install(TARGETS primesieve DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

    # primesieve should build after static_libprimesieve
    if(BUILD_STATIC_LIBS AND BUILD_SHARED_LIBS)
        add_dependencies(primesieve static_libprimesieve)
    endif()
endif()

# Install headers ####################################################

install(FILES include/primesieve.h
              include/primesieve.hpp
              COMPONENT libprimesieve-headers
              DESTINATION ${INCLUDE_INSTALL_DIR})

install(FILES include/primesieve/PrimeSieve.hpp
              include/primesieve/StorePrimes.hpp
              include/primesieve/iterator.h
              include/primesieve/iterator.hpp
              include/primesieve/primesieve_error.hpp
              include/primesieve/pmath.hpp
              COMPONENT libprimesieve-headers
              DESTINATION ${INCLUDE_INSTALL_DIR}/primesieve)

# Regenerate man page ################################################

if(BUILD_PRIMESIEVE)
    find_program(HELP2MAN help2man)
endif()

if(HELP2MAN)
    message(STATUS "Found help2man: ${HELP2MAN}")

    execute_process(COMMAND perl -e "use Locale::gettext;"
                    RESULT_VARIABLE LOCALE_RES
                    OUTPUT_QUIET ERROR_QUIET)

    if(LOCALE_RES EQUAL 0)
        message(STATUS "Found help2man option: --locale=C.UTF-8")
        set(HELP2MAN_LOCALE "--locale=C.UTF-8")
    endif()

    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${HELP2MAN}
        ARGS -s 1
             --manual="primesieve"
             --source="primesieve ${PRIMESIEVE_VERSION}"
             --no-info
             ${HELP2MAN_LOCALE}
             -n "efficient prime number generator"
             -o ${PROJECT_SOURCE_DIR}/doc/primesieve.1
             ./primesieve
        VERBATIM)
endif()

# Install man page ###################################################

if(BUILD_PRIMESIEVE)
    install(FILES ${PROJECT_SOURCE_DIR}/doc/primesieve.1
            DESTINATION ${SHARE_INSTALL_PREFIX}/man/man1)
endif()

# Install primesieve.pc (pkg-config) #################################

configure_file(primesieve.pc.in primesieve.pc @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/primesieve.pc
        DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

# Add subdirectories #################################################

if(BUILD_DOC)
    add_subdirectory(doc)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
