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
/// This class contains 3 useful methods: prime(),
/// next_prime() and previous_prime().
/// @note  prime_iterator objects are very convenient to use but they
///        are usually slower and use more memory (up to 2x) than
///        the callback_primes() methods.
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
    if (!first_)
      return primes_[i_];
    uint64_t next_prime = generate_next_primes();
    return next_prime;
  }

  /// Get the next prime.
  /// @return  Returns the next prime (returns 0 if
  ///          next_prime() > 2^64 - 2^32 * 10).
  ///
  uint64_t next_prime()
  {
    if (!first_ && ++i_ < primes_.size())
      return primes_[i_];
    uint64_t next_prime = generate_next_primes();
    return next_prime;
  }

  /// Get the previous prime.
  /// @return  Returns the previous prime (returns 0 if
  ///          previous_prime() < 2).
  ///
  uint64_t previous_prime()
  {
    if (!first_ && i_ != 0)
      return primes_[--i_];
    uint64_t previous_prime = generate_previous_primes();
    return previous_prime;
  }
private:
  uint64_t i_;
  uint64_t start_;
  bool first_;
  bool adjust_skip_to_;
  std::vector<uint64_t> primes_;
  void check_out_of_range();
  uint64_t generate_next_primes();
  uint64_t generate_previous_primes();
};

} // end namespace

#endif
