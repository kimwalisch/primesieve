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

/**
 * Is used to reset the SieveOfEratosthenes::sieve_ array after each
 * sieve round.
 * Instead of just using @code std::memset(sieve_, 0xff, sieveSize_)
 * @endcode to set the bits of the sieve_ array to 1 ResetSieve 
 * copies a buffer to sieve_ to reset it. In this buffer all
 * multiples of prime numbers <= eliminateUpTo_ (default 19) have
 * been eliminated previously and thus SieveOfEratosthenes does not
 * need to consider these small primes for sieving.
 * The performance benefit of ResetSieve vs. memset is up to 20% when
 * sieving < 10^10.
 */
class ResetSieve {
public:
  ResetSieve(uint32_t);
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
  const uint32_t eliminateUpTo_;
  /**
   * Array used to reset (set bits to 1) the sieve_ array of
   * SieveOfEratosthenes after each sieve round.
   */
  uint8_t* resetBuffer_;
  /** Size of the resetBuffer_ array. */
  uint32_t size_;
  void setSize(uint32_t);
  void initResetBuffer();
};

#endif /* RESETSIEVE_H */
