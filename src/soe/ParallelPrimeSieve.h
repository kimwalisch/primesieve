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
#include <stdint.h>

/// ParallelPrimeSieve sieves primes in parallel, it is
/// derived from PrimeSieve so it has the same API.
/// Please refer to doc/USAGE_EXAMPLES for more information.
///
class ParallelPrimeSieve : public PrimeSieve {
public:
  /// Used for inter-process communication with the
  /// primesieve GUI application.
  struct SharedMemory {
    uint64_t start;
    uint64_t stop;
    uint64_t counts[7];
    double status;
    double seconds;
    int flags;
    int sieveSize;
    int threads;
  };
  ParallelPrimeSieve();
  virtual ~ParallelPrimeSieve() { }
  void init(SharedMemory&);
  static int getMaxThreads();
  int getNumThreads() const;
  void setNumThreads(int numThreads);
  using PrimeSieve::sieve;
  virtual void sieve();
private:
  enum {
    DEFAULT_NUM_THREADS = -1
  };
  SharedMemory* shm_;
  /// Number of threads for sieving
  int numThreads_;
  /// Used to synchronize threads
  void* lock_;
  bool tooMany(int) const;
  int idealNumThreads() const;
  uint64_t getThreadInterval(int) const;
  template <typename T>
  T getLock() {
    return static_cast<T> (lock_);
  }
  virtual void setLock();
  virtual void unsetLock();
  virtual bool updateStatus(uint64_t, bool);
};

#endif
