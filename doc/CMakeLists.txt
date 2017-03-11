find_package(Doxygen)

if(DOXYGEN_FOUND)

    find_program(DOT_FONUD dot)
    if (DOT_FONUD)
        message(STATUS "Found dot (graphviz): ${DOT_FONUD}")
        set(HAVE_DOT "YES")
    else()
        set(HAVE_DOT "NO")
    endif()

    configure_file(Doxyfile.in Doxyfile @ONLY)
    add_custom_target(doc COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile)
endif()
