/*
 * PrimeSieveGUI_const.hpp -- This file is part of primesieve
 *
 * Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef PRIMESIEVEGUI_CONST_HPP
#define PRIMESIEVEGUI_CONST_HPP

#include <QString>
#include <QtGlobal>

/**
 * @def __STDC_LIMIT_MACROS
 * Enable the UINT64_MAX, UINT32_MAX macros from <stdint.h>.
 */
#if !defined(__STDC_LIMIT_MACROS)
  #define __STDC_LIMIT_MACROS
#endif

#include <stdint.h>

const QString APPLICATION_NAME("primesieve");
const QString APPLICATION_HOMEPAGE("http://primesieve.org");
const QString APPLICATION_ABOUT(
    "<p>Copyright &copy; 2016 Kim Walisch</p>"
    "<p>primesieve generates prime numbers and prime k-tuplets using a highly "
    "optimized implementation of the sieve of Eratosthenes. By the date "
    "of release this is the fastest publicly available prime generation software."
    "<br><br>"
    "This is free software: you can redistribute it and/or modify "
    "it under the terms of the GNU General Public License as published by "
    "the Free Software Foundation; either version 3 of the License, or "
    "(at your option) any later version.</p>");

/**
 * Minimum sieve size in kilobytes.
 * For performance reasons the minimum sieve size should not be much
 * smaller than the system's CPU L1 cache size.
 * @pre MINIMUM_SIEVE_SIZE >= 1
 * @see PrimeSieve.cpp
 */
const int MINIMUM_SIEVE_SIZE = 16;

/**
 * @pre MAXIMUM_SIEVE_SIZE <= 2048
 * @see PrimeSieve.cpp
 */
const int MAXIMUM_SIEVE_SIZE = 2048;

/**
 * @pre DEFAULT_L1D_CACHE_SIZE >= MINIMUM_SIEVE_SIZE &&
 *      DEFAULT_L1D_CACHE_SIZE <= MAXIMUM_SIEVE_SIZE
 */
const int DEFAULT_L1D_CACHE_SIZE = 32;

/**
 * PrimeSieve allows numbers up to < 2^64.
 */
const quint64 UPPER_BOUND_LIMIT = UINT64_MAX;
const QString UPPER_BOUND_STR = QString::number(UPPER_BOUND_LIMIT);

/**
 * Print chunks of PRINT_BUFFER_SIZE bytes to the TextEdit.
 */
const int PRINT_BUFFER_SIZE = 1024;

#endif // PRIMESIEVEGUI_CONST_H
