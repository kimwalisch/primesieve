primesieve
==========
[![Build Status](https://travis-ci.org/kimwalisch/primesieve.png)](https://travis-ci.org/kimwalisch/primesieve)

primesieve is a free (BSD-licensed) software program and C/C++
library that generates primes using a highly optimized
<a href="http://en.wikipedia.org/wiki/Sieve_of_Eratosthenes">sieve of
Eratosthenes</a> implementation. It generates the primes below 10^9
in just 0.2 seconds on a single core of an Intel Core i7-4770 3.4GHz
CPU from 2013. primesieve can generate primes and
<a href="http://en.wikipedia.org/wiki/Prime_k-tuple">prime k-tuplets
</a> up to 2^64.

* Homepage: http://primesieve.org
* Downloads: http://primesieve.org/downloads
* API: http://primesieve.org/api

### Algorithm complexity
primesieve generates primes using the segmented sieve of Eratosthenes
with <a href="http://en.wikipedia.org/wiki/Wheel_factorization">wheel
factorization</a>, this algorithm has a complexity of O(n log log n)
operations and uses O(n^0.5) space. primesieve's memory requirement
per thread is about pi(n^0.5) * 8 bytes.

### Requirements
primesieve is written in C++03 and includes C bindings for all of its
functions so that it can easily be used in languages other than C++.
primesieve compiles with every standard compliant C++ compiler and
runs on both little and big endian CPUs. The parallelization is
implemented using OpenMP 2.0 or later.

### Build instructions
Download the latest release tarball from
http://primesieve.org/downloads. On Unix-like operating systems you
can then build and install primesieve using:

```bash
$ ./configure
$ make
$ sudo make install
```

On Windows (MSVC) open a Visual Studio Command Prompt and cd into the
primesieve directory. Then build primesieve using the following
command:

```bash
> nmake -f Makefile.msvc
```

Building primesieve is covered in more detail in the
[INSTALL](INSTALL) file.

### primesieve console application
The primesieve console application can print and count primes and
prime k-tuplets and find the nth prime. Below are a few usage examples:

```bash
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
```bash
$ c++ -O2 primes.cpp -lprimesieve
```

On Windows (MSVC) compile using:
```bash
> cl /O2 /EHsc /Iinclude primes.cpp /link primesieve.lib
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
```bash
$ cc -O2 primes.c -lprimesieve
```

### Reporting bugs
To report a bug or give feedback please send an email to
<<kim.walisch@gmail.com>>.
