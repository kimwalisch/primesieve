# primesieve C examples

This is a short selection of C code snippets that use libprimesieve to generate prime numbers.
These examples cover the most frequently used functionality of libprimesieve. Arguably the most
useful feature provided by libprimesieve is the ```primesieve_iterator``` which lets you
iterate over primes using the ```primesieve_next_prime()``` or ```primesieve_prev_prime()```
functions.

For in-depth documentation please refer to the [C API documentation](https://primesieve.org/api).

## ```primesieve_next_prime()```

By default ```primesieve_next_prime()``` generates primes > 0 i.e. 2, 3, 5, 7, ...

```C
#include <primesieve.h>
#include <stdio.h>

int main()
{
  primesieve_iterator it;
  primesieve_init(&it);

  uint64_t sum = 0;
  uint64_t prime = 0;

  /* iterate over the primes < 10^9 */
  while ((prime = primesieve_next_prime(&it)) < 1000000000ull)
    sum += prime;

  printf("Sum of the primes below 10^9 = %" PRIu64 "\n", sum);
  return 0;
}
```

## ```primesieve_skipto()```

This method changes the start number of the ```primesieve_iterator``` object. (By
default the start number is initialized to 0). The ```stop_hint``` paramater is
used for performance optimization, ```primesieve_iterator``` only buffers primes
up to this limit.

```C
#include <primesieve.h>
#include <stdio.h>

int main()
{
  primesieve_iterator it;
  primesieve_init(&it);

  /* primesieve_skipto(&it, start_number, stop_hint) */
  primesieve_skipto(&it, 1000, 1100);
  uint64_t prime;

  /* iterate over primes from ]1000, 1100] */
  while ((prime = primesieve_next_prime(&it)) <= 1100)
    printf("%" PRIu64 "\n", prime);

  primesieve_free_iterator(&it);
  return 0;
}
```

## ```primesieve_prev_prime()```

Before using ```primesieve_prev_prime()``` you must first change the start
number using the ```primesieve_skipto()``` function as the start number is
initialized to 0 be default.

```C
#include <primesieve.h>
#include <stdio.h>

int main()
{
  primesieve_iterator it;
  primesieve_init(&it);

  /* primesieve_skipto(&it, start_number, stop_hint) */
  primesieve_skipto(&it, 2000, 1000);
  uint64_t prime;

  /* iterate over primes from ]2000, 1000] */
  while ((prime = primesieve_prev_prime(&it)) >= 1000)
    printf("%" PRIu64 "\n", prime);

  primesieve_free_iterator(&it);
  return 0;
}
```

## ```primesieve_generate_primes()```

Stores the primes inside [start, stop] in an array. If you are iterating over
the same primes many times in a loop you will likely get better performance
if you store the primes in an array instead of using a ```primesieve_iterator```
(provided your system has enough memory).

```C
#include <primesieve.h>
#include <stdio.h>

int main()
{
  uint64_t start = 0;
  uint64_t stop = 1000;
  size_t i;
  size_t size;

  /* Get an array with the primes inside [start, stop] */
  int* primes = (int*) primesieve_generate_primes(start, stop, &size, INT_PRIMES);

  for (i = 0; i < size; i++)
    printf("%i\n", primes[i]);

  primesieve_free(primes);
  return 0;
}
```

## ```primesieve_generate_n_primes()```

Stores n primes in an array.

```C
#include <primesieve.h>
#include <stdio.h>

int main()
{
  uint64_t n = 1000;
  uint64_t start = 0;
  size_t i;
  size_t size;

  /* Get an array with the first 1000 primes */
  int64_t* primes = (int64_t*) primesieve_generate_n_primes(n, start, INT64_PRIMES);

  for (i = 0; i < n; i++)
    printf("%i\n", primes[i]);

  primesieve_free(primes);
  return 0;
}
```

## ```primesieve_count_primes()```

Counts the primes inside [start, stop]. This method is multi-threaded and uses all
available CPU cores by default.

```C
#include <primesieve.h>
#include <stdio.h>

int main()
{
  /* primesieve_count_primes(start, stop) */
  uint64_t count = primesieve_count_primes(0, 1000);
  printf("Primes below 1000 = %" PRIu64 "\n", count);

  return 0;
}
```

## ```primesieve_nth_prime()```

This method finds the nth prime e.g. ```nth_prime(25) = 97```. This method is
multi-threaded and uses all available CPU cores by default.

```C
#include <primesieve.h>
#include <stdio.h>

int main(int argc, char** argv)
{
  /* primesieve_nth_prime(n, start) */
  uint64_t prime = primesieve_nth_prime(25, 0);
  printf("%" PRIu64 "th prime = %" PRIu64 "\n", n, prime);

  return 0;
}
```

# How to compile

You can compile any of the C example programs above using:

```C
cc -O2 primes.c -o primes -lprimesieve
```
