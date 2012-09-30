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

/// @brief  ParallelPrimeSieve sieves primes in parallel
///         using OpenMP 3.0 (2008) or later.

#include "ParallelPrimeSieve.h"
#include "PrimeSieve.h"
#include "imath.h"
#include "config.h"

#include <stdint.h>
#include <cstdlib>
#include <cassert>
#include <algorithm>

#ifdef _OPENMP
  #include <omp.h>
  #include "openmp_RAII.h"
#endif

using namespace soe;

ParallelPrimeSieve::ParallelPrimeSieve() :
  shm_(NULL),
  numThreads_(DEFAULT_NUM_THREADS)
{ }

/// API for the primesieve GUI application
void ParallelPrimeSieve::init(SharedMemory& shm)
{
  setStart(shm.start);
  setStop(shm.stop);
  setSieveSize(shm.sieveSize);
  setFlags(shm.flags);
  setNumThreads(shm.threads);
  shm_ = &shm;
}

int ParallelPrimeSieve::getNumThreads() const
{
  if (numThreads_ == DEFAULT_NUM_THREADS) {
    // 1 thread generates primes in arithmetic order
    if (isGenerate())
       return 1;
    else
       return idealNumThreads();
  }
  return numThreads_;
}

void ParallelPrimeSieve::setNumThreads(int threads)
{
  numThreads_ = getInBetween(1, threads, getMaxThreads());
}

/// Get an ideal number of threads for the current
/// set start_ and stop_ numbers.
///
int ParallelPrimeSieve::idealNumThreads() const
{
  uint64_t threshold = std::max(config::MIN_THREAD_INTERVAL, isqrt(stop_) / 5);
  uint64_t threads = getInterval() / threshold;
  threads = getInBetween<uint64_t>(1, threads, getMaxThreads());
  return static_cast<int>(threads);
}

/// Get an interval size that ensures a good load balance
/// when multiple threads are used.
///
uint64_t ParallelPrimeSieve::getThreadInterval(int threads) const
{
  assert(threads > 0);
  uint64_t unbalanced = getInterval() / threads;
  uint64_t balanced = isqrt(stop_) * 1000;
  uint64_t fastest = std::min(balanced, unbalanced);
  uint64_t threadInterval = getInBetween(config::MIN_THREAD_INTERVAL, fastest, config::MAX_THREAD_INTERVAL);
  uint64_t chunks = getInterval() / threadInterval;
  if (chunks < threads * 5u)
    threadInterval = std::max(config::MIN_THREAD_INTERVAL, unbalanced);
  // align to modulo 30 to prevent prime k-tuplet gaps
  threadInterval += 30 - threadInterval % 30;
  return threadInterval;
}

bool ParallelPrimeSieve::tooMany(int threads) const
{
  return (threads > 1 && getInterval() / threads < config::MIN_THREAD_INTERVAL);
}

#ifdef _OPENMP

int ParallelPrimeSieve::getMaxThreads()
{
  return omp_get_max_threads();
}

/// Sieve the primes and prime k-tuplets within [start, stop]
/// in parallel using OpenMP (version 3.0 or later).
///
void ParallelPrimeSieve::sieve()
{
  if (start_ > stop_)
    throw primesieve_error("start must be <= stop");

  int threads = getNumThreads();
  if (tooMany(threads)) threads = idealNumThreads();
  OmpInitLock initLock(&lock_);

  if (threads == 1)
    PrimeSieve::sieve();
  else {
    double t1 = omp_get_wtime();
    reset();
    uint64_t count0 = 0, count1 = 0, count2 = 0, count3 = 0, count4 = 0, count5 = 0, count6 = 0;
    uint64_t align = start_ + 32 - start_ % 30;
    uint64_t threadInterval = getThreadInterval(threads);
    // The sieve interval [start_, stop_] is subdivided into chunks of
    // size 'threadInterval' that are sieved in parallel using
    // multiple threads. This scales well as each thread sieves using
    // its own private memory without need of synchronization.
    #pragma omp parallel for schedule(dynamic) num_threads(threads) \
        reduction(+: count0, count1, count2, count3, count4, count5, count6)
    for (uint64_t n = align; n < stop_; n += threadInterval) {
      uint64_t threadStart = (n == align) ? start_ : n;
      uint64_t threadStop = std::min(n + threadInterval, stop_);
      PrimeSieve ps(*this, omp_get_thread_num());
      ps.sieve(threadStart, threadStop);
      count0 += ps.getCount(0);
      count1 += ps.getCount(1);
      count2 += ps.getCount(2);
      count3 += ps.getCount(3);
      count4 += ps.getCount(4);
      count5 += ps.getCount(5);
      count6 += ps.getCount(6);
    }
    counts_[0] = count0;
    counts_[1] = count1;
    counts_[2] = count2;
    counts_[3] = count3;
    counts_[4] = count4;
    counts_[5] = count5;
    counts_[6] = count6;
    seconds_ = omp_get_wtime() - t1;
  }

  // communicate the sieving results to the
  // primesieve GUI application
  if (shm_ != NULL) {
    std::copy(counts_.begin(), counts_.end(), shm_->counts);
    shm_->seconds = seconds_;
  }
}

/// Calculate the sieving status (in percent).
/// @param processed Sum of recently processed segments.
///
bool ParallelPrimeSieve::updateStatus(uint64_t processed, bool waitForLock)
{
  OmpLockGuard lock(getLock<omp_lock_t*>(), waitForLock);
  if (lock.isSet()) {
    PrimeSieve::updateStatus(processed, false);
    if (shm_ != NULL)
      shm_->status = getStatus();
  }
  return lock.isSet();
}

/// Used to synchronize threads for prime number generation

void ParallelPrimeSieve::setLock()
{
  omp_lock_t* lock = getLock<omp_lock_t*>();
  omp_set_lock(lock);
}

void ParallelPrimeSieve::unsetLock()
{
  omp_lock_t* lock = getLock<omp_lock_t*>();
  omp_unset_lock(lock);
}

#endif /* _OPENMP */

/// If OpenMP is not used then ParallelPrimeSieve behaves like
/// the single threaded PrimeSieve.
///
#if !defined(_OPENMP)

int ParallelPrimeSieve::getMaxThreads()
{
  return 1;
}

void ParallelPrimeSieve::sieve()
{
  PrimeSieve::sieve();
}

bool ParallelPrimeSieve::updateStatus(uint64_t processed, bool waitForLock)
{
  return PrimeSieve::updateStatus(processed, waitForLock);
}

void ParallelPrimeSieve::setLock() { }

void ParallelPrimeSieve::unsetLock() { }

#endif
