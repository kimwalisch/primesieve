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

#ifndef PARALLELPRIMESIEVE_H
#define PARALLELPRIMESIEVE_H

#include "PrimeSieve.h"
#include "config.h"

#include <stdint.h>

/**
 * ParallelPrimeSieve is a multi-threaded implementation of the
 * segmented sieve of Eratosthenes using OpenMP that generates primes
 * and prime k-tuplets (twins, triplets, ...) up to 2^64 maximum.
 * The file primesieve/README describes the algorithms used in more
 * detail, the file primesieve/docs/USAGE_EXAMPLES contains source
 * code examples that show how to use ParallelPrimeSieve objects.

 * @warning ParallelPrimeSieve does not generate primes and prime
 *          k-tuplets in arithmetic order.
 */
class ParallelPrimeSieve: public PrimeSieve {
public:
  using PrimeSieve::sieve;
  /**
   * Used in the primesieve Qt application (../qt-gui)
   * to handle the communication between the GUI process
   * and the ParallelPrimeSieve process.
   */
  struct SharedMemory {
    uint64_t start;
    uint64_t stop;
    uint32_t sieveSize;
    uint32_t flags;
    int threads;
    uint64_t counts[COUNTS_SIZE];
    double status;
    double timeElapsed;
  };
  /*
   * if (numThreads_ == IDEAL_NUM_THREADS) an ideal number
   * of threads will be used for sieving.
   */
  enum { IDEAL_NUM_THREADS = -1 };
  ParallelPrimeSieve();
  void init(SharedMemory*);
  static int getMaxThreads();
  int getNumThreads() const;
  void setNumThreads(int numThreads);
  virtual void sieve();
private:
  /** Number of threads to be used for sieving. */
  int numThreads_;
  SharedMemory* shm_;
  virtual void calcStatus(uint32_t);
  int getIdealNumThreads() const;
  uint64_t getBalancedInterval(int) const;
  void sieveThread(uint64_t, uint64_t);
};

#endif /* PARALLELPRIMESIEVE_H */
