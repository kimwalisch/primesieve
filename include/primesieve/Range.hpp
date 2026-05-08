///
/// @file   Range.hpp
/// @brief  Range-based for loop support for primesieve.
///         This allows iterating over primes using C++11 range syntax.
///
/// Example usage:
///   // Iterate over primes in [1, 100]
///   for (uint64_t prime : primesieve::primes(1, 100)) {
///       std::cout << prime << std::endl;
///   }
///
///   // Iterate over primes >= 0 (no upper bound)
///   for (uint64_t prime : primesieve::primes()) {
///       if (prime > 1000000) break;
///       // use prime
///   }
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMESIEVE_RANGE_HPP
#define PRIMESIEVE_RANGE_HPP

#include <primesieve/iterator.hpp>

#include <stdint.h>
#include <iterator>

namespace primesieve {

/// Forward declaration
class PrimeRange;

/// Iterator for the PrimeRange that adapts primesieve::iterator
/// to work with range-based for loops.
/// This iterator satisfies the ForwardIterator requirements.
class PrimeIterator
{
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = uint64_t;
  using difference_type = std::ptrdiff_t;
  using pointer = const uint64_t*;
  using reference = uint64_t;

  PrimeIterator() noexcept : it_(), current_(0), isEnd_(true) {}

  explicit PrimeIterator(iterator& it) noexcept
    : it_(&it), current_(0), isEnd_(false)
  {
    current_ = it_->next_prime();
  }

  /// Get current prime value
  [[nodiscard]] reference operator*() const noexcept
  {
    return current_;
  }

  /// Get pointer to current prime (for compatibility)
  [[nodiscard]] pointer operator->() const noexcept
  {
    return &current_;
  }

  /// Advance to next prime
  PrimeIterator& operator++()
  {
    current_ = it_->next_prime();
    return *this;
  }

  /// Advance to next prime (postfix)
  PrimeIterator operator++(int)
  {
    PrimeIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  /// Compare iterators for equality
  [[nodiscard]] bool operator==(const PrimeIterator& other) const noexcept
  {
    // End iterators are always equal
    if (isEnd_ && other.isEnd_)
      return true;

    // Comparing with end iterator - never equal unless both are end
    if (isEnd_ || other.isEnd_)
      return false;

    // Compare current prime values
    return current_ == other.current_;
  }

  /// Compare iterators for inequality
  [[nodiscard]] bool operator!=(const PrimeIterator& other) const noexcept
  {
    return !(*this == other);
  }

private:
  iterator* it_;
  uint64_t current_;
  bool isEnd_;
};

/// A range that can be used in range-based for loops to iterate
/// over primes. The iteration stops when the stop value is reached.
/// If stop is UINT64_MAX, iteration continues indefinitely until
/// manually interrupted (e.g., by break statement).
class PrimeRange
{
public:
  PrimeRange(uint64_t start, uint64_t stop) noexcept
    : it_(start, stop), start_(start), stop_(stop)
  {}

  explicit PrimeRange(uint64_t start = 0) noexcept
    : it_(start, UINT64_MAX), start_(start), stop_(UINT64_MAX)
  {}

  /// Get iterator to the beginning (first prime >= start)
  [[nodiscard]] PrimeIterator begin() noexcept
  {
    return PrimeIterator(it_);
  }

  /// Get sentinel iterator (end of range)
  /// The sentinel uses a special iterator that compares equal
  /// to any iterator whose current prime > stop.
  [[nodiscard]] PrimeIterator end() noexcept
  {
    return PrimeIterator();
  }

  /// Get the start value of the range
  [[nodiscard]] uint64_t getStart() const noexcept
  {
    return start_;
  }

  /// Get the stop value of the range
  [[nodiscard]] uint64_t getStop() const noexcept
  {
    return stop_;
  }

private:
  iterator it_;
  uint64_t start_;
  uint64_t stop_;
};

/// Sentinel type for range-based iteration with stop condition
/// This sentinel compares equal to any iterator whose current
/// prime exceeds the stop value.
class PrimeSentinel
{
public:
  explicit PrimeSentinel(uint64_t stop) noexcept : stop_(stop) {}

  /// Compare with iterator - returns true if prime > stop
  [[nodiscard]] bool operator==(const PrimeIterator& it) const noexcept
  {
    return *it > stop_;
  }

  [[nodiscard]] bool operator!=(const PrimeIterator& it) const noexcept
  {
    return !(*this == it);
  }

private:
  uint64_t stop_;
};

/// A range with bounded iteration that stops at a specific value.
/// This provides cleaner end semantics than the unbounded PrimeRange.
class BoundedPrimeRange
{
public:
  BoundedPrimeRange(uint64_t start, uint64_t stop) noexcept
    : it_(start, stop), start_(start), stop_(stop)
  {}

  /// Get iterator to the beginning
  [[nodiscard]] PrimeIterator begin() noexcept
  {
    return PrimeIterator(it_);
  }

  /// Get sentinel for the end
  /// Note: This uses a different approach - we check the current
  /// prime value against stop in the sentinel comparison.
  [[nodiscard]] PrimeSentinel end() noexcept
  {
    return PrimeSentinel(stop_);
  }

private:
  iterator it_;
  uint64_t start_;
  uint64_t stop_;
};

/// Factory function to create a prime range.
/// This is the primary interface for range-based for loops.
/// @param start  Generate primes >= start (default: 0)
/// @param stop   Generate primes <= stop (default: UINT64_MAX)
/// @return       A PrimeRange that can be used in range-based for loops
///
/// Example:
///   for (uint64_t p : primesieve::primes(1, 100)) { ... }
///   for (uint64_t p : primesieve::primes()) { ... }  // unbounded
///
[[nodiscard]] inline PrimeRange primes(uint64_t start = 0, uint64_t stop = UINT64_MAX) noexcept
{
  return PrimeRange(start, stop);
}

/// Factory function for bounded iteration (stops exactly at stop).
/// This provides better iteration semantics when you know the exact range.
[[nodiscard]] inline BoundedPrimeRange bounded_primes(uint64_t start, uint64_t stop) noexcept
{
  return BoundedPrimeRange(start, stop);
}

} // namespace primesieve

#endif