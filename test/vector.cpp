///
/// @file   vector.cpp
/// @brief  Plain old data vector, like std::vector but does not 
///         default initialize memory.
///
/// Copyright (C) 2023 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/Vector.hpp>

#include <cstdlib>
#include <iostream>
#include <random>
#include <numeric>
#include <utility>

using std::size_t;
using primesieve::Array;
using primesieve::Vector;

void check(bool OK)
{
  std::cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    std::exit(1);
}

int main()
{
  // For performance reasons we want Vector::resize() not
  // to free memory when resizing to a smaller size.
  // So this tests verifies this behavior.

  // Allocate from 1 KiB to 128 MiB
  for (size_t i = 10; i <= 27; i++)
  {
    Vector<char> vect;
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
    std::uniform_int_distribution<std::size_t> dist(100, 200);

    std::size_t n = dist(gen);
    Vector<size_t> vect;

    for (size_t i = 0; i <= n; i++)
      vect.push_back(i);

    for (size_t i = 0; i <= n; i++)
    {
      std::cout << "vect.push_back(" << i << ") = " << i;
      check(vect[i] == i);
    }
  }

  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> dist(100, 200);

    std::size_t size = dist(gen);

    // Vector does not default initialize POD types
    // but it does initialize classes and structs with constructors.
    struct pod_t
    {
      pod_t() = default;
      pod_t(int i, int j) : a(i), b(j) { }
      int a = 100;
      int b = 200;
    };

    Vector<pod_t> vect(size);

    for (size_t i = 0; i < size; i++)
    {
      std::cout << "vect[i].a = " << vect[i].a;
      check(vect[i].a == 100);
      std::cout << "vect[i].b = " << vect[i].b;
      check(vect[i].b == 200);
    }

    vect.emplace_back(7, 8);
    std::cout << "vect.emplace_back(7, 8) = " << vect.back().a;
    check(vect.back().a == 7);
    std::cout << "vect.emplace_back(7, 8) = " << vect.back().b;
    check(vect.back().b == 8);
  }

  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> dist(10000, 20000);

    std::size_t n = dist(gen);
    Vector<int> vect;
    vect.resize(0);

    std::cout << "Vect size after resize(0): " << vect.size();
    check(vect.size() == 0);
    std::cout << "Vect capacity after resize(0): " << vect.capacity();
    check(vect.capacity() == 0);

    vect.reserve(n);

    std::cout << "Vect size after reserve(n): " << vect.size();
    check(vect.size() == 0);
    std::cout << "Vect empty after reserve(n): " << vect.empty();
    check(vect.empty() == true);
    std::cout << "Vect capacity after reserve(n): " << vect.capacity();
    check(vect.capacity() == n);

    vect.reserve(n / 2);
    std::cout << "Vect size after reserve(n/2): " << vect.size();
    check(vect.size() == 0);
    std::cout << "Vect empty after reserve(n/2): " << vect.empty();
    check(vect.empty() == true);
    std::cout << "Vect capacity after reserve(n/2): " << vect.capacity();
    check(vect.capacity() == n);

    vect.resize(n);
    std::cout << "Vect size after resize(n): " << vect.size();
    check(vect.size() == n);
    std::cout << "Vect capacity after resize(n): " << vect.capacity();
    check(vect.capacity() == n);

    vect.resize(n);
    std::cout << "Vect size after 2nd resize(n): " << vect.size();
    check(vect.size() == n);
    std::cout << "Vect capacity after 2nd resize(n): " << vect.capacity();
    check(vect.capacity() == n);

    vect.resize(n / 2);
    std::cout << "Vect size after resize(n/2): " << vect.size();
    check(vect.size() == n / 2);
    std::cout << "Vect capacity after resize(n/2): " << vect.capacity();
    check(vect.capacity() == n);

    vect.resize(0);
    std::cout << "Vect size after resize(0): " << vect.size();
    check(vect.size() == 0);
    std::cout << "Vect capacity after resize(0): " << vect.capacity();
    check(vect.capacity() == n);

    vect.resize(n * 2);
    std::cout << "Vect size after resize(n*2): " << vect.size();
    check(vect.size() == n * 2);
    std::cout << "Vect capacity after resize(n*2): " << vect.capacity();
    check(vect.capacity() >= n * 2);
  }

  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(10000, 20000);

    int size = dist(gen);
    Vector<int> vect(size);
    std::fill_n(&vect[0], size, 123);

    // Test if resize does not default initilize
    vect.resize(0);
    vect.resize(size);
    int sum = std::accumulate(&vect[0], &vect[0] + vect.size(), 0);
    std::cout << "Vect sum after resize: " << sum;
    check(sum == 123 * size);
    std::cout << "Vect.end(): " << vect.end();
    check(vect.end() == vect.begin() + vect.size());

    // Test reallocation (old content must be copied into new vector)
    vect.resize(vect.size() * 2);
    sum = std::accumulate(&vect[0], &vect[0] + size, 0);
    std::cout << "Vect sum after reallocation: " << sum;
    check(sum == 123 * size);
  }

  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(10000, 20000);

    int size = dist(gen);
    Vector<int> vect(size);
    std::fill_n(&vect[0], size, 123);

    Vector<int> vect2 = std::move(vect);
    std::cout << "Vect1 empty after std::move: " << vect.empty();
    check(vect.empty() == true);
    int sum = std::accumulate(vect2.begin(), vect2.end(), 0);
    std::cout << "Vect2 sum after std::move: " << sum;
    check(sum == 123 * size);
  }

  {
    Array<unsigned int, 10> arr1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    auto arr2 = arr1;
    arr1.fill(0);

    std::cout << "arr2.size() = " << arr2.size();
    check(arr2.size() == 10);

    for (const auto& value : arr1)
    {
      std::cout << "arr1.value = " << value;
      check(value == 0);
    }

    for (std::size_t i = 0; i < 10; i++)
    {
      std::cout << "arr2[" << i << "] = " << arr2[i];
      check(arr2[i] == i);
    }
  }

  std::cout << std::endl;
  std::cout << "All tests passed successfully!" << std::endl;

  return 0;
}
