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
 * ResetSieve is used to reset the sieve array (set bits to 1) of
 * SieveOfEratosthenes objects after each sieved segment and to remove
 * the multiples of small primes <= limit_ without sieving.
 * The idea is to create a wheel array in which multiples of small
 * primes are crossed off during initialization.
 * After each sieved segment the wheel array is copied to the sieve in
 * order to reset it and remove the multiples of small primes.
 * Pre-sieving multiples of small primes (e.g. <= 19) speeds up my
 * sieve of Eratosthenes implementation by about 20 percent when
 * sieving < 10^10.
 *
 * Pre-sieving multiples of small primes is described in more detail
 * in Joerg Richstein's German doctoral thesis:
 * "Segmentierung und Optimierung von Algorithmen zu Problemen aus der Zahlentheorie", Gießen, Univ., Diss., 1999
 * 3.3.5 Vorsieben kleiner Primfaktoren
 * http://geb.uni-giessen.de/geb/volltexte/1999/73/pdf/RichsteinJoerg-1999-08-06.pdf
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
   * Multiples of small primes <= limit_ (MAX 23) are crossed off in
   * the moduloWheel_ array.
   */
  uint32_t limit_;
  /**
   * Modulo wheel array of size primeProduct(limit_)/30 in which
   * multiples of small primes <= limit_ are crossed off.
   */
  uint8_t* moduloWheel_;
  /** Size of the moduloWheel_ array. */
  uint32_t size_;
  void setSize(uint32_t);
  void initResetBuffer();
};

#endif /* RESETSIEVE_H */
