///
/// @file   cmdoptions.cpp
/// @brief  Parse command-line options for the primesieve console
///         (terminal) application.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
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
#include <cstdlib>
#include <cstddef>
#include <stdint.h>

void help();
void version();

using namespace std;
using primesieve::PrimeSieve;

namespace {

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
  OPTION_NUMBER,
  OPTION_OFFSET,
  OPTION_PRINT,
  OPTION_QUIET,
  OPTION_SIZE,
  OPTION_TEST,
  OPTION_THREADS,
  OPTION_VERSION
};

/// Command-line options
map<string, OptionValues> optionMap;

void initOptionMap()
{
  optionMap["-c"]         = OPTION_COUNT;
  optionMap["--count"]    = OPTION_COUNT;
  optionMap["-h"]         = OPTION_HELP;
  optionMap["--help"]     = OPTION_HELP;
  optionMap["-n"]         = OPTION_NTHPRIME;
  optionMap["--nthprime"] = OPTION_NTHPRIME;
  optionMap["--number"]   = OPTION_NUMBER;
  optionMap["-o"]         = OPTION_OFFSET;
  optionMap["--offset"]   = OPTION_OFFSET;
  optionMap["-p"]         = OPTION_PRINT;
  optionMap["--print"]    = OPTION_PRINT;
  optionMap["-q"]         = OPTION_QUIET;
  optionMap["--quiet"]    = OPTION_QUIET;
  optionMap["-s"]         = OPTION_SIZE;
  optionMap["--size"]     = OPTION_SIZE;
  optionMap["--test"]     = OPTION_TEST;
  optionMap["-t"]         = OPTION_THREADS;
  optionMap["--threads"]  = OPTION_THREADS;
  optionMap["-v"]         = OPTION_VERSION;
  optionMap["--version"]  = OPTION_VERSION;
}

void test()
{
  bool ok = primesieve::test();
  exit(ok ? 0 : 1);
}

int check(int primeType)
{
  primeType--;
  if (primeType < 0 || primeType > 6)
    help();
  return primeType;
}

int getCountFlags(int val)
{
  int flags = 0;
  do {
    flags |= PrimeSieve::COUNT_PRIMES << check(val % 10);
    val /= 10;
  } while (val > 0);
  return flags;
}

int getPrintFlags(int val)
{
  return PrimeSieve::PRINT_PRIMES << check(val);
}

/// e.g. "--threads=8" -> { id = "--threads", value = "8" }
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

} // end namespace

PrimeSieveOptions parseOptions(int argc, char** argv)
{
  PrimeSieveOptions pso;
  initOptionMap();
  try
  {
    for (int i = 1; i < argc; i++)
    {
      Option option = makeOption(argv[i]);
      switch (optionMap[option.id])
      {
        case OPTION_COUNT:    if (option.value.empty())
                                option.value = "1";
                              pso.flags |= getCountFlags(option.getValue<int>());
                              break;
        case OPTION_PRINT:    if (option.value.empty())
                                option.value = "1";
                              pso.flags |= getPrintFlags(option.getValue<int>());
                              pso.quiet = true;
                              break;
        case OPTION_SIZE:     pso.sieveSize = option.getValue<int>(); break;
        case OPTION_THREADS:  pso.threads = option.getValue<int>(); break;
        case OPTION_QUIET:    pso.quiet = true; break;
        case OPTION_NTHPRIME: pso.nthPrime = true; break;
        case OPTION_NUMBER:   pso.n.push_back(option.getValue<uint64_t>()); break;
        case OPTION_OFFSET:   pso.n.push_back(option.getValue<uint64_t>() + pso.n.front()); break;
        case OPTION_TEST:     test(); break;
        case OPTION_VERSION:  version(); break;
        case OPTION_HELP:     help(); break;
      }
    }
  }
  catch (exception&)
  {
    help();
  }
  if (pso.n.size() < 1 || pso.n.size() > 2)
    help();
  return pso;
}
