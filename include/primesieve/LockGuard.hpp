///
/// @file   LockGuard.hpp
/// @brief  LockGuard is RAII-style wrapper for std::mutex
///         which supports lock() and try_lock().
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef LOCKGUARD_HPP
#define LOCKGUARD_HPP

#include <mutex>

namespace primesieve {

class LockGuard
{
public:
  LockGuard(std::mutex& lock, bool wait);
  ~LockGuard();
  bool isSet() const;
private:
  std::mutex& lock_;
  bool isSet_;
};

/// If wait = false do not block the current
/// thread if the lock is not available.
///
LockGuard::LockGuard(std::mutex& lock, bool wait)
  : lock_(lock)
{
  if (!wait)
    isSet_ = lock_.try_lock();
  else {
    lock_.lock();
    isSet_ = true;
  }
}

LockGuard::~LockGuard()
{
  if (isSet())
    lock_.unlock();
}

bool LockGuard::isSet() const
{
  return isSet_;
}

} // namespace

#endif
