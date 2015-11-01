///
/// @file   help.cpp
/// @brief  help() and version() functions of the primesieve
///         console application.
///
/// Copyright (C) 2015 Kim Walisch, <kim.walisch@gmail.com>
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
  "  -c[N+], --count[=N+]     Count primes and prime k-tuplets, 1 <= N <= 6\n"
  "                           N: 1 = primes, 2 = twins, 3 = triplets, ...\n"
  "  -d<N>,  --dist=<N>       Sieve the interval [START, START + N]\n"
  "  -h,     --help           Print this help menu\n"
  "  -n,     --nthprime       Calculate the nth prime,\n"
  "                           e.g. 1 100 -n finds the 1st prime > 100\n"
  "  -p[N],  --print[=N]      Print primes or prime k-tuplets, 1 <= N <= 6\n"
  "                           N: 1 = primes, 2 = twins, 3 = triplets, ...\n"
  "  -q,     --quiet          Quiet mode, prints less output\n"
  "  -s<N>,  --size=<N>       Set the sieve size in kilobytes, 1 <= N <= 2048\n"
  "          --test           Run various sieving tests and exit\n"
  "  -t<N>,  --threads=<N>    Set the number of threads, 1 <= N <= CPU cores\n"
  "          --time           Print the time elapsed in seconds\n"
  "  -v,     --version        Print version and license information\n"
  "\n"
  "Examples:\n"
  "\n"
  "  Print the primes below 1000000\n"
  "  $ primesieve 1e6 --print\n"
  "\n"
  "  Count the twin primes inside [10^9, 10^9 + 2^32]\n"
  "  $ primesieve 1e9 --dist=2**32 -c2"
);

} // end namespace

void help()
{
  cout << helpMenu << endl;
  exit(1);
}

void version()
{
  cout << "primesieve " << primesieve::primesieve_version();
  cout << ", <http://primesieve.org>" << endl;
  cout << "Copyright (C) 2015 Kim Walisch" << endl;
  cout << "BSD 2-Clause License <http://opensource.org/licenses/BSD-2-Clause>" << endl;

  exit(1);
}
