enable_language(C)
file(GLOB files "*.cpp" "*.c")

foreach(file ${files})
    get_filename_component(binary_name ${file} NAME_WE)
    add_executable(${binary_name} ${file})
    target_link_libraries(${binary_name} primesieve::primesieve)
    target_include_directories(${binary_name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_compile_definitions(${binary_name} PRIVATE ${PRIMESIEVE_COMPILE_DEFINITIONS})
    add_test(NAME ${binary_name} COMMAND ${binary_name})

    # Copy primesieve.dll to test directory.
    # On Windows the DLLs must be in the same directory
    # as the binaries that depend on them.
    if (WIN32 AND NOT STATICALLY_LINK_LIBPRIMESIEVE)
        add_custom_command(TARGET ${binary_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:libprimesieve>
                $<TARGET_FILE_DIR:${binary_name}>)
    endif()
endforeach()
