///
/// @file   main.cpp
/// @brief  primesieve console application.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/ParallelSieve.hpp>
#include <primesieve/primesieve_error.hpp>
#include <primesieve/RiemannR.hpp>
#include <primesieve/Vector.hpp>
#include "cmdoptions.hpp"

#include <stdint.h>
#include <exception>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

using primesieve::Array;
using primesieve::ParallelSieve;
using primesieve::primesieve_error;
using primesieve::PRINT_STATUS;

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
  if (opt.numbers.empty())
    throw primesieve_error("missing STOP number");

  ParallelSieve ps;

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

  if (opt.numbers.size() < 2)
    ps.setStop(opt.numbers[0]);
  else
  {
    ps.setStart(opt.numbers[0]);
    ps.setStop(opt.numbers[1]);
  }

  if (!opt.quiet)
    printSettings(ps);

  ps.sieve();

  const Array<std::string, 6> labels =
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
  if (opt.numbers.empty())
    throw primesieve_error("missing input number n");

  ParallelSieve ps;
  int64_t n = opt.numbers[0];
  uint64_t start = 0;

  if (opt.numbers.size() > 1)
    start = opt.numbers[1];
  if (opt.flags)
    ps.setFlags(opt.flags);
  if (opt.sieveSize)
    ps.setSieveSize(opt.sieveSize);
  if (opt.threads)
    ps.setNumThreads(opt.threads);

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

void RiemannR(CmdOptions& opt)
{
  if (opt.numbers.empty())
    throw primesieve_error("missing input number x");

  long double x = (long double) opt.numbers[0];
  long double Rx = primesieve::RiemannR(x);

  std::ostringstream oss;
  oss << std::fixed << std::setprecision(10) << Rx;
  std::string res = oss.str();

  // Remove trailing 0 decimal digits
  if (res.find('.') != std::string::npos)
  {
    std::reverse(res.begin(), res.end());
    res = res.substr(res.find_first_not_of('0'));
    if (res.at(0) == '.')
      res = res.substr(1);

    std::reverse(res.begin(), res.end());
  }

  std::cout << res << std::endl;
}

void RiemannR_inverse(CmdOptions& opt)
{
  if (opt.numbers.empty())
    throw primesieve_error("missing input number x");

  long double x = (long double) opt.numbers[0];
  long double R_inv_x = primesieve::RiemannR_inverse(x);

  std::ostringstream oss;
  oss << std::fixed << std::setprecision(10) << R_inv_x;
  std::string res = oss.str();

  // Remove trailing 0 decimal digits
  if (res.find('.') != std::string::npos)
  {
    std::reverse(res.begin(), res.end());
    res = res.substr(res.find_first_not_of('0'));
    if (res.at(0) == '.')
      res = res.substr(1);

    std::reverse(res.begin(), res.end());
  }

  std::cout << res << std::endl;
}

} // namespace

int main(int argc, char* argv[])
{
  try
  {
    CmdOptions opt = parseOptions(argc, argv);

    switch (opt.option)
    {
      case OPTION_NTH_PRIME: nthPrime(opt); break;
      case OPTION_R:         RiemannR(opt); break;
      case OPTION_R_INVERSE: RiemannR_inverse(opt); break;
      default:               sieve(opt); break;
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "primesieve: " << e.what() << std::endl
              << "Try 'primesieve --help' for more information." << std::endl;
    return 1;
  }

  return 0;
}
