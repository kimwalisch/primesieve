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
#include <primesieve/macros.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/PrimeSieve.hpp>
#include <primesieve/Vector.hpp>
#include "CmdOptions.hpp"

#include <stdint.h>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <future>
#include <mutex>

using primesieve::Array;

namespace {

/// Lookup table of correct prime count results.
/// primeCounts_1e13[i] = PrimePi(i*1e11) - PrimePi((i-1)*1e11)
/// This test sieves up to 10^13 where most memory fits into
/// the CPU's cache. Each thread uses < 5 MiB of memory.
/// This tests puts the highest load on the CPU, but not much
/// load on the RAM.
///
/// The table was generated using this bash program:
///
/// for i in {0..98};
/// do
///     res=$(primesieve $i*1e11 -d1e11 -q);
///     printf "$((res))ull, ";
///     if [ $((($i+1) % 5)) -eq 0 ]; then printf "\n"; fi;
/// done
///
const Array<uint64_t, 100> primeCounts_1e13 =
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
  3345852373ull, 3344702803ull, 3343552482ull, 3342407298ull
};

/// Lookup table of correct prime count results.
/// primeCounts_1e18[i] = PrimePi(1e18+i*1e11) - PrimePi(1e18+(i-1)*1e11)
/// This test sieves near 10^18 where each thread uses about 400 MiB.
/// This test puts higher load on the RAM, but less load on the CPU.
///
/// The table was generated using this bash program:
///
/// for i in {0..98};
/// do
///     res=$(primesieve 1e18+$i*1e11 -d1e11 -q);
///     printf "$((res))ull, ";
///     if [ $((($i+1) % 5)) -eq 0 ]; then printf "\n"; fi;
/// done
///
const Array<uint64_t, 100> primeCounts_1e18 =
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
  2412686405ull, 2412760693ull, 2412749045ull, 2412744369ull
};

/// Lookup table of correct prime count results.
/// primeCounts_1e19[i] = PrimePi(1e19+i*1e11) - PrimePi(1e19+(i-1)*1e11)
/// This test sieves near 10^19 where each thread uses about 1160 MiB.
/// This test puts the highest load on the RAM.
///
/// The table was generated using this bash program:
///
/// for i in {0..98};
/// do
///     res=$(primesieve 1e19+$i*1e11 -d1e11 -q);
///     printf "$((res))ull, ";
///     if [ $((($i+1) % 5)) -eq 0 ]; then printf "\n"; fi;
/// done
///
const Array<uint64_t, 100> primeCounts_1e19 =
{
  /* Start number = */ 10000000000000000000ull,
  2285693139ull, 2285771432ull, 2285721987ull, 2285796471ull, 2285730708ull,
  2285716716ull, 2285733641ull, 2285757285ull, 2285720752ull, 2285801995ull,
  2285743253ull, 2285754041ull, 2285813208ull, 2285795990ull, 2285776261ull,
  2285730339ull, 2285745644ull, 2285771975ull, 2285723622ull, 2285779074ull,
  2285709871ull, 2285687732ull, 2285808429ull, 2285734791ull, 2285743638ull,
  2285721904ull, 2285744974ull, 2285707225ull, 2285745781ull, 2285748093ull,
  2285755858ull, 2285721915ull, 2285805237ull, 2285794779ull, 2285735662ull,
  2285705038ull, 2285779842ull, 2285830487ull, 2285765764ull, 2285693068ull,
  2285769015ull, 2285788780ull, 2285779006ull, 2285788378ull, 2285783472ull,
  2285753193ull, 2285766248ull, 2285778455ull, 2285724140ull, 2285758342ull,
  2285797763ull, 2285740196ull, 2285749654ull, 2285711236ull, 2285755796ull,
  2285772691ull, 2285743328ull, 2285704177ull, 2285773416ull, 2285757020ull,
  2285722476ull, 2285715695ull, 2285770801ull, 2285760821ull, 2285756826ull,
  2285768039ull, 2285696767ull, 2285754334ull, 2285762901ull, 2285731594ull,
  2285845787ull, 2285690625ull, 2285758896ull, 2285739685ull, 2285748823ull,
  2285802237ull, 2285807963ull, 2285761323ull, 2285758845ull, 2285783897ull,
  2285736703ull, 2285778422ull, 2285740667ull, 2285784235ull, 2285726535ull,
  2285756542ull, 2285751248ull, 2285794950ull, 2285817821ull, 2285792397ull,
  2285779113ull, 2285757305ull, 2285785506ull, 2285730168ull, 2285787863ull,
  2285748648ull, 2285751228ull, 2285725270ull, 2285701010ull
};

