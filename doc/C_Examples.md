# libprimesieve C examples

This is a short selection of C code snippets that use libprimesieve to generate prime numbers.
These examples cover the most frequently used functionality of libprimesieve. Arguably the most
useful feature provided by libprimesieve is the ```primesieve_iterator``` which lets you
iterate over primes using the ```primesieve_next_prime()``` or ```primesieve_prev_prime()```
functions.

Additional libprimesieve documentation links:

* [Install libprimesieve](https://github.com/kimwalisch/primesieve#installation)
* [C API Reference](https://kimwalisch.github.io/primesieve/api)
* [libprimesieve performance tips](https://github.com/kimwalisch/primesieve#libprimesieve-performance-tips)

## ```primesieve_next_prime()```

By default ```primesieve_next_prime()``` generates primes > 0 i.e. 2, 3, 5, 7, ...

```C
#include <primesieve.h>
#include <stdio.h>

int main()
{
  primesieve_iterator it;
  primesieve_init(&it);

  uint64_t sum = 0;
  uint64_t prime = 0;

  /* iterate over the primes < 10^9 */
  while ((prime = primesieve_next_prime(&it)) < 1000000000ull)
    sum += prime;

  printf("Sum of the primes below 10^9 = %" PRIu64 "\n", sum);
  primesieve_free_iterator(&it);

  return 0;
}
```

* [Build instructions](#how-to-compile)

## ```primesieve_skipto()```

This method changes the start number of the ```primesieve_iterator``` object. (By
default the start number is initialized to 0). The ```stop_hint``` paramater is
used for performance optimization, ```primesieve_iterator``` only buffers primes
up to this limit.

```C
#include <primesieve.h>
#include <stdio.h>

int main()
{
  primesieve_iterator it;
  primesieve_init(&it);

  /* primesieve_skipto(&it, start_number, stop_hint) */
  primesieve_skipto(&it, 1000, 1100);
  uint64_t prime;

  /* iterate over primes from ]1000, 1100] */
  while ((prime = primesieve_next_prime(&it)) <= 1100)
    printf("%" PRIu64 "\n", prime);

  primesieve_free_iterator(&it);
  return 0;
}
```

* [Build instructions](#how-to-compile)

## ```primesieve_prev_prime()```

Before using ```primesieve_prev_prime()``` you must first change the start
number using the ```primesieve_skipto()``` function as the start number is
initialized to 0 be default.

```C
#include <primesieve.h>
#include <stdio.h>

int main()
{
  primesieve_iterator it;
  primesieve_init(&it);

  /* primesieve_skipto(&it, start_number, stop_hint) */
  primesieve_skipto(&it, 2000, 1000);
  uint64_t prime;

  /* iterate over primes from ]2000, 1000] */
  while ((prime = primesieve_prev_prime(&it)) >= 1000)
    printf("%" PRIu64 "\n", prime);

  primesieve_free_iterator(&it);
  return 0;
}
```

* [Build instructions](#how-to-compile)

## ```primesieve_generate_primes()```

Stores the primes inside [start, stop] in an array. If you are iterating over
the same primes many times in a loop you will likely get better performance
if you store the primes in an array instead of using a ```primesieve_iterator```
(provided your system has enough memory).

```C
#include <primesieve.h>
#include <stdio.h>

int main()
{
  uint64_t start = 0;
  uint64_t stop = 1000;
  size_t i;
  size_t size;

  /* Get an array with the primes inside [start, stop] */
  int* primes = (int*) primesieve_generate_primes(start, stop, &size, INT_PRIMES);

  for (i = 0; i < size; i++)
    printf("%i\n", primes[i]);

  primesieve_free(primes);
  return 0;
}
```

* [Build instructions](#how-to-compile)

## ```primesieve_generate_n_primes()```

Stores n primes in an array.

```C
#include <primesieve.h>
#include <stdio.h>

int main()
{
  uint64_t n = 1000;
  uint64_t start = 0;
  size_t i;
  size_t size;

  /* Get an array with the first 1000 primes */
  int64_t* primes = (int64_t*) primesieve_generate_n_primes(n, start, INT64_PRIMES);

  for (i = 0; i < n; i++)
    printf("%i\n", primes[i]);

  primesieve_free(primes);
  return 0;
}
```

* [Build instructions](#how-to-compile)

## ```primesieve_count_primes()```

Counts the primes inside [start, stop]. This method is multi-threaded and uses all
available CPU cores by default.

```C
#include <primesieve.h>
#include <stdio.h>

int main()
{
  /* primesieve_count_primes(start, stop) */
  uint64_t count = primesieve_count_primes(0, 1000);
  printf("Primes below 1000 = %" PRIu64 "\n", count);

  return 0;
}
```

* [Build instructions](#how-to-compile)

## ```primesieve_nth_prime()```

This method finds the nth prime e.g. ```nth_prime(25) = 97```. This method is
multi-threaded and uses all available CPU cores by default.

```C
#include <primesieve.h>
#include <stdio.h>

int main(int argc, char** argv)
{
  /* primesieve_nth_prime(n, start) */
  uint64_t n = 25;
  uint64_t prime = primesieve_nth_prime(n, 0);
  printf("%" PRIu64 "th prime = %" PRIu64 "\n", n, prime);

  return 0;
}
```

* [Build instructions](#how-to-compile)

# How to compile

### Unix-like OSes

If [libprimesieve is installed](https://github.com/kimwalisch/primesieve#installation)
on your system, then you can compile any of the C example programs above using:

```sh
cc -O3 primes.c -o primes -lprimesieve
```

If you have [built libprimesieve yourself](BUILD.md#primesieve-build-instructions)
then the default installation path is usually ```/usr/local/lib``` which is not
part of ```LD_LIBRARY_PATH``` on many OSes. Hence you may need to export some
environment variables:

```sh
export LIBRARY_PATH=/usr/local/lib:$LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
export C_INCLUDE_PATH=/usr/local/include:$C_INCLUDE_PATH
```

### Microsoft Visual C++

```sh
cl /O2 /EHsc /MD primes.c /I "path\to\primesieve\include" /link "path\to\primesieve.lib"
```

# CMake support

If you are using the CMake build system to compile your program and
[libprimesieve is installed](https://github.com/kimwalisch/primesieve#installation) on your
system, then you can add the following two lines to your ```CMakeLists.txt``` to link your
program against libprimesieve.

```CMake
find_package(primesieve REQUIRED)
target_link_libraries(your_program primesieve::primesieve)
```

To link against the static libprimesieve use:

```CMake
find_package(primesieve REQUIRED static)
target_link_libraries(your_program primesieve::primesieve)
```

# Minimal CMake project file

If you want to build your C program (named ```primes.c```) using CMake, then you can use
the minimal ```CMakeLists.txt``` below. Note that this requires that
[libprimesieve is installed](https://github.com/kimwalisch/primesieve#installation) on your
system. Using CMake has the advantage that you don't need to specify the libprimesieve include
path and the ```-lprimesieve``` linker option when building your project.

```CMake
# File: CMakeLists.txt
cmake_minimum_required(VERSION 3.4...3.19)
project(primes C CXX)
find_package(primesieve REQUIRED)
add_executable(primes primes.c)
target_link_libraries(primes primesieve::primesieve)
```

Put the ```CMakeLists.txt``` file from above into the same directory as your ```primes.c``` file.<br/>
Then open a terminal, cd into that directory and build your project using:

```
cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Using the MSVC compiler (Windows) the build instructions are slightly different. First you should link
against the static libprimesieve in your ```CMakeLists.txt``` using:
```find_package(primesieve REQUIRED static)```. Next open a Visual Studio Command Prompt, cd into your
project's directory and build your project using:

```
cmake -G "Visual Studio 16 2019" .
cmake --build . --config Release
```
