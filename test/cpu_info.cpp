///
/// @file   cpu_info.cpp
/// @brief  Detect the CPUs' cache sizes
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
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
  const CpuInfo cpu;
  string error = cpu.getError();

  if (!error.empty())
  {
    cerr << "Error: " << error << endl;
    return 1;
  }

  if (!cpu.hasCpuCores() &&
      cpu.cpuCores() > 0)
  {
    cerr << "Invalid CPU cores: " << cpu.cpuCores() << endl;
    return 1;
  }

  if (!cpu.hasCpuThreads() &&
      cpu.cpuThreads() > 0)
  {
    cerr << "Invalid CPU threads: " << cpu.cpuThreads() << endl;
    return 1;
  }

  if (!cpu.hasL1Cache() &&
      cpu.l1CacheSize() > 0)
  {
    cerr << "Invalid L1 cache size: " << cpu.l1CacheSize() << endl;
    return 1;
  }

  if (!cpu.hasL2Cache() &&
      cpu.l2CacheSize() > 0)
  {
    cerr << "Invalid L2 cache size: " << cpu.l2CacheSize() << endl;
    return 1;
  }

  if (!cpu.hasL3Cache() &&
      cpu.l3CacheSize() > 0)
  {
    cerr << "Invalid L3 cache size: " << cpu.l3CacheSize() << endl;
    return 1;
  }

  if (!cpu.hasL1Sharing() &&
      cpu.l1Sharing() > 0)
  {
    cerr << "Invalid L1 cache sharing: " << cpu.l1Sharing() << endl;
    return 1;
  }

  if (!cpu.hasL2Sharing() &&
      cpu.l2Sharing() > 0)
  {
    cerr << "Invalid L2 cache sharing: " << cpu.l2Sharing() << endl;
    return 1;
  }

  if (!cpu.hasL3Sharing() &&
      cpu.l3Sharing() > 0)
  {
    cerr << "Invalid L3 cache sharing: " << cpu.l3Sharing() << endl;
    return 1;
  }

  if (!cpu.hasThreadsPerCore() &&
      cpu.threadsPerCore() > 0)
  {
    cerr << "Invalid threads per CPU core: " << cpu.threadsPerCore() << endl;
    return 1;
  }

  if (cpu.hasCpuName())
    cout << cpu.cpuName() << endl;

  cout << "L1 cache size: " << (cpu.l1CacheSize() >> 10) << " KiB" << endl;
  cout << "L2 cache size: " << (cpu.l2CacheSize() >> 10) << " KiB" << endl;
  cout << "L3 cache size: " << (cpu.l3CacheSize() >> 10) << " KiB" << endl;

  if (cpu.hasL2Cache())
  {
    if (cpu.hasPrivateL2Cache())
      cout << "L2 cache: private" << endl;
    else
      cout << "L2 cache: shared"  << endl;
  }

  return 0;
}
