///
/// @file   ParallelSieve.hpp
/// @brief  The ParallelSieve class provides an easy API for
///         multi-threaded prime sieving.
///
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PARALLELSIEVE_HPP
#define PARALLELSIEVE_HPP

#include "PrimeSieve.hpp"
#include <stdint.h>
#include <mutex>

namespace primesieve {

class ParallelSieve : public PrimeSieve
{
public:
  using PrimeSieve::sieve;
  using PrimeSieve::updateStatus;

  /// Used for inter-process communication with the
  /// primesieve GUI application.
  struct SharedMemory
  {
    uint64_t start;
    uint64_t stop;
    uint64_t counts[6];
    double percent;
    double seconds;
    int flags;
    int sieveSize;
    int threads;
  };

  ParallelSieve();
  void init(SharedMemory&);
  static int getMaxThreads();
  int getNumThreads() const;
  int idealNumThreads() const;
  void setNumThreads(int numThreads);
  virtual void sieve();

private:
  std::mutex lock_;
  SharedMemory* shm_ = nullptr;
  int numThreads_ = 0;
  uint64_t getThreadDistance(int) const;
  uint64_t align(uint64_t) const;
  void updateStatus(const PrimeSieve& ps);
};

} // namespace

#endif
