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
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/CpuInfo.hpp>

#include <algorithm>
#include <stdint.h>
#include <cstddef>
#include <exception>
#include <string>
#include <vector>

#if defined(__APPLE__)
  #if !defined(__has_include)
    #define APPLE_SYSCTL
  #elif __has_include(<sys/sysctl.h>)
    #define APPLE_SYSCTL
  #endif
#endif

#if defined(__i386__) || \
    defined(__x86_64__) || \
    defined(_M_IX86) || \
    defined(_M_X64)

#if defined(_MSC_VER)
  #include <intrin.h>
  #include <immintrin.h>
#endif

#define HAS_CPUID

/* %ebx bit flags */
#define bit_AVX512F (1 << 16)

/* %ecx bit flags */
#define bit_AVX512VBMI  (1 << 1)
#define bit_AVX512VBMI2 (1 << 6)
#define bit_POPCNT      (1 << 23)

/* xgetbv bit flags */
#define XSTATE_SSE (1 << 1)
#define XSTATE_YMM (1 << 2)
#define XSTATE_ZMM (7 << 5)

namespace {

void run_cpuid(int eax, int ecx, int* abcd)
{
#if defined(_MSC_VER)
  __cpuidex(abcd, eax, ecx);
#else
  int ebx = 0;
  int edx = 0;

  #if defined(__i386__) && \
      defined(__PIC__)
    /* in case of PIC under 32-bit EBX cannot be clobbered */
    __asm__ ("movl %%ebx, %%edi;"
             "cpuid;"
             "xchgl %%ebx, %%edi;"
             : "=D" (ebx),
               "+a" (eax),
               "+c" (ecx),
               "=d" (edx));
  #else
    __asm__ ("cpuid;"
             : "+b" (ebx),
               "+a" (eax),
               "+c" (ecx),
               "=d" (edx));
  #endif

  abcd[0] = eax;
  abcd[1] = ebx;
  abcd[2] = ecx;
  abcd[3] = edx;
#endif
}

// Get Value of Extended Control Register
int get_xcr0()
{
  int xcr0;

#if defined(_MSC_VER)
  xcr0 = (int) _xgetbv(0);
#else
  __asm__ ("xgetbv" : "=a" (xcr0) : "c" (0) : "%edx" );
#endif

  return xcr0;
}

bool has_AVX512()
{
  int abcd[4];

  run_cpuid(1, 0, abcd);

  // PrimeGenerator::fillNextPrimes() requires POPCNT
  if ((abcd[2] & bit_POPCNT) != bit_POPCNT)
    return false;

  int osxsave_mask = (1 << 27);

  // Ensure OS supports extended processor state management
  if ((abcd[2] & osxsave_mask) != osxsave_mask)
    return false;

  int ymm_mask = XSTATE_SSE | XSTATE_YMM;
  int zmm_mask = XSTATE_SSE | XSTATE_YMM | XSTATE_ZMM;

  int xcr0 = get_xcr0();

  // Check AVX OS support
  if ((xcr0 & ymm_mask) != ymm_mask)
    return false;

  // Check AVX512 OS support
  if ((xcr0 & zmm_mask) != zmm_mask)
    return false;

  run_cpuid(7, 0, abcd);

  // PrimeGenerator::fillNextPrimes() requires AVX512F, AVX512VBMI & AVX512VBMI2
  return ((abcd[1] & bit_AVX512F) == bit_AVX512F &&
          (abcd[2] & (bit_AVX512VBMI | bit_AVX512VBMI2)) == (bit_AVX512VBMI | bit_AVX512VBMI2));
}

} // namespace

#endif

#if defined(_WIN32)

#include <primesieve/pmath.hpp>

#include <windows.h>
#include <iterator>
#include <map>

