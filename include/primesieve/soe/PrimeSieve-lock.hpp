///
/// @file  PrimeSieve-lock.hpp
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMESIEVE_LOCK_HPP
#define PRIMESIEVE_LOCK_HPP

#include "config.hpp"
#include "PrimeSieve.hpp"

namespace primesieve {

/// Block the current PrimeSieve (or ParallelPrimeSieve) thread
/// until it can set a lock, then continue execution.
///
class LockGuard {
public:
  LockGuard(PrimeSieve& ps) : ps_(ps) { ps_.setLock(); }
  ~LockGuard() { ps_.unsetLock(); }
private:
  PrimeSieve& ps_;
  DISALLOW_COPY_AND_ASSIGN(LockGuard);
};

} // namespace primesieve

#endif
