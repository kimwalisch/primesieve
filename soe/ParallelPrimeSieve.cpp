/*
 * ParallelPrimeSieve.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2011 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "ParallelPrimeSieve.h"
#include "PrimeSieve.h"
#include "PrimeNumberFinder.h"
#include "defs.h"
#include "pmath.h"

#if defined(_OPENMP)
  #include <omp.h>
#endif

#include <cstdlib>
#include <stdexcept>

ParallelPrimeSieve::ParallelPrimeSieve() :
  shm_(NULL), numThreads_(USE_IDEAL_NUM_THREADS) {
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

/**
 * Get an ideal number of threads for the current set startNumber_,
 * stopNumber_ and flags_.
 */
int ParallelPrimeSieve::getIdealNumThreads() const {
  // 1 thread to print primes in sequential order
  if (flags_ & PRINT_FLAGS)
    return 1;
  // by test each thread should at least sieve an interval of n^0.5/6
  // and not smaller than 1E8 for a performance benefit
  uint64_t minInterval = std::max<uint64_t>(
      static_cast<uint64_t> (1E8), isqrt(stopNumber_) / 6);
  int idealNumThreads = static_cast<int> (
      std::min<uint64_t>(
          this->getMaxThreads(), (stopNumber_ - startNumber_) / minInterval));
  // 1 >= idealNumThreads <= getMaxThreads()
  return std::max<int>(1, idealNumThreads);
}

/**
 * Get a sieve interval that ensures a good load balance among
 * threads.
 */
uint64_t ParallelPrimeSieve::getIdealSieveInterval() const {
  // the initialization overhead of a sieve interval of n^0.5*500
  // is about 0.4 percent near 1E19
  uint64_t idealInterval = std::max<uint64_t>(
      static_cast<uint64_t> (1E9),
      static_cast<uint64_t> (isqrt(stopNumber_)) * 500);
  uint64_t maxInterval = (stopNumber_ - startNumber_) / 
      static_cast<uint64_t> (this->getNumThreads());
  // 1E9 >= idealInterval <= n^0.5*500 <= entire interval / threads
  return std::min<uint64_t>(idealInterval, maxInterval);
}

/**
 * The start/stop numbers for PrimeSieve objects must be chosen
 * carefully in order to avoid gaps when sieving primes and prime
 * k-tuplets in parallel.
 */
uint64_t ParallelPrimeSieve::getPrimeSieveBound(uint64_t offset) const {
  if (offset == 0)
    return startNumber_;
  uint64_t n = startNumber_ + offset;
  if (n % 30 != 0)
    n += 30 - (n % 30);
  n += 2;
  return std::min<uint64_t>(n, stopNumber_);
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

/**
 * For use with the Qt primesieve application in ../qt-gui.
 * Initializes this ParallelPrimeSieve object with values from a
 * shared memory segment.
 */
void ParallelPrimeSieve::init(SharedMemory* shm) {
  if (shm == NULL)
    throw std::invalid_argument("shared memory segment must not be NULL");
  this->setStartNumber(shm->startNumber);
  this->setStopNumber(shm->stopNumber);
  this->setSieveSize(shm->sieveSize);
  this->setFlags(shm->flags);
  this->setNumThreads(shm->threads);
  // upon completion the sieving results are communicated back to the
  // Qt GUI process via shared memory
  shm_ = shm;
}

/**
 * Calculate the current status in percent of sieve().
 */
void ParallelPrimeSieve::doStatus(uint32_t segment) {
#if defined(_OPENMP)
  #pragma omp critical (doStatus)
#endif
  {
    PrimeSieve::doStatus(segment);
    // communicate the current status via shared memory
    // to the Qt GUI process
    if (shm_ != NULL)
      shm_->status = status_;
  }
}

/**
 * Sieve the prime numbers and/or prime k-tuplets within the interval
 * [startNumber_, stopNumber_] in parallel using multiple threads
 * (OpenMP).
 */
void ParallelPrimeSieve::sieve() {
  if (stopNumber_ < startNumber_)
    throw std::invalid_argument("STOP must be >= START");

#if defined(_OPENMP)
  double t1 = omp_get_wtime();
  this->reset();
  uint64_t interval = this->getIdealSieveInterval();
  if (interval >= 60) {
    uint64_t chunks = (stopNumber_ - startNumber_) / interval;
    uint64_t maxOffset = interval * chunks;
    if (this->getPrimeSieveBound(maxOffset) < stopNumber_)
      chunks++;
    int threads = this->getNumThreads();

    // OpenMP parallel sieving
    #pragma omp parallel for num_threads(threads) schedule(dynamic, 1)
    for (int i = 0; i < static_cast<int> (chunks); i++) {
      uint64_t offset = static_cast<uint64_t> (i) * interval;
      uint64_t start = this->getPrimeSieveBound(offset);
      uint64_t stop = this->getPrimeSieveBound(offset + interval);
      // start sieving a chunk
      PrimeSieve ps(start, stop, this);
      ps.sieve();
      #pragma omp critical (counts)
      {
        for (int j = 0; j < COUNTS_SIZE; j++)
          counts_[j] += ps.getCounts(j);
      }
    }
  } else // single-threaded sieving
    PrimeSieve::sieve();
  timeElapsed_ = omp_get_wtime() - t1;
#else // single-threaded sieving
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
