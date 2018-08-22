#!/bin/bash

# Find the fastest sieve size for the user's CPU.
# Usage:
#   ./find_fastest_sieve_size.sh
# Description:
#   This script runs prime counting benchmarks using different
#   sieve sizes and reports the time elapsed in seconds. The
#   fastest timing indicates the best sieve size for the user's
#   CPU. Note that we run single and multi-threaded benchmarks
#   for small, medium and large primes.

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

start=(0 1e15 1e18)
label=("Small primes" "Medium primes" "Large primes")
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

printTitle() {
    echo
    echo "=== $1: $2-threaded benchmark ==="
    echo
}

for i in {0..2}
do
    printTitle "${label[$i]}" "single"

    for ((size=$l1CacheSize; size<=$l2CacheSize; size*=2))
    do
       $primesieve ${start[$i]} -d1e11 -t1 -s$size
       sleep 1
    done

    if [ "$threads" -gt 1 ]
    then
        printTitle "${label[$i]}" "multi"
        dist=$((10**11 * $cpuCores))

        for ((size=$l1CacheSize; size<=$l2CacheSize; size*=2))
        do
            $primesieve ${start[$i]} -d$dist -s$size
            sleep 1
        done
    fi
done
