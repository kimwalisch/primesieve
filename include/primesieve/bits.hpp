///
/// @file   bits.hpp
/// @brief  Bitmasks to turn off single bits of a byte.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef BITS_HPP
#define BITS_HPP

enum {
  BIT0 = 0xfe, // 11111110
  BIT1 = 0xfd, // 11111101
  BIT2 = 0xfb, // 11111011
  BIT3 = 0xf7, // 11110111
  BIT4 = 0xef, // 11101111
  BIT5 = 0xdf, // 11011111
  BIT6 = 0xbf, // 10111111
  BIT7 = 0x7f  // 01111111
};

#endif
