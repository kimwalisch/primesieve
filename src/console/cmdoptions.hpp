///
/// @file  cmdoptions.hpp
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CMDOPTIONS_HPP
#define CMDOPTIONS_HPP

#include <stdint.h>
#include <deque>

struct CmdOptions
{
  std::deque<uint64_t> numbers;
  int flags;
  int sieveSize;
  int threads;
  bool quiet;
  bool nthPrime;
  bool status;
  bool time;

  CmdOptions() :
    flags(0),
    sieveSize(0),
    threads(0),
    quiet(false),
    nthPrime(false),
    status(true),
    time(false)
  { }
};

CmdOptions parseOptions(int, char**);

#endif
