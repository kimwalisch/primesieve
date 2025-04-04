///
/// @file PreSieve_arm_sve.hpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
/// Copyright (C) 2022 @zielaj, https://github.com/zielaj
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRESIEVE_ARM_SVE_HPP
#define PRESIEVE_ARM_SVE_HPP

#include <arm_sve.h>
#include <stdint.h>
#include <cstddef>

namespace {

#if defined(ENABLE_MULTIARCH_ARM_SVE)
  __attribute__ ((target ("arch=armv8-a+sve")))
#endif
void presieve1_arm_sve(const uint8_t* __restrict preSieved0,
                       const uint8_t* __restrict preSieved1,
                       const uint8_t* __restrict preSieved2,
                       const uint8_t* __restrict preSieved3,
                       uint8_t* __restrict sieve,
                       std::size_t bytes)
{
  for (std::size_t i = 0; i < bytes; i += svcntb())
  {
    svbool_t pg = svwhilelt_b8(i, bytes);

    svst1_u8(pg, &sieve[i],
      svand_u8_x(svptrue_b64(),
        svand_u8_z(pg, svld1_u8(pg, &preSieved0[i]), svld1_u8(pg, &preSieved1[i])),
        svand_u8_z(pg, svld1_u8(pg, &preSieved2[i]), svld1_u8(pg, &preSieved3[i]))));
  }
}

#if defined(ENABLE_MULTIARCH_ARM_SVE)
  __attribute__ ((target ("arch=armv8-a+sve")))
#endif
void presieve2_arm_sve(const uint8_t* __restrict preSieved0,
                       const uint8_t* __restrict preSieved1,
                       const uint8_t* __restrict preSieved2,
                       const uint8_t* __restrict preSieved3,
                       uint8_t* __restrict sieve,
                       std::size_t bytes)
{
  for (std::size_t i = 0; i < bytes; i += svcntb())
  {
    svbool_t pg = svwhilelt_b8(i, bytes);

    svst1_u8(pg, &sieve[i],
      svand_u8_z(pg, svld1_u8(pg, &sieve[i]), svand_u8_x(svptrue_b64(),
        svand_u8_z(pg, svld1_u8(pg, &preSieved0[i]), svld1_u8(pg, &preSieved1[i])),
        svand_u8_z(pg, svld1_u8(pg, &preSieved2[i]), svld1_u8(pg, &preSieved3[i])))));
  }
}

} // namespace

#endif
