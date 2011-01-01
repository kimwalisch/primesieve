/*
 * strtoull.h -- This file is part of primesieve
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

#ifndef STRTOULL_H
#define STRTOULL_H

#include <stdint.h>
#include <cstring>

#define STR_UINT64_MAX "18446744073709551615"

namespace utils {
  /**                      
   * Protable implementation of the ISO C99 strtoull function, converts
   * a string to an unsigned long long integer of base 10.
   * @pre str must be NULL terminated.
   */
  inline uint64_t strtoull(const char* str) {
    size_t maxLen = std::strlen(STR_UINT64_MAX);
    size_t length = std::strlen(str);
    if (length > maxLen)
      return 0;
    if (length == maxLen && std::strcmp(str, STR_UINT64_MAX) > 0)
      return 0;
    uint64_t n = 0;
    uint64_t base10_offset = 1;
    for (int i = static_cast<int> (length - 1); i >= 0; i--) {
      if (str[i] < '0' || str[i] > '9')
        return 0;
      n += (str[i] - '0') * base10_offset;
      base10_offset *= 10;
    }
    return n;
  }
}

#endif /* STRTOULL_H */

