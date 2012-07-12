//
// Copyright (c) 2012 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of primesieve.
// Homepage: http://primesieve.googlecode.com
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials provided
//     with the distribution.
//   * Neither the name of the author nor the names of its
//     contributors may be used to endorse or promote products derived
//     from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PARALLELPRIMESIEVE_H
#define PARALLELPRIMESIEVE_H

#include "PrimeSieve.h"
#include <stdint.h>

#ifdef _OPENMP
  #include <omp.h>
#endif

/// ParallelPrimeSieve uses multiple PrimeSieve objects and OpenMP to
/// sieve primes in parallel. By default it counts primes using
/// multiple threads but generates primes in arithmetic order using a
/// single thread. The README file describes the algorithms used in
/// more detail and doc/USAGE_EXAMPLES contains source code examples.
///
class ParallelPrimeSieve : public PrimeSieve {
public:
  /// Inter-process communication with the primesieve GUI application
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
  /// Get the number of logical CPU cores
  static int getMaxThreads();
  int getNumThreads() const;
  void setNumThreads(int);
private:
  enum { IDEAL_NUM_THREADS = -1 };
  SharedMemory* shm_;
  /// Number of threads for sieving
  int numThreads_;
  int idealNumThreads() const;
  uint64_t getInterval() const;
  uint64_t getThreadInterval(int) const;
#ifdef _OPENMP
public:
  using PrimeSieve::sieve;
  /// Sieve primes in parallel using OpenMP
  virtual void sieve();
private:
  /// OpenMP lock initialization and destroy
  class OmpLockGuard {
  public:
    OmpLockGuard(omp_lock_t* lock) : lock_(*lock) { omp_init_lock(&lock_); }
    ~OmpLockGuard()                               { omp_destroy_lock(&lock_); }
  private:
    omp_lock_t& lock_;
    OmpLockGuard(const OmpLockGuard&);
    OmpLockGuard& operator=(const OmpLockGuard&);
  };
  omp_lock_t lock_;
  virtual void set_lock();
  virtual void unset_lock();
  virtual void updateStatus(int);
#endif
};

#endif
