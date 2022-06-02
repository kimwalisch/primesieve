///
/// @file   main.cpp
/// @brief  primesieve console application.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/ParallelSieve.hpp>
#include <primesieve/pod_vector.hpp>
#include "cmdoptions.hpp"

#include <stdint.h>
#include <iostream>
#include <exception>
#include <iomanip>
#include <string>

using namespace primesieve;

namespace {

void printSettings(const ParallelSieve& ps)
{
  std::cout << "Sieve size = " << ps.getSieveSize() << " KiB" << std::endl;
  std::cout << "Threads = " << ps.idealNumThreads() << std::endl;
}

void printSeconds(double sec)
{
  std::cout << "Seconds: " << std::fixed << std::setprecision(3) << sec << std::endl;
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

  const pod_array<std::string, 6> labels =
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

  // Did we count primes & k-tuplets simultaneously?
  int cnt = 0;
  for (int i = 0; i < 6; i++)
    if (ps.isCount(i))
      cnt++;

  for (int i = 0; i < 6; i++)
  {
    if (ps.isCount(i))
    {
      if (opt.quiet && cnt == 1)
        std::cout << ps.getCount(i) << std::endl;
      else
        std::cout << labels[i] << ps.getCount(i) << std::endl;
    }
  }
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
  ps.setStop(start + std::abs(n * 20));

  if (!opt.quiet)
    printSettings(ps);

  nthPrime = ps.nthPrime(n, start);

  if (opt.time)
    printSeconds(ps.getSeconds());

  if (opt.quiet)
    std::cout << nthPrime << std::endl;
  else
    std::cout << "Nth prime: " << nthPrime << std::endl;
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
  catch (std::exception& e)
  {
    std::cerr << "primesieve: " << e.what() << std::endl
              << "Try 'primesieve --help' for more information." << std::endl;
    return 1;
  }

  return 0;
}
