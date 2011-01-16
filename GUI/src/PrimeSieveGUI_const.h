/*
 * PrimeSieveGUI_const.h -- This file is part of primesieve
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

/**
 * @file PrimeSieveGUI_const.h
 * @brief Various constants used in PrimeSieveGUI.
 * The values are optimized for CPUs with 32 to 64 KiloBytes of L1
 * Data Cache (set in 2010).
 */

#ifndef PRIMESIEVEGUI_CONST_H
#define PRIMESIEVEGUI_CONST_H

#include <QString>
#include <QtGlobal>
#include <stdint.h>

const QString APPLICATION_NAME("primesieve");
const QString APPLICATION_VERSION("1.1");
const QString APPLICATION_HOMEPAGE("http://primesieve.googlecode.com");
const QString APPLICATION_ABOUT(
    "primesieve uses a highly optimized implementation of the sieve of "
    "Eratosthenes to find prime numbers and prime k-tuplets. By the "
    "date of release primesieve is the fastest publicly available "
    "sieve of Eratosthenes program (together with YAFU).<br>"
    "<br>"
    "This is free software: you can redistribute it and/or modify "
    "it under the terms of the GNU General Public License as published by "
    "the Free Software Foundation; either version 3 of the License, or "
    "(at your option) any later version.");

/**
 * Minimum sieve size in KiloBytes.
 * The minimum sieve size should not be much smaller than the
 * systems CPU L1 cache size for performance reasons.
 * @pre MINIMUM_SIEVE_SIZE >= 1
 * @see PrimeSieve.cpp
 */
const int MINIMUM_SIEVE_SIZE = 16;
/**
 * @pre MAXIMUM_SIEVE_SIZE <= 8192
 * @see PrimeSieve.cpp
 */
const int MAXIMUM_SIEVE_SIZE = 8192;
/**
 * The best performance is achieved with a sieve size of the CPU's
 * L1 or L2 cache size. 64 KiloBytes is a good choice as most CPUs
 * have a L1 cache size of 32 to 64 KiloBytes in 2010.
 * @pre DEFAULT_SIEVE_SIZE must be a power of 2.
 */
const int DEFAULT_SIEVE_SIZE = 64;
/**
 * Default value of CPU Cores if the CPU has not been detected.
 * @see PrimeSieveGUI::initMemberVariables()
 */
const int DEFAULT_MAX_CPU_CORES = 512;
/**
 * PrimeSieve allows numbers up to < (2^64-1) - (2^32-1) * 10.
 * @see PrimeSieve.cpp
 */
const quint64 UPPER_BOUND_LIMIT = UINT64_MAX - UINT32_MAX * Q_UINT64_C(10);
/**
 * Use at least an interval of 10^8 (by test) for each CPU core.
 */
const quint64 MINIMUM_THREAD_INTERVAL = Q_UINT64_C(100000000);
/**
 * Print chunks of PRINT_BUFFER_SIZE bytes to the TextEdit.
 */
const int PRINT_BUFFER_SIZE = 1024;

#endif // PRIMESIEVEGUI_CONST_H
