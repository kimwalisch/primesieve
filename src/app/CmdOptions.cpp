///
/// @file   CmdOptions.cpp
/// @brief  Command-line option handling for the primesieve
///         command-line application. The user's command-line options
///         are first parsed in CmdOptions.cpp and stored in a
///         CmdOptions object. Afterwards we execute the function
///         corresponding to the user's command-line options in the
///         main() function in main.cpp.
///
///         How to add a new command-line option:
///
///         1) Add a new option enum in CmdOptions.h.
///         2) Add your option to parseOptions() in CmdOptions.cpp.
///         3) Add your option to main() in main.cpp.
///         4) Document your option in help.cpp (--help option summary)
///            and in doc/primesieve.txt (manpage).
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "CmdOptions.hpp"

#include <primesieve/calculator.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/primesieve_error.hpp>

#include <cstddef>
#include <cctype>
#include <map>
#include <stdint.h>
#include <string>
#include <utility>

void help(int exitCode);

using std::size_t;
using namespace primesieve;

namespace {

/// Some command-line options require an additional parameter.
/// Examples: --threads THREADS, -a ALPHA, ...
enum IsParam
{
  NO_PARAM,
  REQUIRED_PARAM,
  OPTIONAL_PARAM
};

/// Command-line option
struct Option
{
  // Example:
  // str = "--threads=32"
  // opt = "--threads"
  // val = "32"
  std::string str;
  std::string opt;
  std::string val;

  template <typename T>
  T getValue() const
  {
    try {
      return calculator::eval<T>(val);
    }
    catch (std::exception&) {
      throw primesieve_error("invalid option '" + opt + "=" + val + "'");
    }
  }
};

/// Options start with "-" or "--", then
/// follows a Latin ASCII character.
///
bool isOption(const std::string& str)
{
  // Option of type: -o...
  if (str.size() >= 2 &&
      str[0] == '-' &&
      ((str[1] >= 'a' && str[1] <= 'z') ||
       (str[1] >= 'A' && str[1] <= 'Z')))
    return true;

  // Option of type: --o...
  if (str.size() >= 3 &&
      str[0] == '-' &&
      str[1] == '-' &&
      ((str[2] >= 'a' && str[2] <= 'z') ||
       (str[2] >= 'A' && str[2] <= 'Z')))
    return true;

  return false;
}

/// Parse the next command-line option.
/// e.g. "--threads=32"
/// -> opt.str = "--threads=32"
/// -> opt.opt = "--threads"
/// -> opt.val = "8"
///
template <typename T>
Option parseOption(int argc,
                   char* argv[],
                   int& i,
                   const T& optionMap)
{
  Option opt;
  opt.str = argv[i];

  if (opt.str.empty())
    throw primesieve_error("unrecognized option ''");

  // Check if the option has the format:
  // --opt or -o (but not --opt=N)
  if (optionMap.count(opt.str))
  {
    opt.opt = opt.str;
    IsParam isParam = optionMap.at(opt.str).second;

    if (isParam == REQUIRED_PARAM)
    {
      i += 1;

      if (i < argc)
        opt.val = argv[i];

      // Prevent --threads --other-option
      if (opt.val.empty() || isOption(opt.val))
        throw primesieve_error("missing value for option '" + opt.opt + "'");
    }

    // If the option takes an optional argument we
    // assume the next value is an optional argument
    // if the value is not a vaild option.
    if (isParam == OPTIONAL_PARAM &&
        i + 1 < argc &&
        !std::string(argv[i + 1]).empty() &&
        !isOption(argv[i + 1]))
    {
      i += 1;
      opt.val = argv[i];
    }
  }
  else
  {
    // Here the option is either:
    // 1) An option of type: --opt[=N]
    // 2) An option of type: --opt[N]
    // 3) A number (e.g. the start number)

    if (isOption(opt.str))
    {
      size_t pos = opt.str.find('=');

      // Option of type: --opt=N
      if (pos != std::string::npos)
      {
        opt.opt = opt.str.substr(0, pos);
        opt.val = opt.str.substr(pos + 1);

        // Print partial option: --opt (without =N)
        if (!optionMap.count(opt.opt))
          throw primesieve_error("unrecognized option '" + opt.opt + "'");
      }
      else
      {
        // Option of type: --opt[N]
        pos = opt.str.find_first_of("0123456789");

        if (pos == std::string::npos)
          opt.opt = opt.str;
        else
        {
          opt.opt = opt.str.substr(0, pos);
          opt.val = opt.str.substr(pos);
        }

        // Print full option e.g.: --opt123
        if (!optionMap.count(opt.opt))
          throw primesieve_error("unrecognized option '" + opt.str + "'");
      }

      // Prevent '--option='
      if (opt.val.empty() &&
          optionMap.at(opt.opt).second == REQUIRED_PARAM)
        throw primesieve_error("missing value for option '" + opt.opt + "'");
    }
    else
    {
      // Here the option is actually a number or
      // an integer arithmetic expression.
      opt.opt = "--number";
      opt.val = opt.str;

      // This is not a valid number
      if (opt.str.find_first_of("0123456789") == std::string::npos)
        throw primesieve_error("unrecognized option '" + opt.str + "'");

      // Prevent negative numbers as there are
      // no negative prime numbers.
      if (opt.str.at(0) == '-')
        throw primesieve_error("unrecognized option '" + opt.str + "'");
    }
  }

  return opt;
}

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
    default: throw primesieve_error("invalid option '" + opt.str + "'");
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
      default: throw primesieve_error("invalid option '" + opt.str + "'");
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

void optionStressTest(Option& opt,
                      CmdOptions& opts)
{
  opts.option = OPTION_STRESS_TEST;
  std::transform(opt.val.begin(), opt.val.end(), opt.val.begin(),
                 [](unsigned char c){ return std::toupper(c); });

  // If the stress test mode is not specified
  // we use "CPU" by default.
  if (opt.val == "RAM")
    opts.stressTestMode = "RAM";
  else
    opts.stressTestMode = "CPU";
}

} // namespace

