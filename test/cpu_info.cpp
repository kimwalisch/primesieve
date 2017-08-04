///
/// @file   cpu_info.cpp
/// @brief  Detect the CPUs' cache sizes
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
  string error = cpuInfo.getError();

  if (!error.empty())
  {
    cerr << "Error: " << error << endl;
    return 1;
  }

  if (!cpuInfo.hasL1Cache() &&
      cpuInfo.l1CacheSize() > 0)
  {
    cerr << "Invalid L1 cache size: " << cpuInfo.l1CacheSize() << endl;
    return 1;
  }

  if (!cpuInfo.hasL2Cache() &&
      cpuInfo.l2CacheSize() > 0)
  {
    cerr << "Invalid L2 cache size: " << cpuInfo.l2CacheSize() << endl;
    return 1;
  }

  cout << "L1 cache size: " << cpuInfo.l1CacheSize() / 1024 << " KB" << endl;
  cout << "L2 cache size: " << cpuInfo.l2CacheSize() / 1024 << " KB" << endl;

  if (cpuInfo.hasL2Cache())
  {
    if (cpuInfo.privateL2Cache())
      cout << "L2 cache: private" << endl;
    else
      cout << "L2 cache: shared"  << endl;
  }

  return 0;
}
