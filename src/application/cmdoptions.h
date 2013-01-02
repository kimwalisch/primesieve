///
/// @file  cmdoptions.h
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the New BSD License. See the
/// LICENSE file in the top level directory.
///

#ifndef PRIMESIEVE_CMDOPTIONS_H
#define PRIMESIEVE_CMDOPTIONS_H

#include <list>
#include <stdint.h>

struct PrimeSieveSettings
{
  std::list<uint64_t> numbers;
  bool quiet;
  int flags;
  int preSieve;
  int sieveSize;
  int threads;
  PrimeSieveSettings() :
    quiet(false),
    flags(0),
    preSieve(0),
    sieveSize(0),
    threads(0)
  { }
  uint64_t start() const
  {
    return numbers.front();
  }
  uint64_t stop() const
  {
    return numbers.back();
  }
};

PrimeSieveSettings processOptions(int, char**);

#endif
