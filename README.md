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
[wheel factorization](http://en.wikipedia.org/wiki/Wheel_factorization),
this algorithm has a complexity of
<img src="http://primesieve.org/images/Onloglogn.svg" height="20" align="absmiddle"/>
operations and uses
<img src="http://primesieve.org/images/Osqrtn.svg" height="20" align="absmiddle"/>
space. More precisely primesieve's memory usage per thread is about
<img src="http://primesieve.org/images/primesieve_memory_usage.svg" height="20" align="absmiddle"/>
bytes.

Requirements
------------

primesieve is very portable, it compiles with every C++ compiler and
runs on most CPU architectures out there. The parallelization is
implemented using [OpenMP](http://en.wikipedia.org/wiki/OpenMP). The
primesieve GUI application (not built by default) uses the
[Qt framework](http://qt-project.org).

primesieve is also a library, it supports C++ and C directly and there
are [bindings](#bindings-for-other-languages) available for a few
other programming languages. The author's
[primecount](https://github.com/kimwalisch/primecount) program uses
libprimesieve extensively.

Build instructions (Unix-like OSes)
-----------------------------------

Download the latest release tarball from
http://primesieve.org/downloads, extract it and cd into the newly
created directory. Then build and install primesieve using:

```sh
$ ./configure
$ make
$ sudo make install
```

If you have downloaded a zip archive from GitHub then Autotools
(a.k.a. GNU Build System) must be installed and ```autogen.sh``` must
be executed once. To install Autotools install
[GNU&#160;Autoconf](http://www.gnu.org/software/autoconf/),
[GNU&#160;Automake](http://www.gnu.org/software/automake/) and
[GNU&#160;Libtool](http://www.gnu.org/software/libtool/)
using your operating system's package manager.

```sh
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install
```

To enable building the example programs use:
```sh
$ ./configure --enable-examples
```

Build instructions (Microsoft Visual C++)
-----------------------------------------

Open a Visual Studio Command Prompt, cd into the primesieve directory
and run:

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

Console application
-------------------

The primesieve console application can print and count primes and
prime k-tuplets and find the nth prime.

```sh
# Print the primes below 1000000
$ primesieve 1000000 --print

# Print the twin primes below 1000000
$ primesieve 1000000 --print=2

# Count the primes below 1e10 using all CPU cores
$ primesieve 1e10 --count

# Count the primes within [1e10, 2e10] using 4 threads
$ primesieve 1e10 2e10 --count --threads=4

# Print an option summary
$ primesieve --help
```

C++ API
-------

After having installed primesieve you can easily use it in your C++
program, below is an example. primesieve's API is documented
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

C API
-----

All of primesieve's functions are exposed as C API (C99 or later) via
the ```primesieve.h``` header. You can browse primesieve's C API
online at http://primesieve.org/api.

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

  /* get an array with primes below 10000 */
  int* primes = (int*) primesieve_generate_primes(start, stop, &size, INT_PRIMES);

  for (i = 0; i < size; i++)
    printf("%i\n", primes[i]);

  /* deallocate primes array generated using primesieve */
  primesieve_free(primes);
  return 0;
}
```

Linking against libprimesieve
-----------------------------

**Unix-like operating systems**
```sh
$ c++ -O2 primes.cpp -lprimesieve
$ cc  -O2 primes.c   -lprimesieve
```

Note that if you have built primesieve yourself the default installation path
is ```/usr/local/lib``` which is not part of ```LD_LIBRARY_PATH``` on many
systems. Hence you need to export some additional environment variables:

```sh
$ export LIBRARY_PATH=/usr/local/lib:$LIBRARY_PATH
$ export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
$ export CPLUS_INCLUDE_PATH=/usr/local/include:$CPLUS_INCLUDE_PATH
$ export C_INCLUDE_PATH=/usr/local/include:$C_INCLUDE_PATH
```

**Microsoft Visual C++ (Windows)**
```
> cl /O2 /EHsc primes.cpp /I primesieve\include /link primesieve\primesieve.lib
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
