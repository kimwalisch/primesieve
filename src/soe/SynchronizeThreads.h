///
/// @file  SynchronizeThreads.h
///
/// Copyright (C) 2012 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is licensed under the New BSD License. See the LICENSE
/// file in the top-level directory.
///

#ifndef SYNCHRONIZETHREADS_H
#define SYNCHRONIZETHREADS_H

#include "config.h"
#include "PrimeSieve.h"

namespace soe {

/// Block the current PrimeSieve thread until it
/// can set a lock, then continue execution.
///
class SynchronizeThreads {
public:
  SynchronizeThreads(PrimeSieve& ps) : ps_(ps) { ps_.setLock(); }
  ~SynchronizeThreads()                        { ps_.unsetLock(); }
private:
  PrimeSieve& ps_;
  DISALLOW_COPY_AND_ASSIGN(SynchronizeThreads);
};

} // namespace soe

#endif
