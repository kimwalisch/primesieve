Example programs
================

This directory contains simple C and C++ example programs which use
the primesieve library to generate primes.

C++ examples
------------

Below are the most important C++ example programs.

* [store_primes_in_vector.cpp](cpp/store_primes_in_vector.cpp)
* [primesieve_iterator.cpp](cpp/primesieve_iterator.cpp)
* [count_primes.cpp](cpp/count_primes.cpp)

C examples
----------

Below are the most important C example programs.

* [store_primes_in_array.c](c/store_primes_in_array.c)
* [primesieve_iterator.c](c/primesieve_iterator.c)
* [count_primes.c](c/count_primes.c)

Build instructions (Unix-like OSes)
-----------------------------------

Open a terminal and `cd' into the primesieve (parent) directory. Then
run the commands below:

```sh
$ ./configure --enable-examples
$ make
```

Build instructions (Microsoft Visual C++)
-----------------------------------------

Open a Visual Studio Command Prompt and `cd' into the primesieve
(parent) directory. Then run the commands below:

```sh
> nmake -f Makefile.msvc
> nmake -f Makefile.msvc examples
```
