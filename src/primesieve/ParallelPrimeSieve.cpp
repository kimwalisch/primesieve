///
/// @file   ParallelPrimeSieve.cpp
/// @brief  Sieve primes in parallel using OpenMP.
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/ParallelPrimeSieve.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <algorithm>
#include <cassert>
#include <cstddef>

#ifdef _OPENMP
  #include <omp.h>
  #include <primesieve/ParallelPrimeSieve-lock.hpp>
#endif

using namespace std;

namespace primesieve {

ParallelPrimeSieve::ParallelPrimeSieve() :
  lock_(NULL),
  shm_(NULL),
  numThreads_(getMaxThreads())
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
  return numThreads_;
}

void ParallelPrimeSieve::setNumThreads(int threads)
{
  numThreads_ = inBetween(1, threads, getMaxThreads());
}

/// Get an ideal number of threads for
/// the start_ and stop_ numbers.
///
int ParallelPrimeSieve::idealNumThreads() const
{
  if (start_ > stop_)
    return 1;

  uint64_t threshold = max(config::MIN_THREAD_DISTANCE, isqrt(stop_) / 5);
  uint64_t threads = getDistance() / threshold;
  threads = inBetween(1, threads, numThreads_);

  return static_cast<int>(threads);
}

/// Get a thread distance which ensures a good load
/// balance when using multiple threads.
///
uint64_t ParallelPrimeSieve::getThreadDistance(int threads) const
{
  assert(threads > 0);
  uint64_t unbalanced = getDistance() / threads;
  uint64_t balanced = isqrt(stop_) * 1000;
  uint64_t fastest = min(balanced, unbalanced);
  uint64_t threadDistance = inBetween(config::MIN_THREAD_DISTANCE, fastest, config::MAX_THREAD_DISTANCE);
  uint64_t chunks = getDistance() / threadDistance;

  if (chunks < threads * 5u)
    threadDistance = max(config::MIN_THREAD_DISTANCE, unbalanced);

  threadDistance += 30 - threadDistance % 30;
  return threadDistance;
}

/// Align n to modulo 30 + 2 to prevent prime k-tuplet
/// (twin primes, prime triplets, ...) gaps.
///
uint64_t ParallelPrimeSieve::align(uint64_t n) const
{
  if (add_overflow_safe(n, 32) >= stop_)
    return stop_;

  n = add_overflow_safe(n, 32) - n % 30;
  n = min(n, stop_);

  return n;
}

#ifdef _OPENMP

int ParallelPrimeSieve::getMaxThreads()
{
  return max(1, omp_get_max_threads());
}

double ParallelPrimeSieve::getWallTime() const
{
  return omp_get_wtime();
}

/// Sieve the primes and prime k-tuplets within [start_, stop_]
/// in parallel using OpenMP multi-threading.
///
void ParallelPrimeSieve::sieve()
{
  reset();
  OmpInitLock ompInit(&lock_);

  if (start_ > stop_)
    return;

  int threads = idealNumThreads();

  if (threads == 1)
    PrimeSieve::sieve();
  else
  {
    uint64_t threadDistance = getThreadDistance(threads);
    uint64_t count0 = 0, count1 = 0, count2 = 0, count3 = 0, count4 = 0, count5 = 0;
    int64_t iters = 1 + (getDistance() - 1) / threadDistance;
    double t1 = getWallTime();

    #pragma omp parallel for schedule(dynamic) num_threads(threads) \
        reduction(+: count0, count1, count2, count3, count4, count5)
    for (int64_t i = 0; i < iters; i++)
    {
      uint64_t threadStart = start_ + i * threadDistance;
      uint64_t threadStop = add_overflow_safe(threadStart, threadDistance);
      if (i > 0) threadStart = align(threadStart) + 1;
      threadStop = align(threadStop);

      PrimeSieve ps(*this, omp_get_thread_num());
      ps.sieve(threadStart, threadStop);

      count0 += ps.getCount(0);
      count1 += ps.getCount(1);
      count2 += ps.getCount(2);
      count3 += ps.getCount(3);
      count4 += ps.getCount(4);
      count5 += ps.getCount(5);
    }

    seconds_ = getWallTime() - t1;

    counts_[0] = count0;
    counts_[1] = count1;
    counts_[2] = count2;
    counts_[3] = count3;
    counts_[4] = count4;
    counts_[5] = count5;
  }

  if (shm_)
  {
    // communicate the sieving results to
    // the primesieve GUI application
    copy(counts_.begin(), counts_.end(), shm_->counts);
    shm_->seconds = seconds_;
  }
}

/// Calculate the sieving status.
/// @param processed  Sum of recently processed segments.
///
bool ParallelPrimeSieve::updateStatus(uint64_t processed, bool waitForLock)
{
  OmpLockGuard lock(getLock<omp_lock_t*>(), waitForLock);
  if (lock.isSet())
  {
    PrimeSieve::updateStatus(processed);
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

  if (shm_)
  {
    // communicate the sieving results to the
    // primesieve GUI application
    copy(counts_.begin(), counts_.end(), shm_->counts);
    shm_->seconds = seconds_;
  }
}

double ParallelPrimeSieve::getWallTime() const
{
  return PrimeSieve::getWallTime();
}

bool ParallelPrimeSieve::updateStatus(uint64_t processed, bool waitForLock)
{
  bool isUpdate = PrimeSieve::updateStatus(processed, waitForLock);
  if (shm_)
    shm_->status = getStatus();
  return isUpdate;
}

void ParallelPrimeSieve::setLock() { }

void ParallelPrimeSieve::unsetLock() { }

#endif

} // namespace
