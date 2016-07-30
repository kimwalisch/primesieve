///
/// @file   ParallelPrimeSieve.hpp
/// @brief  The ParallelPrimeSieve class provides an easy API for
///         multi-threaded prime sieving.
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PARALLELPRIMESIEVE_HPP
#define PARALLELPRIMESIEVE_HPP

#include "PrimeSieve.hpp"
#include <stdint.h>

namespace primesieve {

/// ParallelPrimeSieve sieves primes in parallel using OpenMP. It
/// is derived from PrimeSieve so it has the same API.
///
class ParallelPrimeSieve : public PrimeSieve
{
public:
  /// Used for inter-process communication with the
  /// primesieve GUI application.
  struct SharedMemory
  {
    uint64_t start;
    uint64_t stop;
    uint64_t counts[6];
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
  int idealNumThreads() const;
  void setNumThreads(int numThreads);
  using PrimeSieve::sieve;
  virtual void sieve();
private:
  void* lock_;
  SharedMemory* shm_;
  int numThreads_;
  uint64_t getThreadDistance(int) const;
  uint64_t align(uint64_t) const;
  template <typename T> T getLock() { return static_cast<T>(lock_); }
  virtual double getWallTime() const;
  virtual void setLock();
  virtual void unsetLock();
  virtual bool updateStatus(uint64_t, bool);
};

} // namespace

#endif
