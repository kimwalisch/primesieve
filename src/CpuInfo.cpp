///
/// @file   CpuInfo.cpp
/// @brief  Get detailed information about the CPU's caches
///         on Windows, macOS and Linux.
///
/// Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/CpuInfo.hpp>

#include <stdint.h>
#include <cstddef>
#include <exception>
#include <string>
#include <vector>

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

  using LPFN_GLPIEX = decltype(&GetLogicalProcessorInformationEx);

  LPFN_GLPIEX glpiex = (LPFN_GLPIEX) (void*) GetProcAddress(
      GetModuleHandle(TEXT("kernel32")),
      "GetLogicalProcessorInformationEx");

  if (!glpiex)
    return;

  DWORD bytes = 0;
  glpiex(RelationAll, 0, &bytes);

  if (!bytes)
    return;

  vector<char> buffer(bytes);
  SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* info;

  if (!glpiex(RelationAll, (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*) &buffer[0], &bytes))
    return;

  for (size_t i = 0; i < bytes; i += info->Size)
  {
    info = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*) &buffer[i];

    if (info->Relationship == RelationCache &&
        info->Cache.GroupMask.Group == 0 &&
        info->Cache.Level >= 1 &&
        info->Cache.Level <= 3 &&
        (info->Cache.Type == CacheData ||
         info->Cache.Type == CacheUnified))
    {
      auto level = info->Cache.Level;
      cacheSizes_[level] = info->Cache.CacheSize;

      // @warning: GetLogicalProcessorInformationEx() reports
      // incorrect data when Windows is run inside a virtual
      // machine. Specifically the GROUP_AFFINITY.Mask will
      // only have 1 or 2 bits set for each CPU cache even if
      // more logical CPU cores share the cache
      auto mask = info->Cache.GroupMask.Mask;
      cacheSharing_[level] = 0;

      // Cache.GroupMask.Mask contains one bit set for
      // each logical CPU core sharing the cache
      for (; mask > 0; mask &= mask - 1)
        cacheSharing_[level]++;
    }

    if (info->Relationship == RelationProcessorCore)
    {
      cpuCores_++;
      threadsPerCore_ = 0;
      size_t size = info->Processor.GroupCount;

      for (size_t j = 0; j < size; j++)
      {
        // Processor.GroupMask.Mask contains one bit set for
        // each thread associated to the current CPU core
        auto mask = info->Processor.GroupMask[j].Mask;
        for (; mask > 0; mask &= mask - 1)
          threadsPerCore_++;
      }

      cpuThreads_ += threadsPerCore_;
    }
  }
// Windows XP or later
#elif _WIN32_WINNT >= 0x0501

  using LPFN_GLPI = decltype(&GetLogicalProcessorInformation);

  LPFN_GLPI glpi = (LPFN_GLPI) (void*) GetProcAddress(
      GetModuleHandle(TEXT("kernel32")),
      "GetLogicalProcessorInformation");

  if (!glpi)
    return;

  DWORD bytes = 0;
  glpi(0, &bytes);

  if (!bytes)
    return;

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
      auto mask = info[i].ProcessorMask;
      for (threadsPerCore_ = 0; mask > 0; threadsPerCore_++)
        mask &= mask - 1;

      cpuCores_++;
      cpuThreads_ += threadsPerCore_;
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
        cacheSharing_[level] = threadsPerCore_;

      // We assume the L3 cache is shared
      if (info[i].Cache.Level == 3)
        cacheSharing_[level] = threadsPerCore_ * cpuCores_;
    }
  }

  // Most Intel CPUs have an L3 cache.
  // But a few Intel CPUs don't have an L3 cache
  // and for many of those CPUs the L2 cache is
  // shared e.g. Intel Core 2 Duo/Quad CPUs and
  // Intel Atom x5-Z8350 CPUs.
  if (hasL2Cache() && !hasL3Cache())
    cacheSharing_[2] = threadsPerCore_ * cpuCores_;
#endif
}

} // namespace

#elif defined(APPLE_SYSCTL)

#include <primesieve/pmath.hpp>

#include <algorithm>
#include <sys/types.h>
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
  auto cpuCores = getSysctl<size_t>("hw.physicalcpu");
  if (!cpuCores.empty())
    cpuCores_ = cpuCores[0];

  auto cpuThreads = getSysctl<size_t>("hw.logicalcpu");
  if (!cpuThreads.empty())
    cpuThreads_ = cpuThreads[0];

  if (hasCpuCores() && hasCpuThreads())
    threadsPerCore_ = cpuThreads_ / cpuCores_;

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
  cpuThreads_ = parseThreadList(cpusOnline);

  string threadSiblingsList = "/sys/devices/system/cpu/cpu0/topology/thread_siblings_list";
  string threadSiblings = "/sys/devices/system/cpu/cpu0/topology/thread_siblings";
  threadsPerCore_ = getThreads(threadSiblingsList, threadSiblings);

  if (hasCpuThreads() &&
      hasThreadsPerCore())
    cpuCores_ = cpuThreads_ / threadsPerCore_;

  for (size_t i = 0; i <= 3; i++)
  {
    string path = "/sys/devices/system/cpu/cpu0/cache/index" + to_string(i);
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
  cpuCores_(0),
  cpuThreads_(0),
  threadsPerCore_(0),
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

size_t CpuInfo::cpuCores() const
{
  return cpuCores_;
}

size_t CpuInfo::cpuThreads() const
{
  return cpuThreads_;
}

size_t CpuInfo::l1CacheSize() const
{
  return cacheSizes_[1];
}

size_t CpuInfo::l2CacheSize() const
{
  return cacheSizes_[2];
}

size_t CpuInfo::l3CacheSize() const
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

size_t CpuInfo::threadsPerCore() const
{
  return threadsPerCore_;
}

string CpuInfo::getError() const
{
  return error_;
}

bool CpuInfo::hasCpuName() const
{
  return !cpuName().empty();
}

bool CpuInfo::hasCpuCores() const
{
  return cpuCores_ >= 1 &&
         cpuCores_ <= (1 << 20);
}

bool CpuInfo::hasCpuThreads() const
{
  return cpuThreads_ >= 1 &&
         cpuThreads_ <= (1 << 20);
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

bool CpuInfo::hasThreadsPerCore() const
{
  return threadsPerCore_ >= 1 &&
         threadsPerCore_ <= (1 << 10);
}

bool CpuInfo::hasPrivateL2Cache() const
{
  return hasL2Cache() &&
         hasL2Sharing() &&
         hasThreadsPerCore() &&
         l2Sharing() <= threadsPerCore_;
}

} // namespace
