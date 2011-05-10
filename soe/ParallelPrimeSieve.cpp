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
#include <vector>

ParallelPrimeSieve::ParallelPrimeSieve() : sharedMemoryPPS_(NULL), 
    numThreads_(USE_IDEAL_NUM_THREADS) {
}

/**
 * Get the maximum number of threads allowed for sieving 
 * (omp_get_max_threads(), i.e. the number of logical CPU cores).
 * @see http://msdn.microsoft.com/en-us/library/ewb30w8w.aspx
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
  return (numThreads_ == USE_IDEAL_NUM_THREADS)
      ? this->getIdealNumThreads()
      : numThreads_;
}

/**
 * Get an ideal number of threads for the current set startNumber,
 * stopNumber and flags.
 */
int ParallelPrimeSieve::getIdealNumThreads() const {
  // 1 thread to generate (print, callback) primes in sequential order
  if (flags_ & GENERATE_FLAGS)
    return 1;

  // I made some tests around 10^19 which showed that each
  // thread should at least sieve an interval of
  // sqrt(stopNumber) / 6 for a performance benefit
  uint64_t minInterval = isqrt(stopNumber_) / 6;
  // do not use multiple threads for small intervals
  if (minInterval < static_cast<uint64_t> (1E8))
    minInterval = static_cast<uint64_t> (1E8);

  uint64_t threads = (stopNumber_ - startNumber_) / minInterval;
  if (threads < 1)
    return 1;
  // use the maximum number of threads for big sieve intervals
  if (threads > static_cast<uint64_t> (this->getMaxThreads()))
    return this->getMaxThreads();
  // use less threads for small sieve intervals
  return static_cast<int> (threads);
}

/**
 * Set the number of threads for sieving.
 * If numThreads is not valid (i.e. numThreads < 1 or 
 * > getMaxThreads()) numThreads is set to USE_IDEAL_NUM_THREADS.
 */
void ParallelPrimeSieve::setNumThreads(int numThreads) {
  numThreads_ = (numThreads < 1 || numThreads > this->getMaxThreads())
      ? USE_IDEAL_NUM_THREADS
      : numThreads;
}

/**
 * For use with the Qt GUI version of primesieve, initializes
 * the ParallelPrimeSieve object with values from a shared memory
 * segment.
 */
void ParallelPrimeSieve::setSharedMemory(SharedMemoryPPS *sharedMemoryPPS) {
  if (sharedMemoryPPS == NULL)
    throw std::invalid_argument("shared memory segment must not be NULL");
  this->setStartNumber(sharedMemoryPPS->startNumber);
  this->setStopNumber(sharedMemoryPPS->stopNumber);
  this->setSieveSize(sharedMemoryPPS->sieveSize);
  this->setFlags(sharedMemoryPPS->flags);
  this->setNumThreads(sharedMemoryPPS->threads);
  // upon completion the sieving results will be communicated back to
  // the Qt GUI process via the shared memory
  sharedMemoryPPS_ = sharedMemoryPPS;
}

/**
 * Print the status (in percent) of the sieving process to the
 * standard output.
 */
void ParallelPrimeSieve::doStatus(uint64_t segment) {
#if defined(_OPENMP)
  #pragma omp critical (doStatus)
#endif
  {
    PrimeSieve::doStatus(segment);
    // communicate the current status via shared memory
    // to the Qt GUI process
    if (sharedMemoryPPS_ != NULL)
      sharedMemoryPPS_->status = status_;
  }
}

/**
 * Sieve the prime numbers and/or prime k-tuplets between startNumber
 * and stopNumber in parallel using multiple threads (OpenMP).
 */
void ParallelPrimeSieve::sieve() {
  if (stopNumber_ < startNumber_)
    throw std::invalid_argument("STOP must be >= START");
  // get the number of threads for sieving
  int numThreads = this->getNumThreads();
  // 1 thread to print primes in sequential order
  if (numThreads > 1 && (flags_ & PRINT_FLAGS))
    throw std::invalid_argument(
        "printing is only allowed using a single thread");

#if defined(_OPENMP)
  uint64_t threadInterval = (stopNumber_ - startNumber_) /
      static_cast<unsigned> (numThreads);
  if (threadInterval < 60 && numThreads > 1)
    throw std::invalid_argument(
        "use at least an interval of 60 for each thread");
  timeElapsed_ = omp_get_wtime();
  this->reset();
  // the threadInterval must be a multiple of 30
  if (threadInterval % 30 != 0)
    threadInterval += 30 - (threadInterval % 30);
  // calculate the start and stop number of the first thread
  uint64_t tStart = startNumber_;
  uint64_t tStop  = tStart + threadInterval;
  // thread stop numbers must be of type n*30+1
  if (tStop % 30 != 0)
    tStop += 30 - (tStop % 30);
  tStop += 1;

  // each thread uses its own PrimeSieve object to sieve
  // a sub-interval of the overall interval
  std::vector<PrimeSieve*> primeSieve;
  while (tStop < stopNumber_) {
    primeSieve.push_back(new PrimeSieve(tStart, tStop, this));
    tStart = tStop + 1;
    tStop += threadInterval;
  }
  primeSieve.push_back(new PrimeSieve(tStart, stopNumber_, this));

  // start parallel sieving
  #pragma omp parallel for num_threads(numThreads)
  for (int i = 0; i < static_cast<int> (primeSieve.size()); i++) {
    primeSieve[i]->sieve();
    for (int j = 0; j < COUNTS_SIZE; j++) {
      #pragma omp atomic
      counts_[j] += primeSieve[i]->getCounts(j);
    }
    delete primeSieve[i];
  }
  timeElapsed_ = omp_get_wtime() - timeElapsed_;
#else // #if !defined(_OPENMP) use single-threaded PrimeSieve
  PrimeSieve::sieve();
#endif
  // communicate the sieving results via shared memory to the
  // Qt GUI process
  if (sharedMemoryPPS_ != NULL) {
    for (int i = 0; i < COUNTS_SIZE; i++)
      sharedMemoryPPS_->counts[i] = counts_[i];
    sharedMemoryPPS_->timeElapsed = timeElapsed_;
  }
}
