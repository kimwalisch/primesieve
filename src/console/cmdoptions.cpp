///
/// @file   cmdoptions.cpp
/// @brief  Parse command-line options for the primesieve console
///         (terminal) application.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve.hpp>
#include "cmdoptions.hpp"
#include "calculator.hpp"

#include <string>
#include <map>
#include <cstddef>
#include <stdint.h>

void help();
void version();

using namespace std;
using namespace primesieve;

namespace {

/// Command-line option
/// e.g. str = "--threads", value = "4"
struct Option
{
  string str;
  string value;
  template <typename T>
  T getValue() const
  {
    if (value.empty())
      throw primesieve_error("missing value for option " + str);
    return calculator::eval<T>(value);
  }
};

enum OptionID
{
  OPTION_COUNT,
  OPTION_HELP,
  OPTION_NTHPRIME,
  OPTION_NO_STATUS,
  OPTION_NUMBER,
  OPTION_DISTANCE,
  OPTION_PRINT,
  OPTION_QUIET,
  OPTION_SIZE,
  OPTION_THREADS,
  OPTION_TIME,
  OPTION_VERSION
};

/// Command-line options
map<string, OptionID> optionMap =
{
  { "-c",          OPTION_COUNT },
  { "--count",     OPTION_COUNT },
  { "-h",          OPTION_HELP },
  { "--help",      OPTION_HELP },
  { "-n",          OPTION_NTHPRIME },
  { "--nthprime",  OPTION_NTHPRIME },
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
  { "-t",          OPTION_THREADS },
  { "--threads",   OPTION_THREADS },
  { "--time",      OPTION_TIME },
  { "-v",          OPTION_VERSION },
  { "--version",   OPTION_VERSION }
};

int check(int primeType)
{
  primeType--;

  if (primeType < 0 ||
      primeType > 5)
    help();

  return primeType;
}

void optionPrint(Option& opt,
                 CmdOptions& opts)
{
  opts.quiet = true;

  if (opt.value.empty())
    opt.value = "1";

  int i = opt.getValue<int>();
  i = check(i);
  opts.flags |= PrimeSieve::PRINT_PRIMES << i;
}

void optionCount(Option& opt,
                 CmdOptions& opts)
{
  if (opt.value.empty())
    opt.value = "1";

  int n = opt.getValue<int>();

  for (; n > 0; n /= 10)
  {
    int i = check(n % 10);
    opts.flags |= PrimeSieve::COUNT_PRIMES << i;
  }
}

/// e.g. "--threads=8"
/// -> opt.str = "--threads"
/// -> opt.value = "8"
///
Option makeOption(const string& str)
{
  Option opt;
  size_t delimiter = str.find_first_of("=0123456789");

  if (delimiter == string::npos)
    opt.str = str;
  else
  {
    opt.str = str.substr(0, delimiter);
    opt.value = str.substr(delimiter + (str.at(delimiter) == '=' ? 1 : 0));
  }

  if (opt.str.empty() && !opt.value.empty())
    opt.str = "--number";

  if (!optionMap.count(opt.str))
    throw primesieve_error("unknown option " + opt.str);

  return opt;
}

} // namespace

CmdOptions parseOptions(int argc, char* argv[])
{
  CmdOptions opts;

  for (int i = 1; i < argc; i++)
  {
    Option opt = makeOption(argv[i]);

    switch (optionMap[opt.str])
    {
      case OPTION_COUNT:     optionCount(opt, opts); break;
      case OPTION_PRINT:     optionPrint(opt, opts); break;
      case OPTION_SIZE:      opts.sieveSize = opt.getValue<int>(); break;
      case OPTION_THREADS:   opts.threads = opt.getValue<int>(); break;
      case OPTION_QUIET:     opts.quiet = true; break;
      case OPTION_NTHPRIME:  opts.nthPrime = true; break;
      case OPTION_NO_STATUS: opts.status = false; break;
      case OPTION_TIME:      opts.time = true; break;
      case OPTION_NUMBER:    opts.numbers.push_back(opt.getValue<uint64_t>()); break;
      case OPTION_DISTANCE:  opts.numbers.push_back(opt.getValue<uint64_t>() + opts.numbers[0]); break;
      case OPTION_VERSION:   version(); break;
      case OPTION_HELP:      help(); break;
    }
  }

  if (opts.numbers.size() < 1 ||
      opts.numbers.size() > 2)
    help();

  if (opts.quiet)
    opts.status = false;
  else
    opts.time = true;

  return opts;
}
