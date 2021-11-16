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
/// Copyright (C) 2021 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/CpuInfo.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <cstddef>
#include <exception>
#include <map>
#include <string>
#include <vector>

#if defined(__APPLE__)
  #if !defined(__has_include)
    #define APPLE_SYSCTL
  #elif __has_include(<sys/sysctl.h>)
    #define APPLE_SYSCTL
  #endif
#endif

#if defined(_WIN32)

#include <windows.h>

#if defined(__i386__) || \
    defined(_M_IX86) || \
    defined(__x86_64__) || \
    defined(_M_X64) || \
    defined(_M_AMD64)
  #define IS_X86
#endif

#if defined(IS_X86)

#include <algorithm>
#include <iterator>

// Check if compiler supports <intrin.h>
#if defined(__has_include)
  #if __has_include(<intrin.h>)
    #define HAS_INTRIN_H
  #endif
#elif defined(_MSC_VER)
  #define HAS_INTRIN_H
#endif

// Check if compiler supports CPUID
#if defined(HAS_INTRIN_H)
  #include <intrin.h>
  #define MSVC_CPUID
#elif defined(__GNUC__) || \
      defined(__clang__)
  #define GNUC_CPUID
#endif

using namespace std;

namespace {

/// CPUID is not portable across all x86 CPU vendors and there
/// are many pitfalls. For this reason we prefer to get CPU
/// information from the operating system instead of CPUID.
/// We only use CPUID for getting the CPU name on Windows x86
/// because there is no other way to get that information.
///
void cpuId(int cpuInfo[4], int eax)
{
#if defined(MSVC_CPUID)
  __cpuid(cpuInfo, eax);
#elif defined(GNUC_CPUID)
  int ebx = 0;
  int ecx = 0;
  int edx = 0;

  #if defined(__i386__) && \
      defined(__PIC__)
    // in case of PIC under 32-bit EBX cannot be clobbered
    __asm__ ("movl %%ebx, %%edi;"
             "cpuid;"
             "xchgl %%ebx, %%edi;"
             : "+a" (eax),
               "=D" (ebx),
               "=c" (ecx),
               "=d" (edx));
  #else
    __asm__ ("cpuid;"
             : "+a" (eax),
               "=b" (ebx),
               "=c" (ecx),
               "=d" (edx));
  #endif

  cpuInfo[0] = eax;
  cpuInfo[1] = ebx;
  cpuInfo[2] = ecx;
  cpuInfo[3] = edx;
#else
  // CPUID is not supported
  eax = 0;

  cpuInfo[0] = eax;
  cpuInfo[1] = 0;
  cpuInfo[2] = 0;
  cpuInfo[3] = 0;
#endif
}

/// Remove all leading and trailing
/// space characters.
///
void trimString(string& str)
{
  string spaceChars = " \f\n\r\t\v";
  size_t pos = str.find_first_not_of(spaceChars);
  str.erase(0, pos);

  pos = str.find_last_not_of(spaceChars);
  if (pos != string::npos)
    str.erase(pos + 1);
}

} // namespace

#endif

using namespace std;

namespace {

string getCpuName()
{
  string cpuName;

#if defined(IS_X86)
  // Get the CPU name using CPUID.
  // Example: Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz
  // https://en.wikipedia.org/wiki/CPUID

  int cpuInfo[4] = { 0, 0, 0, 0 };
  cpuId(cpuInfo, 0x80000000);
  vector<int> vect;

  // check if CPU name is supported
  if ((unsigned) cpuInfo[0] >= 0x80000004u)
  {
    cpuId(cpuInfo, 0x80000002);
    copy_n(cpuInfo, 4, back_inserter(vect));

    cpuId(cpuInfo, 0x80000003);
    copy_n(cpuInfo, 4, back_inserter(vect));

    cpuId(cpuInfo, 0x80000004);
    copy_n(cpuInfo, 4, back_inserter(vect));

    vect.push_back(0);
    cpuName = (char*) vect.data();
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

  vector<char> buffer(bytes);

  if (!glpiex(RelationCache, (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*) &buffer[0], &bytes))
    return;

  struct CpuCoreCacheInfo
  {
    CpuCoreCacheInfo::CpuCoreCacheInfo() :
      cacheSizes{0, 0, 0, 0},
      cacheSharing{0, 0, 0, 0}
    { }
    std::array<size_t, 4> cacheSizes;
    std::array<size_t, 4> cacheSharing;
  };

  using CpuCoreId_t = long;
  std::map<CpuCoreId_t, CpuCoreCacheInfo> cacheInfo;
  SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* info;

  // Fill the cacheInfo map with the L1, L2 & L3 cache
  // sizes and cache sharing of each CPU core.
  for (size_t i = 0; i < bytes; i += info->Size)
  {
    info = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*) &buffer[i];

    if (info->Relationship == RelationCache &&
        info->Cache.Level >= 1 &&
        info->Cache.Level <= 3 &&
        (info->Cache.Type == CacheData ||
         info->Cache.Type == CacheUnified))
    {
      size_t cpuCoreIndex = 0;
      size_t cacheSharing = 0;

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

        // Note that calculating the cpuCoreId this way is not 100%
        // correct as there may be multiple processor groups that
        // are not fully filled. However our formula yields unique
        // cpuCoreId's which is good enough for our usage.
        long cpuCoreId = (long) (processorGroup * maxCpusPerProcessorGroup + cpuCoreIndex);
        auto& cpuCoreCacheInfo = cacheInfo[cpuCoreId];
        cpuCoreCacheInfo.cacheSizes[level] = cacheSize;
        cpuCoreCacheInfo.cacheSharing[level] = cacheSharing;
      }
    }
  }

