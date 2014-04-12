///
/// @file   iterator.hpp
/// @brief  The iterator class allows to easily iterate (forward and
///         backward) over prime numbers.
///
/// Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMESIEVE_ITERATOR_HPP
#define PRIMESIEVE_ITERATOR_HPP

#include <vector>
#include <cstddef>

namespace primesieve {

uint64_t get_max_stop();

/// primesieve::iterator allows to easily iterate over primes both
/// forwards and backwards. Generating the first prime has a
/// complexity of O(r log log r) operations with r = n^0.5, after that
/// any additional prime is generated in amortized O(log n log log n)
/// operations. The memory usage is about pi(n^0.5) * 16 bytes.
/// primesieve::iterator objects are very convenient to use at the
/// cost of being slightly slower than the callback_primes()
/// functions.
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
  /// @pre              start <= 2^64 - 2^32 * 10
  ///
  iterator(uint64_t start = 0, uint64_t stop_hint = get_max_stop());

  /// Reinitialize this iterator object to start.
  /// @param start      Generate primes > start (or < start).
  /// @param stop_hint  Stop number optimization hint, gives significant
  ///                   speed up if few primes are generated. E.g. if
  ///                   you want to generate the primes below 1000 use
  ///                   stop_hint = 1000.
  /// @pre              start <= 2^64 - 2^32 * 10
  ///
  void skipto(uint64_t start, uint64_t stop_hint = get_max_stop());

  /// Advance the iterator by one position.
  /// @return  The next prime.
  ///
  uint64_t next_prime()
  {
    if (i_++ == last_idx_)
      generate_next_primes();
    return primes_[i_];
  }

  /// Decrease the iterator by one position.
  /// @return  The previous prime.
  ///
  uint64_t previous_prime()
  {
    if (i_-- == 0)
      generate_previous_primes();
    return primes_[i_];
  }
private:
  std::size_t i_;
  std::size_t last_idx_;
  std::vector<uint64_t> primes_;
  uint64_t start_;
  uint64_t stop_;
  uint64_t stop_hint_;
  uint64_t tiny_cache_size_;
  uint64_t get_interval_size(uint64_t);
  void generate_primes(uint64_t, uint64_t);
  void generate_next_primes();
  void generate_previous_primes();
};

} // end namespace

#endif
