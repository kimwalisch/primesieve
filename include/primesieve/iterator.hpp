///
/// @file   iterator.hpp
/// @brief  primesieve::iterator allows to easily iterate (forwards
///         and backwards) over prime numbers.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMESIEVE_ITERATOR_HPP
#define PRIMESIEVE_ITERATOR_HPP

#include <stdint.h>
#include <cstddef>
#include <limits>

#if __cplusplus >= 202002L && \
    defined(__has_cpp_attribute)
  #if __has_cpp_attribute(unlikely)
    #define IF_UNLIKELY_PRIMESIEVE(x) if (x) [[unlikely]]
  #endif
#elif defined(__has_builtin)
  #if __has_builtin(__builtin_expect)
    #define IF_UNLIKELY_PRIMESIEVE(x) if (__builtin_expect(!!(x), 0))
  #endif
#endif
#if !defined(IF_UNLIKELY_PRIMESIEVE)
  #define IF_UNLIKELY_PRIMESIEVE(x) if (x)
#endif

#if defined(max)
  #undef max
  #if __cplusplus >= 202302L || defined(__GNUG__)
    #warning "Undefining max() macro from <Windows.h>. Please define NOMINMAX before including <Windows.h>"
  #elif defined(_MSC_VER)
    #pragma message("Undefining max() macro from <Windows.h>. Please define NOMINMAX before including <Windows.h>")
  #endif
#endif

namespace primesieve {

/// primesieve::iterator allows to easily iterate over primes both
/// forwards and backwards. Generating the first prime has a
/// complexity of O(r log log r) operations with r = n^0.5, after that
/// any additional prime is generated in amortized O(log n log log n)
/// operations. The memory usage is PrimePi(n^0.5) * 8 bytes.
///
struct iterator
{
  /// Create a new iterator object.
  /// Generate primes >= 0. The start number is default initialized to
  /// 0 and the stop_hint is default initialized UINT64_MAX.
  ///
  iterator() noexcept;

  /// Create a new iterator object.
  /// @param start      Generate primes >= start (or <= start).
  /// @param stop_hint  Stop number optimization hint, gives significant
  ///                   speed up if few primes are generated. E.g. if
  ///                   you want to generate the primes <= 1000 use
  ///                   stop_hint = 1000.
  ///
  iterator(uint64_t start, uint64_t stop_hint = std::numeric_limits<uint64_t>::max()) noexcept;

  /// Reset the primesieve iterator to start.
  /// @param start      Generate primes >= start (or <= start).
  /// @param stop_hint  Stop number optimization hint, gives significant
  ///                   speed up if few primes are generated. E.g. if
  ///                   you want to generate the primes <= 1000 use
  ///                   stop_hint = 1000.
  ///
  void jump_to(uint64_t start, uint64_t stop_hint = std::numeric_limits<uint64_t>::max()) noexcept;

  /// primesieve::iterator objects cannot be copied.
  iterator(const iterator&) = delete;
  iterator& operator=(const iterator&) = delete;

  /// primesieve::iterator objects support move semantics.
  iterator(iterator&&) noexcept;
  iterator& operator=(iterator&&) noexcept;

  /// Frees all memory
  ~iterator();

  /// Reset the start number to 0 and free most memory.
  /// Keeps some smaller data structures in memory
  /// (e.g. the PreSieve object) that are useful if the
  /// primesieve::iterator is reused. The remaining memory
  /// uses at most 200 kilobytes.
  ///
  void clear() noexcept;

  void generate_next_primes();
  void generate_prev_primes();

  /// Get the next prime.
  /// Throws a primesieve::primesieve_error exception (derived from
  /// std::runtime_error) if any error occurs.
  ///
  uint64_t next_prime()
  {
    i_ += 1;
    IF_UNLIKELY_PRIMESIEVE(i_ >= size_)
      generate_next_primes();
    return primes_[i_];
  }

  /// Get the previous prime.
  /// prev_prime(n) returns 0 for n <= 2.
  /// Note that next_prime() runs up to 2x faster than prev_prime().
  /// Hence if the same algorithm can be written using either
  /// prev_prime() or next_prime() it is preferable to use next_prime().
  ///
  uint64_t prev_prime()
  {
    IF_UNLIKELY_PRIMESIEVE(i_ == 0)
      generate_prev_primes();
    i_ -= 1;
    return primes_[i_];
  }

  std::size_t i_;
  std::size_t size_;
  uint64_t start_;
  uint64_t stop_hint_;
  uint64_t* primes_;
  void* memory_;
};

} // namespace

#endif
