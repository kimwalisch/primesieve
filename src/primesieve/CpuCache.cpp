///
/// @file  CpuCache.cpp
/// @brief Get the CPUs L1 & L2 cache sizes in bytes.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/config.hpp>
#include <primesieve/CpuCache.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <cstdlib>
#include <vector>

#if defined(_WIN32)

#include <windows.h>

typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

#endif

using namespace std;

namespace {

// default L1 cache bytes
const uint_t L1_CACHE_SIZE = 32 << 10;

// default L2 cache bytes
const uint_t L2_CACHE_SIZE = 256 << 10;

}

namespace primesieve {

// Singleton
const CpuCache cpuCache;

CpuCache::CpuCache()
  : l1CacheSize_(L1_CACHE_SIZE),
    l2CacheSize_(L2_CACHE_SIZE)
{
  initL1Cache();
  initL2Cache();

  // L2 cache size >= L1 cache size
  l2CacheSize_ = max(l1CacheSize_, l2CacheSize_);
}

#if defined(_WIN32)

uint_t CpuCache::l1CacheSize() const
{
  return l1CacheSize_;
}

uint_t CpuCache::l2CacheSize() const
{
  return l2CacheSize_;
}

void CpuCache::initL1Cache()
{
  LPFN_GLPI glpi = (LPFN_GLPI) GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");

  if (glpi)
  {
    DWORD info_bytes = 0;
    glpi(0, &info_bytes);
    size_t size = info_bytes / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> info(size);
    glpi(info.data(), &info_bytes);

    for (size_t i = 0; i < size; i++)
    {
      if (info[i].Relationship == RelationCache &&
          info[i].Cache.Level == 1)
      {
        l1CacheSize_ = (uint_t) info[i].Cache.Size;
        break;
      }
    }
  }

  // sieve size limits
  l1CacheSize_ = inBetween(16 << 10, l1CacheSize_, 2048 << 10);
  l1CacheSize_ = floorPower2(l1CacheSize_);
}

void CpuCache::initL2Cache()
{
  LPFN_GLPI glpi = (LPFN_GLPI) GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");

  if (glpi)
  {
    DWORD info_bytes = 0;
    glpi(0, &info_bytes);
    size_t size = info_bytes / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> info(size);
    glpi(info.data(), &info_bytes);

    for (size_t i = 0; i < size; i++)
    {
      if (info[i].Relationship == RelationCache &&
          info[i].Cache.Level == 2)
      {
        l2CacheSize_ = (uint_t) info[i].Cache.Size;
        break;
      }
    }
  }

  // sieve size limits
  l2CacheSize_ = inBetween(16 << 10, l2CacheSize_, 2048 << 10);
  l2CacheSize_ = floorPower2(l2CacheSize_);
}

#else



#endif

} // namespace
