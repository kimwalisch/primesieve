///
/// @file   cpu_info.cpp
/// @brief  Detect the CPUs' cache sizes
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <CpuInfo.hpp>

#include <iostream>
#include <string>

using namespace primesieve;

int main()
{
  const CpuInfo cpu;
  std::string error = cpu.getError();

  if (!error.empty())
  {
    std::cerr << "Error: " << error << std::endl;
    return 1;
  }

  if (!cpu.hasLogicalCpuCores() &&
      cpu.logicalCpuCores() > 0)
  {
    std::cerr << "Invalid logical CPU cores: " << cpu.logicalCpuCores() << std::endl;
    return 1;
  }

  if (!cpu.hasL1Cache() &&
      cpu.l1CacheBytes() > 0)
  {
    std::cerr << "Invalid L1 cache size: " << cpu.l1CacheBytes() << std::endl;
    return 1;
  }

  if (!cpu.hasL2Cache() &&
      cpu.l2CacheBytes() > 0)
  {
    std::cerr << "Invalid L2 cache size: " << cpu.l2CacheBytes() << std::endl;
    return 1;
  }

  if (!cpu.hasL3Cache() &&
      cpu.l3CacheBytes() > 0)
  {
    std::cerr << "Invalid L3 cache size: " << cpu.l3CacheBytes() << std::endl;
    return 1;
  }

  if (!cpu.hasL1Sharing() &&
      cpu.l1Sharing() > 0)
  {
    std::cerr << "Invalid L1 cache sharing: " << cpu.l1Sharing() << std::endl;
    return 1;
  }

  if (!cpu.hasL2Sharing() &&
      cpu.l2Sharing() > 0)
  {
    std::cerr << "Invalid L2 cache sharing: " << cpu.l2Sharing() << std::endl;
    return 1;
  }

  if (!cpu.hasL3Sharing() &&
      cpu.l3Sharing() > 0)
  {
    std::cerr << "Invalid L3 cache sharing: " << cpu.l3Sharing() << std::endl;
    return 1;
  }

  if (cpu.hasCpuName())
    std::cout << cpu.cpuName() << std::endl;

  std::cout << "L1 cache size: " << (cpu.l1CacheBytes() >> 10) << " KiB" << std::endl;
  std::cout << "L2 cache size: " << (cpu.l2CacheBytes() >> 10) << " KiB" << std::endl;
  std::cout << "L3 cache size: " << (cpu.l3CacheBytes() >> 10) << " KiB" << std::endl;

  return 0;
}
