cmake_minimum_required(VERSION 3.1)
project(primesieve)
set(PRIMESIEVE_VERSION_MAJOR 6)
set(PRIMESIEVE_VERSION_MINOR 0)
set(PRIMESIEVE_VERSION "${PRIMESIEVE_VERSION_MAJOR}.${PRIMESIEVE_VERSION_MINOR}")
include_directories(include)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 11)

# primesieve binary source files #####################################

set(BIN_SRC src/apps/console/cmdoptions.cpp
            src/apps/console/help.cpp
            src/apps/console/main.cpp)

# primesieve library source files ####################################

set(PRIMESIEVE_PATH src/primesieve)

set(LIB_SRC ${PRIMESIEVE_PATH}/EratBig.cpp
            ${PRIMESIEVE_PATH}/EratMedium.cpp
            ${PRIMESIEVE_PATH}/EratSmall.cpp
            ${PRIMESIEVE_PATH}/ParallelPrimeSieve.cpp
            ${PRIMESIEVE_PATH}/PreSieve.cpp
            ${PRIMESIEVE_PATH}/PrimeFinder.cpp
            ${PRIMESIEVE_PATH}/PrimeGenerator.cpp
            ${PRIMESIEVE_PATH}/PrimeSieve.cpp
            ${PRIMESIEVE_PATH}/SieveOfEratosthenes.cpp
            ${PRIMESIEVE_PATH}/Wheel.cpp
            ${PRIMESIEVE_PATH}/api-c.cpp
            ${PRIMESIEVE_PATH}/api.cpp
            ${PRIMESIEVE_PATH}/iterator.cpp
            ${PRIMESIEVE_PATH}/nthPrime.cpp
            ${PRIMESIEVE_PATH}/popcount.cpp
            ${PRIMESIEVE_PATH}/primesieve_iterator.cpp
            ${PRIMESIEVE_PATH}/primesieve_test.cpp)

# Check for OpenMP ###################################################

find_package(OpenMP QUIET)
if(OPENMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
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
target_link_libraries(primesieve static_libprimesieve)

# Install primesieve #################################################

install(TARGETS primesieve DESTINATION bin)
install(TARGETS static_libprimesieve DESTINATION lib)
install(TARGETS shared_libprimesieve DESTINATION lib)

install(FILES include/primesieve.hpp DESTINATION include)
install(FILES include/primesieve/PrimeSieve.hpp DESTINATION include)
install(FILES include/primesieve/ParallelPrimeSieve.hpp DESTINATION include)
install(FILES include/primesieve/Callback.hpp DESTINATION include)
install(FILES include/primesieve/cancel_callback.hpp DESTINATION include)
install(FILES include/primesieve/iterator.hpp DESTINATION include)
install(FILES include/primesieve/PushBackPrimes.hpp DESTINATION include)
install(FILES include/primesieve/primesieve_error.hpp DESTINATION include)

install(FILES include/primesieve.h DESTINATION include)
install(FILES include/primesieve/primesieve_iterator.h DESTINATION include)
