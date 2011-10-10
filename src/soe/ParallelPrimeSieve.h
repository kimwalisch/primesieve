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
  /**
   * Pointer to a shared memory segment, for use with the Qt
   * primesieve application (../qt-gui).
   */
  SharedMemory* shm_;
  uint64_t getSieveInterval() const;
  int getIdealNumThreads() const;
  uint64_t getIdealInterval() const;
  virtual void doStatus(uint32_t);
};

#endif // PARALLELPRIMESIEVE_H
