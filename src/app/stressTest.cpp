///
/// @file   stressTest.cpp
/// @brief  Run a stress test (--stress-test[=MODE] command-line
///         option) that puts maximum load on the CPU (default) or RAM
///         and verify that there are no miscalculations due to
///         hardware issues.
///
///         Stress testing keeps on running until a miscalculation
///         occurs (which shouldn't happen on most PCs) or until the
///         user cancels it using Ctrl+C.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/iterator.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/Vector.hpp>
#include "CmdOptions.hpp"

#include <stdint.h>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <future>
#include <mutex>

using primesieve::Array;

namespace {

/// Lookup table of correct prime count results.
/// primeCounts_CpuMode[i] = PrimePi(i*1e11) - PrimePi((i-1)*1e11)
/// This test sieves up to 10^13 where most memory fits into
/// the CPU's cache. Each thread uses < 5 MiB of memory.
/// This tests puts the highest load on the CPU.
///
/// The table was generated using this bash program:
///
/// for i in {0..99};
/// do
///     res=$(primesieve $i*1e11 -d1e11 -q);
///     printf "$((res))ull, ";
///     if [ $((($i+1) % 5)) -eq 0 ]; then printf "\n"; fi;
/// done
///
const Array<uint64_t, 101> primeCounts_CpuMode =
{
  /* Start number = */ 0,
  4118054813ull, 3889050246ull, 3811334076ull, 3762566522ull, 3727130485ull,
  3699365644ull, 3676572524ull, 3657309217ull, 3640604059ull, 3625924432ull,
  3612791400ull, 3600947714ull, 3590161711ull, 3580266494ull, 3571130592ull,
  3562622357ull, 3554715520ull, 3547310538ull, 3540307017ull, 3533730778ull,
  3527508038ull, 3521536373ull, 3515949965ull, 3510579737ull, 3505445520ull,
  3500531339ull, 3495833071ull, 3491305095ull, 3486960151ull, 3482753275ull,
  3478749572ull, 3474839811ull, 3471053925ull, 3467407632ull, 3463895032ull,
  3460433421ull, 3457117553ull, 3453892434ull, 3450773581ull, 3447746462ull,
  3444702138ull, 3441863700ull, 3439036659ull, 3436325635ull, 3433611069ull,
  3430944750ull, 3428459293ull, 3425915324ull, 3423506752ull, 3421088203ull,
  3418770256ull, 3416469253ull, 3414218299ull, 3412006845ull, 3409864335ull,
  3407752910ull, 3405685414ull, 3403637619ull, 3401667635ull, 3399692824ull,
  3397740437ull, 3395890778ull, 3394044263ull, 3392171356ull, 3390424659ull,
  3388640693ull, 3386884666ull, 3385183718ull, 3383444039ull, 3381837156ull,
  3380227778ull, 3378598496ull, 3376990296ull, 3375420221ull, 3373915620ull,
  3372400737ull, 3370910165ull, 3369407408ull, 3367985168ull, 3366526118ull,
  3365100850ull, 3363709833ull, 3362327791ull, 3360990563ull, 3359614618ull,
  3358291592ull, 3357002793ull, 3355683015ull, 3354424950ull, 3353137292ull,
  3351906327ull, 3350687979ull, 3349462327ull, 3348236947ull, 3347061905ull,
  3345852373ull, 3344702803ull, 3343552482ull, 3342407298ull, 3341312290ull
};

/// Lookup table of correct prime count results.
/// primeCounts_RamMode[i] = PrimePi(1e18+i*1e11) - PrimePi(1e18+(i-1)*1e11)
/// This test sieves near 10^18 where each thread uses about 400 MiB.
/// This test puts higher load on the RAM, but less load on the CPU.
///
/// The table was generated using this bash program:
///
/// for i in {0..99};
/// do
///     res=$(primesieve 1e18+$i*1e11 -d1e11 -q);
///     printf "$((res))ull, ";
///     if [ $((($i+1) % 5)) -eq 0 ]; then printf "\n"; fi;
/// done
///
const Array<uint64_t, 101> primeCounts_RamMode =
{
  /* Start number = */ 1000000000000000000ull,
  2412731214ull, 2412797363ull, 2412781034ull, 2412775259ull, 2412726439ull,
  2412765373ull, 2412791513ull, 2412809711ull, 2412753236ull, 2412706641ull,
  2412659448ull, 2412749044ull, 2412714308ull, 2412754638ull, 2412738125ull,
  2412699893ull, 2412718669ull, 2412760534ull, 2412773145ull, 2412746755ull,
  2412748299ull, 2412707071ull, 2412739033ull, 2412750060ull, 2412714714ull,
  2412718992ull, 2412714673ull, 2412820507ull, 2412696545ull, 2412714170ull,
  2412726975ull, 2412709569ull, 2412802038ull, 2412769163ull, 2412766838ull,
  2412716683ull, 2412784827ull, 2412719465ull, 2412741445ull, 2412750232ull,
  2412755807ull, 2412737195ull, 2412742277ull, 2412719679ull, 2412738603ull,
  2412732373ull, 2412713957ull, 2412734446ull, 2412751453ull, 2412734458ull,
  2412752689ull, 2412753272ull, 2412755364ull, 2412746762ull, 2412746615ull,
  2412738550ull, 2412743701ull, 2412755851ull, 2412768210ull, 2412774793ull,
  2412764687ull, 2412755091ull, 2412706068ull, 2412753581ull, 2412788056ull,
  2412733574ull, 2412747096ull, 2412728625ull, 2412693566ull, 2412768899ull,
  2412752135ull, 2412731543ull, 2412754613ull, 2412791419ull, 2412747754ull,
  2412771098ull, 2412730098ull, 2412712274ull, 2412753983ull, 2412753799ull,
  2412718390ull, 2412768080ull, 2412768019ull, 2412737595ull, 2412800284ull,
  2412715726ull, 2412775347ull, 2412705861ull, 2412754859ull, 2412767108ull,
  2412806188ull, 2412724931ull, 2412761773ull, 2412730012ull, 2412700512ull,
  2412686405ull, 2412760693ull, 2412749045ull, 2412744369ull, 2412786014ull
};

} // namespace

