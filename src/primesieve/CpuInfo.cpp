///
/// @file   CpuInfo.cpp
/// @brief  Get the CPUs cache sizes in bytes.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/CpuInfo.hpp>
#include <cstddef>

#if defined(__APPLE__)
  #if !defined(__has_include)
    #define APPLE_SYSCTL
  #elif __has_include(<sys/types.h>) && \
        __has_include(<sys/sysctl.h>)
    #define APPLE_SYSCTL
  #endif
#endif

#if defined(_WIN32)

#include <windows.h>
#include <vector>

#elif defined(APPLE_SYSCTL)

#include <sys/types.h>
#include <sys/sysctl.h>
#include <vector>

#else // all other OSes

#include <exception>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

namespace {

string getString(const string& filename)
{
  ifstream file(filename);
  string str;

  if (file)
  {
    ostringstream oss;
    oss << file.rdbuf();
    str = oss.str();
  }

  return str;
}

size_t getValue(const string& filename)
{
  try
  {
    // first character after number
    size_t idx = 0;
    string str = getString(filename);
    size_t val = stol(str, &idx);

    if (idx < str.size())
    {
      // Last character may be:
      // 'K' = kilobytes
      // 'M' = megabytes
      // 'G' = gigabytes
      if (str[idx] == 'K')
        val *= 1024;
      if (str[idx] == 'M')
        val *= 1024 * 1024;
      if (str[idx] == 'G')
        val *= 1024 * 1024 * 1024;
    }

    return val;
  }
  catch (exception&)
  {
    return 0;
  }
}

} // namespace

#endif

using namespace std;

namespace primesieve {

CpuInfo::CpuInfo()
  : l1CacheSize_(0),
    l2CacheSize_(0),
    privateL2Cache_(false)
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

bool CpuInfo::privateL2Cache() const
{
  return privateL2Cache_;
}

bool CpuInfo::hasL1Cache() const
{
  return l1CacheSize_ >= (1 << 12) &&
         l1CacheSize_ <= (1ull << 40);
}

bool CpuInfo::hasL2Cache() const
{
  return l2CacheSize_ >= (1 << 12) &&
         l2CacheSize_ <= (1ull << 40);
}

#if defined(APPLE_SYSCTL)

void CpuInfo::initCache()
{
  size_t l1Length = sizeof(l1CacheSize_);
  size_t l2Length = sizeof(l2CacheSize_);

  sysctlbyname("hw.l1dcachesize", &l1CacheSize_, &l1Length, NULL, 0);
  sysctlbyname("hw.l2cachesize" , &l2CacheSize_, &l2Length, NULL, 0);

  size_t size = 0;

  if (!sysctlbyname("hw.cacheconfig", NULL, &size, NULL, 0))
  {
    size_t n = size / sizeof(size);
    vector<size_t> values(n);

    if (values.size() > 2)
    {
      // https://developer.apple.com/library/content/releasenotes/Performance/RN-AffinityAPI/index.html
      sysctlbyname("hw.cacheconfig" , &values[0], &size, NULL, 0);

      size_t l1CacheConfig = values[1];
      size_t l2CacheConfig = values[2];

      if (l2CacheConfig > 0 &&
          l1CacheConfig == l2CacheConfig)
      {
        privateL2Cache_ = true;
      }
    }
  }
}

#elif defined(_WIN32)

void CpuInfo::initCache()
{
  typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

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
        if (info[i].Cache.Level == 1 &&
            (info[i].Cache.Type == CacheData ||
             info[i].Cache.Type == CacheUnified))
        {
          l1CacheSize_ = info[i].Cache.Size;
        }

        if (info[i].Cache.Level == 2 &&
            (info[i].Cache.Type == CacheData ||
             info[i].Cache.Type == CacheUnified))
        {
          // Using the Windows API it is not possible to find out
          // whether the L2 cache is private or shared hence
          // we assume the L2 cache is private
          privateL2Cache_ = true;
          l2CacheSize_ = info[i].Cache.Size;
        }
      }
    }
  }
}

#else

/// This works on Linux and Android. We also use this
/// for all unknown OSes, it might work.
///
void CpuInfo::initCache()
{
  string l1CacheMap;
  string l2CacheMap;

  for (int i = 0; i <= 3; i++)
  {
    string filename = "/sys/devices/system/cpu/cpu0/cache/index" + to_string(i);

    string cacheLevel = filename + "/level";
    string cacheSize = filename + "/size";
    string cacheMap = filename + "/shared_cpu_map";
    string cacheType = filename + "/type";

    size_t level = getValue(cacheLevel);
    string type = getString(cacheType);

    if (level == 1 &&
        (type.find("Data") != string::npos ||
         type.find("Unified") != string::npos))
    {
      l1CacheSize_ = getValue(cacheSize);
      l1CacheMap = getString(cacheMap);
    }

    if (level == 2 &&
        (type.find("Data") != string::npos ||
         type.find("Unified") != string::npos))
    {
      l2CacheSize_ = getValue(cacheSize);
      l2CacheMap = getString(cacheMap);
    }
  }

  if (hasL2Cache() &&
      !l2CacheMap.empty() &&
      l2CacheMap == l1CacheMap)
  {
    privateL2Cache_ = true;
  }
}

#endif

/// Singleton
const CpuInfo cpuInfo;

} // namespace
