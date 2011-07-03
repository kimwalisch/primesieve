/*
 * ParallelPrimeSieve.h -- This file is part of primesieve
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

#ifndef PARALLELPRIMESIEVE_H
#define PARALLELPRIMESIEVE_H

#include "PrimeSieve.h"
#include "defs.h"

/**
 * ParallelPrimeSieve is a parallel implementation of the segmented
 * sieve of Eratosthenes using OpenMP.
 * The parallelization is achieved using multiple threads, each thread
 * sieves chunks of the interval [startNumber_, stopNumber_] using
 * PrimeSieve objects until the entire interval has been processed.
 * This approach scales well on multi-core CPUs but the memory usage
 * depends on the number of threads i.e. O(n^0.5) * number of threads,
 * and the primes are not generated or printed in order.
 *
 * == Usage ==
 *
 * The file ../docs/USAGE_EXAMPLES contains source code examples that
 * show how to use PrimeSieve and ParallelPrimeSieve objects to
 * generate primes, count primes, print prime triplets, ...
 *
 * == Memory Requirement ==
 *
 * ParallelPrimeSieve::sieve() uses about:
 * (pi(n^0.5) * 8 Bytes + 500 Kilobytes) * number of threads
 */
class ParallelPrimeSieve: public PrimeSieve {
public:
  /**
   * Used in the Qt primesieve application (../qt-gui) in order to
   * handle the communication between the GUI process and the
   * ParallelPrimeSieve process.
   */
  struct SharedMemory {
    uint64_t startNumber;
    uint64_t stopNumber;
    uint32_t sieveSize;
    uint32_t flags;
    int threads;
    uint64_t counts[COUNTS_SIZE];
    double status;
    double timeElapsed;
  };
  enum {
    /*
     * Use an ideal number of threads for the current set
     * startNumber_, stopNumber_ and flags_.
     */
    USE_IDEAL_NUM_THREADS = -1
  };
  ParallelPrimeSieve();
  static int getMaxThreads();
  int getNumThreads() const;
  void setNumThreads(int numThreads);
  void init(SharedMemory*);
  virtual void sieve();
private:
  /** Number of threads for sieving. */
  int numThreads_;
  /** Each thread sieves at least an interval of minThreadInterval_. */
  uint64_t minThreadInterval_;
  /**
   * Pointer to a shared memory segment, for use with the Qt
   * primesieve application (../qt-gui).
   */
  SharedMemory* shm_;
  uint64_t getSieveInterval() const;
  int getIdealNumThreads() const;
  uint64_t getIdealInterval() const;
  void setMinThreadInterval(uint64_t);
  virtual void doStatus(uint32_t);
};

#endif // PARALLELPRIMESIEVE_H
