# primesieve

[![Build Status](https://travis-ci.org/kimwalisch/primesieve.svg)](https://travis-ci.org/kimwalisch/primesieve)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/kimwalisch/primesieve?branch=master&svg=true)](https://ci.appveyor.com/project/kimwalisch/primesieve)
[![Github Releases](https://img.shields.io/github/release/kimwalisch/primesieve.svg)](https://github.com/kimwalisch/primesieve/releases)

primesieve is a program and C/C++ library that generates primes using a highly optimized
<a href="http://en.wikipedia.org/wiki/Sieve_of_Eratosthenes">sieve of
Eratosthenes</a> implementation. It counts the primes below 10^10 in
just 0.45 seconds on an Intel Core i7-6700 CPU (4 x 3.4GHz).
primesieve can generate primes and
<a href="http://en.wikipedia.org/wiki/Prime_k-tuple">prime k-tuplets</a>
up to 2^64.

- **Homepage:** http://primesieve.org
- **Binaries:** http://primesieve.org/downloads
- **API:** http://primesieve.org/api

![primesieve windows screenshot](https://github.com/kimwalisch/primesieve/blob/gh-pages/screenshots/primesieve_win10.png)

## Algorithm complexity

primesieve generates primes using the segmented sieve of Eratosthenes with
[wheel factorization](http://en.wikipedia.org/wiki/Wheel_factorization).
This algorithm has a run time complexity of
<img src="http://primesieve.org/images/Onloglogn.svg" height="20" align="absmiddle"/>
operations and uses
<img src="http://primesieve.org/images/Osqrtn.svg" height="20" align="absmiddle"/>
memory. Furthermore primesieve uses the
[bucket sieve](http://sweet.ua.pt/tos/software/prime_sieve.html)
algorithm for large sieving primes which reduces the memory usage to
<img src="http://primesieve.org/images/primesieve_memory_usage.svg" height="20" align="absmiddle"/>
bytes per thread.

## Installation

The primesieve console application can be installed using your operating
system's package manager. The primesieve GUI application can be
downloaded from
[http://primesieve.org/downloads](http://primesieve.org/downloads).

```sh
# Debian/Ubuntu
sudo apt-get install primesieve

# Windows Bash
sudo apt-get install primesieve

# macOS
brew install primesieve
```

## Usage examples

The primesieve console application can generate primes and prime
k-tuplets.

```sh
# Count the primes below 1e10 using all CPU cores
primesieve 1e10

# Print the primes below 1000000
primesieve 1000000 --print

# Count the primes within [1e10, 2e10] using 4 threads
primesieve 1e10 2e10 --threads=4

# Print an option summary
primesieve --help
```

## Build instructions

Building primesieve requires a compiler which supports C++11 (or later)
and CMake ≥ 3.1. If your compiler does not yet support C++11 you can fall back 
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
* [Browse primesieve's C++ API online](http://primesieve.org/api/primesieve_8hpp.html)

## C API

primesieve's functions are exposed as C API via the ```primesieve.h```
header.

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
* [Browse primesieve's C API online](http://primesieve.org/api/primesieve_8h.html)

## Linking against libprimesieve

#### Unix-like OSes

```sh
c++ -O2 primes.cpp -lprimesieve
cc  -O2 primes.c   -lprimesieve
```

If you have built primesieve yourself then the default installation path
is ```/usr/local/lib``` which is not part of ```LD_LIBRARY_PATH``` on many
OSes. Hence you need to export some environment variables:

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
        <td><a href="http://hackage.haskell.org/package/primesieve">primesieve-haskell</a></td>
    </tr>
</table>

Many thanks to the developers of these bindings!
