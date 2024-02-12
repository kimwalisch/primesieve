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
/// This test sieves up to 10^13 where most memory fits into the CPU's
/// cache. This tests puts the highest load on the CPU.
/// primeCounts_CpuMode[i] = PrimePi((i+1)*1e11) - PrimePi(i*1e11)
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
const Array<uint64_t, 100> primeCounts_CpuMode =
{
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
/// This test sieves near 10^17 where each thread uses about 135 MB.
/// This test puts higher load on the RAM, but less load on the CPU.
/// primeCounts_RamMode[i] = PrimePi(1e17+(i+1)*1e11) - PrimePi(1e17+i*1e11)
///
/// The table was generated using this bash program:
///
/// for i in {0..99};
/// do
///     res=$(primesieve 1e17+$i*1e11 -d1e11 -q);
///     printf "$((res))ull, ";
///     if [ $((($i+1) % 5)) -eq 0 ]; then printf "\n"; fi;
/// done
///
const Array<uint64_t, 100> primeCounts_RamMode =
{
  2554712095ull, 2554681993ull, 2554649315ull, 2554651498ull, 2554644244ull,
  2554686171ull, 2554691088ull, 2554665891ull, 2554617302ull, 2554660125ull,
  2554706966ull, 2554658780ull, 2554695439ull, 2554707394ull, 2554679470ull,
  2554628893ull, 2554673909ull, 2554623255ull, 2554666481ull, 2554643847ull,
  2554688629ull, 2554639827ull, 2554673925ull, 2554661160ull, 2554695332ull,
  2554712302ull, 2554659626ull, 2554635209ull, 2554695183ull, 2554668181ull,
  2554676415ull, 2554640063ull, 2554632147ull, 2554680747ull, 2554648205ull,
  2554617344ull, 2554665523ull, 2554705497ull, 2554694952ull, 2554620484ull,
  2554659846ull, 2554659419ull, 2554690829ull, 2554704969ull, 2554623780ull,
  2554656411ull, 2554675028ull, 2554644123ull, 2554682647ull, 2554657664ull,
  2554661012ull, 2554652585ull, 2554662768ull, 2554642884ull, 2554649131ull,
  2554695511ull, 2554676175ull, 2554648595ull, 2554701314ull, 2554615169ull,
  2554687615ull, 2554712832ull, 2554665636ull, 2554707500ull, 2554697836ull,
  2554669861ull, 2554673712ull, 2554632018ull, 2554699152ull, 2554683316ull,
  2554673723ull, 2554736577ull, 2554672478ull, 2554721577ull, 2554707863ull,
  2554723050ull, 2554659306ull, 2554661747ull, 2554689312ull, 2554641796ull,
  2554659688ull, 2554675637ull, 2554654642ull, 2554672630ull, 2554643861ull,
  2554651578ull, 2554686668ull, 2554697776ull, 2554670327ull, 2554701883ull,
  2554650228ull, 2554663464ull, 2554655459ull, 2554722937ull, 2554686090ull,
  2554678384ull, 2554664301ull, 2554711315ull, 2554619910ull, 2554663745ull
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
  auto task = [&](const Array<uint64_t, 100>& primeCounts,
                  int threadId,
                  uint64_t start)
  {
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
      for (uint64_t i = 0; i < primeCounts.size(); i++)
      {
        auto t1 = std::chrono::system_clock::now();
        uint64_t ChunkSize = (uint64_t) 1e11;
        uint64_t threadStart = start + ChunkSize * i;
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
                    << ", PrimeCount(" << startStr << i << "*1e11, " << startStr << i+1 << "*1e11) = " << count << "   ERROR" << std::endl;
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
                      << ", PrimeCount(" << startStr << i << "*1e11, " << startStr << i+1 << "*1e11) = " << count << "   OK" << std::endl;
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
    if (opts.stressTestMode == "RAM")
    {
      uint64_t start = 100000000000000000ll;
      futures.emplace_back(std::async(std::launch::async, task, primeCounts_RamMode, threadId, start));
    }
    else // opts.stressTestMode == "CPU"
    {
      uint64_t start = 0;
      futures.emplace_back(std::async(std::launch::async, task, primeCounts_CpuMode, threadId, start));
    }
  }

  // Wait for all threads to finish
  for (auto& future : futures)
    future.wait();
}
