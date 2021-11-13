///
/// @file   cpu_info.cpp
/// @brief  Detect the CPUs' cache sizes
///
/// Copyright (C) 2021 Kim Walisch, <kim.walisch@gmail.com>
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

  if (!cpu.hasLogicalCpuCores() &&
      cpu.logicalCpuCores() > 0)
  {
    cerr << "Invalid logical CPU cores: " << cpu.logicalCpuCores() << endl;
    return 1;
  }

  if (!cpu.hasL1Cache() &&
      cpu.l1CacheBytes() > 0)
  {
    cerr << "Invalid L1 cache size: " << cpu.l1CacheBytes() << endl;
    return 1;
  }

  if (!cpu.hasL2Cache() &&
      cpu.l2CacheBytes() > 0)
  {
    cerr << "Invalid L2 cache size: " << cpu.l2CacheBytes() << endl;
    return 1;
  }

  if (!cpu.hasL3Cache() &&
      cpu.l3CacheBytes() > 0)
  {
    cerr << "Invalid L3 cache size: " << cpu.l3CacheBytes() << endl;
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

  if (cpu.hasCpuName())
    cout << cpu.cpuName() << endl;

  cout << "L1 cache size: " << (cpu.l1CacheBytes() >> 10) << " KiB" << endl;
  cout << "L2 cache size: " << (cpu.l2CacheBytes() >> 10) << " KiB" << endl;
  cout << "L3 cache size: " << (cpu.l3CacheBytes() >> 10) << " KiB" << endl;

  return 0;
}
