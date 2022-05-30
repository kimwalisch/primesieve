///
/// @file  pod_vector.hpp
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef POD_VECTOR_HPP
#define POD_VECTOR_HPP

#include "macros.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace primesieve {

/// pod_vector is a dynamically growing array.
/// It has the same API (though not complete) as std::vector but its
/// resize() method does not default initialize memory for built-in
/// integer types. It does however default initialize classes and
/// struct types if they have a constructor. It also prevents
/// bounds checks which is important for primesieve's performance, e.g.
/// the Fedora Linux distribution compiles with -D_GLIBCXX_ASSERTIONS
/// which enables std::vector bounds checks.
///
template <typename T>
class pod_vector
{
public:
  static_assert(std::is_trivially_destructible<T>::value,
                "pod_vector<T> only supports types with trivial destructors!");

  pod_vector() noexcept = default;

  pod_vector(std::size_t size)
  {
    resize(size);
  }

  ~pod_vector()
  {
    delete [] array_;
  }

  /// Free all memory, the pod_vector
  /// can be reused afterwards.
  void free() noexcept
  {
    delete [] array_;
    array_ = nullptr;
    end_ = nullptr;
    capacity_ = nullptr;
  }

  /// Reset the pod_vector, but do not free its
  /// memory. Same as std::vector.clear().
  void clear() noexcept
  {
    end_ = array_;
  }

  /// Copying is slow, we prevent it
  pod_vector(const pod_vector&) = delete;
  pod_vector& operator=(const pod_vector&) = delete;

  /// Move constructor
  pod_vector(pod_vector&& other) noexcept
  {
    this->swap(other);
  }

  /// Move assignment operator
  pod_vector& operator=(pod_vector&& other) noexcept
  {
    if (this != &other)
      this->swap(other);

    return *this;
  }

  void swap(pod_vector& other) noexcept
  {
    std::swap(array_, other.array_);
    std::swap(end_, other.end_);
    std::swap(capacity_, other.capacity_);
  }

  bool empty() const noexcept
  {
    return array_ == end_;
  }

  T& operator[] (std::size_t pos) noexcept
  {
    return array_[pos];
  }

  T& operator[] (std::size_t pos) const noexcept
  {
    return array_[pos];
  }

  T* data() noexcept
  {
    return array_;
  }

  T* data() const noexcept
  {
    return array_;
  }

  std::size_t size() const noexcept
  {
    assert(end_ >= array_);
    return (std::size_t)(end_ - array_);
  }

  std::size_t capacity() const noexcept
  {
    assert(capacity_ >= array_);
    return (std::size_t)(capacity_ - array_);
  }

  T* begin() noexcept
  {
    return array_;
  }

  T* begin() const noexcept
  {
    return array_;
  }

  T* end() noexcept
  {
    return end_;
  }

  T* end() const noexcept
  {
    return end_;
  }

  T& front() noexcept
  {
    return *array_;
  }

  T& front() const noexcept
  {
    return *array_;
  }

  T& back() noexcept
  {
    assert(end_ != nullptr);
    return *(end_ - 1);
  }

  T& back() const noexcept
  {
    assert(end_ != nullptr);
    return *(end_ - 1);
  }

  ALWAYS_INLINE void push_back(const T& value)
  {
    if_unlikely(end_ >= capacity_)
      reserve((std::size_t)((size() + 1) * 1.5));
    *end_++ = value;
  }

  ALWAYS_INLINE void push_back(T&& value)
  {
    if_unlikely(end_ >= capacity_)
      reserve((std::size_t)((size() + 1) * 1.5));
    *end_++ = value;
  }

  template <class... Args>
  ALWAYS_INLINE void emplace_back(Args&&... args)
  {
    if_unlikely(end_ >= capacity_)
      reserve((std::size_t)((size() + 1) * 1.5));
    *end_++ = T(std::forward<Args>(args)...);
  }

  void reserve(std::size_t n)
  {
    if (n > capacity())
    {
      std::size_t old_size = size();
      resize(n);
      end_ = array_ + old_size;
    }
  }

  template <typename TT>
  ALWAYS_INLINE typename std::enable_if<std::is_pod<TT>::value>::type
  default_initialize_range(TT*, TT*)
  { }

  template <typename TT>
  ALWAYS_INLINE typename std::enable_if<!std::is_pod<TT>::value>::type
  default_initialize_range(TT* first, TT* last)
  {
    for (; first < last; first++)
      *first = TT();
  }

  /// Resize without default initializing memory.
  /// If the pod_vector is not empty the current content
  /// will be copied into the new array.
  ///
  void resize(std::size_t n)
  {
    if (n == size())
      return;
    else if (n <= capacity())
    {
      assert(capacity() > 0);

      // This will only be used for classes
      // and structs with constructors.
      if (n > size())
        default_initialize_range(end_, array_ + n);

      end_ = array_ + n;
    }
    else
    {
      // This default initializes memory of classes and
      // structs with constructors. But it does not default
      // initialize memory for POD types like int, long.
      T* new_array = new T[n];

      if (array_)
      {
        std::copy(array_, end_, new_array);
        delete [] array_;
      }

      array_ = new_array;
      end_ = array_ + n;
      capacity_ = end_;
    }
  }

  using value_type = T;

private:
  T* array_ = nullptr;
  T* end_ = nullptr;
  T* capacity_ = nullptr;
};

} // namespace

#endif
