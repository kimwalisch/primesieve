primesieve
==========
[![Build Status](https://travis-ci.org/kimwalisch/primesieve.svg)](https://travis-ci.org/kimwalisch/primesieve)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/kimwalisch/primesieve?branch=master&svg=true)](https://ci.appveyor.com/project/kimwalisch/primesieve)
[![GitHub license](https://img.shields.io/badge/license-BSD%202-blue.svg)](https://github.com/kimwalisch/primesieve/blob/master/COPYING)

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

Algorithm complexity
--------------------

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

Package managers
----------------

The primesieve console application can be installed using your operating
system's package manager. The primesieve GUI application can be
downloaded from
[http://primesieve.org/downloads](http://primesieve.org/downloads).

```sh
# Debian/Ubuntu
sudo apt-get install primesieve

# macOS
brew install primesieve
```

Console application
-------------------

The primesieve console application can generate primes and prime
k-tuplets.

```sh
# Count the primes below 1e10 using all CPU cores
primesieve 1e10

# Print the primes below 1000000
primesieve 1000000 --print

# Print the twin primes below 1000000
primesieve 1000000 --print=2

# Count the primes within [1e10, 2e10] using 4 threads
primesieve 1e10 2e10 --threads=4

# Print an option summary
primesieve --help
```

Build instructions
------------------

Building primesieve requires a compiler which supports C++11 (or later)
and CMake ≥ 3.1. If your compiler does not yet support C++11 you can
fall back to [primesieve-5.x](https://github.com/kimwalisch/primesieve/tree/v5.7.3)
which is written in C++98.

```sh
cmake .
make -j8
sudo make install
```

To build the C/C++ examples programs use:

```sh
cmake -DBUILD_EXAMPLES=ON .
make -j8
```

To build and run the tests use:

```sh
cmake -DBUILD_TESTS=ON .
make -j8
make test
```

C++ API
-------

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

C API
-----

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

Linking against libprimesieve
-----------------------------

**Unix-like operating systems**
```sh
c++ -O2 primes.cpp -lprimesieve
cc  -O2 primes.c   -lprimesieve
```

If you have built primesieve yourself then the default installation path is
```/usr/local/lib``` which is not part of ```LD_LIBRARY_PATH``` on many
OSes. Hence you need to export some environment variables:

```sh
export LIBRARY_PATH=/usr/local/lib:$LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
export CPLUS_INCLUDE_PATH=/usr/local/include:$CPLUS_INCLUDE_PATH
export C_INCLUDE_PATH=/usr/local/include:$C_INCLUDE_PATH
```

**Microsoft Visual C++ (Windows)**
```
cl /O2 /EHsc primes.cpp /I primesieve\include /link primesieve\primesieve.lib
```

Bindings for other languages
----------------------------

primesieve supports C++ and C directly, and has bindings available for
a few other languages:

<table>
    <tr>
        <td><b>Python:</b></td>
        <td><a href="https://github.com/hickford/primesieve-python">primesieve-python</a></td>
    </tr>
    <tr>
        <td><b>Ruby:</b></td>
        <td><a href="https://github.com/robertjlooby/primesieve-ruby">primesieve-ruby</a></td>
    </tr>
</table>

Many thanks to the developers of these bindings!
