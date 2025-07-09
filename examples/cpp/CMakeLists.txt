file(GLOB files "*.cpp")
foreach(file ${files})
    get_filename_component(binary_name ${file} NAME_WE)
    add_executable(${binary_name} ${file})
    target_link_libraries(${binary_name} primesieve::primesieve)

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
