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
#include <memory>
#include <new>

namespace {

/// malloc_vector is a dynamically growing array.
/// It has the same API (though not complete) as std::vector but it
/// uses malloc as its allocator.
///
template <typename T>
class malloc_vector
{
public:
  using value_type = T;
  malloc_vector() = default;

  ~malloc_vector()
  {
    free(array_);
  }

  /// Copying is slow, we prevent it
  malloc_vector(const malloc_vector&) = delete;
  malloc_vector& operator=(const malloc_vector&) = delete;

  T& operator[](std::size_t pos) noexcept
  {
    ASSERT(pos < size());
    return array_[pos];
  }

  /// Returns the array and releases the ownership to the caller
  /// (which has to free the array after he's done).
  T* release() noexcept
  {
    T* arr = array_;

    array_ = nullptr;
    end_ = nullptr;
    capacity_ = nullptr;

    return arr;
  }

  T* end() noexcept
  {
    return end_;
  }

  const T* end() const noexcept
  {
    return end_;
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

  template <class InputIt>
  void insert(T* const pos, InputIt first, InputIt last)
  {
    // We only support appending to the vector
    ASSERT(pos == end_);
    (void) pos;

    if (first < last)
    {
      std::size_t new_size = size() + (std::size_t) (last - first);
      reserve(new_size);
      std::uninitialized_copy(first, last, end_);
      end_ = array_ + new_size;
    }
  }

  void reserve(std::size_t n)
  {
    if (n > capacity())
      reserve_unchecked(n);
  }

private:
  T* array_ = nullptr;
  T* end_ = nullptr;
  T* capacity_ = nullptr;

  /// Requires n > capacity()
  void reserve_unchecked(std::size_t n)
  {
    ASSERT(n > capacity());
    ASSERT(size() <= capacity());
    std::size_t old_size = size();
    std::size_t old_capacity = capacity();

    // GCC & Clang's std::vector grow the capacity by at least
    // 2x for every call to resize() with n > capacity(). We
    // grow by at least 1.5x as we tend to accurately calculate
    // the amount of memory we need upfront.
    std::size_t new_capacity = (old_capacity * 3) / 2;
    new_capacity = std::max(new_capacity, n);
    ASSERT(old_capacity < new_capacity);

    // If there is not enough memory, the old memory block
    // is not freed and null pointer is returned.
    // https://en.cppreference.com/w/c/memory/realloc
    T* new_array = (T*) realloc((void*) array_, new_capacity * sizeof(T));

    if_unlikely(!new_array)
      throw std::bad_alloc();

    array_ = new_array;
    end_ = array_ + old_size;
    capacity_ = array_ + new_capacity;
    ASSERT(size() < capacity());
  }
};

} // namespace

#endif
