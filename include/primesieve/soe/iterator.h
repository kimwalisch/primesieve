///
/// @file   iterator.h
/// @brief  The iterator class allows to easily iterate (forward and
///         backward) over prime numbers.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMESIEVE_ITERATOR_H
#define PRIMESIEVE_ITERATOR_H

#include <vector>
#include <cstddef>

namespace primesieve {

/// Iterate over prime numbers.
/// The @link next_prime.cpp next_prime.cpp @endlink and @link 
/// previous_prime.cpp previous_prime.cpp @endlink examples show how
/// to use primesieve::iterator objects.
/// @note  primesieve::iterator objects are very convenient to use at
///        the cost of being slightly slower than the
///        callback_primes() functions.
///
class iterator
{
public:
  /// Create a new iterator object.
  /// @param start  Start iterating at this number. If start is a
  ///               prime then first calling either next_prime()
  ///               or previous_prime() will return start.
  /// @pre          start <= 2^64 - 2^32 * 10
  ///
  iterator(uint64_t start = 0);

  /// Reinitialize this iterator object to start.
  /// @param start  Start iterating at this number. If start is a
  ///               prime then first calling either next_prime()
  ///               or previous_prime() will return start.
  /// @pre          start <= 2^64 - 2^32 * 10
  ///
  void skipto(uint64_t start);

  /// Get the current prime.
  uint64_t prime()
  {
    if (first_)
      generate_next_primes();
    return primes_[i_];
  }

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
    if (i_ == 0 || first_)
      generate_previous_primes();
    return primes_[--i_];
  }
private:
  std::size_t i_;
  std::vector<uint64_t> primes_;
  uint64_t start_;
  uint64_t count_;
  bool first_;
  bool adjust_skipto_;
  uint64_t get_interval_size(uint64_t);
  void generate_primes(uint64_t, uint64_t);
  void generate_next_primes();
  void generate_previous_primes();
};

} // end namespace

#endif
