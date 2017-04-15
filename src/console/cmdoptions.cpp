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
#include <exception>
#include <cstddef>
#include <stdint.h>

void help();
void version();

using namespace std;
using namespace primesieve;

namespace {

/// Command-line option
/// e.g. id = "--threads", value = "4"
struct Option
{
  string id;
  string value;
  template <typename T>
  T getValue() const
  {
    return calculator::eval<T>(value);
  }
};

enum OptionValues
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
map<string, OptionValues> optionMap =
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

int getCountFlags(int n)
{
  int flags = 0;

  do {
    int primeType = check(n % 10);
    flags |= PrimeSieve::COUNT_PRIMES << primeType;
    n /= 10;
  } while (n > 0);

  return flags;
}

int getPrintFlags(int n)
{
  n = check(n);
  return PrimeSieve::PRINT_PRIMES << n;
}

/// e.g. "--threads=8" -> (id = "--threads", value = "8")
Option makeOption(const string& str)
{
  Option option;
  size_t delimiter = str.find_first_of("=0123456789");

  if (delimiter == string::npos)
    option.id = str;
  else
  {
    option.id = str.substr(0, delimiter);
    option.value = str.substr(delimiter + (str.at(delimiter) == '=' ? 1 : 0));
  }

  if (option.id.empty() && !option.value.empty())
    option.id = "--number";
  if (optionMap.count(option.id) == 0)
    option.id = "--help";

  return option;
}

} // namespace

CmdOptions parseOptions(int argc, char** argv)
{
  CmdOptions opts;

  for (int i = 1; i < argc; i++)
  {
    Option option = makeOption(argv[i]);

    switch (optionMap[option.id])
    {
      case OPTION_COUNT:     if (option.value.empty())
                               option.value = "1";
                             opts.flags |= getCountFlags(option.getValue<int>());
                             break;
      case OPTION_PRINT:     if (option.value.empty())
                               option.value = "1";
                             opts.flags |= getPrintFlags(option.getValue<int>());
                             opts.quiet = true;
                             break;
      case OPTION_SIZE:      opts.sieveSize = option.getValue<int>(); break;
      case OPTION_THREADS:   opts.threads = option.getValue<int>(); break;
      case OPTION_QUIET:     opts.quiet = true; break;
      case OPTION_NTHPRIME:  opts.nthPrime = true; break;
      case OPTION_NO_STATUS: opts.status = false; break;
      case OPTION_TIME:      opts.time = true; break;
      case OPTION_NUMBER:    opts.numbers.push_back(option.getValue<uint64_t>()); break;
      case OPTION_DISTANCE:  opts.numbers.push_back(option.getValue<uint64_t>() + opts.numbers.front()); break;
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
