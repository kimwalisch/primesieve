///
/// @file   iterator.hpp
/// @brief  The iterator class allows to easily iterate (forwards
///         and backwards) over prime numbers.
///
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMESIEVE_ITERATOR_HPP
#define PRIMESIEVE_ITERATOR_HPP

#include <cstdint>
#include <cstddef>
#include <vector>
#include <memory>

namespace primesieve {

class PrimeGenerator;

uint64_t get_max_stop();

/// primesieve::iterator allows to easily iterate over primes both
/// forwards and backwards. Generating the first prime has a
/// complexity of O(r log log r) operations with r = n^0.5, after that
/// any additional prime is generated in amortized O(log n log log n)
/// operations. The memory usage is PrimePi(n^0.5) * 8 bytes.
///
class iterator
{
public:
  /// Create a new iterator object.
  /// @param start      Generate primes > start (or < start).
  /// @param stop_hint  Stop number optimization hint, gives significant
  ///                   speed up if few primes are generated. E.g. if
  ///                   you want to generate the primes below 1000 use
  ///                   stop_hint = 1000.
  ///
  iterator(uint64_t start = 0, uint64_t stop_hint = get_max_stop());

  /// primesieve::iterator objects cannot be copied.
  iterator(const iterator&) = delete;
  iterator& operator=(const iterator&) = delete;

  /// primesieve::iterator objects support move semantics.
  iterator(iterator&&) noexcept;
  iterator& operator=(iterator&&) noexcept;

  ~iterator();

  /// Reset the primesieve iterator to start.
  /// @param start      Generate primes > start (or < start).
  /// @param stop_hint  Stop number optimization hint, gives significant
  ///                   speed up if few primes are generated. E.g. if
  ///                   you want to generate the primes below 1000 use
  ///                   stop_hint = 1000.
  ///
  void skipto(uint64_t start, uint64_t stop_hint = get_max_stop());

  /// Get the next prime.
  /// Returns UINT64_MAX if next prime > 2^64.
  ///
  uint64_t next_prime()
  {
    if (i_++ == last_idx_)
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
    if (i_-- == 0)
      generate_prev_primes();
    return primes_[i_];
  }

private:
  std::size_t i_;
  std::size_t last_idx_;
  std::vector<uint64_t> primes_;
  uint64_t start_;
  uint64_t stop_;
  uint64_t stop_hint_;
  uint64_t dist_;
  std::unique_ptr<PrimeGenerator> primeGenerator_;
  void generate_next_primes();
  void generate_prev_primes();
};

} // namespace

#endif
