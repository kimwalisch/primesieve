///
/// @file   cmdoptions.cpp
/// @brief  Parse command-line options for the primesieve console
///         (terminal) application.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the New BSD License. See the
/// COPYING file in the top level directory.
///

#include "cmdoptions.h"
#include "ExpressionParser.h"
#include "../../soe/PrimeSieve.h"

#include <string>
#include <map>
#include <exception>
#include <cstdlib>
#include <cstddef>
#include <stdint.h>

void help();
void version();
bool test_ParallelPrimeSieve();

using namespace std;

namespace {

/// e.g. id = "--threads", value = "4"
struct Option {
  string id;
  string value;
  template <typename T>
  T getValue() const
  {
    ExpressionParser<T> parser;
    T result = parser.eval(value);
    return result;
  }
};

enum PrimeSieveOptions {
  OPTION_NUMBER,
  OPTION_COUNT,
  OPTION_HELP,
  OPTION_OFFSET,
  OPTION_PRINT,
  OPTION_QUIET,
  OPTION_PRESIEVE,
  OPTION_SIZE,
  OPTION_TEST,
  OPTION_THREADS,
  OPTION_VERSION
};

/// Command-line options
map<string, PrimeSieveOptions> cmdOptions;

void initCmdOptions()
{
  cmdOptions["-n"]         = OPTION_NUMBER;
  cmdOptions["--number"]   = OPTION_NUMBER;
  cmdOptions["-c"]         = OPTION_COUNT;
  cmdOptions["--count"]    = OPTION_COUNT;
  cmdOptions["-h"]         = OPTION_HELP;
  cmdOptions["--help"]     = OPTION_HELP;
  cmdOptions["-o"]         = OPTION_OFFSET;
  cmdOptions["--offset"]   = OPTION_OFFSET;
  cmdOptions["-p"]         = OPTION_PRINT;
  cmdOptions["--print"]    = OPTION_PRINT;
  cmdOptions["-q"]         = OPTION_QUIET;
  cmdOptions["--quiet"]    = OPTION_QUIET;
  cmdOptions["-r"]         = OPTION_PRESIEVE;
  cmdOptions["--presieve"] = OPTION_PRESIEVE;
  cmdOptions["-s"]         = OPTION_SIZE;
  cmdOptions["--size"]     = OPTION_SIZE;
  cmdOptions["--test"]     = OPTION_TEST;
  cmdOptions["-t"]         = OPTION_THREADS;
  cmdOptions["--threads"]  = OPTION_THREADS;
  cmdOptions["-v"]         = OPTION_VERSION;
  cmdOptions["--version"]  = OPTION_VERSION;
}

void test()
{
  bool ok = test_ParallelPrimeSieve();
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
  if (delimiter == string::npos) {
    option.id = str;
  } else {
    option.id = str.substr(0, delimiter);
    option.value = str.substr(delimiter + (str.at(delimiter) == '=' ? 1 : 0));
  }
  if (option.id.empty() && !option.value.empty())
    option.id = "--number";
  if (cmdOptions.count(option.id) == 0)
    option.id = "--help";
  return option;
}

} // end namespace

PrimeSieveSettings processOptions(int argc, char** argv)
{
  // skip program name in argv[0]
  argc--; argv++;
  PrimeSieveSettings pss;
  initCmdOptions();
  try {
    for (int i = 0; i < argc; i++) {
      Option option = makeOption(argv[i]);

      switch (cmdOptions[option.id]) {
        case OPTION_COUNT:    pss.flags |= getCountFlags(option.getValue<int>()); break;
        case OPTION_PRINT:    pss.flags |= getPrintFlags(option.getValue<int>()); pss.quiet = true; break;
        case OPTION_PRESIEVE: pss.preSieve  = option.getValue<int>(); break;
        case OPTION_SIZE:     pss.sieveSize = option.getValue<int>(); break;
        case OPTION_THREADS:  pss.threads   = option.getValue<int>(); break;
        case OPTION_QUIET:    pss.quiet = true; break;
        case OPTION_NUMBER:   pss.numbers.push_back(option.getValue<uint64_t>()); break;
        case OPTION_OFFSET:   pss.numbers.push_back(option.getValue<uint64_t>() + pss.start()); break;
        case OPTION_TEST:     test();    break;
        case OPTION_VERSION:  version(); break;
        case OPTION_HELP:     help();    break;
      }
    }
  } catch (exception&) {
    help();
  }
  if (pss.numbers.size() == 1) pss.numbers.push_front(0);
  if (pss.numbers.size() != 2) help();
  return pss;
}
