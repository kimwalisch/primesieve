/*
 * EratMedium.h -- This file is part of primesieve
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

#ifndef ERATMEDIUM_H
#define ERATMEDIUM_H

#include "EratBase.h"
#include "WheelFactorization.h"
#include "defs.h"

class SieveOfEratosthenes;

/**
 * Implementation of the segmented sieve of Eratosthenes with wheel
 * factorization (modulo 210 wheel) and 30 numbers per byte.
 * This algorithm is faster than EratSmall for sieving primes that do
 * not have a lot of multiple occurrences per segment, it uses
 * less jump operations (switch, goto).
 */
class EratMedium: public EratBase<Modulo210Wheel> {
public:
  EratMedium(uint32_t, const SieveOfEratosthenes*);
  void sieve(uint8_t*, uint32_t);
};

#endif /* ERATMEDIUM_H */
