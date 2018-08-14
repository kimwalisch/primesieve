///
/// @file   main.cpp
/// @brief  primesieve console application.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/ParallelSieve.hpp>
#include "cmdoptions.hpp"

#include <stdint.h>
#include <iostream>
#include <exception>
#include <iomanip>
#include <string>

using namespace std;
using namespace primesieve;

namespace {

void printSettings(const ParallelSieve& ps)
{
  cout << "Sieve size = " << ps.getSieveSize() << " KiB" << endl;
  cout << "Threads = " << ps.idealNumThreads() << endl;
}

void printSeconds(double sec)
{
  cout << "Seconds: " << fixed << setprecision(3) << sec << endl;
}

/// Count & print primes and prime k-tuplets
void sieve(CmdOptions& opt)
{
  ParallelSieve ps;
  auto& numbers = opt.numbers;

  if (opt.flags)
    ps.setFlags(opt.flags);
  if (opt.status)
    ps.addFlags(PRINT_STATUS);
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
    printSettings(ps);

  ps.sieve();

  const string text[6] =
  {
    "Primes: ",
    "Twin primes: ",
    "Prime triplets: ",
    "Prime quadruplets: ",
    "Prime quintuplets: ",
    "Prime sextuplets: "
  };

  if (opt.time)
    printSeconds(ps.getSeconds());

  for (int i = 0; i < 6; i++)
    if (ps.isCount(i))
      cout << text[i] << ps.getCount(i) << endl;
}

void nthPrime(CmdOptions& opt)
{
  ParallelSieve ps;
  auto& numbers = opt.numbers;

  if (opt.flags)
    ps.setFlags(opt.flags);
  if (opt.sieveSize)
    ps.setSieveSize(opt.sieveSize);
  if (opt.threads)
    ps.setNumThreads(opt.threads);
  if (numbers.size() < 2)
    numbers.push_back(0);

  int64_t n = numbers[0];
  uint64_t start = numbers[1];
  uint64_t nthPrime = 0;
  ps.setStart(start);
  ps.setStop(start + abs(n * 20));

  if (!opt.quiet)
    printSettings(ps);

  nthPrime = ps.nthPrime(n, start);

  if (opt.time)
    printSeconds(ps.getSeconds());

  cout << "Nth prime: " << nthPrime << endl;
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
  catch (exception& e)
  {
    cerr << "primesieve: " << e.what() << endl
         << "Try 'primesieve --help' for more information." << endl;
    return 1;
  }

  return 0;
}
