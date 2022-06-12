///
/// @file  malloc_vector
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef MALLOC_VECTOR_HPP
#define MALLOC_VECTOR_HPP

#include "macros.hpp"

#include <stdlib.h>
#include <algorithm>
#include <cstddef>
#include <new>

namespace {

/// malloc_vector is a dynamically growing array.
/// It has the same API (though not complete) as std::vector but it
/// uses malloc as its allocator. malloc_vector also does not
/// default initialize memory allocated using resize().
///
template <typename T>
class malloc_vector
{
public:
  using value_type = T;
  malloc_vector() = default;

  /// malloc_vector is used by primesieve's C API.
  /// We return the primes array_ to the user, hence we
  /// don't want to delete the array_ in the destructor.
  ~malloc_vector() { }

  /// Copying is slow, we prevent it
  malloc_vector(const malloc_vector&) = delete;
  malloc_vector& operator=(const malloc_vector&) = delete;

  T& operator[](std::size_t pos) noexcept
  {
    ASSERT(pos < size());
    return array_[pos];
  }

  T* data() noexcept
  {
    return array_;
  }

  std::size_t size() const noexcept
  {
    ASSERT(end_ >= array_);
    return (std::size_t)(end_ - array_);
  }

  std::size_t capacity() const noexcept
  {
    ASSERT(capacity_ >= array_);
    return (std::size_t)(capacity_ - array_);
  }

  ALWAYS_INLINE void push_back(const T& value)
  {
    if_unlikely(end_ == capacity_)
      reserve_unchecked(std::max((std::size_t) 1, capacity() * 2));
    *end_++ = value;
  }

  void reserve(std::size_t n)
  {
    if (n > capacity())
      reserve_unchecked(n);
  }

  /// Resize without default initializing memory.
  /// If the malloc_vector is not empty the current content
  /// will be copied into the new array.
  ///
  void resize(std::size_t n)
  {
    if (n > capacity())
      reserve_unchecked(n);

    end_ = array_ + n;
  }

private:
  T* array_ = nullptr;
  T* end_ = nullptr;
  T* capacity_ = nullptr;

  void reserve_unchecked(std::size_t n)
  {
    ASSERT(n > capacity());
    std::size_t new_capacity = get_new_capacity<T>(n);
    std::size_t old_size = size();
    ASSERT(new_capacity >= n);
    ASSERT(new_capacity > old_size);

    // realloc calls malloc if array_ is NULL
    array_ = (T*) realloc((void*) array_, new_capacity * sizeof(T));
    if (!array_)
      throw std::bad_alloc();

    end_ = array_ + old_size;
    capacity_ = array_ + new_capacity;
  }

  template <typename U>
  ALWAYS_INLINE std::size_t get_new_capacity(std::size_t size)
  {
    ASSERT(size > 0);
    // GCC & Clang's std::vector grow the capacity by at least
    // 2x for every call to resize() with n > capacity(). We
    // grow by at least 1.5x as we tend to accurately calculate
    // the amount of memory we need upfront.
    std::size_t new_capacity = (std::size_t)(capacity() * 1.5);
    constexpr std::size_t min_alignment = sizeof(long) * 2;
    constexpr std::size_t min_capacity = min_alignment / sizeof(U);
    return std::max({min_capacity, size, new_capacity});
  }
};

} // namespace

#endif