  struct L1CacheStatistics
  {
    long cpuCoreId = -1;
    size_t cpuCoreCount = 0;
  };

  // Items must be sorted in ascending order
  using CacheSize_t = size_t;
  std::map<CacheSize_t, L1CacheStatistics> l1CacheStatistics;
  size_t totalCpuCores = cacheInfo.size();

  // Fill map with different types of L1 caches
  for (const auto& item : cacheInfo)
  {
    auto cpuCoreId = item.first;
    auto l1CacheSize = item.second.cacheSizes[1];
    auto& mapEntry = l1CacheStatistics[l1CacheSize];
    mapEntry.cpuCoreCount++;
    if (mapEntry.cpuCoreId == -1)
      mapEntry.cpuCoreId = cpuCoreId;
  }

  // Check if one of the L1 cache types is used
  // by more than 80% of all CPU cores.
  for (const auto& item : l1CacheStatistics)
  {
    if (item.second.cpuCoreCount * (1 / 0.8) > totalCpuCores)
    {
      size_t cpuCoreId = item.second.cpuCoreId;
      cacheSizes_ = cacheInfo[cpuCoreId].cacheSizes;
      cacheSharing_ = cacheInfo[cpuCoreId].cacheSharing;
      return;
    }
  }

  // For hybrid CPUs with many different L1 cache types
  // we pick one from the middle that is hopefully
  // representative for the CPU's overall performance.
  size_t i = 0;
  for (const auto& item : l1CacheStatistics)
  {
    size_t cpuCoreId = item.second.cpuCoreId;
    cacheSizes_ = cacheInfo[cpuCoreId].cacheSizes;
    cacheSharing_ = cacheInfo[cpuCoreId].cacheSharing;
    if ((++i * 2) >= l1CacheStatistics.size())
      return;
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

  size_t threadsPerCore = 0;
  size_t size = bytes / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
  vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> info(size);

  if (!glpi(&info[0], &bytes))
    return;

  for (size_t i = 0; i < size; i++)
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

  for (size_t i = 0; i < size; i++)
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

#include <algorithm>
#include <cstddef>
#include <sys/sysctl.h>

using namespace std;

namespace {

/// Get CPU information from the operating
/// system kernel using sysctl.
/// https://www.freebsd.org/cgi/man.cgi?sysctl(3)
///
template <typename T>
vector<T> getSysctl(const string& name)
{
  vector<T> res;
  size_t bytes = 0;

  if (!sysctlbyname(name.data(), 0, &bytes, 0, 0))
  {
    size_t size = ceilDiv(bytes, sizeof(T));
    vector<T> buffer(size, 0);
    if (!sysctlbyname(name.data(), buffer.data(), &bytes, 0, 0))
      res = buffer;
  }

  return res;
}

string getCpuName()
{
  string cpuName;

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
  for (size_t i = 1; i < min(cacheSizes.size(), cacheSizes_.size()); i++)
    cacheSizes_[i] = cacheSizes[i];

  // https://developer.apple.com/library/content/releasenotes/Performance/RN-AffinityAPI/index.html
  auto cacheConfig = getSysctl<size_t>("hw.cacheconfig");
  for (size_t i = 1; i < min(cacheConfig.size(), cacheSharing_.size()); i++)
    cacheSharing_[i] = cacheConfig[i];
}

} // namespace

#else // Linux (and unknown OSes)

#include <primesieve/primesieve_error.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <sstream>

using namespace std;
using namespace primesieve;

namespace {

/// Remove all leading and trailing
/// space characters.
///
void trimString(string& str)
{
  string spaceChars = " \f\n\r\t\v";
  size_t pos = str.find_first_not_of(spaceChars);
  str.erase(0, pos);

  pos = str.find_last_not_of(spaceChars);
  if (pos != string::npos)
    str.erase(pos + 1);
}

string getString(const string& filename)
{
  ifstream file(filename);
  string str;

  // Read the first string,
  // stops at any space character
  if (file && (file >> str))
    return str;
  else
    return {};
}

size_t getValue(const string& filename)
{
  string str = getString(filename);
  size_t val = 0;

  if (!str.empty())
    val = stoul(str);

  return val;
}

size_t getCacheSize(const string& filename)
{
  string str = getString(filename);
  size_t val = 0;

  if (!str.empty())
  {
    val = stoul(str);
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
string getCpuName(const string& line)
{
  // Examples of CPU names:
  // model name : Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz
  // Processor  : ARMv7 Processor rev 5 (v7l)
  // cpu        : POWER9 (raw), altivec supported
  static set<string> cpuLabels
  {
    "model name",
    "Processor",
    "cpu"
  };

  size_t pos = line.find(':');
  string cpuName;

  if (pos != string::npos)
  {
    string label = line.substr(0, pos);
    trimString(label);
    if (cpuLabels.find(label) != cpuLabels.end())
      cpuName = line.substr(pos + 1);
  }

  return cpuName;
}

bool isValid(const string& cpuName)
{
  if (cpuName.empty())
    return false;
  if (cpuName.find_first_not_of("0123456789") == string::npos)
    return false;

  return true;
}

string getCpuName()
{
  ifstream file("/proc/cpuinfo");
  string notFound;

  if (file)
  {
    string line;
    size_t i = 0;

    while (getline(file, line))
    {
      string cpuName = getCpuName(line);
      trimString(cpuName);

      if (isValid(cpuName))
        return cpuName;
      if (++i > 10)
        return notFound;
    }
  }

  return notFound;
}

vector<string> split(const string& str,
                     char delimiter)
{
  vector<string> tokens;
  string token;
  istringstream tokenStream(str);

  while (getline(tokenStream, token, delimiter))
    tokens.push_back(token);

  return tokens;
}

/// A thread list file contains a human
/// readable list of thread IDs.
/// Example: 0-8,18-26
/// https://www.kernel.org/doc/Documentation/cputopology.txt
///
size_t parseThreadList(const string& filename)
{
  size_t threads = 0;
  auto threadList = getString(filename);
  auto tokens = split(threadList, ',');

  for (auto& str : tokens)
  {
    auto values = split(str, '-');
    if (values.size() == 1)
      threads++;
    else
    {
      auto t0 = stoul(values.at(0));
      auto t1 = stoul(values.at(1));
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
size_t parseThreadMap(const string& filename)
{
  size_t threads = 0;
  string threadMap = getString(filename);

  for (char c : threadMap)
  {
    if (c != ',')
    {
      string hexChar { c };
      size_t bitmap = stoul(hexChar, nullptr, 16);
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
size_t getThreads(const string& threadList,
                  const string& threadMap)
{
  size_t threads = parseThreadList(threadList);

  if (threads != 0)
    return threads;
  else
    return parseThreadMap(threadMap);
}

} // namespace

namespace primesieve {

void CpuInfo::init()
{
  string cpusOnline = "/sys/devices/system/cpu/online";
  logicalCpuCores_ = parseThreadList(cpusOnline);

  // For hybrid CPUs with multiple types of CPU cores Linux seems
  // to order the CPU cores within /sys/devices/system/cpu*
  // from fastest to slowest. By picking a CPU core from the middle
  // we hopefully get an average CPU core that is representative
  // for the CPU's overall (multi-threading) performance.
  string cpuNumber = to_string(logicalCpuCores_ / 2);

  // Retrieve CPU cache info
  for (size_t i = 0; i <= 3; i++)
  {
    string path = "/sys/devices/system/cpu" + cpuNumber + "/cpu0/cache/index" + to_string(i);
    string cacheLevel = path + "/level";
    size_t level = getValue(cacheLevel);

    if (level >= 1 &&
        level <= 3)
    {
      string type = path + "/type";
      string cacheType = getString(type);

      if (cacheType == "Data" ||
          cacheType == "Unified")
      {
        string cacheSize = path + "/size";
        string sharedCpuList = path + "/shared_cpu_list";
        string sharedCpuMap = path + "/shared_cpu_map";
        cacheSizes_[level] = getCacheSize(cacheSize);
        cacheSharing_[level] = getThreads(sharedCpuList, sharedCpuMap);
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
  catch (exception& e)
  {
    // We don't trust the operating system to reliably report
    // all CPU information. In case an unexpected error
    // occurs we continue without relying on CpuInfo and
    // primesieve will fallback to using default CPU settings
    // e.g. 32 KiB L1 data cache size.
    error_ = e.what();
  }
}

string CpuInfo::cpuName() const
{
  try
  {
    // On Linux we get the CPU name by parsing /proc/cpuinfo
    // which can be quite slow on PCs without fast SSD.
    // For this reason we don't initialize the CPU name at
    // startup but instead we lazy load it when needed.
    return getCpuName();
  }
  catch (exception&)
  {
    return {};
  }
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

string CpuInfo::getError() const
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
