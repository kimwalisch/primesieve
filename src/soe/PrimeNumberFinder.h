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

#ifndef PRIMENUMBERFINDER_H
#define PRIMENUMBERFINDER_H

#include "config.h"
#include "SieveOfEratosthenes.h"
#include "SieveOfEratosthenes-inline.h"

#include <stdint.h>
#include <vector>

class PrimeSieve;

namespace soe {

/// PrimeNumberFinder is a SieveOfEratosthenes class that is used to
/// generate, count and print primes and prime k-tuplets
/// (twin primes, prime triplets, ...).
///
class PrimeNumberFinder : public SieveOfEratosthenes {
public:
  PrimeNumberFinder(PrimeSieve&);
private:
  enum { END = 0xFF + 1 };
  static const uint_t kBitmasks_[7][5];
  /// Reference to the friend PrimeSieve object
  PrimeSieve& ps_;
  /// Lookup tables that give the count of prime k-tuplets
  /// (twin primes, prime triplets, ...) per byte.
  std::vector<uint_t> kCounts_[7];
  void init_kCounts();
  virtual void segmentProcessed(const uint8_t*, uint_t);
  void count(const uint8_t*, uint_t);
  void generate(const uint8_t*, uint_t) const;
  void callback32_obj(uint32_t) const;
  void callback64_obj(uint64_t) const;
  void callback64_int(uint64_t) const;
  static void print(uint64_t);
  DISALLOW_COPY_AND_ASSIGN(PrimeNumberFinder);
};

} // namespace soe

#endif