namespace {

std::string getCpuName()
{
  std::string cpuName;

#if defined(HAS_CPUID)
  // Get the CPU name using CPUID.
  // Example: Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz
  // https://en.wikipedia.org/wiki/CPUID

  int cpuInfo[4] = { 0, 0, 0, 0 };
  run_cpuid(0x80000000, 0, cpuInfo);
  std::vector<int> vect;

  // check if CPU name is supported
  if ((unsigned) cpuInfo[0] >= 0x80000004u)
  {
    run_cpuid(0x80000002, 0, cpuInfo);
    std::copy_n(cpuInfo, 4, std::back_inserter(vect));

    run_cpuid(0x80000003, 0, cpuInfo);
    std::copy_n(cpuInfo, 4, std::back_inserter(vect));

    run_cpuid(0x80000004, 0, cpuInfo);
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
    pod_array<size_t, 4> cacheSizes;
    pod_array<size_t, 4> cacheSharing;
  };

  struct L1CacheStatistics
  {
    long cpuCoreId = -1;
    std::size_t cpuCoreCount = 0;
  };

  using CacheSize_t = std::size_t;
  // Items must be sorted in ascending order
  std::map<CacheSize_t, L1CacheStatistics> l1CacheStatistics;
  std::vector<CpuCoreCacheInfo> cacheInfo;
  std::size_t totalL1CpuCores = 0;
  SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* info;

  // Fill the cacheInfo vector with the L1, L2 & L3 cache
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

        if (cacheInfo.size() <= cpuCoreId)
          cacheInfo.resize((cpuCoreId + 1) * 2);
        cacheInfo[cpuCoreId].cacheSizes[level] = cacheSize;
        cacheInfo[cpuCoreId].cacheSharing[level] = cacheSharing;

        // Count the number of occurences of each type of L1 cache.
        // If one of these L1 cache types is used predominantly
        // we will use that cache as our default cache size.
        if (level == 1)
        {
          auto& mapEntry = l1CacheStatistics[cacheSize];
          totalL1CpuCores++;
          mapEntry.cpuCoreCount++;
          if (mapEntry.cpuCoreId == -1)
            mapEntry.cpuCoreId = (long) cpuCoreId;
        }
      }
    }
  }

  // Check if one of the L1 cache types is used
  // by more than 80% of all CPU cores.
  for (const auto& item : l1CacheStatistics)
  {
    if (item.second.cpuCoreCount > totalL1CpuCores * 0.80)
    {
      long cpuCoreId = item.second.cpuCoreId;
      cacheSizes_ = cacheInfo[cpuCoreId].cacheSizes;
      cacheSharing_ = cacheInfo[cpuCoreId].cacheSharing;
      return;
    }
  }

  // For hybrid CPUs with many different L1 cache types
  // we pick one from the middle that is hopefully
  // representative for the CPU's overall performance.
  if (!l1CacheStatistics.empty())
  {
    auto iter = l1CacheStatistics.begin();
    std::advance(iter, (l1CacheStatistics.size() - 1) / 2);
    long cpuCoreId = iter->second.cpuCoreId;
    cacheSizes_ = cacheInfo[cpuCoreId].cacheSizes;
    cacheSharing_ = cacheInfo[cpuCoreId].cacheSharing;
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
      cacheSizes_[level] = info[i].Cache.Size;

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
  auto logicalCpuCores = getSysctl<size_t>("hw.logicalcpu");
  if (!logicalCpuCores.empty())
    logicalCpuCores_ = logicalCpuCores[0];

  // https://developer.apple.com/library/content/releasenotes/Performance/RN-AffinityAPI/index.html
  auto cacheSizes = getSysctl<size_t>("hw.cachesize");
  for (std::size_t i = 1; i < std::min(cacheSizes.size(), cacheSizes_.size()); i++)
    cacheSizes_[i] = cacheSizes[i];

  // https://developer.apple.com/library/content/releasenotes/Performance/RN-AffinityAPI/index.html
  auto cacheConfig = getSysctl<size_t>("hw.cacheconfig");
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

size_t getValue(const std::string& filename)
{
  std::string str = getString(filename);
  std::size_t val = 0;

  if (!str.empty())
    val = std::stoul(str);

  return val;
}

size_t getCacheSize(const std::string& filename)
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
size_t parseThreadList(const std::string& filename)
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
size_t parseThreadMap(const std::string& filename)
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
size_t getThreads(const std::string& threadList,
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
  bool identicalL1CacheSizes = false;

  using CacheSize_t = std::size_t;
  // Items must be sorted in ascending order
  std::map<CacheSize_t, std::size_t> l1CacheStatistics;
  std::vector<size_t> cpuIds;
  cpuIds.reserve(3);

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
          std::size_t cacheSize = getCacheSize(path + "/size");
          if (cacheSize > 0)
          {
            if (l1CacheStatistics.find(cacheSize) == l1CacheStatistics.end())
              l1CacheStatistics[cacheSize] = cpuId;
            else
              identicalL1CacheSizes = true;
          }
          break;
        }
      }
    }

    // We have found 2 identical CPU cores.
    // In this case we assume all CPU cores
    // have the same cache hierarchy.
    if (identicalL1CacheSizes)
      break;
  }

  // Retrieve the cache sizes of the CPU core with the middle
  // L1 data cache size. If there are only 2 different L1
  // cache sizes we retrieve the cache sizes of the CPU core
  // with the smaller L1 data cache size.
  if (!l1CacheStatistics.empty())
  {
    auto iter = l1CacheStatistics.begin();
    std::advance(iter, (l1CacheStatistics.size() - 1) / 2);
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
          std::string cacheSize = path + "/size";
          std::string sharedCpuList = path + "/shared_cpu_list";
          std::string sharedCpuMap = path + "/shared_cpu_map";
          cacheSizes_[level] = getCacheSize(cacheSize);
          cacheSharing_[level] = getThreads(sharedCpuList, sharedCpuMap);
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

/// This method is only used by the primesieve command-line app
/// with the --cpu-info option. Therefore we currently don't
/// cache the result of has_AVX512().
///
bool CpuInfo::hasAVX512() const
{
  #if defined(HAS_CPUID)
    return has_AVX512();
  #else
    return false;
  #endif
}

size_t CpuInfo::logicalCpuCores() const
{
  return logicalCpuCores_;
}

size_t CpuInfo::l1CacheBytes() const
{
  return cacheSizes_[1];
}

size_t CpuInfo::l2CacheBytes() const
{
  return cacheSizes_[2];
}

size_t CpuInfo::l3CacheBytes() const
{
  return cacheSizes_[3];
}

size_t CpuInfo::l1Sharing() const
{
  return cacheSharing_[1];
}

size_t CpuInfo::l2Sharing() const
{
  return cacheSharing_[2];
}

size_t CpuInfo::l3Sharing() const
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
