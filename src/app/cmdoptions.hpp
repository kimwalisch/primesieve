///
/// @file  cmdoptions.hpp
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CMDOPTIONS_HPP
#define CMDOPTIONS_HPP

#include <cstdint>
#include <deque>

struct CmdOptions
{
  std::deque<uint64_t> numbers;
  int flags = 0;
  int sieveSize = 0;
  int threads = 0;
  bool quiet = false;
  bool nthPrime = false;
  bool status = true;
  bool time = false;
};

CmdOptions parseOptions(int, char**);

#endif
