/*
 * ParallelPrimeSieve.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2011 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "ParallelPrimeSieve.h"
#include "PrimeNumberFinder.h"
#include "pmath.h"

#if defined(_OPENMP)
#include <omp.h>
#endif

#include <stdint.h>
#include <stdexcept>
#include <sstream>
#include <iostream>
/**
 * @return The maximum number of threads (i.e. the number of logical
 *         CPU cores) or 1 if OpenMP is disabled.
 * @see    http://msdn.microsoft.com/en-us/library/ewb30w8w.aspx
 */
int ParallelPrimeSieve::getMaxThreads() {
#if defined(_OPENMP)
  int maxThreads = omp_get_max_threads();
  // check if the CPU has been detected correctly
  if (maxThreads < 1 || maxThreads > 4096)
    maxThreads = 1;
  return maxThreads;
#else
  return 1;
#endif
}

/**
 * @return The ideal thread count for the current set startNumber,
 *         stopNumber and flags (usually the number of logical
 *         CPU cores).
 */
int ParallelPrimeSieve::getIdealThreadCount() const {
  // printing is only allowed using a single thread
  if (flags_ & PRINT_FLAGS)
    return 1;
  // I made some tests around 10^19 which showed that each thread
  // should at least sieve an interval of sqrt(stopNumber) / 6 for a
  // performance benefit
  uint64_t min = U32SQRT(stopNumber_) / 6;
  // use at least an interval of 10^8 for each thread
  if (min < 100000000)
    min = 100000000;
  uint64_t threads = (stopNumber_ - startNumber_) / min;
  if (threads < 1)
    return 1;
  int maxThreads = getMaxThreads();
  // use all threads for big sieve intervals
  if (threads > static_cast<uint64_t> (maxThreads))
    return maxThreads;
  // use less threads for small sieve intervals
  return static_cast<int> (threads);
}

/**
 * Sieve the prime numbers and/or prime k-tuplets between startNumber
 * and stopNumber in parallel using an ideal number of threads.
 */
void ParallelPrimeSieve::sieve() {
  this->sieve(this->getIdealThreadCount());
}

/**
 * Sieve the prime numbers and/or prime k-tuplets between startNumber
 * and stopNumber in parallel using `threadCount' threads.
 * @pre threadCount >= 1 and <= getMaxThreads()
 */
void ParallelPrimeSieve::sieve(int threadCount) {
  if (threadCount < 1 || threadCount > getMaxThreads()) {
    std::ostringstream message;
    message << "threadCount for this CPU must be >= 1 and <= "
            << getMaxThreads();
    throw std::invalid_argument(message.str());
  }
  if (threadCount > 1 && (flags_ & PRINT_FLAGS))
    throw std::invalid_argument(
        "Printing is only allowed using a single thread");
  if (stopNumber_ < startNumber_)
    throw std::invalid_argument("STOP must be >= START");

#if defined(_OPENMP)
  timeElapsed_ = omp_get_wtime();

  // preliminaries
  uint64_t threadInterval = (stopNumber_ - startNumber_) /
      static_cast<unsigned> (threadCount);
  if (threadInterval < 60 && threadCount > 1)
    throw std::invalid_argument(
        "Use at least an interval of 60 for each thread");
  // the thread interval must be a multiple of 30
  if (threadInterval % 30 != 0)
    threadInterval += 30 - (threadInterval % 30);

  // calculate the start and stop number of the first thread
  uint64_t threadStart = startNumber_;
  uint64_t threadStop  = threadStart + threadInterval;
  // thread stop numbers must be of type n * 30 + 1
  if (threadStop % 30 != 0)
    threadStop += 30 - (threadStop % 30);
  threadStop += 1;

  // each thread uses its own PrimeSieve object to sieve
  // a sub-interval of the overall interval
  PrimeSieve* primeSieve = new PrimeSieve[threadCount];
  int i = 0;
  for (; i + 1 < threadCount && threadStop < stopNumber_; i++) {
    // set up the PrimeSieve object for thread number i
    primeSieve[i].setStartNumber(threadStart);
    primeSieve[i].setStopNumber(threadStop);
    primeSieve[i].setSieveSize(this->getSieveSize());
    primeSieve[i].setFlags(flags_ & (~PRINT_STATUS));
    threadStart = threadStop + 1;
    threadStop += threadInterval;
  }
  // initialize the PrimeSieve object of the last thread
  primeSieve[i].setStartNumber(threadStart);
  primeSieve[i].setStopNumber(stopNumber_);
  primeSieve[i].setSieveSize(this->getSieveSize());
  primeSieve[i].setFlags(flags_);
  ++i;

  // parallel prime sieving (OpenMP)
  #pragma omp parallel for num_threads(i)
  for (int j = 0; j < i; j++)
    primeSieve[j].sieve();

  // sum the results
  for (int k = 0; k < i; k++)
    for (int l = 0; l < COUNTS_SIZE; l++)
      counts_[l] += primeSieve[k].getCounts(l);
  delete[] primeSieve;

  timeElapsed_ = omp_get_wtime() - timeElapsed_;
#else
  // use single-threaded version if OpenMP is disabled
  PrimeSieve::sieve();
#endif
}
