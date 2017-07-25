///
/// @file  CpuInfo.cpp
/// @brief Get the CPUs L1 & L2 cache sizes in bytes.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/CpuInfo.hpp>
#include <primesieve/pmath.hpp>

#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

#if defined(_WIN32)

#include <windows.h>

typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

#endif

using namespace std;

namespace {

/// default L1 cache bytes
const int L1_CACHE_SIZE = 32 << 10;

/// default L2 cache bytes
const int L2_CACHE_SIZE = 256 << 10;

}

namespace primesieve {

CpuInfo::CpuInfo()
  : l1CacheSize_(L1_CACHE_SIZE),
    l2CacheSize_(L2_CACHE_SIZE)
{
  initL1Cache();
  initL2Cache();

  // L2 cache size >= L1 cache size
  l2CacheSize_ = max(l1CacheSize_, l2CacheSize_);
}

int CpuInfo::l1CacheSize() const
{
  return l1CacheSize_;
}

int CpuInfo::l2CacheSize() const
{
  return l2CacheSize_;
}

#if defined(_WIN32)

void CpuInfo::initL1Cache()
{
  LPFN_GLPI glpi = (LPFN_GLPI) GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");

  if (glpi)
  {
    DWORD bytes = 0;
    glpi(0, &bytes);
    size_t size = bytes / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> info(size);
    glpi(info.data(), &bytes);

    for (size_t i = 0; i < size; i++)
    {
      if (info[i].Relationship == RelationCache &&
          info[i].Cache.Level == 1)
      {
        l1CacheSize_ = (int) info[i].Cache.Size;
        break;
      }
    }
  }

  // sieve size limits
  l1CacheSize_ = inBetween(16 << 10, l1CacheSize_, 2048 << 10);
  l1CacheSize_ = floorPowerOf2(l1CacheSize_);
}

void CpuInfo::initL2Cache()
{
  LPFN_GLPI glpi = (LPFN_GLPI) GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");

  if (glpi)
  {
    DWORD bytes = 0;
    glpi(0, &bytes);
    size_t size = bytes / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> info(size);
    glpi(info.data(), &bytes);

    for (size_t i = 0; i < size; i++)
    {
      if (info[i].Relationship == RelationCache &&
          info[i].Cache.Level == 2)
      {
        l2CacheSize_ = (int) info[i].Cache.Size;
        break;
      }
    }
  }

  // sieve size limits
  l2CacheSize_ = inBetween(16 << 10, l2CacheSize_, 2048 << 10);
  l2CacheSize_ = floorPowerOf2(l2CacheSize_);
}

#else

void CpuInfo::initL1Cache()
{
  /// Posix shell script for UNIX like OSes,
  /// returns log2 of L1_CACHE_SIZE in bytes.
  /// The script tries to get the L1 cache size using 3 different approaches:
  /// 1) getconf LEVEL1_DCACHE_SIZE
  /// 2) sysctl hw.l1dcachesize
  /// 3) cat /sys/devices/system/cpu/cpu0/cache/index0/size
  ///
  const string l1Script =
      R"(command -v getconf >/dev/null 2>/dev/null;

      if [ $? -eq 0 ];
      then
          # Returns L1 cache size in bytes
          L1_CACHE_SIZE=$(getconf LEVEL1_DCACHE_SIZE 2>/dev/null);
      fi;

      if [ "x$L1_CACHE_SIZE" = "x" ] || \
         [ "$L1_CACHE_SIZE" = "0" ];
      then
          # This method works on OS X
          command -v sysctl >/dev/null 2>/dev/null;

          if [ $? -eq 0 ];
          then
              # Returns L1 cache size in bytes
              L1_CACHE_SIZE=$(sysctl -n hw.l1dcachesize 2>/dev/null);
          fi;
      fi;

      if [ "x$L1_CACHE_SIZE" = "x" ] || \
         [ "$L1_CACHE_SIZE" = "0" ];
      then
          # Returns L1 cache size like e.g. 32K, 1M
          L1_CACHE_SIZE=$(cat /sys/devices/system/cpu/cpu0/cache/index0/size 2>/dev/null);

          if [ "x$L1_CACHE_SIZE" != "x" ];
          then
              is_kilobytes=$(echo $L1_CACHE_SIZE | grep K);
              if [ "x$is_kilobytes" != "x" ];
              then
                  L1_CACHE_SIZE=$(expr $(echo $L1_CACHE_SIZE | sed -e s'/K$//') '*' 1024);
              fi;
              is_megabytes=$(echo $L1_CACHE_SIZE | grep M);
              if [ "x$is_megabytes" != "x" ];
              then
                  L1_CACHE_SIZE=$(expr $(echo $L1_CACHE_SIZE | sed -e s'/M$//') '*' 1024 '*' 1024);
              fi;
          fi;
      fi;

      if [ "x$L1_CACHE_SIZE" = "x" ];
      then
          exit 1;
      fi;

      LOG2_CACHE_SIZE=0;

      while [ $L1_CACHE_SIZE -ge 2 ];
      do
          L1_CACHE_SIZE=$(expr $L1_CACHE_SIZE '/' 2);
          LOG2_CACHE_SIZE=$(expr $LOG2_CACHE_SIZE '+' 1);
      done;

      exit $LOG2_CACHE_SIZE
  )";

