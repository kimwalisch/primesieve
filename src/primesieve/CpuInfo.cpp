///
/// @file  CpuInfo.cpp
/// @brief Get the CPUs cache sizes in bytes.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/CpuInfo.hpp>

#include <cstddef>
#include <cstdlib>
#include <string>
#include <vector>

#if defined(_WIN32)

#include <windows.h>

typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

#endif

using namespace std;

namespace primesieve {

CpuInfo::CpuInfo()
  : l1CacheSize_(0),
    l2CacheSize_(0),
    l3CacheSize_(0)
{
  initL1Cache();
  initL2Cache();
  initL3Cache();
}

size_t CpuInfo::l1CacheSize() const
{
  return l1CacheSize_;
}

size_t CpuInfo::l2CacheSize() const
{
  return l2CacheSize_;
}

size_t CpuInfo::l3CacheSize() const
{
  return l3CacheSize_;
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
        l1CacheSize_ = info[i].Cache.Size;
        break;
      }
    }
  }
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
        l2CacheSize_ = info[i].Cache.Size;
        break;
      }
    }
  }
}

void CpuInfo::initL3Cache()
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
          info[i].Cache.Level == 3)
      {
        l3CacheSize_ = info[i].Cache.Size;
        break;
      }
    }
  }
}

#else

void CpuInfo::initL1Cache()
{
  // Posix shell script for UNIX like OSes,
  // returns log2 of L1_CACHE_SIZE in bytes.
  // The script tries to get the L1 cache size using 3 different approaches:
  // 1) getconf LEVEL1_DCACHE_SIZE
  // 2) sysctl hw.l1dcachesize
  // 3) cat /sys/devices/system/cpu/cpu0/cache/index0/size
  //
  const string l1Script =
      R"(command -v getconf >/dev/null 2>/dev/null;

      if [ $? -eq 0 ];
      then
          L1_CACHE_SIZE=$(getconf LEVEL1_DCACHE_SIZE 2>/dev/null);
      fi;

      if [ "x$L1_CACHE_SIZE" = "x" ] || \
         [ "$L1_CACHE_SIZE" = "0" ];
      then
          command -v sysctl >/dev/null 2>/dev/null;

          if [ $? -eq 0 ];
          then
              L1_CACHE_SIZE=$(sysctl -n hw.l1dcachesize 2>/dev/null);
          fi;
      fi;

      if [ "x$L1_CACHE_SIZE" = "x" ] || \
         [ "$L1_CACHE_SIZE" = "0" ];
      then
          L1_CACHE_SIZE=$(cat /sys/devices/system/cpu/cpu0/cache/index0/size 2>/dev/null);

          if [ "x$L1_CACHE_SIZE" != "x" ];
          then
              is_kilobyte=$(echo $L1_CACHE_SIZE | grep K);
              if [ "x$is_kilobyte" != "x" ];
              then
                  L1_CACHE_SIZE=$(expr $(echo $L1_CACHE_SIZE | sed -e s'/K$//') '*' 1024);
              fi;
              is_megabyte=$(echo $L1_CACHE_SIZE | grep M);
              if [ "x$is_megabyte" != "x" ];
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

  int exitCode = std::system(l1Script.c_str());

#if defined(WEXITSTATUS)
  exitCode = WEXITSTATUS(exitCode);
#endif

  if (exitCode >= 12 &&
      exitCode <= 40)
    l1CacheSize_ = (size_t) 1 << exitCode;
}

