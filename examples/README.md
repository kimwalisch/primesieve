libprimesieve code examples
===========================

This directory contains examples programs that show how to generate primes
using libprimesieve.

C++ examples
------------

Ordered by importance.

* [store_primes_in_vector.cpp](cpp/store_primes_in_vector.cpp)
* [primesieve_iterator.cpp](cpp/primesieve_iterator.cpp)
* [previous_prime.cpp](cpp/previous_prime.cpp)
* [count_primes.cpp](cpp/count_primes.cpp)
* [More C++ examples](cpp/)

C examples
----------

Ordered by importance.

* [store_primes_in_array.c](c/store_primes_in_array.c)
* [primesieve_iterator.c](c/primesieve_iterator.c)
* [previous_prime.c](c/previous_prime.c)
* [count_primes.c](c/count_primes.c)
* [More C examples](c/)

Build instructions (Unix-like OSes)
-----------------------------------

Open a terminal and cd into the primesieve (parent) directory,
then run the commands:

```sh
./configure --enable-examples
make
```

Build instructions (Microsoft Visual C++)
-----------------------------------------

Open a Visual Studio Command Prompt and cd into the primesieve
(parent) directory, then run the commands:

```sh
nmake -f Makefile.msvc
nmake -f Makefile.msvc examples
```
