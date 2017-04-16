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

#include <stdint.h>
#include <cstddef>
#include <iostream>
#include <exception>
#include <iomanip>
#include <algorithm>
#include <string>

using namespace std;
using namespace primesieve;

namespace {

void printResults(ParallelPrimeSieve& ps, CmdOptions& opt)
{
  cout << left;

  const string text[] =
  {
    "Primes",
    "Twin primes",
    "Prime triplets",
    "Prime quadruplets",
    "Prime quintuplets",
    "Prime sextuplets"
  };

  // largest text size
  size_t size = 0;

  if (opt.time)
    size = max(size, string("Seconds").size());

  for (int i = 0; i < 6; i++)
    if (ps.isCount(i))
      size = max(size, text[i].size());

  int width = (int) size;

  if (opt.time)
    cout << setw(width) << "Seconds" << " : "
         << fixed << setprecision(3) << ps.getSeconds()
         << endl;

  // print results
  for (int i = 0; i < 6; i++)
  {
    if (ps.isCount(i))
      cout << setw(width) << text[i] << " : "
           << ps.getCount(i) << endl;
  }
}

/// Used to count and print primes and prime k-tuplets
void sieve(CmdOptions& opt)
{
  ParallelPrimeSieve ps;
  auto& numbers = opt.numbers;

  if (opt.flags)
    ps.setFlags(opt.flags);
  if (opt.sieveSize)
    ps.setSieveSize(opt.sieveSize);
  if (opt.threads)
    ps.setNumThreads(opt.threads);
  if (ps.isPrint())
    ps.setNumThreads(1);

  if (numbers.size() < 2)
    numbers.push_front(0);

  ps.setStart(numbers[0]);
  ps.setStop(numbers[1]);

  if (!opt.quiet)
  {
    cout << "Sieve size = " << ps.getSieveSize() << " kilobytes" << endl;
    cout << "Threads = " << ps.idealNumThreads() << endl;
  }

  if (opt.status)
    ps.addFlags(ps.PRINT_STATUS);

  ps.sieve();
  printResults(ps, opt);
}

void nthPrime(CmdOptions& opt)
{
  ParallelPrimeSieve ps;
  auto& numbers = opt.numbers;

  if (opt.flags)
    ps.setFlags(opt.flags);
  if (opt.sieveSize)
    ps.setSieveSize(opt.sieveSize);
  if (opt.threads)
    ps.setNumThreads(opt.threads);

  if (numbers.size() < 2)
    numbers.push_back(0);

  uint64_t n = numbers[0];
  uint64_t start = numbers[1];
  uint64_t nthPrime = ps.nthPrime(n, start);

  if (opt.time)
    cout << "Seconds   : " << fixed << setprecision(3)
         << ps.getSeconds() << endl;

  cout << "Nth prime : " << nthPrime << endl;
}

} // namespace

int main(int argc, char* argv[])
{
  try
  {
    CmdOptions opt = parseOptions(argc, argv);

    if (opt.nthPrime)
      nthPrime(opt);
    else
      sieve(opt);
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
