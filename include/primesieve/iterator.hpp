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

/// Iterate over prime numbers.
/// The @link primesieve_iterator.cpp primesieve_iterator.cpp @endlink
/// and @link previous_prime.cpp previous_prime.cpp @endlink examples
/// show how to use primesieve::iterator objects.
/// @note  primesieve::iterator objects are very convenient to use at
///        the cost of being slightly slower than the
///        callback_primes() functions.
///
class iterator
{
public:
  /// Create a new iterator object.
  /// @param start      Start iterating at this number. If start is a
  ///                   prime then first calling either next_prime()
  ///                   or previous_prime() will return start.
  /// @param stop_hint  Stop number optimization hint, gives significant
  ///                   speed up if few primes are generated.
  /// @pre              start <= 2^64 - 2^32 * 10
  ///
  iterator(uint64_t start = 0, uint64_t stop_hint = get_max_stop());

  /// Reinitialize this iterator object to start.
  /// @param start      Start iterating at this number. If start is a
  ///                   prime then first calling either next_prime()
  ///                   or previous_prime() will return start.
  /// @param stop_hint  Stop number optimization hint, gives significant
  ///                   speed up if few primes are generated.
  /// @pre              start <= 2^64 - 2^32 * 10
  ///
  void skipto(uint64_t start, uint64_t stop_hint = get_max_stop());

  /// Set a stop number optimization hint. Setting stop_hint gives
  /// a significant speed up if only few primes are generated.
  /// @pre  stop_hint <= 2^64 - 2^32 * 10
  ///
  void set_stop_hint(uint64_t stop_hint);

  /// Advance the iterator by one position.
  /// @return  The next prime.
  ///
  uint64_t next_prime()
  {
    if (++i_ >= primes_.size() || first_)
      generate_next_primes();
    return primes_[i_];
  }

  /// Decrease the iterator by one position.
  /// @return  The previous prime.
  ///
  uint64_t previous_prime()
  {
    if (i_-- == 0 || first_)
      generate_previous_primes();
    return primes_[i_];
  }

  /// Free all memory, same as skipto(0).
  void clear();
private:
  std::size_t i_;
  std::vector<uint64_t> primes_;
  uint64_t start_;
  uint64_t stop_;
  uint64_t stop_hint_;
  uint64_t count_;
  bool first_;
  bool is_binary_search_;
  uint64_t get_interval_size(uint64_t);
  void generate_primes(uint64_t, uint64_t);
  void generate_next_primes();
  void generate_previous_primes();
};

} // end namespace

#endif