void CpuInfo::initL2Cache()
{
  // Posix shell script for UNIX like OSes,
  // returns log2 of L2_CACHE_SIZE in bytes.
  // The script tries to get the L1 cache size using 3 different approaches:
  // 1) getconf LEVEL2_CACHE_SIZE
  // 2) sysctl hw.l2cachesize
  // 3) cat /sys/devices/system/cpu/cpu0/cache/index2/size
  //
  const string l2Script = R"(
      command -v getconf >/dev/null 2>/dev/null;

      if [ $? -eq 0 ];
      then
          L2_CACHE_SIZE=$(getconf LEVEL2_CACHE_SIZE 2>/dev/null);
      fi;

      if [ "x$L2_CACHE_SIZE" = "x" ] || \
         [ "$L2_CACHE_SIZE" = "0" ];
      then
          command -v sysctl >/dev/null 2>/dev/null;

          if [ $? -eq 0 ];
          then
              L2_CACHE_SIZE=$(sysctl -n hw.l2cachesize 2>/dev/null);
          fi;
      fi;

      if [ "x$L2_CACHE_SIZE" = "x" ] || \
         [ "$L2_CACHE_SIZE" = "0" ];
      then
          L2_CACHE_SIZE=$(cat /sys/devices/system/cpu/cpu0/cache/index2/size 2>/dev/null);

          if [ "x$L2_CACHE_SIZE" != "x" ];
          then
              is_kilobyte=$(echo $L2_CACHE_SIZE | grep K);
              if [ "x$is_kilobyte" != "x" ];
              then
                  L2_CACHE_SIZE=$(expr $(echo $L2_CACHE_SIZE | sed -e s'/K$//') '*' 1024);
              fi;
              is_megabyte=$(echo $L2_CACHE_SIZE | grep M);
              if [ "x$is_megabyte" != "x" ];
              then
                  L2_CACHE_SIZE=$(expr $(echo $L2_CACHE_SIZE | sed -e s'/M$//') '*' 1024 '*' 1024);
              fi;
              is_gigabyte=$(echo $L3_CACHE_SIZE | grep G);
              if [ "x$is_gigabyte" != "x" ];
              then
                  L3_CACHE_SIZE=$(expr $(echo $L3_CACHE_SIZE | sed -e s'/G$//') '*' 1024 '*' 1024 '*' 1024);
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

  int exitCode = std::system(l2Script.c_str());

#if defined(WEXITSTATUS)
  exitCode = WEXITSTATUS(exitCode);
#endif

  if (exitCode >= 12 &&
      exitCode <= 40)
    l2CacheSize_ = (size_t) 1 << exitCode;
}

void CpuInfo::initL3Cache()
{
  // Posix shell script for UNIX like OSes,
  // returns log2 of L3_CACHE_SIZE in bytes.
  // The script tries to get the L1 cache size using 3 different approaches:
  // 1) getconf LEVEL3_CACHE_SIZE
  // 2) sysctl hw.l3cachesize
  // 3) cat /sys/devices/system/cpu/cpu0/cache/index3/size
  //
  const string l3Script = R"(
      command -v getconf >/dev/null 2>/dev/null;

      if [ $? -eq 0 ];
      then
          L3_CACHE_SIZE=$(getconf LEVEL3_CACHE_SIZE 2>/dev/null);
      fi;

      if [ "x$L3_CACHE_SIZE" = "x" ] || \
         [ "$L3_CACHE_SIZE" = "0" ];
      then
          command -v sysctl >/dev/null 2>/dev/null;

          if [ $? -eq 0 ];
          then
              L3_CACHE_SIZE=$(sysctl -n hw.l3cachesize 2>/dev/null);
          fi;
      fi;

      if [ "x$L3_CACHE_SIZE" = "x" ] || \
         [ "$L3_CACHE_SIZE" = "0" ];
      then
          L3_CACHE_SIZE=$(cat /sys/devices/system/cpu/cpu0/cache/index3/size 2>/dev/null);

          if [ "x$L3_CACHE_SIZE" != "x" ];
          then
              is_kilobyte=$(echo $L3_CACHE_SIZE | grep K);
              if [ "x$is_kilobyte" != "x" ];
              then
                  L3_CACHE_SIZE=$(expr $(echo $L3_CACHE_SIZE | sed -e s'/K$//') '*' 1024);
              fi;
              is_megabyte=$(echo $L3_CACHE_SIZE | grep M);
              if [ "x$is_megabyte" != "x" ];
              then
                  L3_CACHE_SIZE=$(expr $(echo $L3_CACHE_SIZE | sed -e s'/M$//') '*' 1024 '*' 1024);
              fi;
              is_gigabyte=$(echo $L3_CACHE_SIZE | grep G);
              if [ "x$is_gigabyte" != "x" ];
              then
                  L3_CACHE_SIZE=$(expr $(echo $L3_CACHE_SIZE | sed -e s'/G$//') '*' 1024 '*' 1024 '*' 1024);
              fi;
          fi;
      fi;

      if [ "x$L3_CACHE_SIZE" = "x" ];
      then
          exit 1;
      fi;

      LOG2_CACHE_SIZE=0;

      while [ $L3_CACHE_SIZE -ge 2 ];
      do
          L3_CACHE_SIZE=$(expr $L3_CACHE_SIZE '/' 2);
          LOG2_CACHE_SIZE=$(expr $LOG2_CACHE_SIZE '+' 1);
      done;

      exit $LOG2_CACHE_SIZE
  )";

  int exitCode = std::system(l3Script.c_str());

#if defined(WEXITSTATUS)
  exitCode = WEXITSTATUS(exitCode);
#endif

  if (exitCode >= 12 &&
      exitCode <= 40)
    l3CacheSize_ = (size_t) 1 << exitCode;
}

#endif

/// Singleton
const CpuInfo cpuInfo;

} // namespace
