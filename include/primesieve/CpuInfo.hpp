///
/// @file  CpuInfo.hpp
///
/// Copyright (C) 2023 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CPUINFO_HPP
#define CPUINFO_HPP

#include "Vector.hpp"

#include <cstddef>
#include <string>

namespace primesieve {

class CpuInfo
{
public:
  CpuInfo();
  bool hasCpuName() const;
  bool hasAVX512() const;
  bool hasLogicalCpuCores() const;
  bool hasL1Cache() const;
  bool hasL2Cache() const;
  bool hasL3Cache() const;
  bool hasL1Sharing() const;
  bool hasL2Sharing() const;
  bool hasL3Sharing() const;
  std::string cpuName() const;
  std::string getError() const;
  std::size_t l1CacheBytes() const;
  std::size_t l2CacheBytes() const;
  std::size_t l3CacheBytes() const;
  std::size_t l1Sharing() const;
  std::size_t l2Sharing() const;
  std::size_t l3Sharing() const;
  std::size_t logicalCpuCores() const;

private:
  void init();
  std::size_t logicalCpuCores_;
  Array<std::size_t, 4> cacheSizes_;
  Array<std::size_t, 4> cacheSharing_;
  std::string error_;
};

// Singleton
extern const CpuInfo cpuInfo;

} // namespace

#endif
