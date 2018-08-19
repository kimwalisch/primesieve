///
/// @file   ParallelSieve.cpp
/// @brief  Multi-threaded prime sieve using std::async.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/ParallelSieve.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/types.hpp>

#include <stdint.h>
#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <future>
#include <mutex>
#include <vector>

using namespace std;
using namespace primesieve;

namespace {

counts_t& operator+=(counts_t& v1, const counts_t& v2)
{
  for (size_t i = 0; i < v1.size(); i++)
    v1[i] += v2[i];
  return v1;
}

} // namespace

namespace primesieve {

ParallelSieve::ParallelSieve() :
  shm_(nullptr)
{
  int threads = get_num_threads();
  setNumThreads(threads);
}

void ParallelSieve::init(SharedMemory& shm)
{
  setStart(shm.start);
  setStop(shm.stop);
  setSieveSize(shm.sieveSize);
  setFlags(shm.flags);
  setNumThreads(shm.threads);
  shm_ = &shm;
}

int ParallelSieve::getMaxThreads()
{
  int maxThreads = thread::hardware_concurrency();
  return max(1, maxThreads);
}

int ParallelSieve::getNumThreads() const
{
  return numThreads_;
}

void ParallelSieve::setNumThreads(int threads)
{
  numThreads_ = inBetween(1, threads, getMaxThreads());
}

/// Get an ideal number of threads for
/// the start_ and stop_ numbers
///
int ParallelSieve::idealNumThreads() const
{
  if (start_ > stop_)
    return 1;

  uint64_t threshold = isqrt(stop_) / 5;
  threshold = max(threshold, config::MIN_THREAD_DISTANCE);
  uint64_t threads = getDistance() / threshold;
  threads = inBetween(1, threads, numThreads_);

  return (int) threads;
}

uint64_t ParallelSieve::getThreadDistance(int threads) const
{
  assert(threads > 0);
  assert(getDistance() > 0);

  uint64_t dist = getDistance();
  uint64_t balanced = isqrt(stop_) * 1000;
  uint64_t unbalanced = dist / threads;
  uint64_t fastest = min(balanced, unbalanced);
  uint64_t iters = dist / fastest;

  // the number of iterations should always be
  // a multiple of threads in order to ensure
  // all threads finish nearly at the same time
  iters = (iters / threads) * threads;
  iters = max(iters, (uint64_t) threads);

  uint64_t threadDist = ((dist - 1) / iters) + 1;
  threadDist = max(threadDist, config::MIN_THREAD_DISTANCE);
  threadDist += 30 - threadDist % 30;

  return threadDist;
}

/// Align n to modulo (30 + 2) to prevent prime k-tuplet
/// (twin primes, prime triplets) gaps
///
uint64_t ParallelSieve::align(uint64_t n) const
{
  uint64_t n32 = checkedAdd(n, 32);

  if (n32 >= stop_)
    return stop_;

  return n32 - n % 30;
}

/// Sieve the primes and prime k-tuplets in [start_, stop_]
/// in parallel using multi-threading
///
void ParallelSieve::sieve()
{
  reset();

  if (start_ > stop_)
    return;

  int threads = idealNumThreads();

  if (threads == 1)
    PrimeSieve::sieve();
  else
  {
    auto t1 = chrono::system_clock::now();
    uint64_t dist = getDistance();
    uint64_t threadDist = getThreadDistance(threads);
    uint64_t iters = ((dist - 1) / threadDist) + 1;
    threads = inBetween(1, threads, iters);
    atomic<uint64_t> i(0);

    // each thread executes 1 task
    auto task = [&]()
    {
      PrimeSieve ps(this);
      uint64_t j;
      counts_t counts;
      counts.fill(0);

      while ((j = i++) < iters)
      {
        uint64_t start = start_ + j * threadDist;
        uint64_t stop = checkedAdd(start, threadDist);
        stop = align(stop);
        if (start > start_)
          start = align(start) + 1;

        // sieve the range [start, stop]
        ps.sieve(start, stop);
        counts += ps.getCounts();
      }

      return counts;
    };

    vector<future<counts_t>> futures;
    futures.reserve(threads);

    for (int t = 0; t < threads; t++)
      futures.emplace_back(async(launch::async, task));

    for (auto &f : futures)
      counts_ += f.get();

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

/// Print status in percent to stdout
/// @processed: Sum of recently processed segments
/// @tryLock:   Do not block if tryLock = true
///
bool ParallelSieve::updateStatus(uint64_t processed, bool tryLock)
{
  unique_lock<mutex> lock(lock_, defer_lock);

  if (tryLock)
    lock.try_lock();
  else
    lock.lock();

  if (lock.owns_lock())
  {
    PrimeSieve::updateStatus(processed);
    if (shm_)
      shm_->status = getStatus();
    return true;
  }

  return false;
}

} // namespace
