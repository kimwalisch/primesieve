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

  if (!cpuInfo.hasCpuCores() &&
      cpuInfo.cpuCores() > 0)
  {
    cerr << "Invalid CPU cores: " << cpuInfo.cpuCores() << endl;
    return 1;
  }

  if (!cpuInfo.hasCpuThreads() &&
      cpuInfo.cpuThreads() > 0)
  {
    cerr << "Invalid CPU threads: " << cpuInfo.cpuThreads() << endl;
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

  if (!cpuInfo.hasL3Cache() &&
      cpuInfo.l3CacheSize() > 0)
  {
    cerr << "Invalid L3 cache size: " << cpuInfo.l3CacheSize() << endl;
    return 1;
  }

  if (!cpuInfo.hasL1Sharing() &&
      cpuInfo.l1Sharing() > 0)
  {
    cerr << "Invalid L1 cache sharing: " << cpuInfo.l1Sharing() << endl;
    return 1;
  }

  if (!cpuInfo.hasL2Sharing() &&
      cpuInfo.l2Sharing() > 0)
  {
    cerr << "Invalid L2 cache sharing: " << cpuInfo.l2Sharing() << endl;
    return 1;
  }

  if (!cpuInfo.hasL3Sharing() &&
      cpuInfo.l3Sharing() > 0)
  {
    cerr << "Invalid L3 cache sharing: " << cpuInfo.l3Sharing() << endl;
    return 1;
  }

  if (!cpuInfo.hasThreadsPerCore() &&
      cpuInfo.threadsPerCore() > 0)
  {
    cerr << "Invalid threads per CPU core: " << cpuInfo.threadsPerCore() << endl;
    return 1;
  }

  if (cpuInfo.hasCpuName())
    cout << cpuInfo.cpuName() << endl;

  cout << "L1 cache size: " << (cpuInfo.l1CacheSize() >> 10) << " KiB" << endl;
  cout << "L2 cache size: " << (cpuInfo.l2CacheSize() >> 10) << " KiB" << endl;
  cout << "L3 cache size: " << (cpuInfo.l3CacheSize() >> 10) << " KiB" << endl;

  if (cpuInfo.hasL2Cache())
  {
    if (cpuInfo.hasPrivateL2Cache())
      cout << "L2 cache: private" << endl;
    else
      cout << "L2 cache: shared"  << endl;
  }

  return 0;
}
