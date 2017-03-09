libprimesieve code examples
===========================

This directory contains examples programs that show how to generate
primes using libprimesieve.

C++ examples
------------

Ordered by importance.

* [store_primes_in_vector.cpp](cpp/store_primes_in_vector.cpp)
* [primesieve_iterator.cpp](cpp/primesieve_iterator.cpp)
* [prev_prime.cpp](cpp/prev_prime.cpp)
* [count_primes.cpp](cpp/count_primes.cpp)
* [More C++ examples](cpp/)

C examples
----------

Ordered by importance.

* [store_primes_in_array.c](c/store_primes_in_array.c)
* [primesieve_iterator.c](c/primesieve_iterator.c)
* [prev_prime.c](c/prev_prime.c)
* [count_primes.c](c/count_primes.c)
* [More C examples](c/)

Build instructions
------------------

Open a terminal and cd into the primesieve (parent) directory,
then run the commands:

```sh
cmake -DBUILD_EXAMPLES=ON .
make
```
