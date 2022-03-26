///
/// @file   IteratorHelper.hpp
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef ITERATOR_HELPER_HPP
#define ITERATOR_HELPER_HPP

#include <cstdint>

namespace primesieve {

class IteratorHelper
{
public:
  static void next(uint64_t* start,
                   uint64_t* stop,
                   uint64_t stopHint,
                   uint64_t* dist);

  static void prev(uint64_t* start,
                   uint64_t* stop,
                   uint64_t stopHint,
                   uint64_t* dist);
};

} // namespace

#endif
