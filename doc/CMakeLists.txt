find_package(Doxygen)

if(DOXYGEN_FOUND)
    find_package(LATEX)
    find_program(DOT_FONUD dot)

    if(LATEX_COMPILER)
        set(HAVE_LATEX "YES")
    else()
        set(HAVE_LATEX "NO")
    endif()

    if(DOT_FONUD)
        message(STATUS "Found dot (graphviz): ${DOT_FONUD}")
        set(HAVE_DOT "YES")
    else()
        set(HAVE_DOT "NO")
    endif()

    configure_file(Doxyfile.in Doxyfile @ONLY)
    add_custom_target(doc COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile)

    if(LATEX_COMPILER)
        find_program(MAKE_FONUD make)

        # LaTeX doc (PDF) must be built using make
        if(MAKE_FONUD)
            add_custom_command(TARGET doc
                               POST_BUILD
                               COMMAND "${MAKE_FONUD}"
                               WORKING_DIRECTORY "latex")
        endif()
    endif()
endif()
