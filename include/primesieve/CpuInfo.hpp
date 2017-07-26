///
/// @file  CpuInfo.hpp
/// @brief Get the CPUs cache sizes in bytes.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CPUINFO_HPP
#define CPUINFO_HPP

#include <cstddef>

namespace primesieve {

class CpuInfo
{
public:
  CpuInfo();
  std::size_t l1CacheSize() const;
  std::size_t l2CacheSize() const;
  std::size_t l3CacheSize() const;

private:
  std::size_t l1CacheSize_;
  std::size_t l2CacheSize_;
  std::size_t l3CacheSize_;
  void initCache();
};

// Singleton
extern const CpuInfo cpuInfo;

} // namespace

#endif
