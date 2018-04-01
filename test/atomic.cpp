///
/// @file   atomic.cpp
/// @brief  Test std::atomic thread synchronization.
///         We use std::atomic for thread synchronization in
///         ParallelSieve.cpp.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <stdint.h>
#include <iostream>
#include <chrono>
#include <atomic>
#include <future>
#include <mutex>
#include <vector>

using namespace std;

void check(bool OK)
{
  cout << "   " << (OK ? "OK" : "ERROR") << "\n";
  if (!OK)
    exit(1);
}

double test_atomic(int threads)
{
  atomic<int> i(0);
  int iters = 1 << 23;
  int total = 0;

  // each thread executes 1 task
  auto task = [&]()
  {
    int count = 0;
    while (i++ < iters)
      count++;
    return count;
  };

  auto t1 = chrono::system_clock::now();

  vector<future<int>> futures;
  futures.reserve(threads);

  for (int t = 0; t < threads; t++)
    futures.emplace_back(async(launch::async, task));

  for (auto &f : futures)
    total += f.get();

  auto t2 = chrono::system_clock::now();
  chrono::duration<double> seconds = t2 - t1;
  double ops = total / seconds.count();

  cout << "Total atomic iters = " << total;
  check(total == iters);

  cout << "Atomic operations per seccond = " << (int) ops << endl;

  return ops;
}

double test_mutex(int threads)
{
  mutex lock;
  int i = 0;
  int iters = 1 << 19;
  int total = 0;

  // each thread executes 1 task
  auto task = [&]()
  {
    int count = 0;

    while (true)
    {
      lock_guard<mutex> guard(lock);
      if (i++ >= iters)
        return count;
      count++;
    }
  };

  auto t1 = chrono::system_clock::now();

  vector<future<int>> futures;
  futures.reserve(threads);

  for (int t = 0; t < threads; t++)
    futures.emplace_back(async(launch::async, task));

  for (auto &f : futures)
    total += f.get();

  auto t2 = chrono::system_clock::now();
  chrono::duration<double> seconds = t2 - t1;
  double ops = total / seconds.count();

  cout << "Total mutex iters = " << total;
  check(total == iters);

  cout << "Mutex operations per seccond = " << (int) ops << endl;

  return ops;
}

int main()
{
  int threads = thread::hardware_concurrency();
  threads = max(1, threads);

  double ops1 = test_atomic(threads);
  double ops2 = test_mutex(threads);

  cout << "Atomic/Mutex speedup: " << ops1 / ops2 << endl << endl;
  cout << "All tests passed successfully!" << endl;

  return 0;
}
