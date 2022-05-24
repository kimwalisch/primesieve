cmake_minimum_required(VERSION 3.4...3.22)
project(primesieve CXX)
set(PRIMESIEVE_VERSION "8.0")
set(PRIMESIEVE_SOVERSION "10.0.0")

# Build options ######################################################

option(BUILD_PRIMESIEVE  "Build primesieve binary"       ON)
option(BUILD_SHARED_LIBS "Build shared libprimesieve"    ON)
option(BUILD_STATIC_LIBS "Build static libprimesieve"    ON)
option(BUILD_DOC         "Build C/C++ API documentation" OFF)
option(BUILD_MANPAGE     "Regenerate man page using a2x" OFF)
option(BUILD_EXAMPLES    "Build example programs"        OFF)
option(BUILD_TESTS       "Build test programs"           OFF)

option(WITH_MULTIARCH       "Enable runtime dispatching to fastest supported CPU instruction set" ON)
option(WITH_MSVC_CRT_STATIC "Link primesieve.lib with /MT instead of the default /MD" OFF)

# libprimesieve sanity check #########################################

if(NOT BUILD_SHARED_LIBS AND NOT BUILD_STATIC_LIBS)
    message(FATAL_ERROR "One or both of BUILD_SHARED_LIBS or BUILD_STATIC_LIBS must be set to ON")
endif()

# Set default build type to Release ##################################

if(NOT CMAKE_VERSION VERSION_LESS 3.9)
    get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
elseif(CMAKE_CONFIGURATION_TYPES)
    set(isMultiConfig TRUE)
endif()

