///
/// @file   main.cpp
/// @brief  primesieve console application.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/ParallelPrimeSieve.hpp>
#include "calculator.hpp"
#include "cmdoptions.hpp"

#include <iostream>
#include <exception>
#include <iomanip>
#include <algorithm>
#include <string>

using namespace std;
using namespace primesieve;

namespace {

void printResults(ParallelPrimeSieve& ps, CmdOptions& opts)
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
    if (ps.isCount(i))
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
         << fixed << setprecision(3) << ps.getSeconds()
         << endl;

  // print results
  for (int i = 0; i < 6; i++)
  {
    if (ps.isCount(i))
      cout << setw(size) << labels[i] << " : "
           << ps.getCount(i) << endl;
  }
}

/// Used to count and print primes and prime k-tuplets
void sieve(CmdOptions& opts)
{
  ParallelPrimeSieve ps;
  auto& numbers = opts.numbers;

  if (opts.flags)
    ps.setFlags(opts.flags);
  if (opts.sieveSize)
    ps.setSieveSize(opts.sieveSize);
  if (opts.threads)
    ps.setNumThreads(opts.threads);
  if (ps.isPrint())
    ps.setNumThreads(1);

  if (numbers.size() < 2)
    numbers.push_front(0);

  ps.setStart(numbers[0]);
  ps.setStop(numbers[1]);

  if (!opts.quiet)
  {
    cout << "Sieve size = " << ps.getSieveSize() << " kilobytes" << endl;
    cout << "Threads = " << ps.idealNumThreads() << endl;
  }

  if (opts.status)
    ps.addFlags(ps.PRINT_STATUS);

  ps.sieve();
  printResults(ps, opts);
}

void nthPrime(CmdOptions& opts)
{
  ParallelPrimeSieve ps;
  auto& numbers = opts.numbers;

  if (opts.flags)
    ps.setFlags(opts.flags);
  if (opts.sieveSize)
    ps.setSieveSize(opts.sieveSize);
  if (opts.threads)
    ps.setNumThreads(opts.threads);

  if (numbers.size() < 2)
    numbers.push_back(0);

  uint64_t n = numbers[0];
  uint64_t start = numbers[1];
  uint64_t nthPrime = ps.nthPrime(n, start);

  if (opts.time)
    cout << "Seconds   : " << fixed << setprecision(3)
         << ps.getSeconds() << endl;

  cout << "Nth prime : " << nthPrime << endl;
}

} // namespace

int main(int argc, char** argv)
{
  try
  {
    CmdOptions opts = parseOptions(argc, argv);

    if (opts.nthPrime)
      nthPrime(opts);
    else
      sieve(opts);
  }
  catch (calculator::error& e)
  {
    cerr << e.what() << "." << endl
         << "Try `primesieve --help' for more information." << endl;
    return 1;
  }
  catch (exception& e)
  {
    cerr << "Error: " << e.what() << "." << endl
         << "Try `primesieve --help' for more information." << endl;
    return 1;
  }

  return 0;
}
