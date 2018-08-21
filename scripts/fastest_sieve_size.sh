#!/bin/bash

# Find the fastest sieve size for the user's CPU.
# Usage:
#   ./find_fastest_sieve_size.sh
# Description:
#   This script runs prime counting benchmarks using different
#   sieve sizes and reports the time elapsed in seconds. The
#   fastest timing indicates the best sieve size for the user's
#   CPU. Note that we run single and multi-thread benchmarks
#   for small and large numbers.

# Find the primesieve binary
command -v ./primesieve >/dev/null 2>/dev/null
if [ $? -eq 0 ]
then
    primesieve="./primesieve"
else
    command -v ../primesieve >/dev/null 2>/dev/null
    if [ $? -eq 0 ]
    then
        primesieve="../primesieve"
    else
        command -v build/primesieve >/dev/null 2>/dev/null
        if [ $? -eq 0 ]
        then
            primesieve="build/primesieve"
        else
            command -v primesieve >/dev/null 2>/dev/null
            if [ $? -eq 0 ]
            then
                primesieve="primesieve"
            else
                echo "Error: failed to find primesieve binary."
                exit 1
            fi
        fi
    fi
fi

# Print CPU info
$primesieve --cpu-info
echo ""

cpuCores=$($primesieve --cpu-info | grep 'Number of cores' | cut -f2 -d':' | cut -f2 -d' ')
threads=$($primesieve --cpu-info | grep 'Number of threads' | cut -f2 -d':' | cut -f2 -d' ')
l1CacheSize=$($primesieve --cpu-info | grep 'L1 cache size' | cut -f2 -d':' | cut -f2 -d' ')
l2CacheSize=$($primesieve --cpu-info | grep 'L2 cache size' | cut -f2 -d':' | cut -f2 -d' ')

if [ -z "$cpuCores" ] || \
   [ "$cpuCores" = "unknown" ]
then
    cpuCores=1
fi

if [ -z "$threads" ] || \
   [ "$threads" = "unknown" ]
then
    threads=1
fi

if [ -z "$l1CacheSize" ] || \
   [ "$l1CacheSize" = "unknown" ]
then
    l1CacheSize=8
fi

if [ -z "$l2CacheSize" ] || \
   [ "$l2CacheSize" = "unknown" ]
then
    l2CacheSize=1024
fi

echo "=== Single-thread benchmark: small numbers ==="
echo

for ((size=$l1CacheSize; size<=$l2CacheSize; size*=2))
do  
   $primesieve 1e11 -t1 -s$size
   sleep 1
done

echo
echo "=== Single-thread benchmark: large numbers ==="
echo

for ((size=$l1CacheSize; size<=$l2CacheSize; size*=2))
do  
   $primesieve 1e18 -d5e10 -t1 -s$size
   sleep 1
done

if [ "$threads" -gt 1 ]
then
    dist=$((25*10**10 * $cpuCores))

    echo
    echo "=== Multi-thread benchmark: small numbers ==="
    echo

    for ((size=$l1CacheSize; size<=$l2CacheSize; size*=2))
    do  
        $primesieve $dist -s$size
        sleep 10
    done

    dist=$((5*10**10 * $cpuCores))

    echo
    echo "=== Multi-thread benchmark: large numbers ==="
    echo

    for ((size=$l1CacheSize; size<=$l2CacheSize; size*=2))
    do  
        $primesieve 1e18 -d$dist -s$size
        if [ "$size" -lt "$l2CacheSize" ]
        then
            sleep 10
        fi
    done
fi
