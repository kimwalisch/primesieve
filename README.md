primesieve
========
primesieve is a software program and C++ library for fast prime number
generation. It generates the primes below 10^9 in just 0.2 seconds on
a single core of an Intel Core i7-4770 CPU from 2013. primesieve can
generate primes and prime k-tuplets (twin primes, prime triplets, ...)
up to 2^64.

### Algorithm complexity
primesieve generates primes using the segmented sieve of Eratosthenes
with wheel factorization, this algorithm has a complexity of
O(n log log n) operations and uses O(n^0.5) space. primesieve's memory
requirement per thread is about: pi(n^0.5) * 8 bytes + 32 kilobytes.

### Requirements
primesieve is written in C++03, it compiles with every standard
compliant C++ compiler and runs on both little and big endian CPUs.
The parallelization is implemented using OpenMP 2.0 or later.

### Build instructions
Please download the latest release tarball from
https://github.com/kimwalisch/primesieve/releases. Then open a
terminal uncompress the source archive, cd into the newly created
directory and run:
```
$ ./configure
$ make
$ sudo make install
```
On Windows (MSVC) open a Visual Studio Command Prompt and cd into the
primesieve directory. Then execute the following command:
```
> nmake -f Makefile.msvc
```

### C++ library
After having built and installed primesieve you can use it in your C++
program to easily generate primes as shown in the primes.cpp example
program below. You can explore primesieve's entire API online at:
http://kimwalisch.github.io/primesieve/doxygen.

```C++
#incldue <primesieve.h>
#include <iostream>
#include <vector>

int main()
{
  std::vector<int> primes;
  // store the primes below 1000
  primesieve::generate_primes(1000, &primes);

  primesieve::prime_iterator pi;
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
> cl /O2 primes.cpp /EHsc /Iinclude /link primesieve.lib
```

### Reporting bugs
To report a bug or give feedback please send an email to Kim Walisch
<<kim.walisch@gmail.com>>.
