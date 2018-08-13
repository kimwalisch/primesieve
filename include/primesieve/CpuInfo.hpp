///
/// @file  CpuInfo.hpp
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CPUINFO_HPP
#define CPUINFO_HPP

#include <cstddef>
#include <string>
#include <array>

namespace primesieve {

class CpuInfo
{
public:
  CpuInfo();
  bool hasCpuName() const;
  bool hasCpuCores() const;
  bool hasCpuThreads() const;
  bool hasL1Cache() const;
  bool hasL2Cache() const;
  bool hasL3Cache() const;
  bool hasL1Sharing() const;
  bool hasL2Sharing() const;
  bool hasL3Sharing() const;
  bool hasThreadsPerCore() const;
  bool hasPrivateL2Cache() const;
  std::string cpuName() const;
  std::string getError() const;
  std::size_t l1CacheSize() const;
  std::size_t l2CacheSize() const;
  std::size_t l3CacheSize() const;
  std::size_t l1Sharing() const;
  std::size_t l2Sharing() const;
  std::size_t l3Sharing() const;
  std::size_t cpuCores() const;
  std::size_t cpuThreads() const;
  std::size_t threadsPerCore() const;

private:
  void init();
  std::size_t cpuCores_;
  std::size_t cpuThreads_;
  std::size_t threadsPerCore_;
  std::array<std::size_t, 4> cacheSizes_;
  std::array<std::size_t, 4> cacheSharing_;
  std::string error_;
};

// Singleton
extern const CpuInfo cpuInfo;

} // namespace

#endif
