///
/// @file   CpuInfo.cpp
/// @brief  Get detailed information about the CPU's caches.
///         Ideally each primesieve thread should use a sieve array
///         size that corresponds to the cache sizes of the CPU core
///         the thread is currently running on. Unfortunately, due to
///         the many different operating systems, compilers and CPU
///         architectures this is difficult to implement (portably).
///         Furthermore, any thread may randomly be moved to another
///         CPU core by the operating system scheduler.
///
///         Hence in order to ensure good scaling we use the following
///         alternative strategy: We detect the cache sizes of one of
///         the CPU cores at startup and all primesieve threads use a
///         sieve array size that is related to that CPU core's cache
///         sizes. For homogeneous CPUs with just one type of CPU core
///         this strategy is optimal. For hybrid CPUs with multiple
///         types of CPU cores we try to detect the cache sizes of the
///         CPU core type that e.g. occurs most frequently.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "CpuInfo.hpp"
#include <primesieve/macros.hpp>

#include <algorithm>
#include <stdint.h>
#include <cstddef>
#include <exception>
#include <string>
#include <vector>

#if defined(__APPLE__) && \
    __has_include(<sys/sysctl.h>)
  #define APPLE_SYSCTL
#endif

#if defined(_WIN32)

#include <primesieve/pmath.hpp>

#include <windows.h>
#include <intrin.h>
#include <iterator>
#include <map>

namespace {

std::string getCpuName()
{
  std::string cpuName;

#if defined(__i386__) || \
    defined(__x86_64__) || \
    defined(_M_IX86) || \
    defined(_M_X64)

  // Get the CPU name using CPUID.
  // Example: Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz
  // https://en.wikipedia.org/wiki/CPUID

  int cpuInfo[4] = { 0, 0, 0, 0 };
  __cpuidex(cpuInfo, 0x80000000, 0);
  std::vector<int> vect;

  // check if CPU name is supported
  if ((unsigned) cpuInfo[0] >= 0x80000004u)
  {
    __cpuidex(cpuInfo, 0x80000002, 0);
    std::copy_n(cpuInfo, 4, std::back_inserter(vect));

    __cpuidex(cpuInfo, 0x80000003, 0);
    std::copy_n(cpuInfo, 4, std::back_inserter(vect));

    __cpuidex(cpuInfo, 0x80000004, 0);
    std::copy_n(cpuInfo, 4, std::back_inserter(vect));

    vect.push_back(0);
    cpuName = (char*) vect.data();

    auto trimString = [](std::string& str)
    {
      std::string spaceChars = " \f\n\r\t\v";
      std::size_t pos = str.find_first_not_of(spaceChars);
      str.erase(0, pos);

      pos = str.find_last_not_of(spaceChars);
      if (pos != std::string::npos)
        str.erase(pos + 1);
    };

    trimString(cpuName);
  }
#endif

  return cpuName;
}

} // namespace

