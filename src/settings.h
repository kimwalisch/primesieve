/*
 * settings.h -- This file is part of primesieve
 *
 * Copyright (C) 2010 Kim Walisch, <kim.walisch@gmail.com>
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

/** 
 * @file settings.h 
 * @brief Constants that set the size of various arrays and upper
 *        bounds of primesieve.
 * The values are optimized for CPUs with 32 to 64 KiloBytes of L1
 * Data Cache.
 */

#ifndef SETTINGS_H
#define SETTINGS_H

namespace settings {
  /**
   * (sieveSize * SIEVESIZE_FACTOR_ERATSMALL) is the upper bound for
   * sieving primes used with EratSmall.
   * Default = 1.5
   * @pre < 5 && <= SIEVESIZE_FACTOR_ERATMEDIUM
   */
  const double SIEVESIZE_FACTOR_ERATSMALL = 1.5;
  enum {
    /**
     * Multiples of prime numbers up to this number will be eliminated
     * without sieving.
     * Default = 19 (uses 315.7 KiloBytes)
     * @pre >= 7 && <= 23
     */
    PREELIMINATE_RESETSIEVE = 19,
    /**
     * Sieve size of the sieve of Eratosthenes that generates the prime
     * numbers up to sqrt(stopNumber) needed for sieving.
     * Default = CPU L1 Cache size
     * @pre >= 1024 && <= 2^23
     */
    SIEVESIZE_PRIMENUMBERGENERATOR = 32 * 1024,
    /**
     * Default sieve size of the main sieve of Eratosthenes
     * implementation, is used if the user does not set his own sieve
     * size.
     * Default = CPU L1 or L2 Cache size
     * @pre >= 1024 && <= 2^23
     */
    DEFAULT_SIEVESIZE_PRIMENUMBERFINDER = 64 * 1024,
    /**
     * (sieveSize * SIEVESIZE_FACTOR_ERATMEDIUM) is the upper bound for
     * sieving primes used with EratMedium.
     * Default = 9
     * @pre >= SIEVESIZE_FACTOR_ERATSMALL
     */
    SIEVESIZE_FACTOR_ERATMEDIUM = 9,
    /**
     * Size of the bucket array within EratBase, buckets are used to
     * store sieving primes.
     * Default = 4096 (32 KiloBytes)
     */
    BUCKETSIZE_ERATBASE = 1 << 12,
    /**
     * Size of the bucket array within EratBig, buckets are used to
     * store sieving primes.
     * Default = 1024 (8 KiloBytes)
     */
    BUCKETSIZE_ERATBIG = 1 << 10
  };
}
#endif /* SETTINGS_H */
