//
// Copyright (c) 2011 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of primesieve.
// Visit: http://primesieve.googlecode.com
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
  numThreads_(USE_IDEAL_NUM_THREADS), shm_(NULL) {
  this->setMinThreadInterval(defs::MIN_THREAD_INTERVAL);
}

/**
 * Get the maximum number of threads allowed for sieving 
 * (omp_get_max_threads(), i.e. the number of logical CPU cores).
 */
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
  return (numThreads_ == USE_IDEAL_NUM_THREADS) ? this->getIdealNumThreads() 
      : numThreads_;
}

uint64_t ParallelPrimeSieve::getSieveInterval() const {
  return stopNumber_ - startNumber_;
}

/**
 * Get an ideal number of threads for the current set startNumber_,
 * stopNumber_ and flags_.
 */
int ParallelPrimeSieve::getIdealNumThreads() const {
  // 1 thread to print primes in sequential order
  if (flags_ & PRINT_FLAGS)
    return 1;
  // each thread should at least sieve an interval of n^0.5/6 and not
  // smaller than minThreadInterval_ for a performance benefit
  uint64_t threadThreshold = std::max<uint64_t>(
      minThreadInterval_,
      isqrt(stopNumber_) / 6);

  int idealNumThreads = static_cast<int> (
      std::min<uint64_t>(
          this->getSieveInterval() / threadThreshold,
          this->getMaxThreads()));
  // 1 <= idealNumThreads <= getMaxThreads()
  return std::max<int>(1, idealNumThreads);
}

/**
 * Get a sieve interval that ensures a good load balance among
 * threads.
 */
uint64_t ParallelPrimeSieve::getIdealInterval() const {
  int numThreads = this->getNumThreads();
  if (numThreads == 1)
    return this->getSieveInterval();

  // idealInterval = n^0.5*2000 (0.1 percent initialization overhead)
  uint64_t idealInterval = std::max<uint64_t>(
      minThreadInterval_,
      isqrt(stopNumber_) * UINT64_C(2000));

  uint64_t maxThreadInterval = this->getSieveInterval() / numThreads;
  // correct the user's thread settings
  if (maxThreadInterval < minThreadInterval_)
    maxThreadInterval = this->getSieveInterval() / this->getIdealNumThreads();

  // minThreadInterval_ <= idealInterval <= maxThreadInterval
  return std::min(idealInterval, maxThreadInterval);
}

/**
 * Set the number of threads for sieving.
 * If numThreads is invalid (numThreads < 1 or > getMaxThreads()) it
 * is set to numThreads = USE_IDEAL_NUM_THREADS.
 */
void ParallelPrimeSieve::setNumThreads(int numThreads) {
  numThreads_ = (numThreads >= 1 && numThreads <= this->getMaxThreads())
      ? numThreads : USE_IDEAL_NUM_THREADS;
}

void ParallelPrimeSieve::setMinThreadInterval(uint64_t minThreadInterval) {
  // prevents gaps when sieving prime k-tuplets in parallel
  if (minThreadInterval < 100)
    throw std::underflow_error(
        "ParallelPrimeSieve: minThreadInterval must be >= 100");
  minThreadInterval_ = minThreadInterval;
}

/**
 * For use with the Qt primesieve application in ../qt-gui.
 * Initializes this ParallelPrimeSieve object with values from a
 * shared memory segment.
 */
void ParallelPrimeSieve::init(SharedMemory* shm) {
  if (shm == NULL)
    throw std::invalid_argument(
        "ParallelPrimeSieve: shared memory segment must not be NULL");
  shm_ = shm;
  this->setStartNumber(shm_->startNumber);
  this->setStopNumber(shm_->stopNumber);
  this->setSieveSize(shm_->sieveSize);
  this->setFlags(shm_->flags);
  this->setNumThreads(shm_->threads);
}

/**
 * Calculate the current status in percent of sieve().
 * @param processed A sieved interval (segment)
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

/**
 * Sieve the prime numbers and/or prime k-tuplets within the interval
 * [startNumber_, stopNumber_] in parallel using OpenMP.
 */
void ParallelPrimeSieve::sieve() {
  if (stopNumber_ < startNumber_)
    throw std::invalid_argument("STOP must be >= START");

#if defined(_OPENMP)
  double t1 = omp_get_wtime();
  this->reset();
  if (this->getSieveInterval() >= minThreadInterval_) {
    uint64_t idealInterval = this->getIdealInterval();
    uint64_t chunks = this->getSieveInterval() / idealInterval;
    uint64_t maxOffset = chunks * idealInterval;
    uint64_t maxStop = startNumber_ + maxOffset + 32 - maxOffset % 30;
    if (maxStop < stopNumber_)
      chunks++;
    int numThreads = this->getNumThreads();
    // OpenMP parallel sieving
    #pragma omp parallel for num_threads(numThreads) schedule(dynamic)
    for (int64_t i = 0; i < static_cast<int64_t> (chunks); i++) {
      uint64_t start = startNumber_ + idealInterval * i;
      uint64_t stop = startNumber_ + idealInterval * (i+1);
      // the start/stop numbers for PrimeSieve objects must be chosen
      // carefully in order to avoid gaps when sieving prime k-tuplets
      // in parallel
      if (i > 0)
        start += 32 - start % 30;
      stop += 32 - stop % 30;
      // sieve the interval [start, stop]
      PrimeSieve ps(start, std::min<uint64_t>(stop, stopNumber_), this);
      ps.sieve();
      #pragma omp critical (counts)
      for (int j = 0; j < COUNTS_SIZE; j++)
        counts_[j] += ps.getCounts(j);
    }
  } else // single-threaded sieving
    PrimeSieve::sieve();
  timeElapsed_ = omp_get_wtime() - t1;
#else
  PrimeSieve::sieve();
#endif
  // communicate the sieving results via shared memory
  // segment to the Qt GUI process
  if (shm_ != NULL) {
    for (int i = 0; i < COUNTS_SIZE; i++)
      shm_->counts[i] = counts_[i];
    shm_->timeElapsed = timeElapsed_;
  }
}
