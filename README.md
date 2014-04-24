primesieve
==========
[![Build Status](https://travis-ci.org/kimwalisch/primesieve.svg)](https://travis-ci.org/kimwalisch/primesieve)

primesieve is a free (BSD-licensed) software program and C/C++
library that generates primes using a highly optimized
[sieve of Eratosthenes](http://en.wikipedia.org/wiki/Sieve_of_Eratosthenes)
implementation. It generates the primes below 10^9 in just 0.2 seconds
on a single core of an Intel Core i7-4770 3.4GHz CPU from 2013.
primesieve can generate primes and
[prime&#160;k&#8209;tuplets](http://en.wikipedia.org/wiki/Prime_k-tuple)
up to 2^64.

* Homepage: http://primesieve.org
* Downloads: http://primesieve.org/downloads
* API: http://primesieve.org/api

### Algorithm complexity

primesieve generates primes using the segmented sieve of Eratosthenes with
[wheel factorization](http://en.wikipedia.org/wiki/Wheel_factorization),
this algorithm has a complexity of
<img src="http://primesieve.org/images/Onloglogn.svg" height="20" align="absmiddle"/>
operations and uses
<img src="http://primesieve.org/images/Osqrtn.svg" height="20" align="absmiddle"/>
space, more precisely primesieve's memory usage per thread is about
<img src="http://primesieve.org/images/primesieve_memory_usage.svg" height="20" align="absmiddle"/>
bytes.

### Requirements

primesieve is written in C++03 and includes C bindings for all of its
functions so that it can easily be used in languages other than C++.
primesieve compiles with every standard compliant C++ compiler and
runs on both little and big endian CPUs. The parallelization is
implemented using OpenMP (2.0 or later). If you have cloned or forked
primesieve then the GNU Build System is a prerequisite for building
primesieve.

### Build instructions (Unix-like OSes)

Download the latest release tarball from
http://primesieve.org/downloads, extract it and cd into the newly
created directory. Then build and install primesieve using:

```sh
$ ./configure
$ make
$ sudo make install
```

In order to clone and build primesieve you need to have installed the
GNU Build System (a.k.a. Autotools). To install the GNU Build System
install
[GNU Autoconf](http://www.gnu.org/software/autoconf/),
[GNU Automake](http://www.gnu.org/software/automake/) and
[GNU Libtool](http://www.gnu.org/software/libtool/) using your packet
manager.

```sh
$ git clone git://github.com/kimwalisch/primesieve.git && cd primesieve
$ ./autogen.sh
$ ./configure
$ make
```

To enable building the example programs use:
```sh
$ ./configure --enable-examples
```

### Build instructions (Microsoft Visual C++)

Open a Visual Studio Command Prompt, cd into the primesieve directory
and build primesieve using:

```sh
> nmake -f Makefile.msvc
```

In order to get the best performance you can indicate your CPU's L1
data cache size in kilobytes per core (default 32), e.g. for a CPU
with 64 kilobytes L1 data cache use:

```sh
> nmake -f Makefile.msvc L1_DCACHE_SIZE=64
```

To build the example programs use:
```sh
> nmake -f Makefile.msvc
> nmake -f Makefile.msvc examples
```

### primesieve console application

The primesieve console application can print and count primes and
prime k-tuplets and find the nth prime. Below are a few usage
examples:

```sh
# Print the primes below 1000000 to the standard output
$ primesieve 1000000 --print

# Print the twin primes below 1000000 to the standard output
$ primesieve 1000000 --print=2

# Count the primes below 1e10 using all CPU cores
$ primesieve 1e10 --count

# Count the primes within [1e10, 2e10] using 4 threads
$ primesieve 1e10 2e10 --count --threads=4

# Print an option summary
$ primesieve --help
```

### primesieve C++ library

After having built and installed primesieve you can easily use it in
your C++ program, below is an example. primesieve's API is documented
online at http://primesieve.org/api.

```C++
#include <primesieve.hpp>
#include <iostream>
#include <vector>

int main()
{
  // store the primes below 1000
  std::vector<int> primes;
  primesieve::generate_primes(1000, &primes);

  primesieve::iterator pi;
  uint64_t prime;

  // iterate over the primes below 10^9
  while ((prime = pi.next_prime()) < 1000000000)
    std::cout << prime << std::endl;

  return 0;
}
```

On Unix-like operating systems compile using:
```sh
# Only needed if your operating system misses the ldconfig program
$ export LIBRARY_PATH=/usr/local/lib:$LIBRARY_PATH
$ export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

$ c++ -O2 primes.cpp -lprimesieve
```

On Windows (MSVC) compile using:
```
> cl /O2 /EHsc primes.cpp /I primesieve\include /link primesieve\primesieve.lib
```

### primesieve C bindings

All of primesieve's functions are exposed as C API (C99 or later) via
the primesieve.h header. You can explore primesieve's C API online
at http://primesieve.org/api.

```C
#include <primesieve.h>
#include <stdint.h>
#include <stdio.h>

int main()
{
  uint64_t start = 0;
  uint64_t stop = 10000;
  size_t i;
  size_t size;

  /* store the primes below 10000 */
  int* primes = (int*) primesieve_generate_primes(start, stop, &size, INT_PRIMES);

  for (i = 0; i < size; i++)
    printf("%i\n", primes[i]);

  /* deallocate primes array generated using primesieve */
  primesieve_free(primes);
  return 0;
}
```

On Unix-like operating systems compile using:
```sh
# Only needed if your operating system misses the ldconfig program
$ export LIBRARY_PATH=/usr/local/lib:$LIBRARY_PATH
$ export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

$ cc -O2 primes.c -lprimesieve
```

### Reporting bugs

To report a bug or give feedback please send an email to
<<kim.walisch@gmail.com>>.
