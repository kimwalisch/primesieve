///
/// @file  PrimeSieve-lock.h
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMESIEVE_LOCK_H
#define PRIMESIEVE_LOCK_H

#include "config.h"
#include "PrimeSieve.h"

namespace soe {

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

} // namespace soe

#endif
