///
/// @file   prime_iterator.h
/// @brief  The prime_iterator class allows to easily iterate
///         (forward and backward) over prime numbers.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIME_ITERATOR_H
#define PRIME_ITERATOR_H

#include <vector>

namespace primesieve {

/// Iterate over prime numbers.
/// The @link next_prime.cpp next_prime.cpp @endlink
/// and @link previous_prime.cpp previous_prime.cpp @endlink
/// examples show how to use prime_iterator objects.
/// @note  prime_iterator objects are very convenient to use at
///        the cost of being slightly slower than the
///        callback_primes() functions.
///
class prime_iterator
{
public:
  /// Create a new prime_iterator object.
  /// @param start  Start iterating at this number. If start is a
  ///               prime then first calling either next_prime()
  ///               or previous_prime() will return start.
  /// @pre          start <= 2^64 - 2^32 * 10
  ///
  prime_iterator(uint64_t start = 0);

  /// Reinitialize this prime_iterator object to start.
  /// @param start  Start iterating at this number. If start is a
  ///               prime then first calling either next_prime()
  ///               or previous_prime() will return start.
  /// @pre          start <= 2^64 - 2^32 * 10
  ///
  void skip_to(uint64_t start);

  /// Get the current prime.
  /// @note  Returns next_prime() if neither next_prime() nor
  ///        previous_prime() have previously been called:
  ///        prime_iterator pi;
  ///        pi.skip_to(4);
  ///        pi.prime() == 5;
  ///
  uint64_t prime()
  {
    if (first_)
      generate_next_primes();
    return primes_[i_];
  }

  /// Advance the prime_iterator by one position.
  /// @return  Returns the next prime (returns 0 if
  ///          next_prime() > 2^64 - 2^32 * 10).
  ///
  uint64_t next_prime()
  {
    if (++i_ >= primes_.size() || first_)
      generate_next_primes();
    return primes_[i_];
  }

  /// Decrease the prime_iterator by one position.
  /// @return  Returns the previous prime (returns 0 if
  ///          previous_prime() < 2).
  ///
  uint64_t previous_prime()
  {
    if (i_ == 0 || first_)
      generate_previous_primes();
    return primes_[--i_];
  }
private:
  uint64_t i_;
  uint64_t start_;
  uint64_t count_;
  bool first_;
  bool adjust_skip_to_;
  std::vector<uint64_t> primes_;
  uint64_t get_interval_size(uint64_t);
  void check_out_of_range();
  void generate_next_primes();
  void generate_previous_primes();
};

} // end namespace

#endif
