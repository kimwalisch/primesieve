///
/// @file   ParallelPrimeSieve.cpp
/// @brief  ParallelPrimeSieve sieves primes in parallel using
///         OpenMP 2.0 (2002) or later.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "config.h"
#include "ParallelPrimeSieve.h"
#include "PrimeSieve.h"
#include "imath.h"

#include <stdint.h>
#include <cstdlib>
#include <cassert>
#include <algorithm>

#ifdef _OPENMP
  #include <omp.h>
  #include "ParallelPrimeSieve-lock.h"
  #include "primesieve_error.h"
#endif

using namespace soe;

ParallelPrimeSieve::ParallelPrimeSieve() :
  numThreads_(DEFAULT_NUM_THREADS),
  shm_(NULL)
{ }

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
    // Use 1 thread to generate primes in arithmetic order
    return (isPrint() || isGenerate()) ? 1 : idealNumThreads();
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
  threadInterval += 30 - threadInterval % 30;
  return threadInterval;
}

/// Align n to modulo 30 + 2 to prevent prime k-tuplet
/// (twin primes, prime triplets, ...) gaps.
///
uint64_t ParallelPrimeSieve::align(uint64_t n) const
{
  if (n == start_)
    return start_;
  n = std::min(n + 32 - n % 30, stop_);
  return n;
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

/// Sieve the primes and prime k-tuplets within [start_, stop_]
/// in parallel using OpenMP multi-threading.
///
void ParallelPrimeSieve::sieve()
{
  if (start_ > stop_)
    throw primesieve_error("start must be <= stop");
  OmpInitLock ompInit(&lock_);

  int threads = getNumThreads();
  if (tooMany(threads))
    threads = idealNumThreads();
  if (threads == 1)
    PrimeSieve::sieve();
  else {
    reset();
    uint64_t threadInterval = getThreadInterval(threads);
    uint64_t count0 = 0, count1 = 0, count2 = 0, count3 = 0, count4 = 0, count5 = 0, count6 = 0;
    double time = omp_get_wtime();

#if _OPENMP >= 200800 /* OpenMP >= 3.0 (2008) */

    #pragma omp parallel for schedule(dynamic) num_threads(threads) \
      reduction(+: count0, count1, count2, count3, count4, count5, count6)
    for (uint64_t n = start_; n < stop_; n += threadInterval) {
      PrimeSieve ps(*this, omp_get_thread_num());
      uint64_t threadStart = align(n);
      uint64_t threadStop  = align(n + threadInterval);
      ps.sieve(threadStart, threadStop);
      count0 += ps.getCount(0);
      count1 += ps.getCount(1);
      count2 += ps.getCount(2);
      count3 += ps.getCount(3);
      count4 += ps.getCount(4);
      count5 += ps.getCount(5);
      count6 += ps.getCount(6);
    }

#else /* OpenMP 2.x */

    int64_t iters = 1 + (getInterval() - 1) / threadInterval;

    #pragma omp parallel for schedule(dynamic) num_threads(threads) \
      reduction(+: count0, count1, count2, count3, count4, count5, count6)
    for (int64_t i = 0; i < iters; i++) {
      PrimeSieve ps(*this, omp_get_thread_num());
      uint64_t n = start_ + i * threadInterval;
      uint64_t threadStart = align(n);
      uint64_t threadStop  = align(n + threadInterval);
      ps.sieve(threadStart, threadStop);
      count0 += ps.getCount(0);
      count1 += ps.getCount(1);
      count2 += ps.getCount(2);
      count3 += ps.getCount(3);
      count4 += ps.getCount(4);
      count5 += ps.getCount(5);
      count6 += ps.getCount(6);
    }

#endif

    seconds_ = omp_get_wtime() - time;
    counts_[0] = count0;
    counts_[1] = count1;
    counts_[2] = count2;
    counts_[3] = count3;
    counts_[4] = count4;
    counts_[5] = count5;
    counts_[6] = count6;
  }

  // communicate the sieving results to the
  // primesieve GUI application
  if (shm_) {
    std::copy(counts_.begin(), counts_.end(), shm_->counts);
    shm_->seconds = seconds_;
  }
}

/// Calculate the sieving status.
/// @param processed  Sum of recently processed segments.
///
bool ParallelPrimeSieve::updateStatus(uint64_t processed, bool waitForLock)
{
  OmpLockGuard lock(getLock<omp_lock_t*>(), waitForLock);
  if (lock.isSet()) {
    PrimeSieve::updateStatus(processed, false);
    if (shm_)
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

/// If OpenMP is disabled then ParallelPrimeSieve behaves like
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
