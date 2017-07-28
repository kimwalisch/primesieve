///
/// @file   cpu_info.cpp
/// @brief  Detect the CPUs' L1, L2 & L3 cache sizes
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/CpuInfo.hpp>
#include <iostream>

using namespace std;
using namespace primesieve;

int main()
{
  int exit_code = 0;

  cout << "L1 cache size: " << cpuInfo.l1CacheSize() / 1024 << " KB" << endl;
  cout << "L2 cache size: " << cpuInfo.l2CacheSize() / 1024 << " KB" << endl;
  cout << "L3 cache size: " << cpuInfo.l3CacheSize() / 1024 << " KB" << endl;

  if (cpuInfo.l1CacheSize() > 0 &&
      (cpuInfo.l1CacheSize() < 1024 ||
       cpuInfo.l1CacheSize() > (1ull << 40)))
  {
    cerr << "Error: L1 cache size does not look right!" << endl;
    exit_code = 1;
  }

  if (cpuInfo.l2CacheSize() > 0 &&
      (cpuInfo.l2CacheSize() < 1024 ||
       cpuInfo.l2CacheSize() > (1ull << 40)))
  {
    cerr << "Error: L2 cache size does not look right!" << endl;
    exit_code = 1;
  }

  if (cpuInfo.l3CacheSize() > 0 &&
      (cpuInfo.l3CacheSize() < 1024 ||
       cpuInfo.l3CacheSize() > (1ull << 40)))
  {
    cerr << "Error: L3 cache size does not look right!" << endl;
    exit_code = 1;
  }

  return exit_code;
}
