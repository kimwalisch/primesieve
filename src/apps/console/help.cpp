///
/// @file   help.cpp
/// @brief  help() and version() functions of the primesieve
///         console application.
///
/// Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
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
  "Use the segmented sieve of Eratosthenes to generate the primes and\n"
  "prime k-tuplets in the interval [START, STOP] < 2^64\n"
  "\n"
  "Options:\n"
  "  -c<N+>, --count=<N+>     Count primes and prime k-tuplets, 1 <= N <= 7\n"
  "                           <N> 1=primes, 2=twins, 3=triplets, ...\n"
  "  -h,     --help           Print this help menu\n"
  "  -n,     --nthprime       Calculate the nth prime\n"
  "                           e.g. 1 100 -n finds the first prime >= 100\n"
  "  -o<N>,  --offset=<N>     Sieve the interval [START, START + N]\n"
  "  -p<N>,  --print=<N>      Print primes or prime k-tuplets, 1 <= N <= 7\n"
  "                           <N> 1=primes, 2=twins, 3=triplets, ...\n"
  "  -q,     --quiet          Quiet mode, prints less output\n"
  "  -s<N>,  --size=<N>       Set the sieve size in kilobytes, 1 <= N <= 2048\n"
  "          --test           Run various sieving tests and exit\n"
  "  -t<N>,  --threads=<N>    Set the number of threads, 1 <= N <= CPU cores\n"
  "  -v,     --version        Print version and license information\n"
  "\n"
  "Example:\n"
  "  Count the primes and print the twin primes below 1000\n"
  "  > primesieve 1000 --count=1 -p2"
);

const string versionInfo(
  "primesieve " PRIMESIEVE_VERSION ", <http://primesieve.org>\n"
  "Copyright (C) 2014 Kim Walisch\n"
  "BSD 2-Clause License <http://opensource.org/licenses/BSD-2-Clause>"
);

} // end namespace

void help()
{
  cout << helpMenu << endl;
  exit(1);
}

void version()
{
  cout << versionInfo << endl;
  exit(1);
}
