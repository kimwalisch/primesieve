///
/// @file   cpu_info.cpp
/// @brief  Detect the CPUs' L1 & L2 cache sizes
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
  auto errors = cpuInfo.errorMsg();

  if (!errors.empty())
  {
    for (auto& msg : errors)
      cerr << "Error: " << msg << endl;

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
