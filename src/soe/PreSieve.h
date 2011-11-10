//
// Copyright (c) 2011 Kim Walisch, <kim.walisch@gmail.com>.
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

#ifndef PRESIEVE_H
#define PRESIEVE_H

#include "defs.h"

/**
 * PreSieve objects are used to pre-sieve multiples of small primes
 * <= limit_ to speed up the sieve of Eratosthenes.
 * The idea is to allocate an array (preSieved_) and remove the
 * multiples of small primes e.g. <= 19 from it at initialization.
 * Whilst sieving the preSieved_ array is copied to the
 * SieveOfEratosthenes sieve at the beginning of each new segment to
 * pre-sieve the multiples of small primes <= limit_.
 * Pre-sieving speeds up my sieve of Eratosthenes implementation by
 * about 20 percent when sieving < 1E10.
 *
 * Pre-sieving multiples of small primes is described in more detail
 * in Joerg Richstein's German doctoral thesis:
 * "Segmentierung und Optimierung von Algorithmen zu Problemen aus der Zahlentheorie", Gießen, Univ., Diss., 1999
 * 3.3.5 Vorsieben kleiner Primfaktoren
 * http://geb.uni-giessen.de/geb/volltexte/1999/73/pdf/RichsteinJoerg-1999-08-06.pdf
 *
 * == Memory Usage ==
 * 
 * PreSieve objects use: primeProduct(limit_) / 30 bytes of memory
 * PreSieve multiples of primes <= 13 uses 1001    bytes
 * PreSieve multiples of primes <= 17 uses   16.62 kilobytes
 * PreSieve multiples of primes <= 19 uses  315.75 kilobytes
 * PreSieve multiples of primes <= 23 uses    7.09 megabytes
 */
class PreSieve {
public:
  PreSieve(uint32_t);
  ~PreSieve();
  uint32_t getLimit() const {
    return limit_;
  }
  void doIt(uint8_t*, uint32_t, uint64_t) const;
private:
  static const uint32_t smallPrimes_[10];
  static const uint32_t unsetBits_[30];
  /** Multiples of small primes <= limit_ (MAX 23) are pre-sieved. */
  const uint32_t limit_;
  /** Product of the primes <= limit_. */
  uint32_t primeProduct_;
  /**
   * Array of size primeProduct(limit_) / 30 in which multiples of
   * small primes <= limit_ are crossed-off at initialization.
   */
  uint8_t* preSieved_;
  /** Size of preSieved_ in bytes. */
  uint32_t size_;
  uint32_t getPrimeProduct(uint32_t) const;
  void initPreSieved();
  /** Uncopyable, declared but not defined. */
  PreSieve(const PreSieve&);
  PreSieve& operator=(const PreSieve&);
};

#endif /* PRESIEVE_H */
