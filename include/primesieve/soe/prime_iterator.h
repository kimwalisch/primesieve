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

#ifndef PRIME_ITERATOR_h
#define PRIME_ITERATOR_h

#include <vector>
#include <cstddef>

namespace primesieve {

/// Iterate over prime numbers.
/// This class contains 3 useful methods:
/// prime(), next_prime() and previous_prime().
///
class prime_iterator
{
public:
  /// Create a new prime_iterator object.
  /// @param start       Start iterating at this number. If start is a
  ///                    prime then first calling either next_prime()
  ///                    or previous_prime() will return start.
  /// @param cache_size  Size of the cache in megabytes.
  ///
  prime_iterator(uint64_t start = 0, std::size_t cache_size = 2);

  /// Get the cache size in megabytes.
  /// The cache size can only be set in the constructor and
  /// cannot be modified during the object's lifetime.
  ///
  std::size_t get_cache_size() const;

  /// Get the current prime.
  /// @warning Returns 0 for undefined behavior e.g.:
  ///          prime_iterator pi(/*start=*/ 4);
  ///          pi.prime() == 0
  /// 
  uint64_t prime()
  {
    if (first_)
      return first_prime();
    return primes_[i_];
  }

  /// Get the next prime.
  uint64_t next_prime()
  {
    if (first_)
      return first_next_prime();
    if (++i_ == size_)
      generate_next_primes();
    return primes_[i_];
  }

  /// Get the previous prime.
  /// @warning previous_prime() returns 0 below 2, e.g.:
  ///          prime_iterator pi(/* start = */ 2);
  ///          prime = pi.previous_prime(); /* prime == 2 */
  ///          prime = pi.previous_prime(); /* prime == 0 */
  ///
  uint64_t previous_prime()
  {
    if (first_)
      return first_previous_prime();
    if (i_ == 0)
      generate_previous_primes();
    return primes_[--i_];
  }
private:
  std::size_t i_;
  std::size_t size_;
  std::size_t max_size_;
  std::size_t cache_size_;
  uint64_t start_;
  bool first_;
  std::vector<uint64_t> primes_;
  uint64_t first_prime();
  uint64_t first_next_prime();
  uint64_t first_previous_prime();
  void generate_next_primes();
  void generate_previous_primes();
};

}// end namespace

#endif
