///
/// @file  Vector.hpp
///
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
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

/// VectorBase holds the pointers of the Vector. This is required
/// to make the Vector's constructor exception safe.
template <typename T, typename Allocator>
struct VectorBase
{
  T* begin_ = nullptr;
  T* end_ = nullptr;
  T* capacity_ = nullptr;

  VectorBase() noexcept = default;

  ~VectorBase() noexcept
  {
    if (begin_)
    {
      ASSERT(capacity_ >= begin_);
      std::size_t cap = std::size_t(capacity_ - begin_);
      Allocator().deallocate(begin_, cap);
    }
  }

  // Move constructor
  VectorBase(VectorBase&& other) noexcept
    : begin_(other.begin_),
      end_(other.end_),
      capacity_(other.capacity_)
  {
    other.begin_ = nullptr;
    other.end_ = nullptr;
    other.capacity_ = nullptr;
  }

  /// We disable copying to avoid severe performance
  /// penalties caused by unintended copying.
  VectorBase(const VectorBase&) = delete;
  VectorBase& operator=(const VectorBase&) = delete;
};

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
class Vector : private VectorBase<T, Allocator>
{
  using VecBase = VectorBase<T, Allocator>;
  using VecBase::begin_;
  using VecBase::end_;
  using VecBase::capacity_;

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

  ~Vector() noexcept
  {
    destroy(begin_, end_);
  }

  /// Free all memory, the Vector
  /// can be reused afterwards.
  void deallocate() noexcept
  {
    if (begin_)
    {
      destroy(begin_, end_);
      Allocator().deallocate(begin_, capacity());
      begin_ = nullptr;
      end_ = nullptr;
      capacity_ = nullptr;
    }
  }

  /// Reset the Vector, but do not free its
  /// memory. Same as std::vector.clear().
  void clear() noexcept
  {
    destroy(begin_, end_);
    end_ = begin_;
  }

  /// We disable copying to avoid severe performance
  /// penalties caused by unintended copying.
  Vector(const Vector&) = delete;
  Vector& operator=(const Vector&) = delete;

  /// Move constructor
  Vector(Vector&& other) noexcept
    : VecBase(std::move(other))
  { }

  /// Move assignment operator
  Vector& operator=(Vector&& other) noexcept
  {
    if (this != &other)
    {
      deallocate();
      begin_ = other.begin_;
      end_ = other.end_;
      capacity_ = other.capacity_;
      other.begin_ = nullptr;
      other.end_ = nullptr;
      other.capacity_ = nullptr;
    }

    return *this;
  }

  /// Better assembly than std::swap(vect1, vect2)
  void swap(Vector& other) noexcept
  {
    T* tmp_array = begin_;
    T* tmp_end = end_;
    T* tmp_capacity = capacity_;

    begin_ = other.begin_;
    end_ = other.end_;
    capacity_ = other.capacity_;

    other.begin_ = tmp_array;
    other.end_ = tmp_end;
    other.capacity_ = tmp_capacity;
  }

  bool empty() const noexcept
  {
    return begin_ == end_;
  }

  T& operator[](std::size_t pos) noexcept
  {
    ASSERT(pos < size());
    return begin_[pos];
  }

  const T& operator[](std::size_t pos) const noexcept
  {
    ASSERT(pos < size());
    return begin_[pos];
  }

  T* data() noexcept
  {
    return begin_;
  }

  const T* data() const noexcept
  {
    return begin_;
  }

  std::size_t size() const noexcept
  {
    ASSERT(end_ >= begin_);
    return std::size_t(end_ - begin_);
  }

  std::size_t capacity() const noexcept
  {
    ASSERT(capacity_ >= begin_);
    return std::size_t(capacity_ - begin_);
  }

  T* begin() noexcept
  {
    return begin_;
  }

  const T* begin() const noexcept
  {
    return begin_;
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
    return *begin_;
  }

  const T& front() const noexcept
  {
    ASSERT(!empty());
    return *begin_;
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
    if_likely(end_ != capacity_)
    {
      new(end_) T(value);
      end_++;
    }
    else
      emplace_back_reallocate(value);
  }

  ALWAYS_INLINE void push_back(T&& value)
  {
    if_likely(end_ != capacity_)
    {
      new(end_) T(std::move(value));
      end_++;
    }
    else
      emplace_back_reallocate(std::move(value));
  }

  template <class... Args>
  ALWAYS_INLINE void emplace_back(Args&&... args)
  {
    if_likely(end_ != capacity_)
    {
      new(end_) T(std::forward<Args>(args)...);
      end_++;
    }
    else
      emplace_back_reallocate(std::forward<Args>(args)...);
  }

