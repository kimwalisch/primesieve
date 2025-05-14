///
/// @file  Vector.hpp
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef VECTOR_HPP
#define VECTOR_HPP

#include "macros.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdint.h>
#include <type_traits>
#include <utility>

namespace primesieve {

/// Vector is a dynamically growing array.
/// It has the same API (though not complete) as std::vector but its
/// resize() method does not default initialize memory for built-in
/// integer types. It does however default initialize classes and
/// struct types if they have a constructor. It also prevents bounds
/// checks in release builds which is important for primesieve's
/// performance, e.g. the Fedora Linux distribution compiles with
/// -D_GLIBCXX_ASSERTIONS which enables std::vector bounds checks.
///
template <typename T,
          typename Allocator = std::allocator<T>>
class Vector
{
public:
  // The default C++ std::allocator is stateless. We use this
  // allocator and do not support other statefull allocators,
  // which simplifies our implementation.
  //
  // "The default allocator is stateless, that is, all instances
  // of the given allocator are interchangeable, compare equal
  // and can deallocate memory allocated by any other instance
  // of the same allocator type."
  // https://en.cppreference.com/w/cpp/memory/allocator
  //
  // "The member type is_always_equal of std::allocator_traits
  // is intendedly used for determining whether an allocator
  // type is stateless."
  // https://en.cppreference.com/w/cpp/named_req/Allocator
  static_assert(std::allocator_traits<Allocator>::is_always_equal::value,
                "Vector<T> only supports stateless allocators!");

  using value_type = T;
  Vector() noexcept = default;

  Vector(std::size_t size)
  {
    resize(size);
  }

  ~Vector()
  {
    destroy(array_, end_);
    Allocator().deallocate(array_, capacity());
  }

  /// Free all memory, the Vector
  /// can be reused afterwards.
  void deallocate() noexcept
  {
    destroy(array_, end_);
    Allocator().deallocate(array_, capacity());
    array_ = nullptr;
    end_ = nullptr;
    capacity_ = nullptr;
  }

  /// Reset the Vector, but do not free its
  /// memory. Same as std::vector.clear().
  void clear() noexcept
  {
    destroy(array_, end_);
    end_ = array_;
  }

  /// Copying is slow, we prevent it
  Vector(const Vector&) = delete;
  Vector& operator=(const Vector&) = delete;

  /// Move constructor
  Vector(Vector&& other) noexcept
  {
    swap(other);
  }

  /// Move assignment operator
  Vector& operator=(Vector&& other) noexcept
  {
    if (this != &other)
      swap(other);

    return *this;
  }

  /// Better assembly than: std::swap(vect1, vect2)
  void swap(Vector& other) noexcept
  {
    T* tmp_array = array_;
    T* tmp_end = end_;
    T* tmp_capacity = capacity_;

    array_ = other.array_;
    end_ = other.end_;
    capacity_ = other.capacity_;

    other.array_ = tmp_array;
    other.end_ = tmp_end;
    other.capacity_ = tmp_capacity;
  }

  bool empty() const noexcept
  {
    return array_ == end_;
  }

  T& operator[](std::size_t pos) noexcept
  {
    ASSERT(pos < size());
    return array_[pos];
  }

  const T& operator[](std::size_t pos) const noexcept
  {
    ASSERT(pos < size());
    return array_[pos];
  }

  T* data() noexcept
  {
    return array_;
  }

  const T* data() const noexcept
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

  T* begin() noexcept
  {
    return array_;
  }

  const T* begin() const noexcept
  {
    return array_;
  }

  T* end() noexcept
  {
    return end_;
  }

  const T* end() const noexcept
  {
    return end_;
  }

  T& front() noexcept
  {
    ASSERT(!empty());
    return *array_;
  }

  const T& front() const noexcept
  {
    ASSERT(!empty());
    return *array_;
  }

  T& back() noexcept
  {
    ASSERT(!empty());
    return *(end_ - 1);
  }

  const T& back() const noexcept
  {
    ASSERT(!empty());
    return *(end_ - 1);
  }

  ALWAYS_INLINE void push_back(const T& value)
  {
    if_unlikely(end_ == capacity_)
      reserve_unchecked(std::max((std::size_t) 1, capacity() * 2));
    // Placement new
    new(end_) T(value);
    end_++;
  }

  ALWAYS_INLINE void push_back(T&& value)
  {
    if_unlikely(end_ == capacity_)
      reserve_unchecked(std::max((std::size_t) 1, capacity() * 2));
    // Without std::move() the copy constructor will
    // be called instead of the move constructor.
    new(end_) T(std::move(value));
    end_++;
  }

  template <class... Args>
  ALWAYS_INLINE void emplace_back(Args&&... args)
  {
    if_unlikely(end_ == capacity_)
      reserve_unchecked(std::max((std::size_t) 1, capacity() * 2));
    // Placement new
    new(end_) T(std::forward<Args>(args)...);
    end_++;
  }

