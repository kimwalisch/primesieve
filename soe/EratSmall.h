/*
 * EratSmall.h -- This file is part of primesieve
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

#ifndef ERATSMALL_H
#define ERATSMALL_H

#include "WheelFactorization.h"
#include "EratBase.h"

#include <stdint.h>

/**
 * Implementation of the segmented sieve of Eratosthenes with wheel
 * factorization (modulo 30 wheel) and 30 numbers per byte.
 * This algorithm is very fast for sieving primes that have a lot of
 * multiple occurrences per segment.
 * The algorithm is similar to the one of Achim Flammenkamp which is
 * described at:
 * http://wwwhomes.uni-bielefeld.de/achim/prime_sieve.html
 */
class EratSmall: public EratBase<Modulo30Wheel> {
public:
  EratSmall(uint32_t, uint64_t, uint32_t);
  void sieve(uint8_t*, uint32_t);
};

#endif /* ERATSMALL_H */
