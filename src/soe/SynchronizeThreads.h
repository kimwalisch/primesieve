#ifndef SYNCHRONIZETHREADS_H
#define SYNCHRONIZETHREADS_H

#include "PrimeSieve.h"
#include "config.h"

namespace soe {

/// Block the current "PrimeSieve" thread until
/// it can set a lock, then continue execution.
/// When done release the lock using RAII.
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