  template <class InputIt>
  void insert(T* const pos, InputIt first, InputIt last)
  {
    // We only support appending to the vector
    ASSERT(pos == end_);
    (void) pos;

    if_likely(first < last)
    {
      std::size_t count = std::size_t(last - first);

      if (size() + count <= capacity())
      {
        std::uninitialized_copy(first, last, end_);
        end_ += count;
      }
      else
        insert_reallocate(first, last);
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
      uninitialized_default_construct(end_, begin_ + n);
      end_ = begin_ + n;
    }
    else if (n < size())
    {
      destroy(begin_ + n, end_);
      end_ = begin_ + n;
    }
  }

private:

  std::size_t increase_capacity()
  {
    // GCC & Clang's std::vector grow the capacity by 2x
    // for every call to resize() with n > capacity().
    std::size_t new_capacity = capacity() * 2;

    // The minimum allocation size of malloc is usually 16 bytes
    constexpr std::size_t min_capacity = std::max(std::size_t(1), 16 / sizeof(T));
    new_capacity = std::max(min_capacity, new_capacity);
    ASSERT(capacity() < new_capacity);

    return new_capacity;
  }

  /// Requires n > capacity()
  void reserve_unchecked(std::size_t n)
  {
    ASSERT(n > capacity());
    ASSERT(size() <= capacity());

    std::size_t old_size = size();
    std::size_t old_capacity = capacity();
    std::size_t new_capacity = increase_capacity();
    new_capacity = std::max(new_capacity, n);
    ASSERT(capacity() < new_capacity);

    T* old = begin_;
    begin_ = Allocator().allocate(new_capacity);
    end_ = begin_ + old_size;
    capacity_ = begin_ + new_capacity;
    ASSERT(size() < capacity());

    // Both primesieve & primecount require that byte arrays are
    // aligned to at least a alignof(uint64_t) boundary. This is
    // needed because our code casts byte arrays into uint64_t arrays
    // in some places in order to improve performance. The default
    // allocator guarantees that each memory allocation is at least
    // aligned to the largest built-in type (usually 16 or 32).
    ASSERT(((uintptr_t) (void*) begin_) % sizeof(uint64_t) == 0);

    // The code below is guaranteed to not throw
    // any exceptions, hence no memory leaks are
    // possible here anymore.
    if (old)
    {
      static_assert(std::is_nothrow_move_constructible<T>::value,
                    "Vector<T> only supports nothrow moveable types!");

      uninitialized_move_n(old, old_size, begin_);
      destroy(old, old + old_size);
      Allocator().deallocate(old, old_capacity);
    }
  }

  template <class... Args>
  void emplace_back_reallocate(Args&&... args)
  {
    std::size_t old_size = size();
    std::size_t old_capacity = capacity();
    std::size_t new_capacity = increase_capacity();
    ASSERT(capacity() < new_capacity);

    // If an exception is thrown during initialization further
    // down this will free the memory using RAII.
    struct AllocateMemory
    {
      T* begin = nullptr;
      std::size_t capacity = 0;

      AllocateMemory(std::size_t cap)
      {
        begin = Allocator().allocate(cap);
        capacity = cap;

        // Both primesieve & primecount require that byte arrays are
        // aligned to at least a alignof(uint64_t) boundary. This is
        // needed because our code casts byte arrays into uint64_t arrays
        // in some places in order to improve performance. The default
        // allocator guarantees that each memory allocation is at least
        // aligned to the largest built-in type (usually 16 or 32).
        ASSERT(((uintptr_t) (void*) begin) % sizeof(uint64_t) == 0);
      }

      ~AllocateMemory()
      {
        if (begin)
          Allocator().deallocate(begin, capacity);
      }
    };

    AllocateMemory new_mem(new_capacity);
    new (new_mem.begin + old_size) T(std::forward<Args>(args)...);

    // The code below is guaranteed to not throw
    // any exceptions, hence no memory leaks are
    // possible here anymore.
    if (begin_)
    {
      static_assert(std::is_nothrow_move_constructible<T>::value,
                    "Vector<T> only supports nothrow moveable types!");

      uninitialized_move_n(begin_, old_size, new_mem.begin);
      destroy(begin_, end_);
      Allocator().deallocate(begin_, old_capacity);
    }

    begin_ = new_mem.begin;
    end_ = begin_ + old_size + 1;
    capacity_ = begin_ + new_capacity;
    new_mem.begin = nullptr;
  }

