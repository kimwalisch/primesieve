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
  // Posix shell script for UNIX like OSes
  // Returns log2 of L1_DCACHE_SIZE in kilobytes
  // https://github.com/kimwalisch/primesieve/blob/master/configure.ac
  const char* shell_script = 
    "command -v getconf >/dev/null 2>/dev/null;                                                        "
    "if [ $? -eq 0 ];                                                                                  "
    "then                                                                                              "
    "    L1_DCACHE_BYTES=$(getconf LEVEL1_DCACHE_SIZE 2>/dev/null);                                    "
    "fi;                                                                                               "
    "if test \"x$L1_DCACHE_BYTES\" = \"x\" || test \"$L1_DCACHE_BYTES\" = \"0\";                       "
    "then                                                                                              "
    "    L1_DCACHE_BYTES=$(cat /sys/devices/system/cpu/cpu0/cache/index0/size 2>/dev/null);            "
    "    if test \"x$L1_DCACHE_BYTES\" != \"x\";                                                       "
    "    then                                                                                          "
    "        is_kilobytes=$(echo $L1_DCACHE_BYTES | grep K);                                           "
    "        if test \"x$is_kilobytes\" != \"x\";                                                      "
    "        then                                                                                      "
    "            L1_DCACHE_BYTES=$(expr $(echo $L1_DCACHE_BYTES | sed -e s'/K$//') '*' 1024);          "
    "        fi;                                                                                       "
    "        is_megabytes=$(echo $L1_DCACHE_BYTES | grep M);                                           "
    "        if test \"x$is_megabytes\" != \"x\";                                                      "
    "        then                                                                                      "
    "            L1_DCACHE_BYTES=$(expr $(echo $L1_DCACHE_BYTES | sed -e s'/M$//') '*' 1024 '*' 1024); "
    "        fi;                                                                                       "
    "    else                                                                                          "
    "        command -v sysctl >/dev/null 2>/dev/null;                                                 "
    "        if [ $? -eq 0 ];                                                                          "
    "        then                                                                                      "
    "            L1_DCACHE_BYTES=$(sysctl hw.l1dcachesize 2>/dev/null | sed -e 's/^.* //');            "
    "        fi;                                                                                       "
    "    fi;                                                                                           "
    "fi;                                                                                               "
    "if test \"x$L1_DCACHE_BYTES\" != \"x\";                                                           "
    "then                                                                                              "
    "    if [ $L1_DCACHE_BYTES -ge 1024 2>/dev/null ];                                                 "
    "    then                                                                                          "
    "        L1_DCACHE_SIZE=$(expr $L1_DCACHE_BYTES '/' 1024);                                         "
    "    fi;                                                                                           "
    "fi;                                                                                               "
    "if test \"x$L1_DCACHE_SIZE\" = \"x\";                                                             "
    "then                                                                                              "
    "   exit 1;                                                                                        "
    "fi;                                                                                               "
    "LOG2_L1_DCACHE_SIZE=0;                                                                            "
    "while [ $L1_DCACHE_SIZE -ge 2 ];                                                                  "
    "do                                                                                                "
    "   L1_DCACHE_SIZE=$(expr $L1_DCACHE_SIZE '/' 2);                                                  "
    "   LOG2_L1_DCACHE_SIZE=$(expr $LOG2_L1_DCACHE_SIZE '+' 1);                                        "
    "done;                                                                                             "
    "exit $LOG2_L1_DCACHE_SIZE;                                                                        ";

  int exit_code = std::system(shell_script);
  exit_code = WEXITSTATUS(exit_code);

  // check if shell script executed without any errors
  if (exit_code <= 1)
    return -1;

  int l1d_cache_size = 1 << exit_code;

  return l1d_cache_size;
}

#endif
