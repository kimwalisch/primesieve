#ifndef ENDIANSAFE_CAST_H
#define ENDIANSAFE_CAST_H

#include <cstddef>
#include <stdint.h>

namespace soe {

// Template metaprogramming, recursively accumulate bytes.
// e.g. endiansafe_cast<int>(array)
// result  = array[0];
// result += array[1] << 8;
// result += array[2] << 16;
// result += array[3] << 24;

template <typename T, std::size_t INDEX>
struct accumulate_bytes
{
  static T do_it(const uint8_t* array)
  {
    T result = accumulate_bytes<T, INDEX - 1>::do_it(array);
    T byte = array[INDEX];
    result += byte << (INDEX * 8);
    return result;
  }
};

template <typename T>
struct accumulate_bytes<T, 0>
{
  static T do_it(const uint8_t* array)
  {
    return array[0];
  }
};

/// Cast bytes in ascending address order.
template <typename T>
inline T endiansafe_cast(const uint8_t* array)
{
  return accumulate_bytes<T, sizeof(T) - 1>::do_it(array);
}

} // namespace soe

#endif
