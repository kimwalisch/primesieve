# Introduction

primesieve generates primes using the segmented
[sieve of Eratosthenes](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes) with
[wheel factorization](https://en.wikipedia.org/wiki/Wheel_factorization).
This algorithm has a run time complexity of $O(n\ \log\ \log\ n)$ operations and uses
$O(\sqrt{n})$ memory. Furthermore primesieve uses the
[bucket sieve](http://sweet.ua.pt/tos/software/prime_sieve.html)
algorithm which improves the cache efficiency when generating primes > 2<sup>32</sup>.
primesieve uses 8 bytes per sieving prime, in practice its memory usage is about
$\pi(\sqrt{n})\times 8$ bytes per thread.

# Algorithm details

Segmentation is currently the best known practical improvement to
the sieve of Eratosthenes. Instead of sieving the interval
[2, n] at once one subdivides the sieve interval into a
number of equal sized segments that are then sieved consecutively.
Segmentation drops the memory requirement of the sieve of Eratosthenes from
$O(n)$ to $O(\sqrt{n})$. The segment size is usually chosen to fit into the
CPU's fast L1 or L2 cache memory which significantly speeds up sieving. A
segmented version of the sieve of Eratosthenes was first published by
Singleton in 1969 [[1]](#references). Here is a
[simple implementation of the segmented sieve of Eratosthenes](https://github.com/kimwalisch/primesieve/wiki/Segmented-sieve-of-Eratosthenes).

Wheel factorization is used to skip multiples of small primes. If a
k-th wheel is added to the sieve of Eratosthenes then only those
multiples are crossed off that are coprime to the first k
primes, i.e. multiples that are divisible by any of the first k
primes are skipped. The 1st wheel considers only odd numbers, the 2nd
wheel (modulo 6) skips multiples of 2 and 3, the 3rd wheel (modulo 30)
skips multiples of 2, 3, 5 and so on. Pritchard has shown in
[[2]](#references) that the running time of the sieve of
Eratosthenes can be reduced by a factor of $O(\log\ \log\ n)$
if the wheel size is proportional to $O(\sqrt{n})$
but for cache reasons the sieve of Eratosthenes usually performs best
with a modulo 30 or 210 wheel. Sorenson explains wheels in
[[3]](#references).

Additionally primesieve uses Tomás Oliveira e Silva's
[cache-friendly bucket list algorithm](http://www.ieeta.pt/~tos/software/prime_sieve.html)
if needed [[4]](#references). This algorithm is relatively
new, it has been devised by Tomás Oliveira e Silva in 2001 in order to
speed up the segmented sieve of Eratosthenes for prime numbers past 32
bits. The idea is to store the sieving primes into lists of buckets
with each list being associated with a segment. A list of sieving
primes related to a specific segment contains only those primes that
have multiple occurrence(s) in that segment. Whilst sieving a segment
only the primes of the related list are used for sieving and each
prime is reassigned to the list responsible for its next multiple when
processed. The benefit of this approach is that it is now possible to
use segments (i.e. sieve arrays) smaller than $\sqrt{n}$
without deteriorating efficiency, this is important as only small
segments that fit into the CPU's L1 or L2 cache provide fast memory
access.

# Implementation

primesieve is written in C++ and does not depend on external libraries.
Some of its algorithms (such as e.g. pre-sieving) have been vectorized
using SIMD instructions and we also use inline assembly in some places, e.g.
for querying CPUID on x86 CPUs. The speed of primesieve is primarily due to the
segmentation of the sieve of Eratosthenes which prevents cache misses when
crossing off multiples in the sieve array and the use of a bit array instead
of a boolean sieve array. primesieve reuses and improves ideas from other
great sieve of Eratosthenes implementations, namely Achim
Flammenkamp's [prime_sieve.c](https://wwwhomes.uni-bielefeld.de/achim/prime_sieve.html),
Tomás Oliveira e Silva's [A1 implementation](http://sweet.ua.pt/tos/software/prime_sieve.html#s)
and the author's older [ecprime](http://primzahlen.de/referenten/Kim_Walisch/index2.htm)
all written in the late '90s and '00s. Furthermore primesieve contains
new optimizations to reduce the branch misprediction rate and it
efficiently uses the CPU's multi level cache hierarchy.

### Optimizations used in primesieve

 * Uses a bit array with 8 flags each 30 numbers for sieving.
 * Pre-sieves multiples of small primes ≤ 163 using SIMD instructions.
 * Compresses the sieving primes in order to improve cache efficiency [[5]](#references).
 * Starts crossing off multiples at the square.
 * Uses a modulo 210 wheel that skips multiples of 2, 3, 5 and 7.
 * Uses specialized algorithms for small, medium and big sieving primes.
 * Uses L1 cache for small sieving primes & L2 cache for medium and big sieving primes.
 * Sorts medium sieving primes to reduce branch misprediction rate.
 * Uses a custom memory pool (for medium & big sieving primes).
 * Multi-threaded using C++11 ```std::async```.

### Highly optimized inner loop

primesieve's inner sieving loop has been optimized using
[extreme loop unrolling](https://github.com/kimwalisch/primesieve/blob/v12.7/src/EratSmall.cpp#L108),
on average crossing off a multiple uses just 1.375 instructions on
x64 CPUs. Below is the assembly GCC generates for primesieve's inner
sieving loop, each andb instruction unsets a bit (crosses off a
multiple) in the sieve array.

```asm
; primesieve inner sieving loop
; Uses only 11 instructions (x86-64) to cross-off the next 8 multiples
.L99:
    andb    $-3, (%rax)
    andb    $-9, (%rax,%r14)
    andb    $127, (%rax,%r12)
    andb    $-33, (%rax,%rbp)
    andb    $-2, (%rax,%r10)
    andb    $-65, (%rax,%rbx)
    andb    $-5, (%rax,%r9)
    andb    $-17, (%rax,%r11)
    addq    %r13, %rax
    cmpq    %rdi, %rax
    jb  .L99
```

# References

1. R. C. Singleton, "An efficient prime number generator", Communications of the ACM 12, 563-564, 1969.
2. Paul Pritchard, "Fast compact prime number sieves (among others)", Journal of Algorithms 4 (1983), 332-344.
3. Jonathan Sorenson, ["An analysis of two prime number sieves"](ftp://ftp.cs.wisc.edu/pub/techreports/1991/TR1028.pdf), Computer Science Technical Report Vol. 1028, 1991.
4. Tomás Oliveira e Silva, ["Fast implementation of the segmented sieve of Eratosthenes"](http://www.ieeta.pt/~tos/software/prime_sieve.html), 2002.
5. Actually not the sieving primes are compressed but their sieve and wheel indexes.
