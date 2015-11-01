///
/// @file  cmdoptions.hpp
///
/// Copyright (C) 2015 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CMDOPTIONS_HPP
#define CMDOPTIONS_HPP

#include <deque>
#include <stdint.h>

struct PrimeSieveOptions
{
  std::deque<uint64_t> numbers;
  int flags;
  int sieveSize;
  int threads;
  bool quiet;
  bool nthPrime;
  bool time;

  PrimeSieveOptions() :
    flags(0),
    sieveSize(0),
    threads(0),
    quiet(false),
    nthPrime(false),
    time(false)
  { }
};

PrimeSieveOptions parseOptions(int, char**);

#endif
