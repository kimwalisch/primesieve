///
/// @file   cmdoptions.cpp
/// @brief  Parse command-line options for the primesieve console
///         (terminal) application.
///
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "cmdoptions.hpp"

#include <primesieve/calculator.hpp>
#include <primesieve/CpuInfo.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/primesieve_error.hpp>

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <map>
#include <stdint.h>
#include <string>

void help();
void test();
void version();

using namespace std;
using namespace primesieve;

namespace {

/// Command-line option
/// e.g. opt = "--threads", val = "4"
struct Option
{
  string str;
  string opt;
  string val;

  template <typename T>
  T getValue() const
  {
    if (val.empty())
      throw primesieve_error("missing value for option " + str);
    return calculator::eval<T>(val);
  }
};

enum OptionID
{
  OPTION_COUNT,
  OPTION_CPU_INFO,
  OPTION_HELP,
  OPTION_NTH_PRIME,
  OPTION_NO_STATUS,
  OPTION_NUMBER,
  OPTION_DISTANCE,
  OPTION_PRINT,
  OPTION_QUIET,
  OPTION_SIZE,
  OPTION_TEST,
  OPTION_THREADS,
  OPTION_TIME,
  OPTION_VERSION
};

/// Command-line options
map<string, OptionID> optionMap =
{
  { "-c",          OPTION_COUNT },
  { "--count",     OPTION_COUNT },
  { "--cpu-info",  OPTION_CPU_INFO },
  { "-h",          OPTION_HELP },
  { "--help",      OPTION_HELP },
  { "-n",          OPTION_NTH_PRIME },
  { "--nthprime",  OPTION_NTH_PRIME },
  { "--nth-prime", OPTION_NTH_PRIME },
  { "--no-status", OPTION_NO_STATUS },
  { "--number",    OPTION_NUMBER },
  { "-d",          OPTION_DISTANCE },
  { "--dist",      OPTION_DISTANCE },
  { "-p",          OPTION_PRINT },
  { "--print",     OPTION_PRINT },
  { "-q",          OPTION_QUIET },
  { "--quiet",     OPTION_QUIET },
  { "-s",          OPTION_SIZE },
  { "--size",      OPTION_SIZE },
  { "--test",      OPTION_TEST },
  { "-t",          OPTION_THREADS },
  { "--threads",   OPTION_THREADS },
  { "--time",      OPTION_TIME },
  { "-v",          OPTION_VERSION },
  { "--version",   OPTION_VERSION }
};

void optionPrint(Option& opt,
                 CmdOptions& opts)
{
  opts.quiet = true;

  // by default print primes
  if (opt.val.empty())
    opt.val = "1";

  switch (opt.getValue<int>())
  {
    case 1: opts.flags |= PRINT_PRIMES; break;
    case 2: opts.flags |= PRINT_TWINS; break;
    case 3: opts.flags |= PRINT_TRIPLETS; break;
    case 4: opts.flags |= PRINT_QUADRUPLETS; break;
    case 5: opts.flags |= PRINT_QUINTUPLETS; break;
    case 6: opts.flags |= PRINT_SEXTUPLETS; break;
    default: throw primesieve_error("invalid option " + opt.str);
  }
}

void optionCount(Option& opt,
                 CmdOptions& opts)
{
  // by default count primes
  if (opt.val.empty())
    opt.val = "1";

  int n = opt.getValue<int>();

  for (; n > 0; n /= 10)
  {
    switch (n % 10)
    {
      case 1: opts.flags |= COUNT_PRIMES; break;
      case 2: opts.flags |= COUNT_TWINS; break;
      case 3: opts.flags |= COUNT_TRIPLETS; break;
      case 4: opts.flags |= COUNT_QUADRUPLETS; break;
      case 5: opts.flags |= COUNT_QUINTUPLETS; break;
      case 6: opts.flags |= COUNT_SEXTUPLETS; break;
      default: throw primesieve_error("invalid option " + opt.str);
    }
  }
}

void optionDistance(Option& opt,
                    CmdOptions& opts)
{
  uint64_t start = 0;
  uint64_t val = opt.getValue<uint64_t>();
  auto& numbers = opts.numbers;

  if (!numbers.empty())
    start = numbers[0];

  numbers.push_back(start + val);
}

/// e.g. "--thread=4" -> return "--thread"
string getOption(const string& str)
{
  size_t pos = str.find_first_of("=0123456789");

  if (pos == string::npos)
    return str;
  else
    return str.substr(0, pos);
}

/// e.g. "--thread=4" -> return "4"
string getValue(const string& str)
{
  size_t pos = str.find_first_of("0123456789");

  if (pos == string::npos)
    return string();
  else
    return str.substr(pos);
}

/// e.g. "--threads=8"
/// -> opt.opt = "--threads"
/// -> opt.val = "8"
///
Option makeOption(const string& str)
{
  Option opt;

  opt.str = str;
  opt.opt = getOption(str);
  opt.val = getValue(str);

  if (opt.opt.empty() && !opt.val.empty())
    opt.opt = "--number";

  if (!optionMap.count(opt.opt))
    throw primesieve_error("unknown option " + str);

  return opt;
}

void optionCpuInfo()
{
  const CpuInfo cpu;

  if (cpu.hasCpuName())
    cout << cpu.cpuName() << endl;
  else
    cout << "CPU name: unknown" << endl;

  if (cpu.hasCpuCores())
    cout << "Number of cores: " << cpu.cpuCores() << endl;
  else
    cout << "Number of cores: unknown" << endl;

  if (cpu.hasCpuThreads())
    cout << "Number of threads: " << cpu.cpuThreads() << endl;
  else
    cout << "Number of threads: unknown" << endl;

  if (cpu.hasThreadsPerCore())
    cout << "Threads per core: " << cpu.threadsPerCore() << endl;
  else
    cout << "Threads per core: unknown" << endl;

  if (cpu.hasL1Cache())
    cout << "L1 cache size: " << cpu.l1CacheSize() / (1 << 10) << " KiB" << endl;

  if (cpu.hasL2Cache())
    cout << "L2 cache size: " << cpu.l2CacheSize() / (1 << 10) << " KiB" << endl;

  if (cpu.hasL3Cache())
    cout << "L3 cache size: " << cpu.l3CacheSize() / (double) (1 << 20) << " MiB" << endl;

  if (cpu.hasL1Cache())
  {
    if (!cpu.hasL1Sharing())
      cout << "L1 cache sharing: unknown" << endl;
    else
      cout << "L1 cache sharing: " << cpu.l1Sharing()
           << ((cpu.l1Sharing() > 1) ? " threads" : " thread") << endl;
  }

  if (cpu.hasL2Cache())
  {
    if (!cpu.hasL2Sharing())
      cout << "L2 cache sharing: unknown" << endl;
    else
      cout << "L2 cache sharing: " << cpu.l2Sharing()
           << ((cpu.l2Sharing() > 1) ? " threads" : " thread") << endl;
  }

  if (cpu.hasL3Cache())
  {
    if (!cpu.hasL3Sharing())
      cout << "L3 cache sharing: unknown" << endl;
    else
      cout << "L3 cache sharing: " << cpu.l3Sharing()
           << ((cpu.l3Sharing() > 1) ? " threads" : " thread") << endl;
  }

  if (!cpu.hasL1Cache() &&
      !cpu.hasL2Cache() &&
      !cpu.hasL3Cache())
  {
    cout << "L1 cache size: unknown" << endl;
    cout << "L2 cache size: unknown" << endl;
    cout << "L3 cache size: unknown" << endl;
    cout << "L1 cache sharing: unknown" << endl;
    cout << "L2 cache sharing: unknown" << endl;
    cout << "L3 cache sharing: unknown" << endl;
  }

  exit(0);
}

} // namespace

