# libprimesieve C API

libprimesieve is a highly optimized library for generating prime numbers, it can generate primes
and [prime k-tuplets](https://en.wikipedia.org/wiki/Prime_k-tuple) up to 2<sup>64</sup>.
This page contains a short selection of C code snippets that use libprimesieve to generate prime
numbers. These examples cover the most frequently used functionality of libprimesieve. Arguably
the most useful feature provided by libprimesieve is the ```primesieve_iterator``` which lets you
iterate over primes using the ```primesieve_next_prime()``` or ```primesieve_prev_prime()```
methods. 

The functions of libprimesieve's C API are defined in the [```<primesieve.h>```](../include/primesieve.h)
and [```<primesieve/iterator.h>```](../include/primesieve/iterator.h) header files. You can
also build libprimesieve's [Doxygen API documentation](BUILD.md#api-documentation) if you need
more detailed information.

## Contents

* [```primesieve_next_prime()```](#primesieve_next_prime)
* [```primesieve_skipto()```](#primesieve_skipto)
* [```primesieve_prev_prime()```](#primesieve_prev_prime)
* [```primesieve_generate_primes()```](#primesieve_generate_primes)
* [```primesieve_generate_n_primes()```](#primesieve_generate_n_primes)
* [```primesieve_count_primes()```](#primesieve_count_primes)
* [```primesieve_nth_prime()```](#primesieve_nth_prime)
* [Error handling](#error-handling)
* [Performance tips](#performance-tips)
* [libprimesieve multi-threading](#libprimesieve-multi-threading)
* [Compiling and linking](#compiling-and-linking)
* [CMake support](#cmake-support)

## ```primesieve_next_prime()```

By default ```primesieve_next_prime()``` generates primes > 0 i.e. 2, 3, 5, 7, ...

```C
#include <primesieve.h>
#include <inttypes.h>
#include <stdio.h>

int main()
{
  primesieve_iterator it;
  primesieve_init(&it);

  uint64_t sum = 0;
  uint64_t prime = 0;

  /* iterate over the primes < 10^9 */
  while ((prime = primesieve_next_prime(&it)) < 1000000000)
    sum += prime;

  printf("Sum of the primes below 10^9 = %" PRIu64 "\n", sum);
  primesieve_free_iterator(&it);

  return 0;
}
```

* [Build instructions](#compiling-and-linking)

## ```primesieve_skipto()```

This method changes the start number of the ```primesieve_iterator``` object. (By
default the start number is initialized to 0). The ```stop_hint``` parameter is
used for performance optimization, ```primesieve_iterator``` only buffers primes
up to this limit.

```C
#include <primesieve.h>
#include <inttypes.h>
#include <stdio.h>

int main()
{
  primesieve_iterator it;
  primesieve_init(&it);

  /* primesieve_skipto(&it, start, stop_hint) */
  primesieve_skipto(&it, 1000, 1100);
  uint64_t prime;

  /* iterate over primes from ]1000, 1100] */
  while ((prime = primesieve_next_prime(&it)) <= 1100)
    printf("%" PRIu64 "\n", prime);

  primesieve_free_iterator(&it);
  return 0;
}
```

* [Build instructions](#compiling-and-linking)

## ```primesieve_prev_prime()```

Before using ```primesieve_prev_prime()``` you must first change the start
number using the ```primesieve_skipto()``` function as the start number is
initialized to 0 be default.

```C
#include <primesieve.h>
#include <inttypes.h>
#include <stdio.h>

int main()
{
  primesieve_iterator it;
  primesieve_init(&it);

  /* primesieve_skipto(&it, start, stop_hint) */
  primesieve_skipto(&it, 2000, 1000);
  uint64_t prime;

  /* iterate over primes from ]2000, 1000] */
  while ((prime = primesieve_prev_prime(&it)) >= 1000)
    printf("%" PRIu64 "\n", prime);

  primesieve_free_iterator(&it);
  return 0;
}
```

* [Build instructions](#compiling-and-linking)

## ```primesieve_generate_primes()```

Stores the primes inside [start, stop] in an array. The last primes ```type``` parameter
may be one of: ```SHORT_PRIMES```, ```USHORT_PRIMES```, ```INT_PRIMES```, ```UINT_PRIMES```,
```LONG_PRIMES```, ```ULONG_PRIMES```, ```LONGLONG_PRIMES```, ```ULONGLONG_PRIMES```,
```INT16_PRIMES```, ```UINT16_PRIMES```, ```INT32_PRIMES```, ```UINT32_PRIMES```,
```INT64_PRIMES```, ```UINT64_PRIMES```.

```C
#include <primesieve.h>
#include <stdio.h>

int main()
{
  uint64_t start = 0;
  uint64_t stop = 1000;
  size_t size;

  /* Get an array with the primes inside [start, stop] */
  int* primes = (int*) primesieve_generate_primes(start, stop, &size, INT_PRIMES);

  for (size_t i = 0; i < size; i++)
    printf("%i\n", primes[i]);

  primesieve_free(primes);
  return 0;
}
```

* [Build instructions](#compiling-and-linking)

## ```primesieve_generate_n_primes()```

Stores the first n primes ≥ start in an array. The last primes ```type``` parameter may
be one of: ```SHORT_PRIMES```, ```USHORT_PRIMES```, ```INT_PRIMES```, ```UINT_PRIMES```,
```LONG_PRIMES```, ```ULONG_PRIMES```, ```LONGLONG_PRIMES```, ```ULONGLONG_PRIMES```,
```INT16_PRIMES```, ```UINT16_PRIMES```, ```INT32_PRIMES```, ```UINT32_PRIMES```,
```INT64_PRIMES```, ```UINT64_PRIMES```.

```C
#include <primesieve.h>
#include <stdio.h>

int main()
{
  uint64_t n = 1000;
  uint64_t start = 0;

  /* Get an array with the first 1000 primes */
  int64_t* primes = (int64_t*) primesieve_generate_n_primes(n, start, INT64_PRIMES);

  for (size_t i = 0; i < n; i++)
    printf("%li\n", primes[i]);

  primesieve_free(primes);
  return 0;
}
```

* [Build instructions](#compiling-and-linking)

## ```primesieve_count_primes()```

Counts the primes inside [start, stop]. This method is multi-threaded and uses all
available CPU cores by default.

```C
#include <primesieve.h>
#include <inttypes.h>
#include <stdio.h>

int main()
{
  /* primesieve_count_primes(start, stop) */
  uint64_t count = primesieve_count_primes(0, 1000);
  printf("Primes below 1000 = %" PRIu64 "\n", count);

  return 0;
}
```

* [Build instructions](#compiling-and-linking)

## ```primesieve_nth_prime()```

This method finds the nth prime e.g. ```nth_prime(25) = 97```. This method is
multi-threaded and uses all available CPU cores by default.

```C
#include <primesieve.h>
#include <inttypes.h>
#include <stdio.h>

int main()
{
  /* primesieve_nth_prime(n, start) */
  uint64_t n = 25;
  uint64_t prime = primesieve_nth_prime(n, 0);
  printf("%" PRIu64 "th prime = %" PRIu64 "\n", n, prime);

  return 0;
}
```

* [Build instructions](#compiling-and-linking)

# Error handling

If an error occurs, libprimesieve functions with a ```uint64_t``` return type return
```PRIMESIEVE_ERROR``` (which is defined as ```UINT64_MAX``` in ```<primesieve.h>```)
and the corresponding error message is printed to the standard error stream.

```C
#include <primesieve.h>
#include <inttypes.h>
#include <stdio.h>

int main()
{
  uint64_t count = primesieve_count_primes(0, 1000);

  if (count != PRIMESIEVE_ERROR)
    printf("Primes below 1000 = %" PRIu64 "\n", count);
  else
    printf("Error in libprimesieve!\n");

  return 0;
}
```

libprimesieve also sets the C ```errno``` variable to ```EDOM``` if an error
occurs. This makes it possible to check if an error has occurred in libprimesieve
functions with a ```void``` return type. ```errno``` is also useful for checking
after a computation that no error has occurred, this way you don't have to
check the return value of every single primesieve function call.

```C
#include <primesieve.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>

int main()
{
  /* Reset errno before computation */
  errno = 0;

  primesieve_iterator it;
  primesieve_init(&it);
  uint64_t sum = 0;
  uint64_t prime = 0;

  while ((prime = primesieve_next_prime(&it)) < 1000000000)
    sum += prime;

  /* Check errno after computation */
  if (errno != EDOM)
    printf("Sum of the primes below 10^9 = %" PRIu64 "\n", sum);
  else
    printf("Error in libprimesieve!\n");

  primesieve_free_iterator(&it);
  return 0;
}
```

# Performance tips

* ```primesieve_next_prime()``` runs up to 2x faster and uses only
half as much memory as ```primesieve_prev_prime()```. Oftentimes algorithms
that iterate over primes using ```primesieve_prev_prime()``` can be rewritten
using ```primesieve_next_prime()``` which improves performance in most cases.

* ```primesieve_iterator``` is single-threaded. See the
[multi-threading](#libprimesieve-multi-threading) section for how to
parallelize an algorithm using multiple ```primesieve_iterator``` objects.

* The ```primesieve_skipto()``` method takes an optional ```stop_hint```
parameter that can provide a significant speedup if the sieving distance
is relatively small e.g.&nbsp;<&nbsp;sqrt(start). If ```stop_hint``` is set
```primesieve_iterator``` will only buffer primes up to this limit.

* Many of libprimesieve's functions e.g. ```primesieve_count_primes(start, stop)``` &
```primesieve_nth_prime(n, start)``` incur an initialization overhead of O(sqrt(start))
even if the total sieving distance is tiny. It is therefore not a good idea to
call these functions repeatedly in a loop unless the sieving distance is
sufficiently large e.g. >&nbsp;sqrt(start). If the sieving distance is mostly
small consider using a ```primesieve_iterator``` instead to avoid the
recurring initialization overhead.

# libprimesieve multi-threading

By default libprimesieve uses multi-threading for counting primes/k-tuplets
and for finding the nth prime. However ```primesieve_iterator``` the most
useful feature provided by libprimesieve runs single-threaded because
it is simply not possible to efficiently parallelize the generation of primes
in sequential order.

Hence if you want to parallelize an algorithm using ```primesieve_iterator```
you need to implement the multi-threading part yourself. The basic technique
for parallelizing an algorithm using ```primesieve_iterator``` is:

* Subdivide the sieving distance into equally sized chunks.
* Process each chunk in its own thread.
* Combine the partial thread results to get the final result.

The C example below calculates the sum of the primes ≤ 10<sup>10</sup> in parallel
using [OpenMP](https://en.wikipedia.org/wiki/OpenMP). Each thread processes a
chunk of size ```(dist / threads) + 1``` using its own ```primesieve_iterator```
object. The OpenMP reduction clause takes care of adding the partial
prime sum results together in a thread safe manner.

```C
#include <primesieve.h>
#include <stdio.h>
#include <omp.h>

int main()
{
  uint64_t sum = 0;
  uint64_t dist = 1e10;
  int threads = omp_get_max_threads();
  uint64_t thread_dist = (dist / threads) + 1;

  #pragma omp parallel for reduction(+: sum)
  for (int i = 0; i < threads; i++)
  {
    uint64_t start = i * thread_dist;
    uint64_t stop = start + thread_dist < dist ? start + thread_dist : dist;
    primesieve_iterator it;
    primesieve_init(&it);
    primesieve_skipto(&it, start, stop);
    uint64_t prime = primesieve_next_prime(&it);

    for (; prime <= stop; prime = primesieve_next_prime(&it))
      sum += prime;
  }

  printf("Sum of the primes below 10^10 = %" PRIu64 "\n", sum);

  return 0;
}
```

<details>
<summary>Build instructions</summary>

```bash
# Unix-like OSes
cc -O3 -fopenmp primesum.c -o primesum -lprimesieve
time ./primesum
```

</details>

# Compiling and linking

### Unix-like OSes

If [libprimesieve is installed](https://github.com/kimwalisch/primesieve#installation)
on your system, then you can compile any of the C example programs above using:

```sh
cc -O3 primes.c -o primes -lprimesieve
```

If you have [built libprimesieve yourself](BUILD.md#primesieve-build-instructions),
then the default installation path is usually ```/usr/local/lib```. Running
the ```ldconfig``` program after ```make install``` ensures that Linux's dynamic
linker/loader will find the shared primesieve library when you execute your program.
However, some OSes are missing the ```ldconfig``` program or ```ldconfig``` does
not include ```/usr/local/lib``` by default. In these cases you need to export
some environment variables:

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

```sh
cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Using the MSVC compiler (Windows) the build instructions are slightly different. First you should link
against the static libprimesieve in your ```CMakeLists.txt``` using:
```find_package(primesieve REQUIRED static)```. Next open a Visual Studio Command Prompt, cd into your
project's directory and build your project using:

```sh
cmake -G "Visual Studio 16 2019" .
cmake --build . --config Release
```
