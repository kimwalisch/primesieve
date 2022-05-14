///
/// @file   pod_vector.cpp
/// @brief  Plain old data vector, like std::vector but does not 
///         default initialize memory.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/pod_vector.hpp>
#include <primesieve/macros.hpp>

#include <cstdlib>
#include <iostream>
#include <random>
#include <numeric>

using std::size_t;
using primesieve::pod_vector;

void check(bool OK)
{
  std::cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    std::exit(1);
}

NOINLINE void resize(pod_vector<char>& vect, size_t size)
{
  vect.clear();
  vect.resize(size);
}

int main()
{
  // The pod_vector class uses std::vector internally. For
  // performance reasons we want vector::resize() not to
  // free memory when resizing to a smaller size. The C++
  // standard seems to indirectly guarantee this behavior,
  // but it is not 100% clear. So this tests verifies this
  // behavior.

  // Allocate from 1 KiB to 128 MiB
  for (size_t i = 10; i <= 27; i++)
  {
    pod_vector<char> vect;
    vect.resize(size_t(1) << i);
    auto capacity1 = vect.capacity();
    vect.resize(100);
    auto capacity2 = vect.capacity();

    std::cout << "vect.resize(100).capacity = " << capacity1;
    check(capacity1 == capacity2);
  }

  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(10000, 20000);

    int size = dist(gen);
    pod_vector<int> vect(size);
    std::fill_n(&vect[0], size, 123);
    
    // Test if resize does not default initilize
    vect.resize(0);
    vect.resize(size);
    int sum = std::accumulate(&vect[0], &vect[0] + size, 0);
    std::cout << "Vect sum after resize: " << sum;
    check(sum == 123 * size);
  }

  // This test runs too slow without
  // compiler optimization flags.
#if defined(__OPTIMIZE__) && \
   !defined(__SANITIZE_ADDRESS__)
  // This test would take forever (3000 secs on i5-12600K
  // CPU from 2022) using std::vector because
  // std::vector.resize() default initializes memory
  // (to 0) on each resize whereas pod_vector does not.
  pod_vector<char> vect;
  for (int i = 0; i < 1000000; i++)
    resize(vect, /* size = 64MiB */ 64 << 20);
#endif

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