CmdOptions parseOptions(int argc, char* argv[])
{
  CmdOptions opts;

  for (int i = 1; i < argc; i++)
  {
    Option opt = makeOption(argv[i]);

    switch (optionMap[opt.opt])
    {
      case OPTION_COUNT:     optionCount(opt, opts); break;
      case OPTION_CPU_INFO:  optionCpuInfo(); break;
      case OPTION_DISTANCE:  optionDistance(opt, opts); break;
      case OPTION_PRINT:     optionPrint(opt, opts); break;
      case OPTION_SIZE:      opts.sieveSize = opt.getValue<int>(); break;
      case OPTION_THREADS:   opts.threads = opt.getValue<int>(); break;
      case OPTION_QUIET:     opts.quiet = true; break;
      case OPTION_NTH_PRIME: opts.nthPrime = true; break;
      case OPTION_NO_STATUS: opts.status = false; break;
      case OPTION_TIME:      opts.time = true; break;
      case OPTION_NUMBER:    opts.numbers.push_back(opt.getValue<uint64_t>()); break;
      case OPTION_HELP:      help(); break;
      case OPTION_TEST:      test(); break;
      case OPTION_VERSION:   version(); break;
    }
  }

  if (opts.numbers.empty())
    throw primesieve_error("missing STOP number");

  if (opts.quiet)
    opts.status = false;
  else
    opts.time = true;

  return opts;
}