void stressTest(const CmdOptions& opts)
{
  int threads;

  if (opts.threads > 0)
    threads = opts.threads;
  else
    threads = std::thread::hardware_concurrency();

  threads = std::max(1, threads);
  int threadIdPadding = (int) std::to_string(threads).size();
  std::mutex mutex;

  // Each thread executes 1 task
  auto task = [&](const Array<uint64_t, 101>& primeCounts,
                  int threadId)
  {
    uint64_t start = primeCounts[0];
    std::string startStr;

    if (start > 0)
    {
      double log10_start = std::log10((double) start);
      int exponent = (int) std::round(log10_start);
      startStr = "1e" + std::to_string(exponent) + "+";
    }

    // The thread keeps on running forever. It only stops if
    // a miscalculation occurs (due to a hardware issue)
    // or if the user cancels it using Ctrl+C.
    while (true)
    {
      for (uint64_t i = 1; i < primeCounts.size(); i++)
      {
        auto t1 = std::chrono::system_clock::now();
        uint64_t ChunkSize = (uint64_t) 1e11;
        uint64_t threadStart = start + ChunkSize * (i - 1);
        uint64_t threadStop = threadStart + ChunkSize;
        uint64_t count = 0;

        // We use 2 different algorithms for counting primes in order
        // to use as many of the CPU's resources as possible.
        // All threads alternately execute algorithm 1 and algorithm 2.
        if ((threadId + i) % 2)
        {
          // Single threaded count primes algorithm
          primesieve::PrimeSieve ps;
          count = ps.countPrimes(threadStart, threadStop);
        }
        else
        {
          // The primesieve::iterator::generate_next_primes() method
          // is vectorized using AVX512 on x64 CPUs.
          primesieve::iterator it(threadStart, threadStop);
          it.generate_next_primes();

          for (; it.primes_[it.size_ - 1] <= threadStop; it.generate_next_primes())
            count += it.size_ - it.i_;
          for (; it.primes_[it.i_] <= threadStop; it.i_++)
            count += 1;
        }

        auto t2 = std::chrono::system_clock::now();
        std::chrono::duration<double> seconds = t2 - t1;

        // If an error occurs we always print it
        // to the standard error stream.
        if (count != primeCounts[i])
        {
          std::unique_lock<std::mutex> lock(mutex);
          std::cerr << "Thread: " << std::setw(threadIdPadding) << std::right << threadId
                    << ", secs: " << std::fixed << std::setprecision(3) << seconds.count()
                    << ", PrimeCount(" << startStr << i-1 << "*1e11, " << startStr << i << "*1e11) = " << count << "   ERROR" << std::endl;
          std::exit(1);
        }
        else
        {
          // We don't wait here. Keeping the CPU buys is more
          // important then printing status output. 
          std::unique_lock<std::mutex> lock(mutex, std::try_to_lock);

          if (lock.owns_lock())
          {
            std::cout << "Thread: " << std::setw(threadIdPadding) << std::right << threadId
                      << ", secs: " << std::fixed << std::setprecision(3) << seconds.count()
                      << ", PrimeCount(" << startStr << i-1 << "*1e11, " << startStr << i << "*1e11) = " << count << "   OK" << std::endl;
          }
        }
      }
    }
  };

  using primesieve::Vector;
  Vector<std::future<void>> futures;
  futures.reserve(threads);

  std::cout << "Started " << opts.stressTestMode << " stress testing using " << threads << " threads." << std::endl;
  std::cout << "Stress testing keeps on running until either a miscalculation occurs" << std::endl;
  std::cout << "(due to a hardware issue) or you cancel it using Ctrl+C." << std::endl;
  std::cout << std::endl;

  // We create 1 thread per CPU core
  for (int threadId = 1; threadId <= threads; threadId++)
  {
    // In CPU stress test mode, we also run 20% of the threads using
    // the RAM stress test (threadId % 5 != 0). Since most PCs are
    // memory bound e.g. Desktop PC CPUs frequently only have 2 memory
    // channels we don't want to use too many RAM stress test threads
    // otherwise the threads might become idle due to the limited
    // memory bandwidth.
    if (opts.stressTestMode == "CPU" && threadId % 5 != 0)
      futures.emplace_back(std::async(std::launch::async, task, primeCounts_CpuMode, threadId));
    else // RAM stress test
      futures.emplace_back(std::async(std::launch::async, task, primeCounts_RamMode, threadId));
  }

  // Wait for all threads to finish
  for (auto& future : futures)
    future.wait();
}
