///
/// @file   help.cpp
/// @brief  help() and version() functions of the primesieve
///         console application.
///
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>

#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

namespace {

const string helpMenu
{
  "Usage: primesieve [START] STOP [OPTION]...\n"
  "Generate the primes and/or prime k-tuplets inside [START, STOP]\n"
  "(< 2^64) using the segmented sieve of Eratosthenes.\n"
  "\n"
  "Options:\n"
  "  -c[N+], --count[=N+]   Count primes and prime k-tuplets, N <= 6,\n"
  "                         e.g. -c1 primes, -c2 twins, -c3 triplets, ...\n"
  "          --cpu-info     Print CPU information\n"
  "  -d<N>,  --dist=<N>     Sieve the interval [START, START + N]\n"
  "  -h,     --help         Print this help menu\n"
  "  -n,     --nth-prime    Calculate the nth prime,\n"
  "                         e.g. 1 100 -n finds the 1st prime > 100\n"
  "          --no-status    Turn off the progressing status\n"
  "  -p[N],  --print[=N]    Print primes or prime k-tuplets, N <= 6,\n"
  "                         e.g. -p1 primes, -p2 twins, -p3 triplets, ...\n"
  "  -q,     --quiet        Quiet mode, prints less output\n"
  "  -s<N>,  --size=<N>     Set the sieve size in KiB, N <= 4096\n"
  "          --test         Run various sieving tests\n"
  "  -t<N>,  --threads=<N>  Set the number of threads, N <= CPU cores\n"
  "          --time         Print the time elapsed in seconds\n"
  "  -v,     --version      Print version and license information\n"
  "\n"
  "Examples:\n"
  "  primesieve 1000        Count the primes below 1000\n"
  "  primesieve 1000 -c2    Count the twin primes below 1000\n"
  "  primesieve 1e6 -p      Print the primes below 10^6\n"
  "  primesieve 100 200 -p  Print the primes inside [100, 200]"
};

} // namespace

void help()
{
  cout << helpMenu << endl;
  exit(0);
}

void version()
{
  cout << "primesieve " << primesieve::primesieve_version();
  cout << ", <https://primesieve.org>" << endl;
  cout << "Copyright (C) 2010 - 2019 Kim Walisch" << endl << endl;
  cout << "BSD 2-Clause License <https://opensource.org/licenses/BSD-2-Clause>" << endl;
  exit(0);
}