  int log2CacheSize = std::system(l1Script.c_str());
  log2CacheSize = WEXITSTATUS(log2CacheSize);

  if (log2CacheSize > 10 &&
      log2CacheSize < 22)
    l1CacheSize_ = 1 << log2CacheSize;

  // sieve size limits
  l1CacheSize_ = inBetween(16 << 10, l1CacheSize_, 2048 << 10);
  l1CacheSize_ = floorPowerOf2(l1CacheSize_);
}

void CpuInfo::initL2Cache()
{
  /// Posix shell script for UNIX like OSes,
  /// returns log2 of L2_CACHE_SIZE in bytes.
  /// The script tries to get the L1 cache size using 3 different approaches:
  /// 1) getconf LEVEL2_CACHE_SIZE
  /// 2) sysctl hw.l2cachesize
  /// 3) cat /sys/devices/system/cpu/cpu0/cache/index2/size
  ///
  const string l2Script = R"(
      command -v getconf >/dev/null 2>/dev/null;

      if [ $? -eq 0 ];
      then
          # Returns L1 cache size in bytes
          L2_CACHE_SIZE=$(getconf LEVEL2_CACHE_SIZE 2>/dev/null);
      fi;

      if [ "x$L2_CACHE_SIZE" = "x" ] || \
         [ "$L2_CACHE_SIZE" = "0" ];
      then
          # This method works on OS X
          command -v sysctl >/dev/null 2>/dev/null;

          if [ $? -eq 0 ];
          then
              # Returns L1 cache size in bytes
              L2_CACHE_SIZE=$(sysctl -n hw.l2cachesize 2>/dev/null);
          fi;
      fi;

      if [ "x$L2_CACHE_SIZE" = "x" ] || \
         [ "$L2_CACHE_SIZE" = "0" ];
      then
          # Returns L1 cache size like e.g. 32K, 1M
          L2_CACHE_SIZE=$(cat /sys/devices/system/cpu/cpu0/cache/index2/size 2>/dev/null);

          if [ "x$L2_CACHE_SIZE" != "x" ];
          then
              is_kilobytes=$(echo $L2_CACHE_SIZE | grep K);
              if [ "x$is_kilobytes" != "x" ];
              then
                  L2_CACHE_SIZE=$(expr $(echo $L2_CACHE_SIZE | sed -e s'/K$//') '*' 1024);
              fi;
              is_megabytes=$(echo $L2_CACHE_SIZE | grep M);
              if [ "x$is_megabytes" != "x" ];
              then
                  L2_CACHE_SIZE=$(expr $(echo $L2_CACHE_SIZE | sed -e s'/M$//') '*' 1024 '*' 1024);
              fi;
          fi;
      fi;

      if [ "x$L2_CACHE_SIZE" = "x" ];
      then
          exit 1;
      fi;

      LOG2_CACHE_SIZE=0;

      while [ $L2_CACHE_SIZE -ge 2 ];
      do
          L2_CACHE_SIZE=$(expr $L2_CACHE_SIZE '/' 2);
          LOG2_CACHE_SIZE=$(expr $LOG2_CACHE_SIZE '+' 1);
      done;

      exit $LOG2_CACHE_SIZE
  )";

  int log2CacheSize = std::system(l2Script.c_str());
  log2CacheSize = WEXITSTATUS(log2CacheSize);

  if (log2CacheSize > 10 &&
      log2CacheSize < 22)
    l2CacheSize_ = 1 << log2CacheSize;

  // sieve size limits
  l2CacheSize_ = inBetween(16 << 10, l2CacheSize_, 2048 << 10);
  l2CacheSize_ = floorPowerOf2(l2CacheSize_);
}

#endif

/// Singleton
const CpuInfo cpuInfo;

} // namespace
