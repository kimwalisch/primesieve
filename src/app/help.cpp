///
/// @file   help.cpp
/// @brief  help() and version() functions of the primesieve
///         console application.
///
/// Copyright (C) 2023 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>

#include <iostream>
#include <cstdlib>
#include <string>

void help(int exitCode)
{
  const std::string helpMenu =
    "Usage: primesieve [START] STOP [OPTION]...\n"
    "Generate the primes and/or prime k-tuplets inside [START, STOP]\n"
    "(< 2^64) using the segmented sieve of Eratosthenes.\n"
    "\n"
    "Options:\n"
    "  -c, --count[=NUM+]  Count primes and/or prime k-tuplets, NUM <= 6.\n"
    "                      Count primes: -c or --count (default option),\n"
    "                      count twin primes: -c2 or --count=2,\n"
    "                      count prime triplets: -c3 or --count=3, ...\n"
    "      --cpu-info      Print CPU information (cache sizes).\n"
    "  -d, --dist=DIST     Sieve the interval [START, START + DIST].\n"
    "  -h, --help          Print this help menu.\n"
    "  -n, --nth-prime     Find the nth prime.\n"
    "                      primesieve 100 -n: finds the 100th prime,\n"
    "                      primesieve 2 100 -n: finds the 2nd prime > 100.\n"
    "      --no-status     Turn off the progressing status.\n"
    "  -p, --print[=NUM]   Print primes or prime k-tuplets, NUM <= 6.\n"
    "                      Print primes: -p or --print,\n"
    "                      print twin primes: -p2 or --print=2,\n"
    "                      print prime triplets: -p3 or --print=3, ...\n"
    "  -q, --quiet         Quiet mode, prints less output.\n"
    "  -s, --size=SIZE     Set the sieve size in KiB, SIZE <= 8192.\n"
    "                      By default primesieve uses a sieve size that\n"
    "                      matches your CPU's L1 cache size (per core) or is\n"
    "                      slightly smaller than your CPU's L2 cache size.\n"
    "      --test          Run various sieving tests.\n"
    "  -t, --threads=NUM   Set the number of threads, NUM <= CPU cores.\n"
    "                      Default setting: use all available CPU cores.\n"
    "      --time          Print the time elapsed in seconds.\n"
    "  -v, --version       Print version and license information.";

  std::cout << helpMenu << std::endl;
  std::exit(exitCode);
}

void version()
{
  std::cout << "primesieve " << primesieve::primesieve_version();
  std::cout << ", <https://github.com/kimwalisch/primesieve>" << std::endl;
  std::cout << "Copyright (C) 2010 - 2023 Kim Walisch" << std::endl;
  std::cout << "BSD 2-Clause License <https://opensource.org/licenses/BSD-2-Clause>" << std::endl;
  std::exit(0);
}
