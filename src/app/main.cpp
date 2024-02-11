///
/// @file   main.cpp
/// @brief  primesieve console application.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/CpuInfo.hpp>
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
void sieve(const CmdOptions& opts)
{
  if (opts.numbers.empty())
    throw primesieve_error("missing STOP number");

  ParallelSieve ps;

  if (opts.flags)
    ps.setFlags(opts.flags);
  if (opts.status)
    ps.addFlags(PRINT_STATUS);
  if (opts.sieveSize)
    ps.setSieveSize(opts.sieveSize);
  if (opts.threads)
    ps.setNumThreads(opts.threads);
  if (ps.isPrint())
    ps.setNumThreads(1);

  if (opts.numbers.size() < 2)
    ps.setStop(opts.numbers[0]);
  else
  {
    ps.setStart(opts.numbers[0]);
    ps.setStop(opts.numbers[1]);
  }

  if (!opts.quiet)
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

  if (opts.time)
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
      if (opts.quiet && cnt == 1)
        std::cout << ps.getCount(i) << std::endl;
      else
        std::cout << labels[i] << ps.getCount(i) << std::endl;
    }
  }
}

void nthPrime(const CmdOptions& opts)
{
  if (opts.numbers.empty())
    throw primesieve_error("missing n number");

  ParallelSieve ps;
  int64_t n = opts.numbers[0];
  uint64_t start = 0;

  if (opts.numbers.size() > 1)
    start = opts.numbers[1];
  if (opts.flags)
    ps.setFlags(opts.flags);
  if (opts.sieveSize)
    ps.setSieveSize(opts.sieveSize);
  if (opts.threads)
    ps.setNumThreads(opts.threads);

  uint64_t nthPrime = 0;
  ps.setStart(start);
  ps.setStop(start + std::abs(n * 20));

  if (!opts.quiet)
    printSettings(ps);

  nthPrime = ps.nthPrime(n, start);

  if (opts.time)
    printSeconds(ps.getSeconds());

  if (opts.quiet)
    std::cout << nthPrime << std::endl;
  else
    std::cout << "Nth prime: " << nthPrime << std::endl;
}

void RiemannR(const CmdOptions& opts)
{
  if (opts.numbers.empty())
    throw primesieve_error("missing x number");

  long double x = (long double) opts.numbers[0];
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

void RiemannR_inverse(const CmdOptions& opts)
{
  if (opts.numbers.empty())
    throw primesieve_error("missing x number");

  long double x = (long double) opts.numbers[0];
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

void cpuInfo()
{
  const primesieve::CpuInfo cpu;

  if (cpu.hasCpuName())
    std::cout << cpu.cpuName() << std::endl;
  else
    std::cout << "CPU name: unknown" << std::endl;

  if (cpu.hasLogicalCpuCores())
    std::cout << "Logical CPU cores: " << cpu.logicalCpuCores() << std::endl;
  else
    std::cout << "Logical CPU cores: unknown" << std::endl;

  // We only show AVX512 info if libprimesieve has been compiled
  // with AVX512 support. If "AVX512: yes" then primesieve::iterator
  // uses the AVX512 version of PrimeGenerator::fillNextPrimes().
  #if defined(MULTIARCH_AVX512)
    if (cpu.hasAVX512())
      std::cout << "Has AVX512: yes" << std::endl;
    else
      std::cout << "Has AVX512: no" << std::endl;
  #endif

  if (cpu.hasL1Cache())
    std::cout << "L1 cache size: " << (cpu.l1CacheBytes() >> 10) << " KiB" << std::endl;

  if (cpu.hasL2Cache())
    std::cout << "L2 cache size: " << (cpu.l2CacheBytes() >> 10) << " KiB" << std::endl;

  if (cpu.hasL3Cache())
    std::cout << "L3 cache size: " << (cpu.l3CacheBytes() >> 20) << " MiB" << std::endl;

  if (cpu.hasL1Cache())
  {
    if (!cpu.hasL1Sharing())
      std::cout << "L1 cache sharing: unknown" << std::endl;
    else
      std::cout << "L1 cache sharing: " << cpu.l1Sharing()
                << ((cpu.l1Sharing() > 1) ? " threads" : " thread") << std::endl;
  }

  if (cpu.hasL2Cache())
  {
    if (!cpu.hasL2Sharing())
      std::cout << "L2 cache sharing: unknown" << std::endl;
    else
      std::cout << "L2 cache sharing: " << cpu.l2Sharing()
                << ((cpu.l2Sharing() > 1) ? " threads" : " thread") << std::endl;
  }

  if (cpu.hasL3Cache())
  {
    if (!cpu.hasL3Sharing())
      std::cout << "L3 cache sharing: unknown" << std::endl;
    else
      std::cout << "L3 cache sharing: " << cpu.l3Sharing()
                << ((cpu.l3Sharing() > 1) ? " threads" : " thread") << std::endl;
  }

  if (!cpu.hasL1Cache() &&
      !cpu.hasL2Cache() &&
      !cpu.hasL3Cache())
  {
    std::cout << "L1 cache size: unknown" << std::endl;
    std::cout << "L2 cache size: unknown" << std::endl;
    std::cout << "L3 cache size: unknown" << std::endl;
    std::cout << "L1 cache sharing: unknown" << std::endl;
    std::cout << "L2 cache sharing: unknown" << std::endl;
    std::cout << "L3 cache sharing: unknown" << std::endl;
  }
}

} // namespace

int main(int argc, char* argv[])
{
  try
  {
    CmdOptions opts = parseOptions(argc, argv);

    switch (opts.option)
    {
      case OPTION_CPU_INFO:  cpuInfo(); break;
      case OPTION_NTH_PRIME: nthPrime(opts); break;
      case OPTION_R:         RiemannR(opts); break;
      case OPTION_R_INVERSE: RiemannR_inverse(opts); break;
      default:               sieve(opts); break;
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
