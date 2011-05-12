/*
 * ResetSieve.h -- This file is part of primesieve
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

#ifndef RESETSIEVE_H
#define RESETSIEVE_H

#include "defs.h"

class PrimeSieve;

/**
 * ResetSieve is used to reset the sieve_ array of SieveOfEratosthenes
 * objects after each sieved segment (i.e. reset bits to 1).
 * ResetSieve therefore creates a modulo wheel array of size
 * primeProduct(limit)/30 in which all multiples of primes <= limit
 * are crossed-off. After each sieved segment the modulo wheel array
 * is copied to the sieve array to reset it (i.e. reset bits to 1),
 * this also eliminates multiples of primes <= limit for the next
 * segment without sieving. The performance benefit of a modulo wheel
 * array vs. std::memset(sieve_, 0xff, sieveSize_) is up to 20% when
 * sieving < 10^10.
 * @see http://en.wikipedia.org/wiki/Wheel_factorization
 */
class ResetSieve {
public:
  ResetSieve(PrimeSieve*);
  ~ResetSieve();
  uint32_t getResetIndex(uint64_t) const;
  uint32_t getLimit() const {
    return limit_;
  }
  void reset(uint8_t*, uint32_t, uint32_t*);
private:
  /**
   * All multiples of prime numbers <= limit_ will be crossed-off in
   * the moduloWheel_ array.
   */
  uint32_t limit_;
  /**
   * Modulo wheel array used to reset (set bits to 1) the sieve_ array
   * of SieveOfEratosthenes objects after each sieved segment.
   * @see http://en.wikipedia.org/wiki/Wheel_factorization
   */
  uint8_t* moduloWheel_;
  /** Size of the moduloWheel_ array. */
  uint32_t size_;
  void setSize(uint32_t);
  void initResetBuffer();
};

#endif /* RESETSIEVE_H */
