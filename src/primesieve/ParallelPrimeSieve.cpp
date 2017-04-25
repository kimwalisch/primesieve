///
/// @file   ParallelPrimeSieve.cpp
/// @brief  Multi-threaded prime sieve.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/ParallelPrimeSieve.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/LockGuard.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <mutex>
#include <thread>

using namespace std;

namespace primesieve {

ParallelPrimeSieve::ParallelPrimeSieve() :
  shm_(nullptr),
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

  return (int) threads;
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
  if (checkedAdd(n, 32) >= stop_)
    return stop_;

  n = checkedAdd(n, 32) - n % 30;
  n = min(n, stop_);

  return n;
}

int ParallelPrimeSieve::getMaxThreads()
{
  return max(1, (int) thread::hardware_concurrency());
}

/// Sieve the primes and prime k-tuplets within [start_, stop_]
/// in parallel using multi-threading.
///
void ParallelPrimeSieve::sieve()
{
  reset();

  if (start_ > stop_)
    return;

  int64_t threads = idealNumThreads();

  if (threads == 1)
    PrimeSieve::sieve();
  else
  {
    auto t1 = chrono::system_clock::now();
    uint64_t threadDistance = getThreadDistance(threads);
    int64_t iters = ((getDistance() - 1) / threadDistance) + 1;
    int64_t i = 0;
    mutex lock;

    auto task = [&]()
    {
      vector<uint64_t> counts(6, 0);

      while (true)
      {
        uint64_t start;
        {
          lock_guard<mutex> guard(lock);
          if (i >= iters)
            break;
          start = start_ + i * threadDistance;
          i += 1;
        }

        uint64_t stop = checkedAdd(start, threadDistance);
        stop = align(stop);
        if (start > start_)
          start = align(start) + 1;

        PrimeSieve ps(*this);
        ps.sieve(start, stop);

        for (size_t j = 0; j < counts.size(); j++)
          counts[j] += ps.getCount(j);
      }

      lock_guard<mutex> guard(lock);
      for (size_t j = 0; j < counts.size(); j++)
        counts_[j] += counts[j];
    };

    threads = min(threads, iters);
    vector<thread> pool;
    pool.reserve(threads);

    for (int64_t t = 0; t < threads; t++)
      pool.emplace_back(task);

    for (thread &t : pool)
      t.join();

    auto t2 = chrono::system_clock::now();
    chrono::duration<double> seconds = t2 - t1;
    seconds_ = seconds.count();
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
bool ParallelPrimeSieve::updateStatus(uint64_t processed, bool wait)
{
  LockGuard lock(lock_, wait);

  if (lock.isSet())
  {
    PrimeSieve::updateStatus(processed);
    if (shm_)
      shm_->status = getStatus();
  }

  return lock.isSet();
}

} // namespace
