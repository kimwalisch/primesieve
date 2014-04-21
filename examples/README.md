Example programs
----------------

This directory contains simple example programs that show how to use
the primesieve library to generate primes. Below is a list of the
most important example programs that you should have a look at first.

* [store_primes_in_vector.cpp](store_primes_in_vector.cpp)
* [primesieve_iterator.cpp](primesieve_iterator.cpp)
* [count_primes.cpp](count_primes.cpp)

### Build instructions (Unix-like OSes)
Open a terminal and `cd' into the primesieve (parent) directory. Then
run the commands below:

```sh
$ ./configure --enable-examples
$ make
```

### Build instructions (Microsoft Visual C++)
Open a Visual Studio Command Prompt and `cd' into the primesieve
(parent) directory. Then run the commands below:

```sh
> nmake -f Makefile.msvc
> nmake -f Makefile.msvc examples
```
