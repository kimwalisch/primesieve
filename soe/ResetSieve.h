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

#include <stdint.h>

class PrimeSieve;

/**
 * ResetSieve is used to reset a SieveOfEratosthenes::sieve_ array
 * i.e. set all bits to 1.
 * ResetSieve therefore creates a modulo wheel array of size 
 * prime_product(p)/30 in which all multiples of primes <= p are
 * crossed off. After each sieve round the modulo wheel array is
 * copied to the sieve array to reset it. This does not only reset the
 * sieve array but also eliminates the multiples of primes <= p.
 * The performance benefit of a modulo wheel array vs.
 * std::memset(sieve_, 0xff, sieveSize_) is up to 20% when
 * sieving < 10^10.
 */
class ResetSieve {
public:
  ResetSieve(PrimeSieve*);
  ~ResetSieve();
  uint32_t getResetIndex(uint64_t) const;
  uint32_t getEliminateUpTo() const {
    return eliminateUpTo_;
  }
  void reset(uint8_t*, uint32_t, uint32_t*);
private:
  /**
   * All multiples of prime numbers <= eliminateUpTo_ will be
   * eliminated in resetBuffer_.
   */
  uint32_t eliminateUpTo_;
  /**
   * Array used to reset (set bits to 1) the sieve_ array of
   * SieveOfEratosthenes objects after each sieve round.
   */
  uint8_t* resetBuffer_;
  /** Size of the resetBuffer_ array. */
  uint32_t size_;
  void setSize(uint32_t);
  void initResetBuffer();
};

#endif /* RESETSIEVE_H */
