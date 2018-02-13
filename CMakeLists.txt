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

# If BUILD_SHARED_LIBS is disabled the static libprimesieve will
# be built using the libprimesieve target instead of the
# static_libprimesieve target

if(BUILD_STATIC_LIBS)
    set(BUILD_STATIC_LIBPRIMESIEVE ${BUILD_SHARED_LIBS})
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

# Required includes ##################################################

include(GNUInstallDirs)
include(CMakePushCheckState)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)

# Set default build type to Release ##################################

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

# Silence GCC switch fall through warnings ###########################

check_cxx_compiler_flag(-Wno-implicit-fallthrough Wno_fallthrough)

if(Wno_fallthrough)
    set_source_files_properties(src/EratSmall.cpp PROPERTIES COMPILE_FLAGS -Wno-implicit-fallthrough)
endif()

# Check if libatomic is needed #######################################

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

# libprimesieve ######################################################

add_library(libprimesieve ${LIB_SRC})
set_target_properties(libprimesieve PROPERTIES OUTPUT_NAME primesieve)
find_package(Threads REQUIRED)
target_link_libraries(libprimesieve PRIVATE Threads::Threads ${LIBATOMIC})

target_compile_features(libprimesieve
PUBLIC
    cxx_alias_templates
PRIVATE
    cxx_auto_type
    cxx_lambdas
    cxx_nullptr
    cxx_range_for
    cxx_uniform_initialization)

target_include_directories(libprimesieve PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

if(BUILD_SHARED_LIBS)
    string(REPLACE "." ";" SOVERSION_LIST ${PRIMESIEVE_SOVERSION})
    list(GET SOVERSION_LIST 0 PRIMESIEVE_SOVERSION_MAJOR)
    set_target_properties(libprimesieve PROPERTIES SOVERSION ${PRIMESIEVE_SOVERSION_MAJOR})
    set_target_properties(libprimesieve PROPERTIES VERSION ${PRIMESIEVE_SOVERSION})
endif()

install(TARGETS libprimesieve DESTINATION ${CMAKE_INSTALL_LIBDIR})

# static_libprimesieve ###############################################

if(BUILD_STATIC_LIBPRIMESIEVE)
    add_library(static_libprimesieve STATIC ${LIB_SRC})
    set_target_properties(static_libprimesieve PROPERTIES OUTPUT_NAME primesieve)
    target_link_libraries(static_libprimesieve PRIVATE Threads::Threads ${LIBATOMIC})
    add_dependencies(static_libprimesieve libprimesieve)

    target_compile_features(static_libprimesieve
    PUBLIC
        cxx_alias_templates
    PRIVATE
        cxx_auto_type
        cxx_lambdas
        cxx_nullptr
        cxx_range_for
        cxx_uniform_initialization)

    target_include_directories(static_libprimesieve PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

    install(TARGETS static_libprimesieve DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

# primesieve binary ##################################################

if(BUILD_PRIMESIEVE)
    add_executable(primesieve ${BIN_SRC})
    target_link_libraries(primesieve libprimesieve)
    target_compile_features(primesieve PRIVATE cxx_auto_type)
    install(TARGETS primesieve DESTINATION ${CMAKE_INSTALL_BINDIR})

    if(BUILD_STATIC_LIBPRIMESIEVE)
        add_dependencies(primesieve static_libprimesieve)
    endif()
endif()

# Install headers ####################################################

install(FILES include/primesieve.h
              include/primesieve.hpp
              COMPONENT libprimesieve-headers
              DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES include/primesieve/PrimeSieve.hpp
              include/primesieve/StorePrimes.hpp
              include/primesieve/iterator.h
              include/primesieve/iterator.hpp
              include/primesieve/primesieve_error.hpp
              include/primesieve/pmath.hpp
              COMPONENT libprimesieve-headers
              DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/primesieve)

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
            DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
endif()

# Install primesieve.pc (pkg-config) #################################

configure_file(primesieve.pc.in primesieve.pc @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/primesieve.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

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
