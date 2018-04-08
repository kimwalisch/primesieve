# C++ classes

* **PrimeSieve** is a high level class that coordinates prime sieving.
  It is used for printing and counting primes and for computing the nth
  prime. PrimeSieve's main method is ```PrimeSieve::sieve(start, stop)```
  which sieves the primes inside the interval [start, stop].

* **ParallelSieve** launches multiple threads using ```std::async```
  and each thread sieves a part of the interval [start, stop] using a
  PrimeSieve object. At the end all partial results are combined to get
  the final result.

* **Erat** is an implementation of the segmented sieve of Eratosthenes
  using a bit array with 30 numbers per byte, each byte of the sieve array
  holds the 8 offsets ```k = { 7, 11, 13, 17, 19, 23, 29, 31 }```.
  Its main methods are ```addSievingPrime(prime)``` which is called
  consecutively for all primes ≤ sqrt(n) and ```sieveSegment()``` which
  sieves the next segment. Erat uses the EratSmall, EratMedium and
  EratBig classes to cross-off multiples.

* **EratSmall** is derived from Wheel. EratSmall is a segmented
  sieve of Eratosthenes algorithm with a hard-coded modulo 30 wheel
  that skips multiples of 2, 3 and 5. This algorithm is optimized
  for small sieving primes that have many multiples in each
  segment. EratSmall is a further optimized implementation of Achim
  Flammenkamp's algorithm
  [[1]](https://github.com/kimwalisch/primesieve/tree/master/src#references).

* **EratMedium** is derived from Wheel. EratMedium is a segmented
  sieve of Eratosthenes algorithm with a modulo 210 wheel that skips
  multiples of 2, 3, 5 and 7. The wheel is implemented using a
  precomputed lookup table (```wheel210``` array from
  ```Wheel.cpp```). This algorithm is optimized for medium sieving
  primes with a few multiples per segment.

* **EratBig** is derived from Wheel. EratBig is a segmented sieve of
  Eratosthenes algorithm with Tomás Oliveira's improvement for big
  sieving primes [[2]](https://github.com/kimwalisch/primesieve/tree/master/src#references)
  and a modulo 210 wheel that skips multiples of 2, 3, 5 and 7. The
  wheel is implemented using a precomputed lookup table (```wheel210```
  array from ```Wheel.cpp```). EratBig is optimized for big sieving
  primes that have less than one multiple per segment.

* **PreSieve** is used to pre-sieve multiples of small primes ≤ 19
  to speed up the sieve of Eratosthenes. Upon creation the
  multiples of small primes are removed from a buffer. Later this
  buffer is simply copied to the sieve array to remove (pre-sieve)
  the multiples of small primes.

* **SievingPrimes** is derived from Erat. The SievingPrimes class is used
  to generate the sieving primes ≤ sqrt(stop). SievingPrimes is used
  by the PrintPrimes and NextPrimes classes.

* **PrintPrimes** is derived from Erat. PrintPrimes is used for printing
  primes to stdout and for counting primes. After a segment has been
  sieved (using Erat) PrintPrimes is used to reconstruct primes and prime
  k-tuplets from 1 bits of the sieve array.

* **NextPrimes** is derived from Erat. NextPrimes is used to fill an
  array with the primes inside the next 64-bit word of the sieve array.
  NextPrimes is used by primesieve::iterator.

* **primesieve::iterator** allows to easily iterate over primes. It
  provides ```next_prime()``` and ```prev_prime()``` methods.
  primesieve::iterator is also used for storing primes in a vector
  or an array.
  
* **CpuInfo** is used to get the CPU's L1 and L2 cache sizes. The
  best prime sieving performance is achieved using a sieve array
  size that matches the CPU's L1 or L2 cache size (depending on the
  CPU type).

* **Wheel** factorization is used to skip multiples of small primes
  ≤ 7 to speed up the sieve of Eratosthenes. The abstract **Wheel** class
  is used to initialize sieving primes i.e. ```Wheel::addSievingPrime()```
  calculates the first multiple ≥ start of each sieving prime and the position
  within the sieve array of that multiple. ```Wheel::unsetBit()``` is
  used to cross-off a multiple (unset a bit) and to calculate the sieving
  prime's next multiple.

# References

1. Achim Flammenkamp, "The Art of Prime Sieving", 1998. <br/>
   https://wwwhomes.uni-bielefeld.de/achim/prime_sieve.html
2. Tomás Oliveira e Silva, "Fast implementation of the segmented
   sieve of Eratosthenes", 2002. <br/>
   http://www.ieeta.pt/~tos/software/prime_sieve.html