void stressTestInfo(const std::string& stressTestMode,
                    int threads)
{
  ASSERT(stressTestMode == "CPU" || stressTestMode == "RAM");

  std::cout << "Started " << stressTestMode << " stress testing using " << threads << " threads.\n";
  std::cout << "The expected memory usage is: " << threads << " threads * ";

  if (stressTestMode == "CPU")
  {
    int threads_1e18 = threads / 5;
    int threads_1e13 = threads - threads_1e18;
    int avgMiB = (threads_1e13 * 3 /* MiB */ + threads_1e18 * 400 /* MiB */) / threads;
    std::cout << avgMiB << " MiB = " << threads * avgMiB << " MiB.\n";
  }
  else // stressTestMode == "RAM"
    std::cout << "1.16 GiB = " << std::fixed << std::setprecision(2) << threads * 1.16 << " GiB.\n";

  std::cout << "Stress testing keeps on running until either a miscalculation occurs\n";
  std::cout << "(due to a hardware issue) or you cancel it using Ctrl+C.\n";
  std::cout << std::endl;
}

std::string getStartString(uint64_t start)
{
  ASSERT(start % 10 == 0);

  if (start == 0)
    return std::string();
  else
  {
    double log10_start = std::log10(start);
    int exponent = (int) std::round(log10_start);
    return "1e" + std::to_string(exponent) + "+";
  }
}

/// We evenly distribute the start indexes of the
/// different threads. (dist % 2 == 1) ensures that we
/// get both even and odd start indexes.
///
uint64_t getStartIndex(int threadId,
                       int threads,
                       const Array<uint64_t, 100>& primeCounts)
{
  uint64_t dist = primeCounts.size() / threads;
  dist += (dist % 2 == 0);
  ASSERT(dist >= 1 && dist % 2 == 1);
  return 1 + (dist * threadId) % primeCounts.size();
}

/// Count primes using a PrimeSieve object, on x64 CPUs this
/// uses the POPCNT instruction for counting primes.
/// PrimeSieve objects use a single thread.
///
uint64_t countPrimesAlgo1(uint64_t start, uint64_t stop)
{
  primesieve::PrimeSieve ps;
  return ps.countPrimes(start, stop);
}

/// Count primes using a primesieve::iterator, this uses the
/// PrimeGenerator::fillNextPrimes() method which is
/// vectorized using AVX512 on x64 CPUs.
///
uint64_t countPrimesAlgo2(uint64_t start, uint64_t stop)
{
  primesieve::iterator it(start, stop);
  it.generate_next_primes();
  uint64_t count = 0;

  for (; it.primes_[it.size_ - 1] <= stop; it.generate_next_primes())
    count += it.size_ - it.i_;
  for (; it.primes_[it.i_] <= stop; it.i_++)
    count += 1;

  return count;
}

} // namespace

