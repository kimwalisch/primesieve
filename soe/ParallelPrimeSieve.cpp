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
#include "pmath.h"

#if defined(_OPENMP)
#include <omp.h>
#endif

#include <stdint.h>
#include <stdexcept>
#include <sstream>

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
  if (stopNumber_ < startNumber_)
    throw std::invalid_argument("STOP must be >= START");
  // 1 thread for sequential printing
  if (flags_ & PRINT_FLAGS)
    return 1;
  // I made some tests around 10^19 which showed that each
  // thread should at least sieve an interval of
  // sqrt(stopNumber) / 6 for a performance benefit
  uint64_t min = U32SQRT(stopNumber_) / 6;
  // do not use multiple-threads for small intervals
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
 * Print the status (in percent) of the sieving process to the
 * standard output.
 */
void ParallelPrimeSieve::doStatus(uint64_t segment) {
#if defined(_OPENMP)
  #pragma omp critical (doStatus)
  {
#endif
    PrimeSieve::doStatus(segment);
#if defined(_OPENMP)
  } // critical
#endif
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
  if (stopNumber_ < startNumber_)
    throw std::invalid_argument("STOP must be >= START");
  if (threadCount < 1 ||
      threadCount > getMaxThreads()) {
    std::ostringstream message;
    message << "use a number of threads >= 1 and <= "
            << getMaxThreads()
            << " for this CPU";
    throw std::invalid_argument(message.str());
  }
  if (threadCount > 1 && (flags_ & PRINT_FLAGS))
    throw std::invalid_argument(
        "printing is only allowed using a single thread");

#if defined(_OPENMP)
  uint64_t threadInterval = (stopNumber_ - startNumber_) /
      static_cast<unsigned> (threadCount);
  if (threadInterval < 60 && threadCount > 1)
    throw std::invalid_argument(
        "use at least an interval of 60 for each thread");

  timeElapsed_ = omp_get_wtime();
  this->reset();

  // threadInterval must be a multiple of 30
  if (threadInterval % 30 != 0)
    threadInterval += 30 - (threadInterval % 30);
  // calculate the start and stop number of the first thread
  uint64_t tStart = startNumber_;
  uint64_t tStop  = tStart + threadInterval;
  // thread stop numbers must be of type n * 30 + 1
  if (tStop % 30 != 0)
    tStop += 30 - (tStop % 30);
  tStop += 1;

  // each thread uses its own PrimeSieve object to sieve
  // a sub-interval of the overall interval
  PrimeSieve** primeSieve = new PrimeSieve*[threadCount];
  int i = 0;
  while (i + 1 < threadCount && tStop < stopNumber_) {
    primeSieve[i] = new PrimeSieve;
    primeSieve[i++]->set(tStart, tStop, this);
    tStart = tStop + 1;
    tStop += threadInterval;
  }
  primeSieve[i] = new PrimeSieve;
  primeSieve[i++]->set(tStart, stopNumber_, this);
  threadCount = i;

  // parallel prime sieving
  #pragma omp parallel for num_threads(threadCount)
  for (int j = 0; j < threadCount; j++) {
    primeSieve[j]->sieve();
    for (int k = 0; k < COUNTS_SIZE; k++) {
      #pragma omp atomic
      counts_[k] += primeSieve[j]->getCounts(k);
    }
    delete primeSieve[j];
  }

  delete[] primeSieve;
  timeElapsed_ = omp_get_wtime() - timeElapsed_;
#else
  // use single-threaded version if OpenMP is disabled
  PrimeSieve::sieve();
#endif
}
