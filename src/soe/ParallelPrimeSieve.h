///
/// @file   ParallelPrimeSieve.h
/// @brief  The ParallelPrimeSieve class provides an easy API for
///         multi-threaded prime sieving.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PARALLELPRIMESIEVE_H
#define PARALLELPRIMESIEVE_H

#include "PrimeSieve.h"
#include <stdint.h>

/// ParallelPrimeSieve sieves primes in parallel, it is
/// derived from PrimeSieve so it has the same API.
/// Please refer to doc/EXAMPLES for more information.
///
class ParallelPrimeSieve : public PrimeSieve {
public:
  /// Used for inter-process communication with the
  /// primesieve GUI application.
  struct SharedMemory {
    uint64_t start;
    uint64_t stop;
    uint64_t counts[7];
    double status;
    double seconds;
    int flags;
    int sieveSize;
    int threads;
  };
  ParallelPrimeSieve();
  virtual ~ParallelPrimeSieve() { }
  void init(SharedMemory&);
  static int getMaxThreads();
  int getNumThreads() const;
  void setNumThreads(int numThreads);
  using PrimeSieve::sieve;
  virtual void sieve();
private:
  enum {
    DEFAULT_NUM_THREADS = -1
  };
  /// Used to synchronize threads
  void* lock_;
  /// Number of threads for sieving
  int numThreads_;
  SharedMemory* shm_;
  bool tooMany(int) const;
  int idealNumThreads() const;
  uint64_t getThreadInterval(int) const;
  uint64_t align(uint64_t) const;
  template <typename T>
  T getLock() {
    return static_cast<T>(lock_);
  }
  virtual void setLock();
  virtual void unsetLock();
  virtual bool updateStatus(uint64_t, bool);
};

#endif
