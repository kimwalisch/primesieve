# primesieve

[![Build Status](https://travis-ci.org/kimwalisch/primesieve.svg)](https://travis-ci.org/kimwalisch/primesieve)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/kimwalisch/primesieve?branch=master&svg=true)](https://ci.appveyor.com/project/kimwalisch/primesieve)
[![Github Releases](https://img.shields.io/github/release/kimwalisch/primesieve.svg)](https://github.com/kimwalisch/primesieve/releases)

primesieve is a program and C/C++ library that generates primes using a highly optimized
[sieve of Eratosthenes](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes)
implementation. It counts the primes below 10^10 in just 0.4 seconds on an
Intel Core i7-6700 CPU (4 x 3.4 GHz). primesieve can generate primes and
[prime k-tuplets](https://en.wikipedia.org/wiki/Prime_k-tuple) up to 2^64.

* **Homepage:** https://primesieve.org
* **Binaries:** https://primesieve.org/downloads
* **API:** https://primesieve.org/api

![primesieve windows screenshot](https://github.com/kimwalisch/primesieve/blob/gh-pages/screenshots/primesieve_win10.png)

## Algorithms

primesieve generates primes using the segmented sieve of Eratosthenes with
[wheel factorization](https://en.wikipedia.org/wiki/Wheel_factorization).
This algorithm has a run time complexity of
<img src="https://primesieve.org/images/Onloglogn.svg" height="20" align="absmiddle"/>
operations and uses
<img src="https://primesieve.org/images/Osqrtn.svg" height="20" align="absmiddle"/>
memory. Furthermore primesieve uses the
[bucket sieve](http://sweet.ua.pt/tos/software/prime_sieve.html)
algorithm which improves the cache efficiency when generating primes > 2^32.
primesieve uses 8 bytes per sieving prime, hence its memory usage is about
<img src="http://primesieve.org/images/primesieve_memory_usage.svg" height="20" align="absmiddle"/>
bytes per thread.

## Installation

The primesieve console application can be installed using your operating system's
package manager. You can also download the primesieve console and GUI applications
from [https://primesieve.org/downloads](https://primesieve.org/downloads).

```sh
# Debian, Ubuntu
sudo apt install primesieve

# Homebrew (macOS) or Linuxbrew
brew install primesieve

# Windows using Chocolatey package manager
choco install primesieve
```

## Usage examples

```sh
# Count the primes below 1e10 using all CPU cores
primesieve 1e10

# Print the primes below 1000000
primesieve 1000000 --print

# Print the twin primes below 1000000
primesieve 1000000 --print=2

# Count the prime triplets inside [1e10, 1e10+2^32]
primesieve 1e10 --dist=2^32 --count=3
```

## Command-line options

```
Usage: primesieve [START] STOP [OPTION]...
Generate the primes and/or prime k-tuplets inside [START, STOP]
(< 2^64) using the segmented sieve of Eratosthenes.

Options:
  -c[N+], --count[=N+]   Count primes and prime k-tuplets, N <= 6,
                         e.g. -c1 primes, -c2 twins, -c3 triplets, ...
          --cpu-info     Print CPU information
  -d<N>,  --dist=<N>     Sieve the interval [START, START + N]
  -h,     --help         Print this help menu
  -n,     --nth-prime    Calculate the nth prime,
                         e.g. 1 100 -n finds the 1st prime > 100
          --no-status    Turn off the progressing status
  -p[N],  --print[=N]    Print primes or prime k-tuplets, N <= 6,
                         e.g. -p1 primes, -p2 twins, -p3 triplets, ...
  -q,     --quiet        Quiet mode, prints less output
  -s<N>,  --size=<N>     Set the sieve size in KiB, N <= 4096
          --test         Run various sieving tests
  -t<N>,  --threads=<N>  Set the number of threads, N <= CPU cores
          --time         Print the time elapsed in seconds
  -v,     --version      Print version and license information
```

## Build instructions

Building primesieve requires a compiler which supports C++11 (or later)
and CMake ≥ 3.4. If your compiler does not yet support C++11 you can fall back 
to [primesieve-5.7.3](https://github.com/kimwalisch/primesieve/tree/v5.7.3)
which is written in C++98.

```sh
cmake .
make -j
sudo make install
```

#### Build C/C++ examples

```sh
cmake -DBUILD_EXAMPLES=ON .
make -j
```

#### Run the tests

```sh
cmake -DBUILD_TESTS=ON .
make -j
make test
```

## C++ API

Below is an example with the most common libprimesieve use cases.

```C++
#include <primesieve.hpp>
#include <iostream>
#include <vector>

int main()
{
  // store the primes below 1000
  std::vector<int> primes;
  primesieve::generate_primes(1000, &primes);

  primesieve::iterator it;
  uint64_t prime = it.next_prime();

  // iterate over the primes below 10^6
  for (; prime < 1000000; prime = it.next_prime())
    std::cout << prime << std::endl;

  return 0;
}
```

* [More C++ examples](examples/cpp)
* [Browse primesieve's C++ API online](https://primesieve.org/api/primesieve_8hpp.html)

## C API

primesieve's functions are exposed as C API via the ```primesieve.h``` header.

```C
#include <primesieve.h>
#include <stdio.h>

int main()
{
  primesieve_iterator it;
  primesieve_init(&it);
  uint64_t prime;

  /* iterate over the primes below 10^6 */
  while ((prime = primesieve_next_prime(&it)) < 1000000)
    printf("%llu\n", prime);

  primesieve_free_iterator(&it);
  return 0;
}
```

* [More C examples](examples/c)
* [Browse primesieve's C API online](https://primesieve.org/api/primesieve_8h.html)

## Linking against libprimesieve

#### Unix-like OSes

```sh
c++ -O2 primes.cpp -lprimesieve
cc  -O2 primes.c   -lprimesieve
```

If you have built primesieve yourself then the default installation path is 
```/usr/local/lib``` which is not part of ```LD_LIBRARY_PATH``` on many OSes.
Hence you need to export some environment variables:

```sh
export LIBRARY_PATH=/usr/local/lib:$LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
export CPLUS_INCLUDE_PATH=/usr/local/include:$CPLUS_INCLUDE_PATH
export C_INCLUDE_PATH=/usr/local/include:$C_INCLUDE_PATH
```

#### Microsoft Visual C++

```sh
cl /O2 /EHsc primes.cpp /I primesieve\include /link primesieve.lib
```

## CMake support

Since primesieve-6.4 you can easily link against libprimesieve in your
```CMakeLists.txt```:

```CMake
find_package(primesieve REQUIRED)
target_link_libraries(your_target primesieve::primesieve)
```

To link against the static libprimesieve use:

```CMake
find_package(primesieve REQUIRED static)
target_link_libraries(your_target primesieve::primesieve)
```

## Bindings for other languages

primesieve natively supports C and C++ and has bindings available for:

<table>
    <tr>
        <td><b>Python:</b></td>
        <td><a href="https://github.com/hickford/primesieve-python">primesieve-python</a></td>
    </tr>
    <tr>
        <td><b>Perl:</b></td>
        <td><a href="https://github.com/CurtTilmes/perl6-primesieve">perl6-primesieve</a></td>
    </tr>
    <tr>
        <td><b>Ruby:</b></td>
        <td><a href="https://github.com/robertjlooby/primesieve-ruby">primesieve-ruby</a></td>
    </tr>
    <tr>
        <td><b>Rust:</b></td>
        <td><a href="https://github.com/pthariensflame/primesieve.rs">primesieve.rs</a></td>
    </tr>
    <tr>
        <td><b>Haskell:</b></td>
        <td><a href="https://hackage.haskell.org/package/primesieve">primesieve-haskell</a></td>
    </tr>
</table>

Many thanks to the developers of these bindings!
