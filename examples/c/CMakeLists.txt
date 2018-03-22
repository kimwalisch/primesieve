enable_language(C)
file(GLOB files "*.c")

foreach(file ${files})
    get_filename_component(binary_name ${file} NAME_WE)
    add_executable(${binary_name}_c ${file})
    target_link_libraries(${binary_name}_c primesieve::primesieve)
endforeach()
