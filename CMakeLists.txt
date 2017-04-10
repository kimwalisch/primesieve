cmake_minimum_required(VERSION 3.1)
project(primesieve CXX)
set(PRIMESIEVE_VERSION "6.0")
set(PRIMESIEVE_SOVERSION "8.0.0")
set(CMAKE_BUILD_TYPE Release)

# Build options ######################################################

option(WITH_OPENMP       "Enable OpenMP support"      ON)
option(BUILD_PRIMESIEVE  "Build primesieve binary"    ON)
option(BUILD_SHARED_LIBS "Build shared libprimesieve" ON)
option(BUILD_EXAMPLES    "Build example programs"     OFF)
option(BUILD_TESTS       "Build test programs"        OFF)

if(WIN32)
    set(BUILD_SHARED_LIBS OFF)
endif()

# Compiler must support C++11 or later ###############################

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# primesieve binary source files #####################################

set(BIN_SRC src/console/cmdoptions.cpp
            src/console/help.cpp
            src/console/main.cpp)

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
            src/primesieve/api.cpp
            src/primesieve/api-c.cpp
            src/primesieve/iterator.cpp
            src/primesieve/iterator-c.cpp
            src/primesieve/nthPrime.cpp
            src/primesieve/popcount.cpp)

# Check for OpenMP ###################################################

if(WITH_OPENMP)
    find_package(OpenMP QUIET)
    if(OPENMP_FOUND)
        message(STATUS "Found OpenMP: ${OpenMP_CXX_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    endif()
endif()

# Detect CPU L1 cache size ###########################################

execute_process(COMMAND sh ${PROJECT_SOURCE_DIR}/scripts/L1_cache_size.sh
                RESULT_VARIABLE L1_RES
                OUTPUT_VARIABLE L1_CACHE_SIZE
                ERROR_QUIET)

if(L1_RES EQUAL 0)
    string(STRIP ${L1_CACHE_SIZE} L1_CACHE_SIZE)
    message(STATUS "Found CPU L1 cache size: ${L1_CACHE_SIZE} KB")
    add_definitions(-DL1_DCACHE_SIZE=${L1_CACHE_SIZE})
endif()

# libprimesieve ######################################################

add_library(libprimesieve ${LIB_SRC})
set_target_properties(libprimesieve PROPERTIES OUTPUT_NAME primesieve)

target_include_directories(libprimesieve PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

if(BUILD_SHARED_LIBS)
    string(REPLACE "." ";" SOVERSION_LIST ${PRIMESIEVE_SOVERSION})
    list(GET SOVERSION_LIST 0 PRIMESIEVE_SOVERSION_MAJOR)
    set_target_properties(libprimesieve PROPERTIES SOVERSION ${PRIMESIEVE_SOVERSION_MAJOR})
    set_target_properties(libprimesieve PROPERTIES VERSION ${PRIMESIEVE_SOVERSION})
endif()

install(TARGETS libprimesieve
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# static_libprimesieve ###############################################

if(BUILD_SHARED_LIBS)
    add_library(static_libprimesieve STATIC ${LIB_SRC})
    set_target_properties(static_libprimesieve PROPERTIES OUTPUT_NAME primesieve)

    target_include_directories(static_libprimesieve PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

    install(TARGETS static_libprimesieve
            DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
endif()

# primesieve binary ##################################################

if(BUILD_PRIMESIEVE)
    add_executable(primesieve ${BIN_SRC})
    target_link_libraries(primesieve libprimesieve)
    install(TARGETS primesieve
            DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()

# Install headers ####################################################

install(FILES include/primesieve.h
              include/primesieve.hpp        
              DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

install(FILES include/primesieve/PrimeSieve.hpp
              include/primesieve/Callback.hpp
              include/primesieve/StorePrimes.hpp
              include/primesieve/iterator.h
              include/primesieve/iterator.hpp
              include/primesieve/primesieve_error.hpp
              DESTINATION ${CMAKE_INSTALL_PREFIX}/include/primesieve)

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
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1)
endif()

# Install primesieve.pc (pkg-config) #################################

configure_file(primesieve.pc.in primesieve.pc @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/primesieve.pc
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

# Add subdirectories #################################################

add_subdirectory(doc)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