  template <class InputIt>
  void insert_reallocate(InputIt first, InputIt last)
  {
    ASSERT(first < last);

    std::size_t old_size = size();
    std::size_t count = std::size_t(last - first);
    std::size_t new_size = old_size + count;
    std::size_t old_capacity = capacity();
    std::size_t new_capacity = increase_capacity();
    new_capacity = std::max(new_capacity, new_size);
    ASSERT(capacity() < new_capacity);

    // If an exception is thrown during initialization further
    // down this will free the memory using RAII.
    struct AllocateMemory
    {
      T* begin = nullptr;
      std::size_t capacity = 0;

      AllocateMemory(std::size_t cap)
      {
        begin = Allocator().allocate(cap);
        capacity = cap;

        // Both primesieve & primecount require that byte arrays are
        // aligned to at least a alignof(uint64_t) boundary. This is
        // needed because our code casts byte arrays into uint64_t arrays
        // in some places in order to improve performance. The default
        // allocator guarantees that each memory allocation is at least
        // aligned to the largest built-in type (usually 16 or 32).
        ASSERT(((uintptr_t) (void*) begin) % sizeof(uint64_t) == 0);
      }

      ~AllocateMemory()
      {
        if (begin)
          Allocator().deallocate(begin, capacity);
      }
    };

    AllocateMemory new_mem(new_capacity);
    std::uninitialized_copy(first, last, new_mem.begin + old_size);

    // The code below is guaranteed to not throw
    // any exceptions, hence no memory leaks are
    // possible here anymore.
    if (begin_)
    {
      static_assert(std::is_nothrow_move_constructible<T>::value,
                    "Vector<T> only supports nothrow moveable types!");

      uninitialized_move_n(begin_, old_size, new_mem.begin);
      destroy(begin_, end_);
      Allocator().deallocate(begin_, old_capacity);
    }

    begin_ = new_mem.begin;
    end_ = begin_ + new_size;
    capacity_ = begin_ + new_capacity;
    new_mem.begin = nullptr;
  }

  /// For POD types like int, long our Vector implementation does
  /// not default initialize memory. This allows allocating a
  /// large vector and initializing the memory later without any
  /// unnecessary performance overhead.
  template <typename TT = T>
  ALWAYS_INLINE typename std::enable_if<std::is_trivially_default_constructible<TT>::value, void>::type
  uninitialized_default_construct(T*, T*)
  { }

  /// This default initializes memory for classes and structs with
  /// constructors (and with in-class initialization of non-static
  /// members).
  template <typename TT = T>
  ALWAYS_INLINE typename std::enable_if<!std::is_trivially_default_constructible<TT>::value, void>::type
  uninitialized_default_construct(T* first, T* last)
  {
    struct ExceptionGuard
    {
      Vector* vec_;
      T* pos_;
      T* first_;
      T* last_;

      ExceptionGuard(Vector* vector, T* first, T* last)
        : vec_(vector),
          pos_(first),
          first_(first),
          last_(last)
      { }

      ~ExceptionGuard()
      {
        if (pos_ != last_)
          vec_->destroy(first_, pos_);
      }
    };

    // If an exception is thrown during initialization further
    // down this will destroy the new objects using RAII.
    ExceptionGuard guard(this, first, last);

    // Default initialize vector using placement new.
    // Note that `new (ptr) T();` zero initializes built-in integer types,
    // whereas `new (ptr) T;` does not initialize built-in integer types.
    for (; guard.pos_ != guard.last_; guard.pos_++)
      new (guard.pos_) T;
  }

  template <typename TT = T>
  ALWAYS_INLINE typename std::enable_if<std::is_trivially_copyable<TT>::value, void>::type
  uninitialized_move_n(T* __restrict first,
                       std::size_t count,
                       T* __restrict d_first)
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
  template <typename TT = T>
  ALWAYS_INLINE typename std::enable_if<!std::is_trivially_copyable<TT>::value, void>::type
  uninitialized_move_n(T* __restrict first,
                       std::size_t count,
                       T* __restrict d_first)
  {
    for (std::size_t i = 0; i < count; i++)
      new (d_first++) T(std::move(*first++));
  }

  template <typename TT = T>
  ALWAYS_INLINE typename std::enable_if<std::is_trivially_destructible<TT>::value, void>::type
  destroy(T*, T*)
  { }

  /// Same as std::destroy() from C++17.
  /// https://en.cppreference.com/w/cpp/memory/destroy
  template <typename TT = T>
  ALWAYS_INLINE typename std::enable_if<!std::is_trivially_destructible<TT>::value, void>::type
  destroy(T* first, T* last)
  {
    // Theoretically deallocating in reverse order is more
    // cache efficient. Clang's std::vector implementation
    // also deallocates in reverse order.
    while (first != last)
      (--last)->~T();
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
