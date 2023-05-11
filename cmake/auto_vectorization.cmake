# The andBuffers() function in PreSieve.cpp is important for
# performance and therefore it is important that this function is
# auto-vectorized by the compiler. For GCC & Clang we can enable
# auto vectorization using -ftree-vectorize.

# GCC/Clang enable auto-vectorization with -O2 and -O3, but for -O2
# GCC uses the "very-cheap" cost model which prevents our andBuffers()
# function from getting auto vectorized. But compiling with e.g.
# "-O2 -ftree-vectorize -fvect-cost-model=dynamic" fixes this issue.

include(CheckCXXCompilerFlag)

cmake_push_check_state()
set(CMAKE_REQUIRED_FLAGS -Werror)
check_cxx_compiler_flag(-ftree-vectorize ftree_vectorize)
cmake_pop_check_state()

if(ftree_vectorize)
    set(FTREE_VECTORIZE_FLAG "-ftree-vectorize")

    cmake_push_check_state()
    set(CMAKE_REQUIRED_FLAGS -Werror)
    check_cxx_compiler_flag(-fvect-cost-model=dynamic fvect_cost_model)
    cmake_pop_check_state()

    if(fvect_cost_model)
        set(FVECT_COST_MODEL_FLAG "-fvect-cost-model=dynamic")
    endif()
endif()
