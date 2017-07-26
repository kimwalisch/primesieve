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
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#endif

#if !defined(_WIN32)

#include <array>
#include <cstdio>
#include <memory>

using namespace std;

namespace {

/// Execute command and return its stdout.
/// @see https://stackoverflow.com/a/478960/363778
///
string exec(string cmd)
{
  shared_ptr<FILE> file(popen(cmd.c_str(), "r"), pclose);
  string stdout;

  if (file)
  {
    array<char, 128> buffer;

    while (!feof(file.get()))
    {
      if (fgets(buffer.data(), buffer.size(), file.get()))
        stdout += buffer.data();
    }
  }

  return stdout;
}

/// Find cacheId in string and return its size.
/// Example: "L1_CACHE_SIZE=32768;"
///
size_t getCacheSize(string stdout, string cacheId)
{
  size_t pos = stdout.find(cacheId);
  size_t cacheSize = 0;
  string cacheStr;

  if (pos != string::npos)
  {
    size_t n1 = stdout.find_first_of('=', pos) + 1;
    size_t n2 = stdout.find_first_not_of("0123456789", n1);

    if (n1 != string::npos &&
        n2 != string::npos)
    {
      size_t length = n2 - n1;
      cacheStr = stdout.substr(n1, length);

      if (!cacheStr.empty())
        cacheSize = stol(cacheStr);

      if (cacheStr[n2] == 'K')
        cacheSize *= 1024;
      if (cacheStr[n2] == 'M')
        cacheSize *= 1024 * 1024;
      if (cacheStr[n2] == 'G')
        cacheSize *= 1024 * 1024 * 1024;
    }
  }

  return cacheSize;
}

} // namespace

#endif

using namespace std;

namespace primesieve {

CpuInfo::CpuInfo()
  : l1CacheSize_(0),
    l2CacheSize_(0),
    l3CacheSize_(0)
{
  initCache();
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

typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

void CpuInfo::initCache()
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
      if (info[i].Relationship == RelationCache)
      {
        if (info[i].Cache.Level == 1)
          l1CacheSize_ = info[i].Cache.Size;
        if (info[i].Cache.Level == 2)
          l2CacheSize_ = info[i].Cache.Size;
        if (info[i].Cache.Level == 3)
          l3CacheSize_ = info[i].Cache.Size;
      }
    }
  }
}

#else

void CpuInfo::initCache()
{
  // Posix shell script for UNIX like OSes
  // The script tries to get the cache size using 3 different approaches:
  // 1) getconf LEVEL*_CACHE_SIZE
  // 2) sysctl hw.l*cachesize
  // 3) cat /sys/devices/system/cpu/cpu0/cache/index*/size
  //
  const string cacheSizeScript = R"(
      command -v getconf >/dev/null 2>/dev/null;

      if [ $? -eq 0 ];
      then
          L1_CACHE_SIZE=$(getconf LEVEL1_DCACHE_SIZE 2>/dev/null);
          L2_CACHE_SIZE=$(getconf LEVEL2_CACHE_SIZE 2>/dev/null);
          L3_CACHE_SIZE=$(getconf LEVEL3_CACHE_SIZE 2>/dev/null);
      fi;

      command -v sysctl >/dev/null 2>/dev/null;

      if [ $? -eq 0 ];
      then
          if [ "x$L1_CACHE_SIZE" = "x" ] || \
             [ "$L1_CACHE_SIZE" = "0" ];
          then
              L1_CACHE_SIZE=$(sysctl -n hw.l1dcachesize 2>/dev/null);
          fi;

          if [ "x$L2_CACHE_SIZE" = "x" ] || \
             [ "$L2_CACHE_SIZE" = "0" ];
          then
              L2_CACHE_SIZE=$(sysctl -n hw.l2cachesize 2>/dev/null);
          fi;

          if [ "x$L3_CACHE_SIZE" = "x" ] || \
             [ "$L3_CACHE_SIZE" = "0" ];
          then
              L3_CACHE_SIZE=$(sysctl -n hw.l3cachesize 2>/dev/null);
          fi;
      fi;

      if [ "x$L1_CACHE_SIZE" = "x" ] || \
         [ "$L1_CACHE_SIZE" = "0" ];
      then
          L1_CACHE_SIZE=$(cat /sys/devices/system/cpu/cpu0/cache/index0/size 2>/dev/null);
      fi;

      if [ "x$L2_CACHE_SIZE" = "x" ] || \
         [ "$L2_CACHE_SIZE" = "0" ];
      then
          L2_CACHE_SIZE=$(cat /sys/devices/system/cpu/cpu0/cache/index2/size 2>/dev/null);
      fi;

      if [ "x$L3_CACHE_SIZE" = "x" ] || \
         [ "$L3_CACHE_SIZE" = "0" ];
      then
          L3_CACHE_SIZE=$(cat /sys/devices/system/cpu/cpu0/cache/index3/size 2>/dev/null);
      fi;

      echo "L1_CACHE_SIZE=$L1_CACHE_SIZE; \
            L2_CACHE_SIZE=$L2_CACHE_SIZE; \
            L3_CACHE_SIZE=$L3_CACHE_SIZE;"
  )";

  string stdout = exec(cacheSizeScript);

  l1CacheSize_ = getCacheSize(stdout, "L1_CACHE_SIZE");
  l2CacheSize_ = getCacheSize(stdout, "L2_CACHE_SIZE");
  l3CacheSize_ = getCacheSize(stdout, "L3_CACHE_SIZE");
}

#endif

/// Singleton
const CpuInfo cpuInfo;

} // namespace
