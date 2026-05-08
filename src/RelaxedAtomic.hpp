///
/// @file  RelaxedAtomic.hpp
///        An atomic variable using the std::memory_order_relaxed
///        memory order, that prevents CPU false sharing by ensuring
///        it is stored on its own separate cache line.
///
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef RELAXEDATOMIC_HPP
#define RELAXEDATOMIC_HPP

#include <primesieve/macros.hpp>
#include <primesieve/config.hpp>

#include <atomic>

namespace {

template <typename T>
class RelaxedAtomic
{
public:
  RelaxedAtomic(T n) : atomic_(n) { }
  // Postfix Increment
  T operator++(int)
  {
    return atomic_.fetch_add(1, std::memory_order_relaxed);
  }
private:
  // Use padding to avoid CPU false sharing
  MAYBE_UNUSED char pad1[config::MAX_CACHE_LINE_SIZE];
  std::atomic<T> atomic_;
  MAYBE_UNUSED char pad2[config::MAX_CACHE_LINE_SIZE];
};

} // namespace

#endif
