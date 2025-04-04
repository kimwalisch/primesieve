///
/// @file PreSieve_arm_neon.hpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
/// Copyright (C) 2022 @zielaj, https://github.com/zielaj
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRESIEVE_ARM_NEON_HPP
#define PRESIEVE_ARM_NEON_HPP

#include <arm_neon.h>
#include <stdint.h>
#include <cstddef>

namespace {

void presieve1_arm_neon(const uint8_t* __restrict preSieved0,
                        const uint8_t* __restrict preSieved1,
                        const uint8_t* __restrict preSieved2,
                        const uint8_t* __restrict preSieved3,
                        uint8_t* __restrict sieve,
                        std::size_t bytes)
{
  std::size_t i = 0;
  std::size_t limit = bytes - bytes % sizeof(uint8x16_t);

  for (; i < limit; i += sizeof(uint8x16_t))
  {
    vst1q_u8(&sieve[i],
      vandq_u8(
        vandq_u8(vld1q_u8(&preSieved0[i]), vld1q_u8(&preSieved1[i])),
        vandq_u8(vld1q_u8(&preSieved2[i]), vld1q_u8(&preSieved3[i]))));
  }

  for (; i < bytes; i++)
    sieve[i] = preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
}

void presieve2_arm_neon(const uint8_t* __restrict preSieved0,
                        const uint8_t* __restrict preSieved1,
                        const uint8_t* __restrict preSieved2,
                        const uint8_t* __restrict preSieved3,
                        uint8_t* __restrict sieve,
                        std::size_t bytes)
{
  std::size_t i = 0;
  std::size_t limit = bytes - bytes % sizeof(uint8x16_t);

  for (; i < limit; i += sizeof(uint8x16_t))
  {
    vst1q_u8(&sieve[i],
      vandq_u8(vld1q_u8(&sieve[i]), vandq_u8(
        vandq_u8(vld1q_u8(&preSieved0[i]), vld1q_u8(&preSieved1[i])),
        vandq_u8(vld1q_u8(&preSieved2[i]), vld1q_u8(&preSieved3[i])))));
  }

  for (; i < bytes; i++)
    sieve[i] &= preSieved0[i] & preSieved1[i] & preSieved2[i] & preSieved3[i];
}

} // namespace

#endif