  template <class InputIt>
  void insert(T* const pos, InputIt first, InputIt last)
  {
    static_assert(std::is_trivially_copyable<T>::value,
                  "Vector<T>::insert() supports only trivially copyable types!");

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

  void resize(std::size_t n)
  {
    if (n > size())
    {
      if (n > capacity())
        reserve_unchecked(n);

      // This default initializes memory of classes and structs
      // with constructors (and with in-class initialization of
      // non-static members). But it does not default initialize
      // memory for POD types like int, long.
      if (!std::is_trivially_default_constructible<T>::value)
        uninitialized_default_construct(end_, array_ + n);

      end_ = array_ + n;
    }
    else if (n < size())
    {
      destroy(array_ + n, end_);
      end_ = array_ + n;
    }
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

    T* old = array_;
    array_ = Allocator().allocate(new_capacity);
    end_ = array_ + old_size;
    capacity_ = array_ + new_capacity;
    ASSERT(size() < capacity());

    // Both primesieve & primecount require that byte arrays are
    // aligned to at least a alignof(uint64_t) boundary. This is
    // needed because our code casts byte arrays into uint64_t arrays
    // in some places in order to improve performance. The default
    // allocator guarantees that each memory allocation is at least
    // aligned to the largest built-in type (usually 16 or 32).
    ASSERT(((uintptr_t) (void*) array_) % sizeof(uint64_t) == 0);

    if (old)
    {
      static_assert(std::is_nothrow_move_constructible<T>::value,
                    "Vector<T> only supports nothrow moveable types!");

      uninitialized_move_n(old, old_size, array_);
      Allocator().deallocate(old, old_capacity);
    }
  }

  template <typename U>
  ALWAYS_INLINE typename std::enable_if<std::is_trivially_copyable<U>::value, void>::type
  uninitialized_move_n(U* __restrict first,
                       std::size_t count,
                       U* __restrict d_first)
  {
    // We can use memcpy to move trivially copyable types.
    // https://en.cppreference.com/w/cpp/language/classes#Trivially_copyable_class
    // https://stackoverflow.com/questions/17625635/moving-an-object-in-memory-using-stdmemcpy
    std::uninitialized_copy_n(first, count, d_first);
  }

  /// Same as std::uninitialized_move_n() from C++17.
  /// https://en.cppreference.com/w/cpp/memory/uninitialized_move_n
  ///
  /// Unlike std::uninitialized_move_n() our implementation uses
  /// __restrict pointers which improves the generated assembly
  /// (using GCC & Clang). We can do this because we only use this
  /// method for non-overlapping arrays.
  template <typename U>
  ALWAYS_INLINE typename std::enable_if<!std::is_trivially_copyable<U>::value, void>::type
  uninitialized_move_n(U* __restrict first,
                       std::size_t count,
                       U* __restrict d_first)
  {
    for (std::size_t i = 0; i < count; i++)
      new (d_first++) T(std::move(*first++));
  }

  /// Same as std::uninitialized_default_construct() from C++17.
  /// https://en.cppreference.com/w/cpp/memory/uninitialized_default_construct
  ALWAYS_INLINE void uninitialized_default_construct(T* first, T* last)
  {
    // Default initialize array using placement new.
    // Note that `new (first) T();` zero initializes built-in integer types,
    // whereas `new (first) T;` does not initialize built-in integer types.
    for (; first != last; first++)
      new (first) T;
  }

  /// Same as std::destroy() from C++17.
  /// https://en.cppreference.com/w/cpp/memory/destroy
  ALWAYS_INLINE void destroy(T* first, T* last)
  {
    if (!std::is_trivially_destructible<T>::value)
    {
      // Theoretically deallocating in reverse order is more
      // cache efficient. Clang's std::vector implementation
      // also deallocates in reverse order.
      while (first != last)
        (--last)->~T();
    }
  }
};

/// Array has the same API as std::array, but unlike std::array
/// our Array is guaranteed to not use any bounds checks in release
/// builds. E.g. the Fedora Linux distribution compiles with
/// -D_GLIBCXX_ASSERTIONS which enables std::array bounds checks.
///
template <typename T, std::size_t N>
class Array
{
public:
  using value_type = T;
  T array_[N];

  T& operator[](std::size_t pos) noexcept
  {
    ASSERT(pos < size());
    return array_[pos];
  }

  const T& operator[](std::size_t pos) const noexcept
  {
    ASSERT(pos < size());
    return array_[pos];
  }

  void fill(const T& value)
  {
    std::fill_n(begin(), size(), value);
  }

  T* data() noexcept
  {
    return array_;
  }

  const T* data() const noexcept
  {
    return array_;
  }

  T* begin() noexcept
  {
    return array_;
  }

  const T* begin() const noexcept
  {
    return array_;
  }

  T* end() noexcept
  {
    return array_ + N;
  }

  const T* end() const noexcept
  {
    return array_ + N;
  }

  T& back() noexcept
  {
    ASSERT(N > 0);
    return array_[N - 1];
  }

  const T& back() const noexcept
  {
    ASSERT(N > 0);
    return array_[N - 1];
  }

  constexpr std::size_t size() const noexcept
  {
    return N;
  }
};

} // namespace

#endif
