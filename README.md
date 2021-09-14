# primesieve

[![Build Status](https://ci.appveyor.com/api/projects/status/github/kimwalisch/primesieve?branch=master&svg=true)](https://ci.appveyor.com/project/kimwalisch/primesieve)
[![Github Releases](https://img.shields.io/github/release/kimwalisch/primesieve.svg)](https://github.com/kimwalisch/primesieve/releases)

primesieve is a command-line program and C/C++ library for quickly generating prime numbers.
It is very cache efficient, it detects your CPU's L1 & L2 cache sizes and allocates its main
data structures accordingly. It is also multi-threaded by default, it uses all available CPU
cores whenever possible i.e. if sequential ordering is not required. primesieve
can generate primes and [prime k-tuplets](https://en.wikipedia.org/wiki/Prime_k-tuple)
up to 2<sup>64</sup>.

primesieve generates primes using the segmented
[sieve of Eratosthenes](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes) with
[wheel factorization](https://en.wikipedia.org/wiki/Wheel_factorization).
This algorithm has a run time complexity of
<img src="https://github.com/kimwalisch/primesieve/blob/gh-pages/images/Onloglogn.svg" height="20" align="absmiddle"/>
operations and uses
<img src="https://github.com/kimwalisch/primesieve/blob/gh-pages/images/Osqrtn.svg" height="20" align="absmiddle"/>
memory. Furthermore primesieve uses the
[bucket sieve](http://sweet.ua.pt/tos/software/prime_sieve.html)
algorithm which improves the cache efficiency when generating primes > 2<sup>32</sup>.
primesieve uses 8 bytes per sieving prime, hence its memory usage is about
<img src="https://github.com/kimwalisch/primesieve/blob/gh-pages/images/primesieve_memory_usage.svg" height="20" align="absmiddle"/>
bytes per thread.

* [More algorithm details](doc/ALGORITHMS.md)

## Installation

The primesieve command-line program can be installed using your operating system's
package manager. For doing development with libprimesieve you may need
to install ```libprimesieve-dev``` or ```libprimesieve-devel```.

<table>
    <tr>
        <td><b>Windows:</b></td>
        <td><code>choco install primesieve</code></td>
    </tr>
    <tr>
        <td><b>macOS:</b></td>
        <td><code>brew install primesieve</code></td>
    </tr>
    <tr>
        <td><b>Arch Linux:</b></td>
        <td><code>sudo pacman -S primesieve</code></td>
    </tr>
    <tr>
        <td><b>Debian/Ubuntu:</b></td>
        <td><code>sudo apt install primesieve</code></td>
    </tr>
    <tr>
        <td><b>Fedora:</b></td>
        <td><code>sudo dnf install primesieve</code></td>
    </tr>
    <tr>
        <td><b>openSUSE:</b></td>
        <td><code>sudo zypper install primesieve</code></td>
    </tr>
    <tr>
        <td><b>FreeBSD:</b></td>
        <td><code>pkg install primesieve</code></td>
    </tr>
</table>

## Usage examples

```sh
# Count the primes below 1e10 using all CPU cores
primesieve 1e10

# Print the primes below 1000000
primesieve 1000000 --print

# Print the twin primes below 1000000
primesieve 1000000 --print=2

# Count the prime triplets inside [1e10, 1e10+2^32]
primesieve 1e10 --dist=2^32 --count=3
```

## Command-line options

```
Usage: primesieve [START] STOP [OPTION]...
Generate the primes and/or prime k-tuplets inside [START, STOP]
(< 2^64) using the segmented sieve of Eratosthenes.

Options:
  -c, --count[=NUM+]  Count primes and/or prime k-tuplets, NUM <= 6.
                      Count primes: -c or --count (default option),
                      count twin primes: -c2 or --count=2,
                      count prime triplets: -c3 or --count=3, ...
      --cpu-info      Print CPU information (cache sizes).
  -d, --dist=DIST     Sieve the interval [START, START + DIST].
  -h, --help          Print this help menu.
  -n, --nth-prime     Find the nth prime.
                      primesieve 100 -n: finds the 100th prime,
                      primesieve 2 100 -n: finds the 2nd prime > 100.
      --no-status     Turn off the progressing status.
  -p, --print[=NUM]   Print primes or prime k-tuplets, NUM <= 6.
                      Print primes: -p or --print,
                      print twin primes: -p2 or --print=2,
                      print prime triplets: -p3 or --print=3, ...
  -q, --quiet         Quiet mode, prints less output.
  -s, --size=SIZE     Set the sieve size in KiB, SIZE <= 4096.
                      By default primesieve uses a sieve size that
                      matches your CPU's L1 cache size (per core) or is
                      slightly smaller than your CPU's L2 cache size.
      --test          Run various sieving tests.
  -t, --threads=NUM   Set the number of threads, NUM <= CPU cores.
                      Default setting: use all available CPU cores.
      --time          Print the time elapsed in seconds.
  -v, --version       Print version and license information.
```

## Build instructions

You need to have installed a C++ compiler which supports C++11 (or later)
and CMake ≥ 3.4.

```sh
cmake .
make -j
sudo make install
```

* [Detailed build instructions](doc/BUILD.md)

## C++ API

Below is a C++ example with the most common libprimesieve use case.

```C++
#include <primesieve.hpp>
#include <iostream>

int main()
{
  primesieve::iterator it;
  uint64_t prime = it.next_prime();

  // Iterate over the primes below 10^6
  for (; prime < 1000000; prime = it.next_prime())
    std::cout << prime << std::endl;

  return 0;
}
```

* [More C++ examples](doc/CPP_Examples.md)
* [C++ API documentation](https://kimwalisch.github.io/primesieve/api)

## C API

libprimesieve's functions are exposed as C API via the ```primesieve.h``` header.

```C
#include <primesieve.h>
#include <stdio.h>

int main()
{
  primesieve_iterator it;
  primesieve_init(&it);
  uint64_t prime;

  /* Iterate over the primes below 10^6 */
  while ((prime = primesieve_next_prime(&it)) < 1000000)
    printf("%llu\n", prime);

  primesieve_free_iterator(&it);
  return 0;
}
```

* [More C examples](doc/C_Examples.md)
* [C API documentation](https://kimwalisch.github.io/primesieve/api)

## libprimesieve performance tips

* ```primesieve::iterator::next_prime()``` runs up to 2x faster and uses only
half as much memory as ```prev_prime()```. Oftentimes algorithms that iterate
over primes using ```prev_prime()``` can be rewritten using ```next_prime()```
which improves performance in most cases.

* ```primesieve::iterator``` is single-threaded. See the
[multi-threading](#libprimesieve-multi-threading) section for how to
parallelize an algorithm using multiple ```primesieve::iterator``` objects.

* The ```primesieve::iterator``` constructor and the
```primesieve::iterator::skipto()``` method take an optional ```stop_hint```
parameter that can provide a significant speedup if the sieving distance
is relatively small e.g.&nbsp;<&nbsp;sqrt(start). If ```stop_hint``` is set
```primesieve::iterator``` will only buffer primes up to this limit.

* Many of libprimesieve's functions e.g. ```count_primes(start, stop)``` &
```nth_prime(n, start)``` incur an initialization overhead of O(sqrt(start))
even if the total sieving distance is tiny. It is therefore not a good idea to
call these functions repeatedly in a loop unless the sieving distance is
sufficiently large e.g. >&nbsp;sqrt(start). If the sieving distance is mostly
small consider using a ```primesieve::iterator``` instead to avoid the
recurring initialization overhead.

## libprimesieve multi-threading

By default libprimesieve uses multi-threading for counting primes/k-tuplets
and for finding the nth prime. However ```primesieve::iterator``` the most
useful feature provided by libprimesieve runs single-threaded because
it is simply not possible to efficiently parallelize the generation of primes
in sequential order.

Hence if you want to parallelize an algorithm using ```primesieve::iterator```
you need to implement the multi-threading part yourself. The basic technique
for parallelizing an algorithm using ```primesieve::iterator``` is:

* Subdivide the sieving distance into equally sized chunks.
* Process each chunk in its own thread.
* Combine the partial thread results to get the final result.

The C++ example below calculates the sum of the primes ≤ 10<sup>10</sup> in parallel
using [OpenMP](https://en.wikipedia.org/wiki/OpenMP). Each thread processes a
chunk of size ```(dist / threads) + 1``` using its own ```primesieve::iterator```
object. The OpenMP reduction clause takes care of adding the partial
prime sum results together in a thread safe manner.

```C++
#include <primesieve.hpp>
#include <iostream>
#include <omp.h>

int main()
{
  uint64_t sum = 0;
  uint64_t dist = 1e10;
  int threads = omp_get_max_threads();
  uint64_t thread_dist = (dist / threads) + 1;

  #pragma omp parallel for reduction(+: sum)
  for (int i = 0; i < threads; i++)
  {
    uint64_t start = i * thread_dist;
    uint64_t stop = std::min(start + thread_dist, dist);
    primesieve::iterator it(start, stop);
    uint64_t prime = it.next_prime();

    for (; prime <= stop; prime = it.next_prime())
      sum += prime;
  }

  std::cout << "Sum of the primes below " << dist << ": " << sum << std::endl;

  return 0;
}
```

<details>
<summary>Build instructions</summary>

```bash
# Unix-like OSes
wget https://kimwalisch.github.io/primesieve/primesum.cpp
c++ -O3 -fopenmp primesum.cpp -o primesum -lprimesieve
time ./primesum
```

</details>

## Linking against libprimesieve

#### Unix-like OSes

```sh
c++ -O2 primes.cpp -lprimesieve
cc  -O2 primes.c   -lprimesieve
```

If you have
[built libprimesieve yourself](doc/BUILD.md#primesieve-build-instructions) then
the default installation path is usually ```/usr/local/lib``` which is not part
of ```LD_LIBRARY_PATH``` on many OSes. Hence you may need to export some
environment variables:

```sh
export LIBRARY_PATH=/usr/local/lib:$LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
export CPLUS_INCLUDE_PATH=/usr/local/include:$CPLUS_INCLUDE_PATH
export C_INCLUDE_PATH=/usr/local/include:$C_INCLUDE_PATH
```

#### Microsoft Visual C++

```sh
cl /O2 /EHsc /MD primes.cpp /I "path\to\primesieve\include" /link "path\to\primesieve.lib"
```

## CMake support

Since primesieve-6.4 you can easily link against libprimesieve in your
```CMakeLists.txt```:

```CMake
find_package(primesieve REQUIRED)
target_link_libraries(your_target primesieve::primesieve)
```

To link against the static libprimesieve use:

```CMake
find_package(primesieve REQUIRED static)
target_link_libraries(your_target primesieve::primesieve)
```

* Example [CMakeLists.txt](doc/C_Examples.md#minimal-cmake-project-file) for C programs
* Example [CMakeLists.txt](doc/CPP_Examples.md#minimal-cmake-project-file) for C++ programs

## Bindings for other languages

primesieve natively supports C and C++ and has bindings available for:

<table>
    <tr>
        <td><b>Common Lisp:</b></td>
        <td><a href="https://github.com/AaronChen0/cl-primesieve">cl-primesieve</a></td>
    </tr>
    <tr>
        <td><b>Julia:</b></td>
        <td><a href="https://github.com/jlapeyre/PrimeSieve.jl">PrimeSieve.jl</a></td>
    </tr>
    <tr>
        <td><b>Nim:</b></td>
        <td><a href="https://github.com/nandub/primesievec-nim">primesievec-nim</a></td>
    </tr>
    <tr>
        <td><b>Haskell:</b></td>
        <td><a href="https://hackage.haskell.org/package/primesieve">primesieve-haskell</a></td>
    </tr>
    <tr>
        <td><b>Pascal:</b></td>
        <td><a href="https://github.com/JulStrat/primesieve-pas">primesieve-pas</a></td>
    </tr> 
    <tr>
        <td><b>Perl:</b></td>
        <td><a href="https://gitlab.com/oesiman/primesieve">Primesieve</a></td>
    </tr>
    <tr>
        <td><b>Python:</b></td>
        <td><a href="https://github.com/kimwalisch/primesieve-python">primesieve-python</a></td>
    </tr>
    <tr>
        <td><b>Raku:</b></td>
        <td><a href="https://github.com/CurtTilmes/raku-primesieve">raku-primesieve</a></td>
    </tr>
    <tr>
        <td><b>Ruby:</b></td>
        <td><a href="https://github.com/robertjlooby/primesieve-ruby">primesieve-ruby</a></td>
    </tr>
    <tr>
        <td><b>Rust:</b></td>
        <td><a href="https://github.com/pthariensflame/primesieve.rs">primesieve.rs</a></td>
    </tr>   
</table>

Many thanks to the developers of these bindings!
