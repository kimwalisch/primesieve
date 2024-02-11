///
/// @file  cmdoptions.hpp
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CMDOPTIONS_HPP
#define CMDOPTIONS_HPP

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
  OPTION_VERSION
};

struct CmdOptions
{
  primesieve::Vector<uint64_t> numbers;
  std::string stressTestMode;
  int option = -1;
  int flags = 0;
  int sieveSize = 0;
  int threads = 0;
  bool quiet = false;
  bool status = true;
  bool time = false;
};

CmdOptions parseOptions(int, char**);

#endif
