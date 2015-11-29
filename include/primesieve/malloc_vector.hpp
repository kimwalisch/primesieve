///
/// @file  malloc_vector
///
/// Copyright (C) 2015 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef MALLOC_VECTOR_HPP
#define MALLOC_VECTOR_HPP

#include <stdlib.h>
#include <algorithm>
#include <cstddef>
#include <new>

namespace primesieve {

/// malloc_vector is a dynamically growing array.
/// It has the same API (though not complete) as std::vector but it
/// uses malloc as its allocator.
///
template <typename T>
class malloc_vector
{
public:
  malloc_vector()
    : array_(0),
      size_(0),
      capacity_(0),
      is_free_(true)
  {
    resize(16);
  }

  malloc_vector(std::size_t n)
    : array_(0),
      size_(0),
      capacity_(0),
      is_free_(true)
  {
    resize(n);
  }

  ~malloc_vector()
  {
    if (is_free_)
      free((void*) array_);
  }

  void push_back(const T& val)
  {
    array_[size_++] = val;
    if (size_ >= capacity_)
      resize(size_ * 2);
  }

  void reserve(std::size_t n)
  {
    if (n > capacity_)
      resize(n);
  }

  void resize(std::size_t n)
  {
    n = std::max(n, (std::size_t) 16);
    T* new_array = (T*) realloc((void*) array_, n * sizeof(T));

    if (!new_array)
      throw std::bad_alloc();

    array_ = new_array;
    capacity_ = n;
    size_ = std::min(size_, capacity_);
  }

  T& operator[] (T n)
  {
    return array_[n];
  }

  T* data()
  {
    return array_;
  }

  std::size_t size() const
  {
    return size_;
  }

  void disable_free()
  {
    is_free_ = false;
  }

public:
  typedef T value_type;

private:
  T* array_;
  std::size_t size_;
  std::size_t capacity_;
  bool is_free_;
};

} // namespace primesieve

#endif
