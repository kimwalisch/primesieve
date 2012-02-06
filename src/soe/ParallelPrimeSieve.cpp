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

#include "ParallelPrimeSieve.h"
#include "PrimeSieve.h"
#include "PrimeNumberFinder.h"
#include "config.h"
#include "imath.h"

#if defined(_OPENMP)
  #include <omp.h>
#endif

#include <stdint.h>
#include <stdexcept>
#include <cstdlib>
#include <cassert>
#include <algorithm>

using namespace soe;

ParallelPrimeSieve::ParallelPrimeSieve() :
  numThreads_(IDEAL_NUM_THREADS),
  shm_(NULL)
{
  // prevents prime k-tuplet gaps
  static_assert(config::MIN_THREAD_INTERVAL >= 100,
      "config::MIN_THREAD_INTERVAL must not be < 100");
  static_assert(config::MIN_THREAD_INTERVAL <= config::MAX_THREAD_INTERVAL,
      "config::MIN_THREAD_INTERVAL must not be > config::MAX_THREAD_INTERVAL");
#if defined(_OPENMP)
  omp_init_lock(&lock_);
#endif
}

ParallelPrimeSieve::~ParallelPrimeSieve() {
#if defined(_OPENMP)
  omp_destroy_lock(&lock_);
#endif
}

/**
 * API for the primesieve Qt application in src/qt-gui.
 * Initializes the ParallelPrimeSieve object with values from
 * a shared memory segment.
 */
void ParallelPrimeSieve::init(SharedMemory* shm) {
  if (shm == NULL)
    throw std::invalid_argument("ParallelPrimeSieve: shared memory segment must not be NULL");
  shm_ = shm;
  setStart(shm->start);
  setStop(shm->stop);
  setSieveSize(shm->sieveSize);
  setFlags(shm->flags);
  setNumThreads(shm->threads);
}

int ParallelPrimeSieve::getMaxThreads() {
#if defined(_OPENMP)
  return omp_get_max_threads();
#else
  return 1;
#endif
}

int ParallelPrimeSieve::getNumThreads() const {
  return (numThreads_ != IDEAL_NUM_THREADS) ? numThreads_ : getIdealNumThreads();
}

/**
 * Set the number of threads for sieving.
 * If numThreads is invalid IDEAL_NUM_THREADS are used.
 */
void ParallelPrimeSieve::setNumThreads(int numThreads) {
  numThreads_ = numThreads;
  if (numThreads_ < 1 || numThreads_ > getMaxThreads())
    numThreads_ = IDEAL_NUM_THREADS;
}

/**
 * Get an ideal number of threads for the current set
 * start_, stop_ and flags_.
 */
int ParallelPrimeSieve::getIdealNumThreads() const {
  // 1 thread generates primes in arithmetic order
  if (testFlags(GENERATE_FLAGS))
    return 1;
  // each thread sieves at least an interval of size x^0.5/5
  // but not smaller than MIN_THREAD_INTERVAL
  uint64_t threshold = std::max(config::MIN_THREAD_INTERVAL, isqrt(stop_) / 5);
  uint64_t idealNumThreads = getBoundedValue<uint64_t>(1, (stop_ - start_) / threshold, getMaxThreads());
  return static_cast<int>(idealNumThreads);
}

#if defined(_OPENMP)

void ParallelPrimeSieve::set_lock() {
  omp_set_lock(&lock_);
}

void ParallelPrimeSieve::unset_lock() {
  omp_unset_lock(&lock_);
}

/**
 * Calculate the current status in percent of sieve().
 * @param segment  The interval size of the processed segment
 */
void ParallelPrimeSieve::calcStatus(uint32_t segment) {
  #pragma omp critical (calcStatus)
  {
    PrimeSieve::calcStatus(segment);
    // communicate the current status via shared
    // memory to the Qt GUI process
    if (shm_ != NULL)
      shm_->status = getStatus();
  }
}

/** Get a thread interval size that ensures a good load balance. */
uint64_t ParallelPrimeSieve::getBalancedInterval(int threads) const {
  assert(threads > 1);
  uint64_t unbalanced = std::max(config::MIN_THREAD_INTERVAL, (stop_ - start_) / threads);
  // balanced interval = x^0.5*1000, 0.5% initialization overhead
  uint64_t balanced = getBoundedValue<uint64_t>(
      config::MIN_THREAD_INTERVAL, isqrt(stop_) * 1000, config::MAX_THREAD_INTERVAL);
  // align to mod 30 to prevent prime k-tuplet gaps
  unbalanced += 30 - unbalanced % 30;
  balanced += 30 - balanced % 30;
  return std::min(balanced, unbalanced);
}

/**
 * Sieve the primes and prime k-tuplets within [start_, stop_] in
 * parallel using OpenMP (version 3.0 or later).
 */
void ParallelPrimeSieve::sieve() {
  if (stop_ < start_)
    throw std::invalid_argument("STOP must be >= START");

  int threads = getNumThreads();
  // correct the user's bad number of threads
  if (threads >= 2 && (stop_ - start_) / threads < config::MIN_THREAD_INTERVAL)
    threads = getIdealNumThreads();
  if (threads == 1)
    PrimeSieve::sieve();
  else {
    double t1 = omp_get_wtime();
    reset();
    uint64_t count0 = 0, count1 = 0, count2 = 0, count3 = 0, count4 = 0, count5 = 0, count6 = 0;
    uint64_t balanced = getBalancedInterval(threads);
    uint64_t align = start_ + 32 - start_ % 30;
    #pragma omp parallel for schedule(dynamic) num_threads(threads) \
        reduction(+: count0, count1, count2, count3, count4, count5, count6)
    for (uint64_t n = align; n < stop_; n += balanced) {
      uint64_t threadStart = (n > align) ? n : start_;
      uint64_t threadStop = std::min(n + balanced, stop_);
      PrimeSieve ps(this);
      ps.sieve(threadStart, threadStop);
      count0 += ps.getCounts(0);
      count1 += ps.getCounts(1);
      count2 += ps.getCounts(2);
      count3 += ps.getCounts(3);
      count4 += ps.getCounts(4);
      count5 += ps.getCounts(5);
      count6 += ps.getCounts(6);
    }
    counts_[0] = count0;
    counts_[1] = count1;
    counts_[2] = count2;
    counts_[3] = count3;
    counts_[4] = count4;
    counts_[5] = count5;
    counts_[6] = count6;
    timeElapsed_ = omp_get_wtime() - t1;
  }
  // communicate the sieving results via shared memory
  // segment to the Qt GUI process
  if (shm_ != NULL) {
    shm_->timeElapsed = timeElapsed_;
    for (int i = 0; i < COUNTS_SIZE; i++)
      shm_->counts[i] = counts_[i];
  }
}

#endif /* _OPENMP */
