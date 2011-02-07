/*
 * bits.h -- This file is part of primesieve
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

#ifndef BITS_H
#define BITS_H

/**
 * Bitmasks used with the bitwise '&' operator to unset specific bits
 * of the SieveOfEratosthenes::sieve_ array.
 */
enum {
    BIT0 = 0xfe, // 1111.1110
    BIT1 = 0xfd, // 1111.1101
    BIT2 = 0xfb, // 1111.1011
    BIT3 = 0xf7, // 1111.0111
    BIT4 = 0xef, // 1110.1111
    BIT5 = 0xdf, // 1101.1111
    BIT6 = 0xbf, // 1011.1111
    BIT7 = 0x7f  // 0111.1111
};

#endif /* BITS_H */

