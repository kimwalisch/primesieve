///
/// @file PreSieve_default.hpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
/// Copyright (C) 2022 @zielaj, https://github.com/zielaj
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRESIEVE_DEFAULT_HPP
#define PRESIEVE_DEFAULT_HPP

#include <stdint.h>
#include <cstddef>
#include <cstring>

namespace {

void presieve1_default(const uint8_t* __restrict preSieved0,
                       const uint8_t* __restrict preSieved1,
                       const uint8_t* __restrict preSieved2,
                       const uint8_t* __restrict preSieved3,
                       uint8_t* __restrict sieve,
                       std::size_t bytes)
{
  std::size_t limit = bytes - bytes % sizeof(uint64_t);

  // Process 8 bytes at a time.
  // std::memcpy is required to avoid unaligned memory
  // accesses which would cause undefined behavior.
  for (std::size_t i = 0; i < limit; i += sizeof(uint64_t))
  {
    uint64_t a, b, c, d;
    std::memcpy(&a, &preSieved0[i], sizeof(uint64_t));
    std::memcpy(&b, &preSieved1[i], sizeof(uint64_t));
    std::memcpy(&c, &preSieved2[i], sizeof(uint64_t));
    std::memcpy(&d, &preSieved3[i], sizeof(uint64_t));

    uint64_t result = a & b & c & d;
    std::memcpy(&sieve[i], &result, sizeof(uint64_t));
  }

  // Process the remaining bytes
  for (std::size_t i = limit; i < bytes; i++)
    sieve[i] = preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
}

void presieve2_default(const uint8_t* __restrict preSieved0,
                       const uint8_t* __restrict preSieved1,
                       const uint8_t* __restrict preSieved2,
                       const uint8_t* __restrict preSieved3,
                       uint8_t* __restrict sieve,
                       std::size_t bytes)
{
  std::size_t limit = bytes - bytes % sizeof(uint64_t);

  // Process 8 bytes at a time.
  // std::memcpy is required to avoid unaligned memory
  // accesses which would cause undefined behavior.
  for (std::size_t i = 0; i < limit; i += sizeof(uint64_t))
  {
    uint64_t a, b, c, d, e;
    std::memcpy(&a, &preSieved0[i], sizeof(uint64_t));
    std::memcpy(&b, &preSieved1[i], sizeof(uint64_t));
    std::memcpy(&c, &preSieved2[i], sizeof(uint64_t));
    std::memcpy(&d, &preSieved3[i], sizeof(uint64_t));
    std::memcpy(&e, &sieve[i], sizeof(uint64_t));

    uint64_t result = a & b & c & d & e;
    std::memcpy(&sieve[i], &result, sizeof(uint64_t));
  }

  // Process the remaining bytes
  for (std::size_t i = limit; i < bytes; i++)
    sieve[i] &= preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
}

} // namespace

#endif
