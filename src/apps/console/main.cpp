///
/// @file   main.cpp
/// @brief  This file contains the main() function of the primesieve
///         console (terminal) application.
///
/// Copyright (C) 2015 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/ParallelPrimeSieve.hpp>
#include "cmdoptions.hpp"

#include <iostream>
#include <exception>
#include <iomanip>
#include <algorithm>
#include <string>

using namespace std;
using namespace primesieve;

namespace {

void printResults(ParallelPrimeSieve& pps,
                  PrimeSieveOptions& options)
{
  cout << left;

  const string labels[] =
  {
    "Primes",
    "Twin primes",
    "Prime triplets",
    "Prime quadruplets",
    "Prime quintuplets",
    "Prime sextuplets"
  };

  // largest label size, computed below
  int size = 0;

  for (int i = 0; i < 6; i++)
  {
    if (pps.isCount(i))
    {
      int label_size = (int) labels[i].size();
      size = max(size, label_size);
    }
  }

  if (options.time)
  {
    int label_size = (int) string("Seconds").size();
    size = max(size, label_size);
  }

  // print results
  for (int i = 0; i < 6; i++)
  {
    if (pps.isCount(i))
      cout << setw(size) << labels[i] << " : "
           << pps.getCount(i) << endl;
  }

  if (options.time)
    cout << setw(size) << "Seconds" << " : "
         << fixed << setprecision(3) << pps.getSeconds()
         << endl;
}

/// Used to count and print primes and prime k-tuplets
void sieve(PrimeSieveOptions& options)
{
  ParallelPrimeSieve pps;
  deque<uint64_t>& numbers = options.numbers;

  if (options.flags     != 0) pps.setFlags(options.flags);
  if (options.sieveSize != 0) pps.setSieveSize(options.sieveSize);
  if (options.threads   != 0) pps.setNumThreads(options.threads);
  else if (pps.isPrint())     pps.setNumThreads(1);

  if (numbers.size() < 2)
    numbers.push_front(0);

  pps.setStart(numbers[0]);
  pps.setStop (numbers[1]);

  if (!options.quiet)
  {
    cout << "Sieve size = " << pps.getSieveSize() << " kilobytes" << endl;
    cout << "Threads    = " << pps.getNumThreads() << endl;

    // enable printing status
    if (!pps.isPrint())
      pps.addFlags(pps.PRINT_STATUS);
  }

  pps.sieve();
  printResults(pps, options);
}

void nthPrime(PrimeSieveOptions& options)
{
  ParallelPrimeSieve pps;
  deque<uint64_t>& numbers = options.numbers;

  if (options.flags     != 0) pps.setFlags(options.flags);
  if (options.sieveSize != 0) pps.setSieveSize(options.sieveSize);
  if (options.threads   != 0) pps.setNumThreads(options.threads);

  if (numbers.size() < 2)
    numbers.push_back(0);

  uint64_t n = numbers[0];
  uint64_t start = numbers[1];
  uint64_t nthPrime = pps.nthPrime(n, start);

  cout << "Nth prime : " << nthPrime << endl;

  if (options.time)
    cout << "Seconds   : " << fixed << setprecision(3)
         << pps.getSeconds() << endl;
}

} // namespace

int main(int argc, char** argv)
{
  PrimeSieveOptions options = parseOptions(argc, argv);
  bool isNthPrime = options.nthPrime;

  try
  {
    if (isNthPrime)
      nthPrime(options);
    else
      sieve(options);
  }
  catch (exception& e)
  {
    cerr << "Error: " << e.what() << "." << endl
         << "Try `primesieve --help' for more information." << endl;

    return 1;
  }

  return 0;
}
