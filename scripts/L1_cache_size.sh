#!/bin/sh
# Script that prints the CPU's cache line size in bytes
# Usage: ./L1_cache_size.sh

command -v getconf >/dev/null 2>/dev/null
if [ $? -eq 0 ]
then
    # Returns L1 cache size in bytes
    L1_DCACHE_BYTES=$(getconf LEVEL1_DCACHE_SIZE 2>/dev/null)
fi

if test "x$L1_DCACHE_BYTES" = "x" || \
   test "$L1_DCACHE_BYTES" = "0"
then
    # Returns L1 cache size like e.g. 32K, 1M
    L1_DCACHE_BYTES=$(cat /sys/devices/system/cpu/cpu0/cache/index0/size 2>/dev/null)

    if test "x$L1_DCACHE_BYTES" != "x"
    then
        is_kilobytes=$(echo $L1_DCACHE_BYTES | grep K)
        if test "x$is_kilobytes" != "x"
        then
            L1_DCACHE_BYTES=$(expr $(echo $L1_DCACHE_BYTES | sed -e s'/K$//') '*' 1024)
        fi
        is_megabytes=$(echo $L1_DCACHE_BYTES | grep M)
        if test "x$is_megabytes" != "x"
        then
            L1_DCACHE_BYTES=$(expr $(echo $L1_DCACHE_BYTES | sed -e s'/M$//') '*' 1024 '*' 1024)
        fi
    else
        command -v sysctl >/dev/null 2>/dev/null
        if [ $? -eq 0 ]
        then
            # Returns L1 cache size in bytes
            L1_DCACHE_BYTES=$(sysctl hw.l1dcachesize 2>/dev/null | sed -e 's/^.* //')
        fi
    fi
fi

if test "x$L1_DCACHE_BYTES" != "x"
then
    if [ $L1_DCACHE_BYTES -ge 2048 2>/dev/null ] && \
       [ $L1_DCACHE_BYTES -le 2097152 2>/dev/null ]
    then
        # Convert L1 cache size to kilobytes
        L1_DCACHE_SIZE=$(expr $L1_DCACHE_BYTES '/' 1024)
        echo $L1_DCACHE_SIZE
        exit 0
    fi
fi

# Failed to detect L1 cache size
exit 1
