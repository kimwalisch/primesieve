#ifndef BITS_PRIMESIEVE_H
#define BITS_PRIMESIEVE_H

namespace soe {

/// Bit patterns used to unset specific bits of a byte
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

} // namespace soe

#endif
