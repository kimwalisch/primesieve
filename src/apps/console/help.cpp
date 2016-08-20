///
/// @file   help.cpp
/// @brief  help() and version() functions of the primesieve
///         console application.
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
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

const string helpMenu(
  "Usage: primesieve [START] STOP [OPTION]...\n"
  "Generate the primes and/or prime k-tuplets in the interval [START, STOP]\n"
  "(< 2^64) using the segmented sieve of Eratosthenes.\n"
  "\n"
  "Options:\n"
  "\n"
  "  -c[N+], --count[=N+]    Count primes and prime k-tuplets, N <= 6,\n"
  "                          e.g. -c1 primes, -c2 twins, -c3 triplets, ...\n"
  "  -d<N>,  --dist=<N>      Sieve the interval [START, START + N]\n"
  "  -h,     --help          Print this help menu\n"
  "  -n,     --nthprime      Calculate the nth prime,\n"
  "                          e.g. 1 100 -n finds the 1st prime > 100\n"
  "          --no-status     Turn off the progressing status\n"
  "  -p[N],  --print[=N]     Print primes or prime k-tuplets, N <= 6,\n"
  "                          e.g. -p1 primes, -p2 twins, -p3 triplets, ...\n"
  "  -q,     --quiet         Quiet mode, prints less output\n"
  "  -s<N>,  --size=<N>      Set the sieve size in kilobytes, N <= 2048\n"
  "          --test          Run various sieving tests and exit\n"
  "  -t<N>,  --threads=<N>   Set the number of threads, N <= CPU cores\n"
  "          --time          Print the time elapsed in seconds\n"
  "  -v,     --version       Print version and license information\n"
  "\n"
  "Examples:\n"
  "\n"
  "  primesieve 1000         Count the primes below 1000\n"
  "  primesieve 1000 -c2     Count the twin primes below 1000\n"
  "  primesieve 1e6 --print  Print the primes below 10^6\n"
  "  primesieve 100 200 -p   Print the primes inside [100, 200]\n"
);

} // namespace

void help()
{
  cout << helpMenu << endl;
  exit(1);
}

void version()
{
  cout << "primesieve " << primesieve::primesieve_version();
  cout << ", <http://primesieve.org>" << endl;
  cout << "Copyright (C) 2010-2016 Kim Walisch" << endl;
  cout << endl;

  cout << "BSD 2-Clause License <http://opensource.org/licenses/BSD-2-Clause>" << endl;
  cout << endl;

  exit(1);
}
