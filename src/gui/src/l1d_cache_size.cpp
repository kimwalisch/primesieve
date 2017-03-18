///
/// @file   l1d_cache_size.cpp
/// @brief  Get the L1 cache size in kilobytes on Windows
///         and most Unix-like operating systems.
///
/// Copyright (C) 2015 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License.
///

#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

/// Get the CPU's L1 data cache size (per core) in kilobytes.
/// Successfully tested on Windows x64.
/// @return L1 data cache size in kilobytes or -1 if an error occurred.
///
int get_l1d_cache_size()
{
  LPFN_GLPI glpi = (LPFN_GLPI) GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");

  // GetLogicalProcessorInformation not supported
  if (glpi == NULL)
    return -1;

  DWORD buffer_bytes = 0;
  int cache_size = 0;

  glpi(0, &buffer_bytes);
  std::size_t size = buffer_bytes / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
  SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer = new SYSTEM_LOGICAL_PROCESSOR_INFORMATION[size];
  glpi(buffer, &buffer_bytes);

  for (std::size_t i = 0; i < size; i++)
  {
    if (buffer[i].Relationship == RelationCache &&
        buffer[i].Cache.Level == 1)
    {
      cache_size = (int) buffer[i].Cache.Size;
      break;
    }
  }

  delete buffer;

  return cache_size / 1024;
}

#else

/// Get the CPU's L1 data cache size (per core) in kilobytes.
/// Successfully tested on Linux and Mac OS X.
/// @return L1 data cache size in kilobytes or -1 if an error occurred.
///
int get_l1d_cache_size()
{
  // Posix shell script for UNIX like OSes,
  // Returns log2 of L1_DCACHE_SIZE in kilobytes.
  // The script tries to get the L1 cache size using 3 different approaches:
  // 1) getconf LEVEL1_DCACHE_SIZE
  // 2) cat /sys/devices/system/cpu/cpu0/cache/index0/size
  // 3) sysctl hw.l1dcachesize

  const char* shell_script = 
    "command -v getconf >/dev/null 2>/dev/null;                                                        \n"
    "if [ $? -eq 0 ];                                                                                  \n"
    "then                                                                                              \n"
    "    # Returns L1 cache size in bytes                                                              \n"
    "    L1_DCACHE_BYTES=$(getconf LEVEL1_DCACHE_SIZE 2>/dev/null);                                    \n"
    "fi;                                                                                               \n"
    "                                                                                                  \n"
    "if test \"x$L1_DCACHE_BYTES\" = \"x\" || test \"$L1_DCACHE_BYTES\" = \"0\";                       \n"
    "then                                                                                              \n"
    "    # Returns L1 cache size like e.g. 32K, 1M                                                     \n"
    "    L1_DCACHE_BYTES=$(cat /sys/devices/system/cpu/cpu0/cache/index0/size 2>/dev/null);            \n"
    "                                                                                                  \n"
    "    if test \"x$L1_DCACHE_BYTES\" != \"x\";                                                       \n"
    "    then                                                                                          \n"
    "        is_kilobytes=$(echo $L1_DCACHE_BYTES | grep K);                                           \n"
    "        if test \"x$is_kilobytes\" != \"x\";                                                      \n"
    "        then                                                                                      \n"
    "            L1_DCACHE_BYTES=$(expr $(echo $L1_DCACHE_BYTES | sed -e s'/K$//') '*' 1024);          \n"
    "        fi;                                                                                       \n"
    "        is_megabytes=$(echo $L1_DCACHE_BYTES | grep M);                                           \n"
    "        if test \"x$is_megabytes\" != \"x\";                                                      \n"
    "        then                                                                                      \n"
    "            L1_DCACHE_BYTES=$(expr $(echo $L1_DCACHE_BYTES | sed -e s'/M$//') '*' 1024 '*' 1024); \n"
    "        fi;                                                                                       \n"
    "    else                                                                                          \n"
    "        # This method works on OS X                                                               \n"
    "        command -v sysctl >/dev/null 2>/dev/null;                                                 \n"
    "        if [ $? -eq 0 ];                                                                          \n"
    "        then                                                                                      \n"
    "            # Returns L1 cache size in bytes                                                      \n"
    "            L1_DCACHE_BYTES=$(sysctl hw.l1dcachesize 2>/dev/null | sed -e 's/^.* //');            \n"
    "        fi;                                                                                       \n"
    "    fi;                                                                                           \n"
    "fi;                                                                                               \n"
    "                                                                                                  \n"
    "if test \"x$L1_DCACHE_BYTES\" != \"x\";                                                           \n"
    "then                                                                                              \n"
    "    if [ $L1_DCACHE_BYTES -ge 1024 2>/dev/null ];                                                 \n"
    "    then                                                                                          \n"
    "        # Convert to kilobytes                                                                    \n"
    "        L1_DCACHE_SIZE=$(expr $L1_DCACHE_BYTES '/' 1024);                                         \n"
    "    fi;                                                                                           \n"
    "fi;                                                                                               \n"
    "                                                                                                  \n"
    "if test \"x$L1_DCACHE_SIZE\" = \"x\";                                                             \n"
    "then                                                                                              \n"
    "   exit 1;                                                                                        \n"
    "fi;                                                                                               \n"
    "                                                                                                  \n"
    "LOG2_L1_DCACHE_SIZE=0;                                                                            \n"
    "while [ $L1_DCACHE_SIZE -ge 2 ];                                                                  \n"
    "do                                                                                                \n"
    "   L1_DCACHE_SIZE=$(expr $L1_DCACHE_SIZE '/' 2);                                                  \n"
    "   LOG2_L1_DCACHE_SIZE=$(expr $LOG2_L1_DCACHE_SIZE '+' 1);                                        \n"
    "done;                                                                                             \n"
    "                                                                                                  \n"
    "exit $LOG2_L1_DCACHE_SIZE;                                                                        \n";

  int exit_code = std::system(shell_script);
  exit_code = WEXITSTATUS(exit_code);

  // check if shell script executed without any errors
  if (exit_code <= 2)
    return -1;

  int l1d_cache_size = 1 << exit_code;

  return l1d_cache_size;
}

#endif
