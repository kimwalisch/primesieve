clang++ -I../include -O3 -DNDEBUG -DENABLE_MULTIARCH_AVX512 ../src/*.cpp ../src/app/*.cpp -o primesieve.exe "C:\Program Files\LLVM\lib\clang\18\lib\windows\clang_rt.builtins-x86_64.lib"
