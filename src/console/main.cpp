///
/// @file   main.cpp
/// @brief  primesieve console application.
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
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

void printResults(ParallelPrimeSieve& pps, CmdOptions& opts)
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

  if (opts.time)
  {
    int label_size = (int) string("Seconds").size();
    size = max(size, label_size);
  }

  if (opts.time)
    cout << setw(size) << "Seconds" << " : "
         << fixed << setprecision(3) << pps.getSeconds()
         << endl;

  // print results
  for (int i = 0; i < 6; i++)
  {
    if (pps.isCount(i))
      cout << setw(size) << labels[i] << " : "
           << pps.getCount(i) << endl;
  }
}

/// Used to count and print primes and prime k-tuplets
void sieve(CmdOptions& opts)
{
  ParallelPrimeSieve pps;
  deque<uint64_t>& numbers = opts.numbers;

  if (opts.flags     != 0) pps.setFlags(opts.flags);
  if (opts.sieveSize != 0) pps.setSieveSize(opts.sieveSize);
  if (opts.threads   != 0) pps.setNumThreads(opts.threads);
  else if (pps.isPrint())  pps.setNumThreads(1);

  if (numbers.size() < 2)
    numbers.push_front(0);

  pps.setStart(numbers[0]);
  pps.setStop(numbers[1]);

  if (!opts.quiet)
  {
    cout << "Sieve size = " << pps.getSieveSize() << " kilobytes" << endl;
    cout << "Threads = " << pps.idealNumThreads() << endl;
  }

  if (opts.status)
    pps.addFlags(pps.PRINT_STATUS);

  pps.sieve();
  printResults(pps, opts);
}

void nthPrime(CmdOptions& opts)
{
  ParallelPrimeSieve pps;
  deque<uint64_t>& numbers = opts.numbers;

  if (opts.flags     != 0) pps.setFlags(opts.flags);
  if (opts.sieveSize != 0) pps.setSieveSize(opts.sieveSize);
  if (opts.threads   != 0) pps.setNumThreads(opts.threads);

  if (numbers.size() < 2)
    numbers.push_back(0);

  uint64_t n = numbers[0];
  uint64_t start = numbers[1];
  uint64_t nthPrime = pps.nthPrime(n, start);

  if (opts.time)
    cout << "Seconds   : " << fixed << setprecision(3)
         << pps.getSeconds() << endl;

  cout << "Nth prime : " << nthPrime << endl;
}

} // namespace

int main(int argc, char** argv)
{
  CmdOptions opts = parseOptions(argc, argv);

  try
  {
    if (opts.nthPrime)
      nthPrime(opts);
    else
      sieve(opts);
  }
  catch (exception& e)
  {
    cerr << "Error: " << e.what() << "." << endl
         << "Try `primesieve --help' for more information." << endl;

    return 1;
  }

  return 0;
}
