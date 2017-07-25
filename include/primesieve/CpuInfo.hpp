///
/// @file  CpuInfo.hpp
/// @brief Get the CPUs L1 & L2 cache sizes in bytes.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CPUINFO_HPP
#define CPUINFO_HPP

namespace primesieve {

class CpuInfo
{
public:
  CpuInfo();
  int l1CacheSize() const;
  int l2CacheSize() const;

private:
  int l1CacheSize_;
  int l2CacheSize_;
  void initL1Cache();
  void initL2Cache();
};

// Singleton
extern const CpuInfo cpuInfo;

} // namespace

#endif
