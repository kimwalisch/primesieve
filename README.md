primesieve
==========
primesieve is a software program and C/C++ library for fast prime
number generation. It generates the primes below 10^9 in just 0.2
seconds on a single core of an Intel Core i7-4770 CPU from 2013.
primesieve can generate primes and prime k-tuplets (twin primes, prime
triplets, ...) up to 2^64.

### Algorithm complexity
primesieve generates primes using the segmented sieve of Eratosthenes
with wheel factorization, this algorithm has a complexity of
O(n log log n) operations and uses O(sqrt(n)) space. primesieve's
memory requirement per thread is about pi(sqrt(n)) * 8 bytes + 32
kilobytes.

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

```
$ ./configure
$ make
$ sudo make install
```

On Windows (MSVC) open a Visual Studio Command Prompt and cd into the
primesieve directory. Then build primesieve using the following
command:

```
> nmake -f Makefile.msvc
```

Building primesieve is covered in more detail in the
[INSTALL](INSTALL) file.

### primesieve console application
The primesieve console application can print and count primes and
prime k-tuplets and find the nth prime. Below are two usage examples,
to print an option summary use the ```--help``` flag.

Print the primes below 1000 to the standard output:
```
$ ./primesieve 1000 --print
```

Count the prime triplets inside the interval [1e10, 2e10]:
```
$ ./primesieve 1e10 2e10 --count=3
```

### primesieve C++ library
After having built and installed primesieve you can use it in your C++
program to easily generate primes as shown in the primes.cpp example
program below. You can explore primesieve's entire API online at
http://primesieve.org/api.

```C++
#include <primesieve.hpp>
#include <iostream>
#include <vector>

int main()
{
  std::vector<int> primes;
  // store the primes below 1000
  primesieve::generate_primes(1000, &primes);

  primesieve::iterator pi;
  // iterate over the primes below 1000
  while (pi.next_prime() < 1000)
    std::cout << pi.prime() << std::endl;

  return 0;
}
```

On Unix-like operating systems compile using:
```
$ c++ -O2 primes.cpp -lprimesieve
```

On Windows (MSVC) compile using:
```
> cl /O2 /EHsc /Iinclude primes.cpp /link primesieve.lib
```

### primesieve C bindings
All of primesieve's functions are exposed as C API via the
primesieve.h header. You can explore primesieve's C API online
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
  int* primes = (int*) generate_primes(start, stop, &size, INT_PRIMES);

  for (i = 0; i < size; i++)
    printf("%i\n", primes[i]);

  /* deallocate primes array created using generate_primes() */
  primesieve_free(primes);

  primesieve_iterator pi;
  primesieve_init(&pi);

  uint64_t sum = 0;
  uint64_t prime = 0;

  /* iterate over the primes below 10^9 */
  while ((prime = primesieve_next(&pi)) < 1000000000)
    sum += prime;

  primesieve_free_iterator(&pi);
  printf("Sum of the primes below 10^9 = %llu\n", sum);
  return 0;
}
```

On Unix-like operating systems compile using:
```
$ cc -O2 primes.c -lprimesieve
```

### Reporting bugs
To report a bug or give feedback please send an email to
<<kim.walisch@gmail.com>>.
