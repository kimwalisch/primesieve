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

#include <vector>

namespace primesieve {

template <typename T>
struct inttype_without_default_initialization
{
  T n;
  // Empty constructor, disables default initialization
  inttype_without_default_initialization() { };
  inttype_without_default_initialization(T x) : n(x) { };
  // User defined implicit conversions. Our type should work
  // like standard integer types. However we only define the
  // operators that are used in our code.
  operator T() { return n; }
  operator T() const { return n; }
  void operator=(T x) { n = x; }
};

/// Plain old data vector, does not default initialize memory.
/// Since primesieve may allocate gigabytes of memory and
/// afterwards initalize that memory using multiple threads,
/// we don't want our vector to default initialize our memory
/// otherwise we would initialize the same memory twice.
///
/// @TODO: Use std::vector.resize_uninitialized() instead once
///        it becomes available (if it ever will). 
///
template <typename T>
using pod_vector = std::vector<inttype_without_default_initialization<T>>;

} // namespace

#endif
