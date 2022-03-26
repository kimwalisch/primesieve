///
/// @file   resizeUninitialized.hpp
/// @brief  std::vector.resize() default initializes memory.
///         This is a workaround to avoid default initialization
///         in order to avoid unnecessary overhead.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef RESIZEUNINITIALIZED_HPP
#define RESIZEUNINITIALIZED_HPP

#include <cstdint>
#include <cstddef>
#include <vector>

namespace {

void resizeUninitialized(std::vector<uint64_t>& vect,
                         std::size_t size)
{
  struct NoInitType
  {
    NoInitType() { };
    uint64_t val;
  };

  using noInitVector = std::vector<NoInitType>;
  auto noInitVect = (noInitVector*) &vect;
  noInitVect->resize(size);
}

} // namespace

#endif
