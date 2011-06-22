/*
 * PreSieve.h -- This file is part of primesieve
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

#ifndef PRESIEVE_H
#define PRESIEVE_H

#include "defs.h"

/**
 * PreSieve objects are used to reset the sieve array (set bits to 1)
 * of SieveOfEratosthenes objects after each sieved segment and to
 * pre-sieve multiples of small primes <= limit_.
 * The idea is to create a wheel array in which multiples of small
 * primes are crossed off at initialization.
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
 *
 * == Memory Usage ==
 * 
 * PreSieve uses: primeProduct(limit_)/30 Bytes of memory.
 *
 * PreSieve multiples of primes <= 11 uses 77 Bytes
 * PreSieve multiples of primes <= 13 uses 1001 Bytes
 * PreSieve multiples of primes <= 17 uses 16.62 Kilobytes
 * PreSieve multiples of primes <= 19 uses 315.75 KiloBytes
 * PreSieve multiples of primes <= 23 uses 7.09 Megabytes
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
  /** Multiples of small primes <= limit_ (MAX 23) are pre-sieved. */
  uint32_t limit_;
  /** Product of the primes <= limit_. */
  uint32_t primeProduct_;
  /**
   * Wheel array of size primeProduct(limit_)/30 in which multiples of
   * small primes <= limit_ are crossed off.
   */
  uint8_t* wheelArray_;
  /** Size of wheelArray_. */
  uint32_t size_;
  void initWheelArray();
};

#endif /* PRESIEVE_H */