namespace primesieve {

void CpuInfo::init()
{
// Windows 7 (2009) or later
#if _WIN32_WINNT >= 0x0601

  using LPFN_GAPC = decltype(&GetActiveProcessorCount);
  LPFN_GAPC getCpuCoreCount = (LPFN_GAPC) (void*) GetProcAddress(
      GetModuleHandle(TEXT("kernel32")), "GetActiveProcessorCount");

  if (getCpuCoreCount)
    logicalCpuCores_ = getCpuCoreCount(ALL_PROCESSOR_GROUPS);

  using LPFN_GLPIEX = decltype(&GetLogicalProcessorInformationEx);
  LPFN_GLPIEX glpiex = (LPFN_GLPIEX) (void*) GetProcAddress(
      GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformationEx");

  if (!glpiex)
    return;

  DWORD bytes = 0;
  glpiex(RelationCache, 0, &bytes);

  if (!bytes)
    return;

  std::vector<char> buffer(bytes);

  if (!glpiex(RelationCache, (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*) &buffer[0], &bytes))
    return;

  struct CpuCoreCacheInfo
  {
    CpuCoreCacheInfo() :
      cacheSizes{0, 0, 0, 0},
      cacheSharing{0, 0, 0, 0}
    { }
    Array<std::size_t, 4> cacheSizes;
    Array<std::size_t, 4> cacheSharing;
  };

  struct L1CacheInfo
  {
    long cpuCoreId = -1;
    std::size_t cpuCoreCount = 0;
  };

  using CacheSize_t = std::size_t;
  // Items must be sorted in ascending order
  std::map<CacheSize_t, L1CacheInfo> l1CacheSizes;
  std::map<std::size_t, CpuCoreCacheInfo> cpuCores;
  std::size_t totalL1CpuCores = 0;
  SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* info;

  // Fill the cpuCores map with the L1, L2 & L3 cache
  // sizes and cache sharing of each CPU core.
  for (std::size_t i = 0; i < bytes; i += info->Size)
  {
    info = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*) &buffer[i];

    if (info->Relationship == RelationCache &&
        info->Cache.Level >= 1 &&
        info->Cache.Level <= 3 &&
        (info->Cache.Type == CacheData ||
         info->Cache.Type == CacheUnified))
    {
      std::size_t cpuCoreIndex = 0;
      std::size_t cacheSharing = 0;

      // Cache.GroupMask.Mask contains one bit set for
      // each logical CPU core sharing the cache.
      for (auto mask = info->Cache.GroupMask.Mask; mask > 0; mask &= mask - 1)
        cacheSharing++;

      auto cacheSize = info->Cache.CacheSize;
      auto level = info->Cache.Level;
      auto processorGroup = info->Cache.GroupMask.Group;
      using Mask_t = decltype(info->Cache.GroupMask.Mask);
      auto maxCpusPerProcessorGroup = numberOfBits<Mask_t>();
      auto mask = info->Cache.GroupMask.Mask;
      mask = (mask == 0) ? 1 : mask;

      for (; mask > 0; mask &= mask - 1)
      {
        // Convert next 1 bit into cpuCoreIndex,
        // by counting trailing zeros.
        while (!(mask & (((Mask_t) 1) << cpuCoreIndex)))
          cpuCoreIndex++;

        // Note that calculating the cpuCoreId this way is not
        // 100% correct as processor groups may not be fully
        // filled (they may have less than maxCpusPerProcessorGroup
        // CPU cores). However our formula yields unique
        // cpuCoreIds which is good enough for our usage.
        std::size_t cpuCoreId = processorGroup * maxCpusPerProcessorGroup + cpuCoreIndex;

        // If the CPU core has multiple caches of the same level,
        // then we are only interested in the smallest such
        // cache since this is likely the fastest cache.
        if (cpuCores[cpuCoreId].cacheSizes[level] > 0 &&
            cpuCores[cpuCoreId].cacheSizes[level] <= cacheSize)
          continue;

        cpuCores[cpuCoreId].cacheSizes[level] = cacheSize;
        cpuCores[cpuCoreId].cacheSharing[level] = cacheSharing;
      }
    }
  }

  // Iterate over all CPU cores and create a map
  // with the different L1 cache sizes.
  for (const auto& cpuCore : cpuCores)
  {
    const auto& cpuCoreInfo = cpuCore.second;
    std::size_t l1CacheSize = cpuCoreInfo.cacheSizes[1];
    std::size_t cpuCoreId = cpuCore.first;

    if (l1CacheSize > 0)
    {
      totalL1CpuCores++;
      auto& mapEntry = l1CacheSizes[l1CacheSize];
      mapEntry.cpuCoreCount++;
      if (mapEntry.cpuCoreId == -1)
        mapEntry.cpuCoreId = (long) cpuCoreId;
    }
  }

  // Check if one of the L1 cache types is used
  // by more than 80% of all CPU cores.
  for (const auto& l1CacheSize : l1CacheSizes)
  {
    if (l1CacheSize.second.cpuCoreCount > totalL1CpuCores * 0.80)
    {
      long cpuCoreId = l1CacheSize.second.cpuCoreId;
      cacheSizes_ = cpuCores[cpuCoreId].cacheSizes;
      cacheSharing_ = cpuCores[cpuCoreId].cacheSharing;
      return;
    }
  }

  // For hybrid CPUs with many different L1 cache types
  // we pick one from the middle that is hopefully
  // representative for the CPU's overall performance.
  if (!l1CacheSizes.empty())
  {
    auto iter = l1CacheSizes.begin();
    std::advance(iter, (l1CacheSizes.size() - 1) / 2);
    long cpuCoreId = iter->second.cpuCoreId;
    cacheSizes_ = cpuCores[cpuCoreId].cacheSizes;
    cacheSharing_ = cpuCores[cpuCoreId].cacheSharing;
  }

// Windows XP or later
#elif _WIN32_WINNT >= 0x0501

  using LPFN_GLPI = decltype(&GetLogicalProcessorInformation);
  LPFN_GLPI glpi = (LPFN_GLPI) (void*) GetProcAddress(
      GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");

  if (!glpi)
    return;

  DWORD bytes = 0;
  glpi(0, &bytes);

  if (!bytes)
    return;

  std::size_t threadsPerCore = 0;
  std::size_t size = bytes / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
  std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> info(size);

  if (!glpi(&info[0], &bytes))
    return;

  for (std::size_t i = 0; i < size; i++)
  {
    if (info[i].Relationship == RelationProcessorCore)
    {
      // ProcessorMask contains one bit set for
      // each logical CPU core related to the
      // current physical CPU core
      threadsPerCore = 0;
      for (auto mask = info[i].ProcessorMask; mask > 0; mask &= mask - 1)
        threadsPerCore++;

      logicalCpuCores_ += threadsPerCore;
    }
  }

  for (std::size_t i = 0; i < size; i++)
  {
    if (info[i].Relationship == RelationCache &&
        info[i].Cache.Level >= 1 &&
        info[i].Cache.Level <= 3 &&
        (info[i].Cache.Type == CacheData ||
         info[i].Cache.Type == CacheUnified))
    {
      auto level = info[i].Cache.Level;
      auto cacheSize = info[i].Cache.Size;

      // If the CPU core has multiple caches of the same level,
      // then we are only interested in the smallest such
      // cache since this is likely the fastest cache.
      if (cacheSizes_[level] > 0 &&
          cacheSizes_[level] <= cacheSize)
        continue;

      cacheSizes_[level] = cacheSize;

      // We assume the L1 and L2 caches are private
      if (info[i].Cache.Level <= 2)
        cacheSharing_[level] = threadsPerCore;

      // We assume the L3 cache is shared
      if (info[i].Cache.Level == 3)
        cacheSharing_[level] = logicalCpuCores_;
    }
  }

  // Most Intel CPUs have an L3 cache.
  // But a few Intel CPUs don't have an L3 cache
  // and for many of those CPUs the L2 cache is
  // shared e.g. Intel Core 2 Duo/Quad CPUs and
  // Intel Atom x5-Z8350 CPUs.
  if (hasL2Cache() && !hasL3Cache())
    cacheSharing_[2] = logicalCpuCores_;
#endif
}

} // namespace

#elif defined(APPLE_SYSCTL)

#include <primesieve/pmath.hpp>
#include <sys/sysctl.h>

namespace {

/// Get CPU information from the operating
/// system kernel using sysctl.
/// https://www.freebsd.org/cgi/man.cgi?sysctl(3)
///
template <typename T>
std::vector<T> getSysctl(const std::string& name)
{
  std::vector<T> res;
  std::size_t bytes = 0;

  if (!sysctlbyname(name.data(), 0, &bytes, 0, 0))
  {
    std::size_t size = ceilDiv(bytes, sizeof(T));
    std::vector<T> buffer(size, 0);
    if (!sysctlbyname(name.data(), buffer.data(), &bytes, 0, 0))
      res = buffer;
  }

  return res;
}

std::string getCpuName()
{
  std::string cpuName;

  auto buffer = getSysctl<char>("machdep.cpu.brand_string");
  if (!buffer.empty())
    cpuName = buffer.data();

  return cpuName;
}

} // namespace

namespace primesieve {

void CpuInfo::init()
{
  auto logicalCpuCores = getSysctl<std::size_t>("hw.logicalcpu");
  if (!logicalCpuCores.empty())
    logicalCpuCores_ = logicalCpuCores[0];

  // https://developer.apple.com/library/content/releasenotes/Performance/RN-AffinityAPI/index.html
  auto cacheSizes = getSysctl<std::size_t>("hw.cachesize");
  for (std::size_t i = 1; i < std::min(cacheSizes.size(), cacheSizes_.size()); i++)
    cacheSizes_[i] = cacheSizes[i];

  // https://developer.apple.com/library/content/releasenotes/Performance/RN-AffinityAPI/index.html
  auto cacheConfig = getSysctl<std::size_t>("hw.cacheconfig");
  for (std::size_t i = 1; i < std::min(cacheConfig.size(), cacheSharing_.size()); i++)
    cacheSharing_[i] = cacheConfig[i];
}

} // namespace

#else // Linux (and unknown OSes)

#include <primesieve/primesieve_error.hpp>

#include <cctype>
#include <fstream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>

using namespace primesieve;

namespace {

/// Remove all leading and trailing
/// space characters.
///
void trimString(std::string& str)
{
  std::string spaceChars = " \f\n\r\t\v";
  std::size_t pos = str.find_first_not_of(spaceChars);
  str.erase(0, pos);

  pos = str.find_last_not_of(spaceChars);
  if (pos != std::string::npos)
    str.erase(pos + 1);
}

std::string getString(const std::string& filename)
{
  std::ifstream file(filename);
  std::string str;

  // Read the first string,
  // stops at any space character
  if (file && (file >> str))
    return str;
  else
    return {};
}

std::size_t getValue(const std::string& filename)
{
  std::string str = getString(filename);
  std::size_t val = 0;

  if (!str.empty())
    val = std::stoul(str);

  return val;
}

std::size_t getCacheSize(const std::string& filename)
{
  std::string str = getString(filename);
  std::size_t val = 0;

  if (!str.empty())
  {
    val = std::stoul(str);
    char lastChar = str.back();

    // The last character may be:
    // 'K' = KiB (kibibyte)
    // 'M' = MiB (mebibyte)
    // 'G' = GiB (gibibyte)
    switch (lastChar)
    {
      case 'K': val *= 1 << 10; break;
      case 'M': val *= 1 << 20; break;
      case 'G': val *= 1 << 30; break;
      default:
        if (!isdigit(lastChar))
          throw primesieve_error("invalid cache size: " + str);
    }
  }

  return val;
}

/// Converts /proc/cpuinfo line into CPU name.
/// Returns an empty string if line does
/// not contain the CPU name.
///
std::string getCpuName(const std::string& line)
{
  // Examples of CPU names:
  // model name : Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz
  // Processor  : ARMv7 Processor rev 5 (v7l)
  // cpu        : POWER9 (raw), altivec supported
  const std::set<std::string> cpuLabels
  {
    "model name",
    "Processor",
    "cpu"
  };

  std::size_t pos = line.find(':');
  std::string cpuName;

  if (pos != std::string::npos)
  {
    std::string label = line.substr(0, pos);
    trimString(label);
    if (cpuLabels.find(label) != cpuLabels.end())
      cpuName = line.substr(pos + 1);
  }

  return cpuName;
}

bool isValid(const std::string& cpuName)
{
  if (cpuName.empty())
    return false;
  if (cpuName.find_first_not_of("0123456789") == std::string::npos)
    return false;

  return true;
}

std::string getCpuName()
{
  std::ifstream file("/proc/cpuinfo");
  std::string notFound;

  if (file)
  {
    std::string line;
    std::size_t i = 0;

    while (std::getline(file, line))
    {
      std::string cpuName = getCpuName(line);
      trimString(cpuName);

      if (isValid(cpuName))
        return cpuName;
      if (++i > 10)
        return notFound;
    }
  }

  return notFound;
}

std::vector<std::string> split(const std::string& str,
                               char delimiter)
{
  std::string token;
  std::vector<std::string> tokens;
  std::istringstream tokenStream(str);

  while (std::getline(tokenStream, token, delimiter))
    tokens.push_back(token);

  return tokens;
}

/// A thread list file contains a human
/// readable list of thread IDs.
/// Example: 0-8,18-26
/// https://www.kernel.org/doc/Documentation/cputopology.txt
///
std::size_t parseThreadList(const std::string& filename)
{
  std::size_t threads = 0;
  auto threadList = getString(filename);
  auto tokens = split(threadList, ',');

  for (auto& str : tokens)
  {
    auto values = split(str, '-');
    if (values.size() == 1)
      threads++;
    else
    {
      auto t0 = std::stoul(values.at(0));
      auto t1 = std::stoul(values.at(1));
      threads += t1 - t0 + 1;
    }
  }

  return threads;
}

/// A thread map file contains a hexadecimal
/// or binary string where each set bit
/// corresponds to a specific thread ID.
/// Example: 00000000,00000000,00000000,07fc01ff
/// https://www.kernel.org/doc/Documentation/cputopology.txt
///
std::size_t parseThreadMap(const std::string& filename)
{
  std::size_t threads = 0;
  std::string threadMap = getString(filename);

  for (char c : threadMap)
  {
    if (c != ',')
    {
      std::string hexChar { c };
      std::size_t bitmap = std::stoul(hexChar, nullptr, 16);
      for (; bitmap > 0; threads++)
        bitmap &= bitmap - 1;
    }
  }

  return threads;
}

/// Some information inside /sys/devices/system/cpu
/// is available twice:
/// 1) As a human readable list file.
/// 2) As a map file (with binary or hexadecimal strings).
/// But you cannot know in advance if any of these
/// files exist, hence you need to try both.
///
std::size_t getThreads(const std::string& threadList,
                       const std::string& threadMap)
{
  std::size_t threads = parseThreadList(threadList);

  if (threads != 0)
    return threads;
  else
    return parseThreadMap(threadMap);
}

} // namespace

namespace primesieve {

void CpuInfo::init()
{
  std::string cpusOnline = "/sys/devices/system/cpu/online";
  logicalCpuCores_ = parseThreadList(cpusOnline);

  using CacheSize_t = std::size_t;
  // Items must be sorted in ascending order
  std::map<CacheSize_t, std::size_t> l1CacheSizes;
  std::vector<std::size_t> cpuIds;
  cpuIds.reserve(3);

  // Based on my tests, for hybrid CPUs the Linux kernel always lists
  // all performance CPU cores first and all efficiency CPU cores
  // last, regardless of the actual physical CPU core layout on the
  // die. I tested this using an Intel Arrow Lake 245K CPU where the
  // physical CPU core layout (2 P-cores, 8 E-cores, 4 P-cores) on the
  // die is different from what the Linux kernel reports.

  // Check 1st, last & middle CPU core
  cpuIds.push_back(0);
  if (logicalCpuCores_ >= 2)
    cpuIds.push_back(logicalCpuCores_ - 1);
  if (logicalCpuCores_ >= 3)
    cpuIds.push_back(logicalCpuCores_ / 2);

  // Because of hybrid CPUs with big & little CPU cores we
  // first check whether there are CPU cores with different
  // L1 data cache sizes in the system. Because these
  // checks are slow, we only check 3 different CPU cores.
  for (std::size_t cpuId : cpuIds)
  {
    for (std::size_t i = 0; i <= 3; i++)
    {
      std::string path = "/sys/devices/system/cpu/cpu" + std::to_string(cpuId) + "/cache/index" + std::to_string(i);
      std::string cacheLevel = path + "/level";
      std::size_t level = getValue(cacheLevel);

      if (level == 1)
      {
        std::string type = path + "/type";
        std::string cacheType = getString(type);

        if (cacheType == "Data" ||
            cacheType == "Unified")
        {
          std::string cacheSizePath = path + "/size";
          std::size_t cacheSize = getCacheSize(cacheSizePath);

          if (cacheSize > 0)
          {
            if (l1CacheSizes.find(cacheSize) == l1CacheSizes.end())
              l1CacheSizes[cacheSize] = cpuId;
            break;
          }
        }
      }
    }
  }

  // Retrieve the cache sizes of the CPU core with the middle
  // L1 data cache size. If there are only 2 different L1
  // cache sizes we retrieve the cache sizes of the CPU core
  // with the smaller L1 data cache size.
  if (!l1CacheSizes.empty())
  {
    auto iter = l1CacheSizes.begin();
    std::advance(iter, (l1CacheSizes.size() - 1) / 2);
    std::size_t cpuId = iter->second;

    for (std::size_t i = 0; i <= 3; i++)
    {
      std::string path = "/sys/devices/system/cpu/cpu" + std::to_string(cpuId) + "/cache/index" + std::to_string(i);
      std::string cacheLevel = path + "/level";
      std::size_t level = getValue(cacheLevel);

      if (level >= 1 &&
          level <= 3)
      {
        std::string type = path + "/type";
        std::string cacheType = getString(type);

        if (cacheType == "Data" ||
            cacheType == "Unified")
        {
          std::string cacheSizePath = path + "/size";
          std::string sharedCpuList = path + "/shared_cpu_list";
          std::string sharedCpuMap = path + "/shared_cpu_map";
          std::size_t cacheSize = getCacheSize(cacheSizePath);
          std::size_t cacheSharing = getThreads(sharedCpuList, sharedCpuMap);

          // If the CPU core has multiple caches of the same level,
          // then we are only interested in the smallest such
          // cache since this is likely the fastest cache.
          if (cacheSizes_[level] > 0 &&
              cacheSizes_[level] <= cacheSize)
            continue;

          cacheSizes_[level] = cacheSize;
          cacheSharing_[level] = cacheSharing;
        }
      }
    }
  }
}

} // namespace

#endif

namespace primesieve {

/// Singleton (initialized at startup)
const CpuInfo cpuInfo;

CpuInfo::CpuInfo() :
  logicalCpuCores_(0),
  cacheSizes_{0, 0, 0, 0},
  cacheSharing_{0, 0, 0, 0}
{
  try
  {
    init();
  }
  catch (std::exception& e)
  {
    // We don't trust the operating system to reliably report
    // all CPU information. In case an unexpected error
    // occurs we continue without relying on CpuInfo and
    // primesieve will fallback to using default CPU settings
    // e.g. 32 KiB L1 data cache size.
    error_ = e.what();
  }
}

std::string CpuInfo::cpuName() const
{
  try
  {
    // On Linux we get the CPU name by parsing /proc/cpuinfo
    // which can be quite slow on PCs without fast SSD.
    // For this reason we don't initialize the CPU name at
    // startup but instead we lazy load it when needed.
    return getCpuName();
  }
  catch (std::exception&)
  {
    return {};
  }
}

std::size_t CpuInfo::logicalCpuCores() const
{
  return logicalCpuCores_;
}

std::size_t CpuInfo::l1CacheBytes() const
{
  return cacheSizes_[1];
}

std::size_t CpuInfo::l2CacheBytes() const
{
  return cacheSizes_[2];
}

std::size_t CpuInfo::l3CacheBytes() const
{
  return cacheSizes_[3];
}

std::size_t CpuInfo::l1Sharing() const
{
  return cacheSharing_[1];
}

std::size_t CpuInfo::l2Sharing() const
{
  return cacheSharing_[2];
}

std::size_t CpuInfo::l3Sharing() const
{
  return cacheSharing_[3];
}

std::string CpuInfo::getError() const
{
  return error_;
}
bool CpuInfo::hasCpuName() const
{
  return !cpuName().empty();
}

bool CpuInfo::hasLogicalCpuCores() const
{
  return logicalCpuCores_ >= 1 &&
         logicalCpuCores_ <= (1 << 20);
}

bool CpuInfo::hasL1Cache() const
{
  return cacheSizes_[1] >= (1 << 12) &&
         cacheSizes_[1] <= (1 << 30);
}

bool CpuInfo::hasL2Cache() const
{
  uint64_t cacheSize = cacheSizes_[2];

  return cacheSize >= (1 << 12) &&
         cacheSize <= (1ull << 40);
}

bool CpuInfo::hasL3Cache() const
{
  uint64_t cacheSize = cacheSizes_[3];

  return cacheSize >= (1 << 12) &&
         cacheSize <= (1ull << 40);
}

bool CpuInfo::hasL1Sharing() const
{
  return cacheSharing_[1] >= 1 &&
         cacheSharing_[1] <= (1 << 20);
}

bool CpuInfo::hasL2Sharing() const
{
  return cacheSharing_[2] >= 1 &&
         cacheSharing_[2] <= (1 << 20);
}

bool CpuInfo::hasL3Sharing() const
{
  return cacheSharing_[3] >= 1 &&
         cacheSharing_[3] <= (1 << 20);
}

} // namespace
