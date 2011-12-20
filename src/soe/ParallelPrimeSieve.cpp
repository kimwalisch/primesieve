//
// Copyright (c) 2011 Kim Walisch, <kim.walisch@gmail.com>.
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
#include "defs.h"
#include "imath.h"

#if defined(_OPENMP)
  #include <omp.h>
#endif

#include <cstdlib>
#include <stdexcept>
#include <algorithm>

ParallelPrimeSieve::ParallelPrimeSieve() :
  numThreads_(USE_IDEAL_NUM_THREADS),
  shm_(NULL)
{
  // prevents prime k-tuplet gaps
  static_assert(defs::MIN_THREAD_INTERVAL >= 100, "defs::MIN_THREAD_INTERVAL >= 100");
}

/**
 * Used with the primesieve Qt application in ../qt-gui.
 * Initializes the ParallelPrimeSieve object with values from
 * a shared memory segment.
 */
void ParallelPrimeSieve::init(SharedMemory* shm) {
  if (shm == NULL) {
    throw std::invalid_argument(
        "ParallelPrimeSieve: shared memory segment must not be NULL");
  }
  shm_ = shm;
  setStartNumber(shm_->startNumber);
  setStopNumber(shm_->stopNumber);
  setSieveSize(shm_->sieveSize);
  setFlags(shm_->flags);
  setNumThreads(shm_->threads);
}

/**
 * Calculate the current status in percent of sieve().
 * @param processed  The size of the processed segment (interval)
 */
void ParallelPrimeSieve::doStatus(uint32_t processed) {
#if defined(_OPENMP)
  #pragma omp critical (doStatus)
#endif
  {
    PrimeSieve::doStatus(processed);
    // communicate the current status via shared memory
    // to the Qt GUI process
    if (shm_ != NULL)
      shm_->status = status_;
  }
}

int ParallelPrimeSieve::getMaxThreads() {
#if defined(_OPENMP)
  return omp_get_max_threads();
#else
  return 1;
#endif
}

/**
 * Get the current set number of threads for sieving.
 */
int ParallelPrimeSieve::getNumThreads() const {
  return (numThreads_ == USE_IDEAL_NUM_THREADS)
      ? getIdealNumThreads() 
      : numThreads_;
}

/**
 * Set the number of threads for sieving.
 * If numThreads is invalid (numThreads < 1 or > getMaxThreads()) it
 * is set to numThreads = USE_IDEAL_NUM_THREADS.
 */
void ParallelPrimeSieve::setNumThreads(int numThreads) {
  numThreads_ = (numThreads < 1 || numThreads > getMaxThreads())
      ? USE_IDEAL_NUM_THREADS
      : numThreads;
}

/**
 * Get an ideal number of threads for the current set
 * startNumber_, stopNumber_ and flags_.
 */
int ParallelPrimeSieve::getIdealNumThreads() const {
  // 1 thread to generate primes and k-tuplets in sequential order
  if (testFlags(GENERATE_FLAGS))
    return 1;

  // each thread sieves at least an interval of size sqrt(n) / 6
  // but not smaller than defs::MIN_THREAD_INTERVAL
  uint64_t minInterval = isqrt(stopNumber_) / 6;
  uint64_t threadThreshold = std::max<uint64_t>(defs::MIN_THREAD_INTERVAL, minInterval);

  // use getMaxThreads() if the interval size is sufficiently large
  uint64_t idealMaxThreads = getInterval() / threadThreshold;
  uint64_t idealNumThreads = std::min<uint64_t>(idealMaxThreads, getMaxThreads());
  if (idealNumThreads < 1)
    idealNumThreads = 1;

  return static_cast<int> (idealNumThreads);
}

uint64_t ParallelPrimeSieve::getInterval() const {
  return stopNumber_ - startNumber_;
}

/**
 * Get an interval size that ensures a good load
 * balance among threads.
 */
uint64_t ParallelPrimeSieve::getIdealInterval() const {
  uint64_t threads  = getNumThreads();
  uint64_t interval = getInterval();
  if (threads == 1)
    return interval;

  // idealInterval = sqrt(n) * 2000, 0.1% initialization overhead
  uint64_t sqrtStop = isqrt(stopNumber_);
  uint64_t idealInterval = std::max<uint64_t>(defs::MIN_THREAD_INTERVAL, sqrtStop * 2000);

  uint64_t maxInterval = interval / threads;
  // correct the user's bad settings
  if (maxInterval < interval && maxInterval < defs::MIN_THREAD_INTERVAL)
    maxInterval = interval / getIdealNumThreads();

  return std::min(idealInterval, maxInterval);
}

/**
 * Sieve the primes and prime k-tuplets within [startNumber, stopNumber]
 * in parallel using OpenMP.
 */
void ParallelPrimeSieve::sieve() {
  if (stopNumber_ < startNumber_)
    throw std::invalid_argument("STOP must be >= START");

#if defined(_OPENMP)
  double t1 = omp_get_wtime();
  reset();
  uint64_t idealInterval = getIdealInterval();
   int64_t chunks        = (idealInterval > 0) ? getInterval() / idealInterval : 1;
  uint64_t maxStop       = startNumber_ + idealInterval * chunks;
  maxStop += 32 - maxStop % 30;
  if (maxStop < stopNumber_) 
    chunks += 1;

  // split the sieve interval [startNumber_, stopNumber_]
  // into 'chunks' sub-intervals that are processed in
  // parallel using OpenMP
  int threads = getNumThreads();
  #pragma omp parallel for num_threads(threads) schedule(dynamic)
  for (int64_t i = 0; i < chunks; i++) {
    uint64_t start = startNumber_ + idealInterval * i;
    uint64_t stop  = startNumber_ + idealInterval * (i+1);
    // start/stop numbers must be chosen carefully
    // to prevent prime k-tuplets gaps
    if (i > 0)
      start += 32 - start % 30;
    stop += 32 - stop % 30;
    // sieve the primes within [start, stop]
    PrimeSieve ps(start, std::min(stop, stopNumber_), this);
    ps.sieve();
    #pragma omp critical (counts)
    for (uint32_t j = 0; j < COUNTS_SIZE; j++)
      counts_[j] += ps.getCounts(j);
  }
  timeElapsed_ = omp_get_wtime() - t1;
#else
  PrimeSieve::sieve();
#endif
  // communicate the sieving results via shared memory
  // segment to the Qt GUI process
  if (shm_ != NULL) {
    for (uint32_t i = 0; i < COUNTS_SIZE; i++)
      shm_->counts[i] = counts_[i];
    shm_->timeElapsed = timeElapsed_;
  }
}
