clang++ -I../include -O3 -DNDEBUG -DMULTIARCH_AVX512 ../src/*.cpp ../src/app/*.cpp -o primesieve.exe "C:\Program Files\LLVM\lib\clang\17\lib\windows\clang_rt.builtins-x86_64.lib"