if(NOT isMultiConfig AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# primesieve binary source files #####################################

set(BIN_SRC src/app/cmdoptions.cpp
            src/app/help.cpp
            src/app/main.cpp
            src/app/test.cpp)

# primesieve library source files ####################################

set(LIB_SRC src/api-c.cpp
            src/api.cpp
            src/CountPrintPrimes.cpp
            src/CpuInfo.cpp
            src/Erat.cpp
            src/EratSmall.cpp
            src/EratMedium.cpp
            src/EratBig.cpp
            src/iterator-c.cpp
            src/iterator.cpp
            src/IteratorHelper.cpp
            src/LookupTables.cpp
            src/MemoryPool.cpp
            src/PrimeGenerator.cpp
            src/nthPrime.cpp
            src/ParallelSieve.cpp
            src/popcount.cpp
            src/PreSieve.cpp
            src/PrimeSieve.cpp
            src/SievingPrimes.cpp)

# Required includes ##################################################

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# MSVC DLL support (Windows) #########################################

if(WIN32 AND NOT MINGW)
    set(WIN32_MSVC_COMPATIBLE ON)
    if(BUILD_SHARED_LIBS)
        # Export symbols to .def file
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()
endif()

# Check if libatomic is needed (old 32-bit systems) ##################

include("${PROJECT_SOURCE_DIR}/cmake/libatomic.cmake")

# Check if compiler supports x64 multiarch ###########################

if(WITH_MULTIARCH)
    include("${PROJECT_SOURCE_DIR}/cmake/multiarch_popcnt_bmi.cmake")
    include("${PROJECT_SOURCE_DIR}/cmake/multiarch_avx512.cmake")
endif()

# libprimesieve (shared library) #####################################

find_package(Threads REQUIRED QUIET)

if(BUILD_SHARED_LIBS)
    add_library(libprimesieve SHARED ${LIB_SRC})
    set_target_properties(libprimesieve PROPERTIES OUTPUT_NAME primesieve)
    target_link_libraries(libprimesieve PRIVATE Threads::Threads ${LIBATOMIC})
    string(REPLACE "." ";" SOVERSION_LIST ${PRIMESIEVE_SOVERSION})
    list(GET SOVERSION_LIST 0 PRIMESIEVE_SOVERSION_MAJOR)
    set_target_properties(libprimesieve PROPERTIES SOVERSION ${PRIMESIEVE_SOVERSION_MAJOR})
    set_target_properties(libprimesieve PROPERTIES VERSION ${PRIMESIEVE_SOVERSION})

    if(multiarch_popcnt_bmi)
        target_compile_definitions(libprimesieve PRIVATE "MULTIARCH_POPCNT_BMI")
    endif()
    if(multiarch_avx512)
        target_compile_definitions(libprimesieve PRIVATE "MULTIARCH_AVX512")
    endif()

    if(WIN32_MSVC_COMPATIBLE)
        # On Windows the shared library will be named primesieve.dll
        # and the associated import library will be named
        # primesieve.lib. This is an issue as the static library
        # is also named primesieve.lib. Hence we rename the import
        # library to primesieve.dll.lib. The Rust programming
        # language uses the same naming convention.
        set_target_properties(libprimesieve PROPERTIES IMPORT_SUFFIX ".dll.lib")
    endif()

    target_compile_features(libprimesieve
    PUBLIC
        cxx_alias_templates
    PRIVATE
        cxx_constexpr
        cxx_uniform_initialization
        cxx_lambdas)

    target_include_directories(libprimesieve PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

    install(TARGETS libprimesieve
            EXPORT primesieveShared
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

# libprimesieve-static ###############################################

if(BUILD_STATIC_LIBS)
    add_library(libprimesieve-static STATIC ${LIB_SRC})
    set_target_properties(libprimesieve-static PROPERTIES OUTPUT_NAME primesieve)
    target_link_libraries(libprimesieve-static PRIVATE Threads::Threads ${LIBATOMIC})

    if(multiarch_popcnt_bmi)
        target_compile_definitions(libprimesieve-static PRIVATE "MULTIARCH_POPCNT_BMI")
    endif()
    if(multiarch_avx512)
        target_compile_definitions(libprimesieve-static PRIVATE "MULTIARCH_AVX512")
    endif()

    if(WITH_MSVC_CRT_STATIC)
        set_target_properties(libprimesieve-static PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded")
    endif()

    if(TARGET libprimesieve)
        add_dependencies(libprimesieve-static libprimesieve)
    endif()

    target_compile_features(libprimesieve-static
    PUBLIC
        cxx_alias_templates
    PRIVATE
        cxx_constexpr
        cxx_uniform_initialization
        cxx_lambdas)

    target_include_directories(libprimesieve-static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

    install(TARGETS libprimesieve-static
            EXPORT primesieveStatic
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

# Static or shared linking ###########################################

# On Unix-like OSes we use shared linking against libprimesieve by
# default mainly because this is required by Linux distributions.
# On Windows we use static linking by default because the library's
# path is not encoded into the binary. This means that the binary
# will only work if the DLL is in the same directory or a directory
# that is in the PATH environment variable.

if(WIN32 AND TARGET libprimesieve-static)
    set(STATIC_LINKING ON)
elseif(NOT TARGET libprimesieve)
    set(STATIC_LINKING ON)
endif()

if(STATIC_LINKING)
    add_library(primesieve::primesieve ALIAS libprimesieve-static)
else()
    add_library(primesieve::primesieve ALIAS libprimesieve)
endif()

# primesieve binary ##################################################

if(BUILD_PRIMESIEVE)
    add_executable(primesieve ${BIN_SRC})
    target_link_libraries(primesieve primesieve::primesieve)
    target_compile_features(primesieve PRIVATE cxx_auto_type)
    install(TARGETS primesieve DESTINATION ${CMAKE_INSTALL_BINDIR})

    if(multiarch_avx512)
        target_compile_definitions(primesieve PRIVATE "MULTIARCH_AVX512")
    endif()

    if(WITH_MSVC_CRT_STATIC)
        set_target_properties(primesieve PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded")
    endif()

    if(TARGET libprimesieve-static)
        add_dependencies(primesieve libprimesieve-static)
    endif()
endif()

# Install headers ####################################################

install(FILES include/primesieve.h
              include/primesieve.hpp
              COMPONENT libprimesieve-headers
              DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES include/primesieve/iterator.h
              include/primesieve/iterator.hpp
              include/primesieve/StorePrimes.hpp
              include/primesieve/primesieve_error.hpp
              COMPONENT libprimesieve-headers
              DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/primesieve)

# CMake find_package(primesieve) support #############################

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/primesieveConfigVersion.cmake"
    VERSION ${PRIMESIEVE_VERSION}
    COMPATIBILITY SameMajorVersion)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/primesieveConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/primesieveConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/primesieve")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/primesieveConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/primesieveConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/primesieve")

if(TARGET libprimesieve)
    install(EXPORT primesieveShared
            NAMESPACE primesieve::
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/primesieve")
endif()

if(TARGET libprimesieve-static)
    install(EXPORT primesieveStatic
            NAMESPACE primesieve::
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/primesieve")
endif()

# Regenerate man page ################################################

if(BUILD_MANPAGE)
    find_program(A2X_EXECUTABLE a2x)

    if(NOT A2X_EXECUTABLE)
        message(FATAL_ERROR "Missing a2x program (required for man page generation)!")
    else()
        message(STATUS "Found a2x: ${A2X_EXECUTABLE}")

        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${A2X_EXECUTABLE}
            ARGS --format=manpage
                 -D "${PROJECT_SOURCE_DIR}/doc"
                 "${PROJECT_SOURCE_DIR}/doc/primesieve.txt"
            VERBATIM)
    endif()
endif()

# Install man page ###################################################

if(BUILD_PRIMESIEVE)
    install(FILES ${PROJECT_SOURCE_DIR}/doc/primesieve.1
            DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
endif()

# Install primesieve.pc (pkg-config) #################################

configure_file(primesieve.pc.in primesieve.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/primesieve.pc
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