CmdOptions parseOptions(int argc, char* argv[])
{
  // No command-line options provided
  if (argc <= 1)
    help(/* exitCode */ 1);

  /// primesieve command-line options
  const std::map<std::string, std::pair<OptionID, IsParam>> optionMap =
  {
    { "-c",            std::make_pair(OPTION_COUNT, OPTIONAL_PARAM) },
    { "--count",       std::make_pair(OPTION_COUNT, OPTIONAL_PARAM) },
    { "--cpu-info",    std::make_pair(OPTION_CPU_INFO, NO_PARAM) },
    { "-h",            std::make_pair(OPTION_HELP, NO_PARAM) },
    { "--help",        std::make_pair(OPTION_HELP, NO_PARAM) },
    { "-n",            std::make_pair(OPTION_NTH_PRIME, NO_PARAM) },
    { "--nthprime",    std::make_pair(OPTION_NTH_PRIME, NO_PARAM) },
    { "--nth-prime",   std::make_pair(OPTION_NTH_PRIME, NO_PARAM) },
    { "--no-status",   std::make_pair(OPTION_NO_STATUS, NO_PARAM) },
    { "--number",      std::make_pair(OPTION_NUMBER, REQUIRED_PARAM) },
    { "-d",            std::make_pair(OPTION_DISTANCE, REQUIRED_PARAM) },
    { "--dist",        std::make_pair(OPTION_DISTANCE, REQUIRED_PARAM) },
    { "-p",            std::make_pair(OPTION_PRINT, OPTIONAL_PARAM) },
    { "--print",       std::make_pair(OPTION_PRINT, OPTIONAL_PARAM) },
    { "-q",            std::make_pair(OPTION_QUIET, NO_PARAM) },
    { "--quiet",       std::make_pair(OPTION_QUIET, NO_PARAM) },
    { "-R",            std::make_pair(OPTION_R, NO_PARAM) },
    { "--RiemannR",    std::make_pair(OPTION_R, NO_PARAM) },
    { "--R-inverse",   std::make_pair(OPTION_R_INVERSE, NO_PARAM) },
    { "-s",            std::make_pair(OPTION_SIZE, REQUIRED_PARAM) },
    { "--size",        std::make_pair(OPTION_SIZE, REQUIRED_PARAM) },
    { "--stress-test", std::make_pair(OPTION_STRESS_TEST, OPTIONAL_PARAM) },
    { "--test",        std::make_pair(OPTION_TEST, NO_PARAM) },
    { "-t",            std::make_pair(OPTION_THREADS, REQUIRED_PARAM) },
    { "--threads",     std::make_pair(OPTION_THREADS, REQUIRED_PARAM) },
    { "--time",        std::make_pair(OPTION_TIME, NO_PARAM) },
    { "-v",            std::make_pair(OPTION_VERSION, NO_PARAM) },
    { "--version",     std::make_pair(OPTION_VERSION, NO_PARAM) }
  };

  CmdOptions opts;

  for (int i = 1; i < argc; i++)
  {
    Option opt = parseOption(argc, argv, i, optionMap);
    OptionID optionID = optionMap.at(opt.opt).first;

    switch (optionID)
    {
      case OPTION_COUNT:       optionCount(opt, opts); break;
      case OPTION_DISTANCE:    optionDistance(opt, opts); break;
      case OPTION_PRINT:       optionPrint(opt, opts); break;
      case OPTION_STRESS_TEST: optionStressTest(opt, opts); break;
      case OPTION_SIZE:        opts.sieveSize = opt.getValue<int>(); break;
      case OPTION_THREADS:     opts.threads = opt.getValue<int>(); break;
      case OPTION_QUIET:       opts.quiet = true; break;
      case OPTION_NO_STATUS:   opts.status = false; break;
      case OPTION_TIME:        opts.time = true; break;
      case OPTION_NUMBER:      opts.numbers.push_back(opt.getValue<uint64_t>()); break;
      default:                 opts.option = optionID;
    }
  }

  if (opts.quiet)
    opts.status = false;
  if (!opts.quiet)
    opts.time = true;

  return opts;
}
