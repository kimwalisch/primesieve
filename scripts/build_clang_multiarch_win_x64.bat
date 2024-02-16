clang++ -I../include -O3 -DNDEBUG -DMULTIARCH_POPCNT_BMI -DMULTIARCH_AVX512 -c ../src/*.cpp ../src/app/*.cpp
clang-cl *.o /link "C:\Program Files\LLVM\lib\clang\17\lib\windows\clang_rt.builtins-x86_64.lib" /OUT:primesieve.exe
