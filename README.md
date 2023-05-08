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
This algorithm has a run time complexity of $O(n\log{\log{n}})$ operations and uses
$O(\sqrt{n})$ memory. Furthermore primesieve uses the
[bucket sieve](http://sweet.ua.pt/tos/software/prime_sieve.html)
algorithm which improves the cache efficiency when generating primes > 2<sup>32</sup>.
primesieve uses 8 bytes per sieving prime, in practice its memory usage is about
$\pi(\sqrt{n})\times 8$ bytes per thread.

* [More algorithm details](doc/ALGORITHMS.md)

## Installation

The primesieve command-line program can be installed using your operating system's
package manager. For doing development with libprimesieve you may need
to install ```libprimesieve-dev``` or ```libprimesieve-devel```.

<table>
    <tr>
        <td><b>Windows:</b></td>
        <td><code>winget install primesieve</code></td>
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
        <td><b>Chocolatey:</b></td>
        <td><code>choco install primesieve</code></td>
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
        <td><b>FreeBSD:</b></td>
        <td><code>pkg install primesieve</code></td>
    </tr>
    <tr>
        <td><b>openSUSE:</b></td>
        <td><code>sudo zypper install primesieve</code></td>
    </tr>
</table>

## Usage examples

```sh
# Count the primes ≤ 1e10 using all CPU cores
primesieve 1e10

# Print the primes ≤ 1000000
primesieve 1000000 --print

# Store the primes ≤ 1000000 in a text file
primesieve 1000000 --print > primes.txt

# Print the twin primes ≤ 1000000
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
  -s, --size=SIZE     Set the sieve size in KiB, SIZE <= 8192.
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
cmake --build . --parallel
sudo cmake --install .
sudo ldconfig
```

* [Detailed build instructions](doc/BUILD.md)

## C++ API

Include the ```<primesieve.hpp>``` header to use libprimesieve's C++ API.

```C++
#include <primesieve.hpp>
#include <iostream>

int main()
{
  primesieve::iterator it;
  uint64_t prime = it.next_prime();

  // Iterate over the primes < 10^6
  for (; prime < 1000000; prime = it.next_prime())
    std::cout << prime << std::endl;

  return 0;
}
```

* [C++ API documentation](doc/CPP_API.md)

## C API

Include the ```<primesieve.h>``` header to use libprimesieve's C API.

```C
#include <primesieve.h>
#include <inttypes.h>
#include <stdio.h>

int main()
{
  primesieve_iterator it;
  primesieve_init(&it);
  uint64_t prime;

  /* Iterate over the primes < 10^6 */
  while ((prime = primesieve_next_prime(&it)) < 1000000)
    printf("%" PRIu64 "\n", prime);

  primesieve_free_iterator(&it);
  return 0;
}
```

* [C API documentation](doc/C_API.md)

## Bindings for other languages

primesieve natively supports C and C++ and has bindings available for:

<table>
    <tr>
        <td><b>Common Lisp:</b></td>
        <td><a href="https://github.com/AaronChen0/cl-primesieve">cl-primesieve</a></td>
    </tr>
    <tr>
        <td><b>Janet:</b></td>
        <td><a href="https://github.com/bunder/janet-primesieve">janet-primesieve</a></td>
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
        <td><a href="https://github.com/shlomif/primesieve-python">primesieve-python</a></td>
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
