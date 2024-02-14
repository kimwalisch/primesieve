///
/// @file  CmdOptions.hpp
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CMDOPTIONS_HPP
#define CMDOPTIONS_HPP

#include <primesieve/primesieve_error.hpp>
#include <primesieve/Vector.hpp>
#include <stdint.h>
#include <string>

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
  OPTION_R,
  OPTION_R_INVERSE,
  OPTION_SIZE,
  OPTION_STRESS_TEST,
  OPTION_TEST,
  OPTION_THREADS,
  OPTION_TIME,
  OPTION_TIMEOUT,
  OPTION_VERSION
};

struct CmdOptions
{
  primesieve::Vector<uint64_t> numbers;
  std::string stressTestMode;
  std::string optionStr;
  int option = -1;
  int flags = 0;
  int sieveSize = 0;
  int threads = 0;
  // Stress test timeout in seconds.
  // The default timeout is 24 hours (same as stress-ng).
  int64_t timeout = 24 * 3600;
  bool quiet = false;
  bool status = true;
  bool time = false;

  void setMainOption(OptionID optionID,
                     const std::string& optStr)
  {
    // Multiple main options are not allowed
    if (!optionStr.empty())
      throw primesieve::primesieve_error("incompatible options: " + optionStr + " " + optStr);
    else
    {
      optionStr = optStr;
      option = optionID;
    }
  }
};

CmdOptions parseOptions(int, char**);

#endif
