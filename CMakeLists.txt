cmake_minimum_required(VERSION 3.1)
project(primesieve)
set(PRIMESIEVE_VERSION_MAJOR 6)
set(PRIMESIEVE_VERSION_MINOR 0)
set(PRIMESIEVE_VERSION "${PRIMESIEVE_VERSION_MAJOR}.${PRIMESIEVE_VERSION_MINOR}")
include_directories(include)
set(CMAKE_BUILD_TYPE Release)

# Build options ######################################################

option(BUILD_EXAMPLES "Build example programs" OFF)
option(BUILD_TESTS    "Build test programs"    ON)

# Compiler must support C++11 or later ###############################

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build libprimesieve example programs ###############################

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Add test programs ##################################################

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# primesieve binary source files #####################################

set(BIN_SRC src/apps/console/cmdoptions.cpp
            src/apps/console/help.cpp
            src/apps/console/main.cpp)

# primesieve library source files ####################################

set(LIB_SRC src/primesieve/EratBig.cpp
            src/primesieve/EratMedium.cpp
            src/primesieve/EratSmall.cpp
            src/primesieve/ParallelPrimeSieve.cpp
            src/primesieve/PreSieve.cpp
            src/primesieve/PrimeFinder.cpp
            src/primesieve/PrimeGenerator.cpp
            src/primesieve/PrimeSieve.cpp
            src/primesieve/SieveOfEratosthenes.cpp
            src/primesieve/Wheel.cpp
            src/primesieve/api-c.cpp
            src/primesieve/api.cpp
            src/primesieve/iterator.cpp
            src/primesieve/nthPrime.cpp
            src/primesieve/popcount.cpp
            src/primesieve/primesieve_iterator.cpp)

# Check for OpenMP ###################################################

find_package(OpenMP QUIET)
if(OPENMP_FOUND)
    message(STATUS "Found OpenMP: ${OpenMP_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Detect CPU L1 cache size ###########################################

execute_process(COMMAND sh ${PROJECT_SOURCE_DIR}/scripts/L1_cache_size.sh
                        RESULT_VARIABLE L1_FAILED
                        OUTPUT_VARIABLE L1_CACHE_SIZE)

if(NOT L1_FAILED)
    string(STRIP ${L1_CACHE_SIZE} L1_CACHE_SIZE)
    message(STATUS "CPU L1 data cache size: ${L1_CACHE_SIZE} KB")
    add_definitions(-DL1_DCACHE_SIZE=${L1_CACHE_SIZE})
endif()

# Build static library ###############################################

add_library(static_libprimesieve STATIC ${LIB_SRC})
set_target_properties(static_libprimesieve PROPERTIES OUTPUT_NAME primesieve)

# Build shared library ###############################################

add_library(shared_libprimesieve SHARED ${LIB_SRC})
set_target_properties(shared_libprimesieve PROPERTIES OUTPUT_NAME primesieve)
set_target_properties(shared_libprimesieve PROPERTIES SOVERSION ${PRIMESIEVE_VERSION_MAJOR})
set_target_properties(shared_libprimesieve PROPERTIES VERSION ${PRIMESIEVE_VERSION})

# Build binary #######################################################

add_executable(primesieve ${BIN_SRC})
target_link_libraries(primesieve shared_libprimesieve)

# Install primesieve #################################################

install(TARGETS primesieve
                DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

install(TARGETS static_libprimesieve
                shared_libprimesieve
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

install(FILES include/primesieve.hpp
              include/primesieve/PrimeSieve.hpp
              include/primesieve/ParallelPrimeSieve.hpp
              include/primesieve/Callback.hpp
              include/primesieve/StorePrimes.hpp
              include/primesieve/iterator.hpp
              include/primesieve/primesieve_error.hpp
              DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

install(FILES include/primesieve.h
              include/primesieve/primesieve_iterator.h
              DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

# Regenerate man page ################################################

find_program(HELP2MAN help2man)

if(HELP2MAN)
    message(STATUS "Found help2man: ${HELP2MAN}")

    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${HELP2MAN}
        ARGS -s 1
             --manual="primesieve"
             --source="primesieve ${PRIMESIEVE_VERSION}"
             --no-info
             -n "efficient prime number generator"
             -o ${PROJECT_SOURCE_DIR}/doc/primesieve.1
             ./primesieve
        VERBATIM)
endif()

# Install man page ###################################################

install(FILES doc/primesieve.1
              DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1)

# Install primesieve.pc (pkg-config) #################################

configure_file(primesieve.pc.in primesieve.pc @ONLY)
install(FILES primesieve.pc
              DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