void stressTest(const CmdOptions& opts)
{
  int maxThreads = std::thread::hardware_concurrency();
  int threads = (opts.threads > 0) ? opts.threads : maxThreads;
  threads = inBetween(1, threads, maxThreads);
  int threadIdPadding = (int) std::to_string(threads).size();
  auto lastStatusOutput = std::chrono::system_clock::now();
  std::mutex mutex;

  // Each thread executes 1 task
  auto task = [&](int threadId, const Array<uint64_t, 100>& primeCounts)
  {
    uint64_t start = primeCounts[0];
    std::size_t maxIndex = primeCounts.size() - 1;
    int iPadding = (int) std::to_string(maxIndex).size();
    std::string startStr = getStartString(start);
    uint64_t i = getStartIndex(threadId, threads, primeCounts);

    // The thread keeps on running forever. It only stops if
    // a miscalculation occurs (due to a hardware issue)
    // or if the user cancels it using Ctrl+C.
    for (; true; i = 1)
    {
      for (; i < primeCounts.size(); i++)
      {
        auto t1 = std::chrono::system_clock::now();
        uint64_t ChunkSize = (uint64_t) 1e11;
        uint64_t threadStart = start + ChunkSize * (i - 1);
        uint64_t threadStop = threadStart + ChunkSize;
        uint64_t count;

        // We use 2 different algorithms for counting primes in order
        // to use as many of the CPU's resources as possible.
        // All threads alternately execute algorithm 1 and algorithm 2.
        if (i % 2)
          count = countPrimesAlgo1(threadStart, threadStop);
        else
          count = countPrimesAlgo2(threadStart, threadStop);

        auto t2 = std::chrono::system_clock::now();
        std::chrono::duration<double> secsThread = t2 - t1;

        // If an error occurs we always print it
        // to the standard error stream.
        if (count != primeCounts[i])
        {
          std::unique_lock<std::mutex> lock(mutex);
          std::cerr << "Thread: " << std::setw(threadIdPadding) << std::right << threadId
                    << ", secs: " << std::fixed << std::setprecision(3) << secsThread.count()
                    << ", PrimeCount(" << startStr << std::setw(iPadding) << std::right << i-1 << "e11, "
                    << startStr << std::setw(iPadding) << std::right << i << "e11) = " << count << "   ERROR" << std::endl;
          std::exit(1);
        }
        else
        {
          // We don't wait here. Keeping the CPU busy is more
          // important then printing status output. 
          std::unique_lock<std::mutex> lock(mutex, std::try_to_lock);

          if (lock.owns_lock())
          {
            std::chrono::duration<double> secsLastStatusOutput = t2 - lastStatusOutput;

            // Prevent excessive status output when using many
            // threads. We print one result every 10 secs.
            if (secsLastStatusOutput.count() >= 10)
            {
              lastStatusOutput = t2;
              std::cout << "Thread: " << std::setw(threadIdPadding) << std::right << threadId
                        << ", secs: " << std::fixed << std::setprecision(3) << secsThread.count()
                        << ", PrimeCount(" << startStr << std::setw(iPadding) << std::right << i-1 << "e11, "
                        << startStr << std::setw(iPadding) << std::right << i << "e11) = " << count << "   OK" << std::endl;
            }
          }
        }
      }
    }
  };

  stressTestInfo(opts.stressTestMode, threads);

  using primesieve::Vector;
  Vector<std::future<void>> futures;
  futures.reserve(threads);

  // We create 1 thread per CPU core
  for (int threadId = 1; threadId <= threads; threadId++)
  {
    if (opts.stressTestMode == "RAM")
      futures.emplace_back(std::async(std::launch::async, task, threadId, primeCounts_1e19));
    else
    {
      // In CPU stress test mode, we run 80% of the threads near 10^13
      // where most memory fits into the CPU's cache. Then we also run
      // 20% of the threads near 10^18 where each thread uses about
      // 400 MiB of memory. We don't want to run too many threads near
      // 10^18 otherwise the threads might become idle due to the
      // limited memory bandwidth on most PCs.
      if (threadId % 5 != 0)
        futures.emplace_back(std::async(std::launch::async, task, threadId, primeCounts_1e13));
      else
        futures.emplace_back(std::async(std::launch::async, task, threadId, primeCounts_1e18));
    }
  }

  for (auto& future : futures)
    future.wait();
}
