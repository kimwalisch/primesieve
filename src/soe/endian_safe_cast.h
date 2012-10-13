#ifndef ENDIAN_SAFE_CAST_H
#define ENDIAN_SAFE_CAST_H

#include <cstddef>
#include <stdint.h>

namespace soe {

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
    T result = array[0];
    return result;
  }
};

/// Cast bytes in ascending address order.
/// endian_safe_cast<T>(array) == reinterpret_cast<T*>(array)[0]
/// on little endian CPUs.
///
template <typename T>
T endian_safe_cast(const uint8_t* array)
{
  return accumulate_bytes<T, sizeof(T) - 1>::do_it(array);
}

} // namespace soe

#endif
