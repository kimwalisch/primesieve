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
  cout << "L1 cache size: " << cpuInfo.l1CacheSize() / 1024 << " KB" << endl;
  cout << "L2 cache size: " << cpuInfo.l2CacheSize() / 1024 << " KB" << endl;
  cout << "L3 cache size: " << cpuInfo.l3CacheSize() / 1024 << " KB" << endl;

  return 0;
}
