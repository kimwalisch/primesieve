# libprimesieve C++ API

libprimesieve is a highly optimized library for generating prime numbers, it can generate primes
and [prime k-tuplets](https://en.wikipedia.org/wiki/Prime_k-tuple) up to 2<sup>64</sup>.
libprimesieve generates primes using the segmented
[sieve of Eratosthenes](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes) with
[wheel factorization](https://en.wikipedia.org/wiki/Wheel_factorization).
This algorithm has a run time complexity of $O(n\ \log\ \log\ n)$ operations and uses
$O(\sqrt{n})$ memory. This page contains a selection of C++ code snippets that show how to use
libprimesieve to generate prime numbers. These examples cover the most frequently used
functionality of libprimesieve. Arguably the most useful feature provided by libprimesieve is the
```primesieve::iterator``` which lets you iterate over primes using the ```next_prime()``` or
```prev_prime()``` methods.

The functions of libprimesieve's C++ API are defined in the [```<primesieve.hpp>```](../include/primesieve.hpp)
and [```<primesieve/iterator.hpp>```](../include/primesieve/iterator.hpp) header files. If you
need detailed information about libprimesieve's function signatures, e.g. because you want to
write libprimesieve bindings for another programming language, then I suggest you read
the libprimesieve header files which also contain additional documentation about the function
parameters and return values.

## Contents

* [```primesieve::iterator::next_prime()```](#primesieveiteratornext_prime)
* [```primesieve::iterator::jump_to()```](#primesieveiteratorjump_to-since-primesieve-110)
* [```primesieve::iterator::prev_prime()```](#primesieveiteratorprev_prime)
* [```primesieve::generate_primes()```](#primesievegenerate_primes)
* [```primesieve::generate_n_primes()```](#primesievegenerate_n_primes)
* [```primesieve::count_primes()```](#primesievecount_primes)
* [```primesieve::nth_prime()```](#primesieventh_prime)
* [Error handling](#error-handling)
* [Performance tips](#performance-tips)
* [Multi-threading](#Multi-threading)
* [SIMD (vectorization)](#SIMD-vectorization)
* [Compiling and linking](#compiling-and-linking)
* [pkgconf support](#pkgconf-support)
* [CMake support](#cmake-support)

## ```primesieve::iterator::next_prime()```

By default ```primesieve::iterator::next_prime()``` generates primes ≥ 0 i.e. 2, 3, 5, 7, ...

* If you have specified a non-default start number in the ```primesieve::iterator```
  constructor or in the ```jump_to()``` method, then the first ```next_prime()``` invocation
  returns the first prime ≥ start number. If want to generate primes > start number you need to
  use e.g. ```jump_to(start+1)```.
* Note that ```primesieve::iterator``` is not ideal if you are
  repeatedly iterating over the same primes in a loop, in this case it is better
  to [store the primes in a vector](#primesievegenerate_primes) (provided your PC has
  sufficient RAM memory).
* If needed, you can also use multiple ```primesieve::iterator``` objects within the
  same program.

```C++
#include <primesieve.hpp>
#include <iostream>

int main()
{
  primesieve::iterator it;
  uint64_t prime = it.next_prime();
  uint64_t sum = 0;

  // Iterate over the primes <= 10^9
  for (; prime <= 1000000000; prime = it.next_prime())
    sum += prime;

  std::cout << "Sum of the primes <= 10^9: " << sum << std::endl;

  return 0;
}
```

* [Build instructions](#compiling-and-linking)

## ```primesieve::iterator::jump_to()``` <sub><sup>*(since primesieve-11.0)*</sup></sub>

This method changes the start number of the ```primesieve::iterator``` object. (By default
the start number is initialized to 0). Note that you can also specify the start number in
the constructor of the ```primesieve::iterator``` object.

* The first ```next_prime()``` call after ```jump_to()``` returns the first
  prime ≥ start number. If want to generate primes > start number you need to use e.g.
  ```jump_to(start+1)```.
* The first ```next_prime()``` call after ```jump_to()``` incurs an initialization
  overhead of $O(\sqrt{start}\ \times\ \log\ \log\ \sqrt{start})$ operations. After that, any
  additional ```next_prime()``` call executes in amortized
  $O(\log\ n\ \times\ \log\ \log\ n)$ operations.

```C++
#include <primesieve.hpp>
#include <iostream>

int main()
{
  primesieve::iterator it;

  // Iterate over primes >= 1000
  it.jump_to(1000);
  uint64_t prime = it.next_prime();

  // Iterate over primes from [1000, 1100]
  for (; prime <= 1100; prime = it.next_prime())
    std::cout << prime << std::endl;

  return 0;
}
```

* [Build instructions](#compiling-and-linking)

The ```primesieve::iterator::jump_to()``` method (and the ```primesieve::iterator```
constructor) take an optional ```stop_hint``` parameter for performance optimization.
If ```stop_hint``` is set ```primesieve::iterator``` will only buffer primes up to
this limit.

```C++
#include <primesieve.hpp>
#include <iostream>

int main()
{
  uint64_t start = 1000;
  uint64_t stop_hint = 1100;

  // Iterate over primes >= start
  primesieve::iterator it(start, stop_hint);
  uint64_t prime = it.next_prime();

  // Iterate over primes from [1000, 1100]
  for (; prime <= 1100; prime = it.next_prime())
    std::cout << prime << std::endl;

  return 0;
}
```

* [Build instructions](#compiling-and-linking)

## ```primesieve::iterator::skipto()``` <sub><sup>*(removed in primesieve-11.0)*</sup></sub>

Similar to ```primesieve::iterator::jump_to()```, the ```primesieve::iterator::skipto()```
method changes the start number of the ```primesieve::iterator``` object. However, when
calling ```next_prime()``` or ```prev_prime()``` for the first time the start number will
be excluded. Hence ```next_prime()``` will generate primes > start and ```prev_prime()```
will generate primes < start. ```primesieve::iterator::skipto()``` has been replaced by
```primesieve::iterator::jump_to()``` in primesieve-11.0, because the use of the
```skipto()``` method required to correct the start number in most cases using e.g.
```iter.skipto(start-1)```.

* The first ```next_prime()``` call after ```skipto()``` incurs an initialization
  overhead of $O(\sqrt{start}\ \times\ \log\ \log\ \sqrt{start})$ operations. After that, any
  additional ```next_prime()``` call executes in amortized
  $O(\log\ n\ \times\ \log\ \log\ n)$ operations.

```C++
#include <primesieve.hpp>
#include <iostream>

int main()
{
  primesieve::iterator it;

  // Iterate over primes > 13
  it.skipto(13);
  uint64_t prime = it.next_prime();

  // Iterate over primes from ]13, 1100]
  for (; prime <= 1100; prime = it.next_prime())
    std::cout << prime << std::endl;

  return 0;
}
```

* [Build instructions](#compiling-and-linking)

## ```primesieve::iterator::prev_prime()```

Before using ```primesieve::iterator::prev_prime()``` you must change the start number
either in the constructor or using the ```jump_to()``` method (because the start number is
initialized to 0 be default).

* Please note that the first ```prev_prime()``` invocation returns the first prime ≤ start number.
  If want to generate primes < start number you need to use e.g. ```jump_to(start-1)```.
* As a special case, ```prev_prime()``` returns 0 after the prime 2 (i.e. when there are no more
  primes). This makes it possible to conveniently iterate backwards over all primes > 0 as can be
  seen in the example below.

```C++
#include <primesieve.hpp>
#include <iostream>

int main()
{
  // Iterate over primes <= 1000
  primesieve::iterator it(1000);
  uint64_t prime = it.prev_prime();

  // Iterate over primes from [1000, 0[
  for (; prime > 0; prime = it.prev_prime())
    std::cout << prime << std::endl;

  return 0;
}
```

* [Build instructions](#compiling-and-linking)

## ```primesieve::generate_primes()```

Stores the primes inside [start, stop] in a ```std::vector```. If you are repeatedly iterating over the same primes
many times in a loop you will likely get better performance if you store the primes in a vector
instead of using a ```primesieve::iterator``` (provided your system has enough memory).

```C++
#include <primesieve.hpp>
#include <vector>

int main()
{
  std::vector<int> primes;

  // Store primes <= 1000
  primesieve::generate_primes(1000, &primes);

  primes.clear();

  // Store primes inside [1000, 2000]
  primesieve::generate_primes(1000, 2000, &primes);

  return 0;
}
```

* [Build instructions](#compiling-and-linking)

## ```primesieve::generate_n_primes()```

Stores n primes in a ```std::vector```.

```C++
#include <primesieve.hpp>
#include <vector>

int main()
{
  std::vector<int> primes;

  // Store first 1000 primes
  primesieve::generate_n_primes(1000, &primes);

  primes.clear();

  // Store first 10 primes >= 1000
  primesieve::generate_n_primes(10, 1000, &primes);

  return 0;
}
```

* [Build instructions](#compiling-and-linking)

## ```primesieve::count_primes()```

Counts the primes inside [start, stop]. This function is multi-threaded and uses all
available CPU cores by default.

```C++
#include <primesieve.hpp>
#include <iostream>

int main()
{
  uint64_t count = primesieve::count_primes(0, 1000);
  std::cout << "Primes <= 1000: " << count << std::endl;

  return 0;
}
```

* [Build instructions](#compiling-and-linking)

## ```primesieve::nth_prime()```

This function finds the nth prime e.g. ```nth_prime(25) = 97```. This function is
multi-threaded and uses all available CPU cores by default.

```C++
#include <primesieve.hpp>
#include <iostream>

int main()
{
  uint64_t n = 25;
  uint64_t nth_prime = primesieve::nth_prime(n);
  std::cout << n << "th prime = " << nth_prime << std::endl;

  return 0;
}
```

* [Build instructions](#compiling-and-linking)

# Error handling

If an error occurs libprimesieve throws a ```primesieve::primesieve_error``` exception that is
derived from ```std::runtime_error```. Note that libprimesieve very rarely throws an exception,
the two main cases which will trigger an exception are: memory allocation failure (throws
```std::bad_alloc```) and trying to generate primes > 2^64 (throws
```primesieve::primesieve_error```).

```C++
#include <primesieve.hpp>
#include <iostream>

int main()
{
  try
  {
    // Try generating primes > 2^64
    uint64_t start = ~0ull - 1;
    uint64_t n = 1000;
    std::vector<uint64_t> primes;
    primesieve::generate_n_primes(n, start, &primes);
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
```

# Performance tips

* If you are repeatedly iterating over the same primes in a loop, you should
use ```primesieve::generate_primes()``` or
```primesieve::generate_n_primes()``` to store these primes in a vector
(provided your PC has sufficient RAM memory) instead of using a
```primesieve::iterator```.

* ```primesieve::iterator::next_prime()``` runs up to 2x faster and uses only
half as much memory as ```prev_prime()```. Oftentimes algorithms that iterate
over primes using ```prev_prime()``` can be rewritten using ```next_prime()```
which improves performance in most cases.

* ```primesieve::iterator``` is single-threaded. See the
[Multi-threading](#Multi-threading) section for how to
parallelize an algorithm using multiple ```primesieve::iterator``` objects.

*  The ```primesieve::iterator``` data structure allows you to access the underlying
64-bit ```primes``` array, together with the ```generate_next_primes()``` method,
this can be used for all kinds of low-level optimizations. E.g. the
[SIMD (vectorization)](#SIMD-vectorization) section contains an example that shows how
to process primes using SIMD instructions.

* The ```primesieve::iterator``` constructor and the
```primesieve::iterator::jump_to()``` method take an optional ```stop_hint```
parameter that can provide a significant speedup if the sieving distance
is relatively small e.g.&nbsp;<&nbsp;sqrt(start). If ```stop_hint``` is set
```primesieve::iterator``` will only buffer primes up to this limit.

* Many of libprimesieve's functions e.g. ```count_primes(start, stop)``` &
```nth_prime(n, start)``` incur an initialization overhead of $O(\sqrt{start})$
even if the total sieving distance is tiny. It is therefore not a good idea to
call these functions repeatedly in a loop unless the sieving distance is
sufficiently large e.g. >&nbsp;sqrt(start). If the sieving distance is mostly
small consider using a ```primesieve::iterator``` instead to avoid the
recurring initialization overhead.

# Multi-threading

By default libprimesieve uses multi-threading for counting primes/k-tuplets
and for finding the nth prime. However ```primesieve::iterator``` the most
useful feature provided by libprimesieve runs single-threaded because
it is simply not possible to efficiently parallelize the generation of primes
in sequential order.

Hence if you want to parallelize an algorithm using ```primesieve::iterator```
you need to implement the multi-threading part yourself. The basic technique
for parallelizing an algorithm using ```primesieve::iterator``` is:

* Subdivide the sieving distance into equally sized chunks.
* Process each chunk in its own thread.
* Combine the partial thread results to get the final result.

The C++ example below calculates the sum of the primes ≤ 10<sup>10</sup> in parallel
using [OpenMP](https://en.wikipedia.org/wiki/OpenMP). Each thread processes a
chunk of size ```(dist / threads) + 1``` using its own ```primesieve::iterator```
object. The OpenMP reduction clause takes care of adding the partial
prime sum results together in a thread safe manner.

```C++
#include <primesieve.hpp>
#include <iostream>
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
    uint64_t stop = std::min(start + thread_dist, dist + 1);
    primesieve::iterator it(start, stop);
    uint64_t prime = it.next_prime();

    // Sum primes inside [start, stop[
    for (; prime < stop; prime = it.next_prime())
      sum += prime;
  }

  std::cout << "Sum of the primes <= " << dist << ": " << sum << std::endl;

  return 0;
}
```

<details>
<summary>Build instructions</summary>

```bash
# Unix-like OSes
c++ -O3 -fopenmp primesum.cpp -o primesum -lprimesieve
time ./primesum
```

</details>

# SIMD (vectorization)

SIMD stands for Single Instruction/Multiple Data, it is also commonly known as
vectorization. SIMD is supported by most CPUs e.g. all ARM64 CPUs support the ARM NEON
instruction set and most x64 CPUs support the AVX2 or AVX512 instruction sets. Using
SIMD instructions can significantly speed up some algorithms. The
```primesieve::iterator``` data structure allows you to access the underlying 64-bit
```primes``` array and process its elements using SIMD instructions.

The C++ example below calculates the sum of all primes ≤ 10^10 using the AVX512 vector
instruction set for x64 CPUs. This code uses the ```generate_next_primes()```
method to generate the next 2^10 primes in a loop and then calculates their sum using
AVX512 vector intrinsics. Note that ```generate_next_primes()``` is also used under
the hood by the ```next_prime()``` method.

```C
#include <primesieve.hpp>
#include <immintrin.h>
#include <iostream>

int main()
{
  primesieve::iterator it;
  it.generate_next_primes();

  uint64_t limit = 10000000000;
  __m512i sums = _mm512_setzero_si512();

  while (it.primes_[it.size_ - 1] <= limit)
  {
    // Sum 64-bit primes using AVX512
    for (std::size_t i = 0; i < it.size_; i += 8) {
      __mmask8 mask = (i + 8 < it.size_) ? 0xff : 0xff >> (i + 8 - it.size_);
      __m512i primes = _mm512_maskz_loadu_epi64(mask, (__m512i*) &it.primes_[i]);
      sums = _mm512_add_epi64(sums, primes);
    }

    // Generate up to 2^10 new primes
    it.generate_next_primes();
  }

  // Sum the 8 partial sums
  uint64_t sum = _mm512_reduce_add_epi64(sums);

  // Process the remaining primes (at most 2^10)
  for (std::size_t i = 0; it.primes_[i] <= limit; i++)
    sum += it.primes_[i];

  std::cout << "Sum of the primes <= " << limit << ": " << sum << std::endl;

  return 0;
}
```

<details>
<summary>Build instructions</summary>

```bash
# Unix-like OSes
c++ -O3 -mavx512f -funroll-loops primesum.cpp -o primesum -lprimesieve
time ./primesum
```

</details>

# Compiling and linking

### Unix-like OSes

If [libprimesieve is installed](https://github.com/kimwalisch/primesieve#installation)
on your system, then you can compile any of the C++ example programs above using:

```sh
c++ -O3 primes.cpp -o primes -lprimesieve
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
export CPLUS_INCLUDE_PATH=/usr/local/include:$CPLUS_INCLUDE_PATH
```

### Microsoft Visual C++

```sh
cl /O2 /EHsc /MD primes.cpp /I "path\to\primesieve\include" /link "path\to\primesieve.lib"
```

# pkgconf support

primesieve also has support for the
[pkgconf](https://github.com/pkgconf/pkgconf) program which
allows to easily compile C and C++ programs depending on libprimesieve
without having to care about the library and include paths:

```sh
c++ -O3 main.cpp -o main $(pkgconf --libs --cflags primesieve)
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

## Minimal CMake project file

If you want to build your C++ program (named ```primes.cpp```) using CMake, then you can use
the minimal ```CMakeLists.txt``` below. Note that this requires that
[libprimesieve is installed](https://github.com/kimwalisch/primesieve#installation) on your
system. Using CMake has the advantage that you don't need to specify the libprimesieve include
path and the ```-lprimesieve``` linker option when building your project.

```CMake
# File: CMakeLists.txt
cmake_minimum_required(VERSION 3.4...3.19)
project(primes CXX)
find_package(primesieve REQUIRED)
add_executable(primes primes.cpp)
target_link_libraries(primes primesieve::primesieve)
```

Put the ```CMakeLists.txt``` file from above into the same directory as your ```primes.cpp``` file.<br/>
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
