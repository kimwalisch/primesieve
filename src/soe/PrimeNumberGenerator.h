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

#ifndef PRIMENUMBERGENERATOR_H
#define PRIMENUMBERGENERATOR_H

#include "config.h"
#include "PrimeNumberGenerator.h"
#include "PrimeNumberFinder.h"
#include "SieveOfEratosthenes.h"
#include "SieveOfEratosthenes-inline.h"
#include "GENERATE.h"

#include <stdint.h>
#include <cassert>

namespace soe {

/// PrimeNumberGenerator is a SieveOfEratosthenes class that is used
/// to generate the primes up to sqrt(n) needed for sieving by
/// PrimeNumberFinder.
///
class PrimeNumberGenerator : public SieveOfEratosthenes {
public:
  PrimeNumberGenerator(PrimeNumberFinder& finder) :
    SieveOfEratosthenes(
      finder.getPreSieve() + 1,
      finder.getSqrtStop(),
      config::SIEVESIZE,
      config::PRESIEVE_PRIMENUMBERGENERATOR),
    finder_(finder)
  {
    static_assert(config::SIEVESIZE <= 4096, "sieveSize must not be > 4096 kilobytes");
    assert(getStop() <= ~0u);
  }
private:
  PrimeNumberFinder& finder_;
  /// Executed after each sieved segment, generates the primes within
  /// the current segment and uses them to sieve with finder_.
  /// @see GENERATE.h
  ///
  void segmentProcessed(const uint8_t* sieve, uint_t sieveSize) {
    GENERATE_PRIMES(finder_.sieve, uint_t);
  }
};

} // namespace soe

#endif
