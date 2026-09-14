///
/// @file   stressTest.cpp
/// @brief  Run a stress test (--stress-test[=MODE] command-line
///         option) that puts maximum load on the CPU (default) or RAM.
///         The stress test keeps on running until either a
///         miscalculation occurs (due to a hardware issue) or the
///         timeout (--timeout=SECS option) expires.
///
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "CmdOptions.hpp"

#include <primesieve.hpp>
#include <PrimeSieveClass.hpp>
#include <primesieve/macros.hpp>
#include <primesieve/pmath.hpp>
#include <primesieve/Vector.hpp>

#include <stdint.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <mutex>
#include <thread>
#include <sstream>

using primesieve::Array;

namespace {

/// Lookup table of correct prime count results.
/// primeCounts_1e13[i] = PrimePi(1e13+i*1e11) - PrimePi(1e13+(i-1)*1e11)
/// This test sieves near 10^13 where most memory fits into
/// the CPU's cache. Each thread uses < 5 MiB of memory.
/// This test puts the highest load on the CPU, but not much
/// load on the RAM.
///
/// The table was generated using this bash program:
///
/// for i in {0..98};
/// do
///     res=$(primesieve 1e13+$i*1e11 -d1e11 -q);
///     printf "$((res))ull, ";
///     if [ $((($i+1) % 5)) -eq 0 ]; then printf "\n"; fi;
/// done
///
const Array<uint64_t, 100> primeCounts_1e13 =
{
  /* Start number = */ 10000000000000,
  3340141707, 3339037770, 3337978139, 3336895789, 3335816088,
  3334786528, 3333711223, 3332674785, 3331678168, 3330629301,
  3329607166, 3328673627, 3327628347, 3326668678, 3325640524,
  3324742444, 3323791292, 3322806916, 3321871448, 3320978003,
  3320071119, 3319135499, 3318180524, 3317331622, 3316460192,
  3315535967, 3314685498, 3313824325, 3312975770, 3312115313,
  3311302346, 3310438260, 3309566639, 3308822830, 3307965666,
  3307206437, 3306366382, 3305523133, 3304756621, 3303985935,
  3303188494, 3302450534, 3301624455, 3300931434, 3300140636,
  3299387997, 3298659572, 3297919672, 3297202595, 3296420883,
  3295716204, 3294964942, 3294305835, 3293606447, 3292847935,
  3292190654, 3291459406, 3290784567, 3290083004, 3289386555,
  3288770253, 3288049408, 3287445692, 3286757785, 3286108293,
  3285403869, 3284758824, 3284148268, 3283516237, 3282842708,
  3282210028, 3281607239, 3280971749, 3280348811, 3279699440,
  3279124815, 3278501300, 3277898840, 3277282614, 3276682694,
  3276121352, 3275505636, 3274928897, 3274299689, 3273743021,
  3273135693, 3272563375, 3272020535, 3271457321, 3270889981,
  3270322147, 3269766399, 3269190820, 3268634444, 3268093100,
  3267530619, 3267004191, 3266440817, 3265923128
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
  2285693139, 2285771432, 2285721987, 2285796471, 2285730708,
  2285716716, 2285733641, 2285757285, 2285720752, 2285801995,
  2285743253, 2285754041, 2285813208, 2285795990, 2285776261,
  2285730339, 2285745644, 2285771975, 2285723622, 2285779074,
  2285709871, 2285687732, 2285808429, 2285734791, 2285743638,
  2285721904, 2285744974, 2285707225, 2285745781, 2285748093,
  2285755858, 2285721915, 2285805237, 2285794779, 2285735662,
  2285705038, 2285779842, 2285830487, 2285765764, 2285693068,
  2285769015, 2285788780, 2285779006, 2285788378, 2285783472,
  2285753193, 2285766248, 2285778455, 2285724140, 2285758342,
  2285797763, 2285740196, 2285749654, 2285711236, 2285755796,
  2285772691, 2285743328, 2285704177, 2285773416, 2285757020,
  2285722476, 2285715695, 2285770801, 2285760821, 2285756826,
  2285768039, 2285696767, 2285754334, 2285762901, 2285731594,
  2285845787, 2285690625, 2285758896, 2285739685, 2285748823,
  2285802237, 2285807963, 2285761323, 2285758845, 2285783897,
  2285736703, 2285778422, 2285740667, 2285784235, 2285726535,
  2285756542, 2285751248, 2285794950, 2285817821, 2285792397,
  2285779113, 2285757305, 2285785506, 2285730168, 2285787863,
  2285748648, 2285751228, 2285725270, 2285701010
};

/// Time format: 3h 15m 57s
std::string getTimeElapsed(int64_t secs)
{
  // Seconds per: year, day, hour, minute, second
  Array<int64_t, 5> time = { 365 * 24 * 3600, 24 * 3600, 3600, 60, 1 };
  Array<char, 5> suffix = { 'y', 'd', 'h', 'm', 's' };
  std::string timeStr;

  for (std::size_t i = 0; i < time.size(); i++)
  {
    if (secs > time[i])
    {
      timeStr += timeStr.empty() ? "" : " ";
      timeStr += std::to_string(secs / time[i]) + suffix[i];
      secs %= time[i];
    }
  }

  return timeStr;
}

void stressTestInfo(const CmdOptions& opts,
                    int threads)
{
  std::cout << "Started " << opts.stressTestMode << " stress testing using " << threads << " threads.\n";
  std::cout << "The expected memory usage is: " << threads << " threads * ";

  if (opts.stressTestMode == "CPU")
  {
    double sieveSizeKiB = primesieve::get_sieve_size();
    double avgMiB = 2.6 + (sieveSizeKiB / 1024.0);
    std::cout << std::fixed << std::setprecision(2) << avgMiB << " MiB = "
              << std::fixed << std::setprecision(2) << threads * avgMiB << " MiB.\n";
  }
  else // RAM stress test
    std::cout << "1.16 GiB = " << std::fixed << std::setprecision(2) << threads * 1.16 << " GiB.\n";

  std::cout << "The stress test keeps on running until either a miscalculation occurs\n";
  std::cout << "(due to a hardware issue) or the timeout of " << getTimeElapsed(opts.timeout) << " expires.\n";
  std::cout << "You may cancel the stress test at any time using Ctrl+C.\n";
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

/// Date time format: "[Jan 13 22:07] "
std::string getDateTime()
{
#if 0
  // Thread safe.
  // Requires C23, but does not work yet with the
  // MSVC 2022 and MinGW-w64 compilers.
  std::tm result;
  std::time_t currentTime = std::time(nullptr);
  if (!localtime_r(&currentTime, &result))
    return "";

  std::ostringstream oss;
  oss << std::put_time(&result, "[%b %d %H:%M] ");
  return oss.str();
#else
  // Not thread safe.
  // But we lock a mutex before calling getDateTime()
  // hence this is not an issue for us.
  #if defined(_MSC_VER)
    #pragma warning(disable : 4996)
  #endif
  std::time_t currentTime = std::time(nullptr);
  std::tm* currentDateTime = std::localtime(&currentTime);
  if (!currentDateTime)
    return "";

  std::ostringstream oss;
  oss << std::put_time(currentDateTime, "[%b %d %H:%M] ");
  return oss.str();
#endif
}

void printResult(int threadId,
                 int threads,
                 uint64_t i,
                 uint64_t count,
                 const std::chrono::duration<double>& secsThread,
                 const Array<uint64_t, 100>& primeCounts)
{
  uint64_t start = primeCounts[0];
  std::string startStr = getStartString(start);
  std::size_t maxIndex = primeCounts.size() - 1;
  int iPadding = (int) std::to_string(maxIndex).size();
  int threadIdPadding = (int) std::to_string(threads).size();
  std::ostringstream oss;

  if (count == primeCounts[i])
  {
    oss << getDateTime()
        << "Thread " << std::setw(threadIdPadding) << std::right << threadId << ", "
        << std::fixed << std::setprecision(2) << secsThread.count() << " secs, "
        << "PrimePi(" << startStr << std::setw(iPadding) << std::right << i-1 << "e11, "
        << startStr << std::setw(iPadding) << std::right << i << "e11) = " << count << "   OK\n";

    std::cout << oss.str() << std::flush;
  }
  else
  {
    oss << getDateTime()
        << "Thread " << std::setw(threadIdPadding) << std::right << threadId << ", "
        << std::fixed << std::setprecision(2) << secsThread.count() << " secs, "
        << "PrimePi(" << startStr << std::setw(iPadding) << std::right << i-1 << "e11, "
        << startStr << std::setw(iPadding) << std::right << i << "e11) = " << count << "   ERROR\n\n"
        << "Miscalculation detected after running for: " << getTimeElapsed((int64_t) secsThread.count()) << "\n";

    std::cerr << oss.str();
  }
}

/// Count primes using a PrimeSieve object, on x64 CPUs this
/// uses the POPCNT instruction for counting primes.
/// PrimeSieve objects use a single thread.
///
NOINLINE uint64_t countPrimes1(uint64_t start, uint64_t stop)
{
  INDETERMINATE primesieve::PrimeSieve ps;
  return ps.countPrimes(start, stop);
}

/// Count primes using a primesieve::iterator, this uses the
/// PrimeGenerator::fillNextPrimes() method which is
/// vectorized using AVX512 on x64 CPUs.
///
NOINLINE uint64_t countPrimes2(uint64_t start, uint64_t stop)
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

/// We use 2 different algorithms for counting primes in order
/// to use as many of the CPU's resources as possible. All
/// threads alternately execute algorithm 1 and 2.
///
uint64_t countPrimes(uint64_t threadIndex,
                     uint64_t start,
                     uint64_t stop)
{
  if (threadIndex % 2)
    return countPrimes1(start, stop);
  else
    return countPrimes2(start, stop);
}

} // namespace

void stressTest(const CmdOptions& opts)
{
  using primesieve::inBetween;
  int maxThreads = std::thread::hardware_concurrency();
  int threads = (opts.threads > 0) ? opts.threads : maxThreads;
  threads = inBetween(1, threads, maxThreads);
  auto timeBeginning = std::chrono::steady_clock::now();
  auto lastStatusOutput = timeBeginning;
  int statusOutputDelay = 0;
  std::mutex mutex;

  // Each thread executes 1 task
  auto task = [&](int threadId, const Array<uint64_t, 100>& primeCounts)
  {
    try
    {
      // We evenly distribute the start indexes of the
      // different threads. (dist % 2 == 1) ensures that we
      // get both even and odd start indexes.
      uint64_t start = primeCounts[0];
      uint64_t dist = primeCounts.size() / threads;
      dist += (dist % 2 == 0);
      ASSERT(dist >= 1 && dist % 2 == 1);
      uint64_t i = 1 + (dist * threadId) % primeCounts.size();

      // The thread keeps on running forever. It only stops if
      // a miscalculation occurs (due to a hardware issue)
      // or if the user cancels it using Ctrl+C.
      for (; true; i = 1)
      {
        for (; i < primeCounts.size(); i++)
        {
          uint64_t chunkSize = (uint64_t) 1e11;
          uint64_t threadStart = start + chunkSize * (i - 1);
          uint64_t threadStop = threadStart + chunkSize;

          auto t1 = std::chrono::steady_clock::now();
          uint64_t count = countPrimes(i, threadStart, threadStop);
          auto t2 = std::chrono::steady_clock::now();
          std::chrono::duration<double> secsThread = t2 - t1;

          // If an error occurs we always print it
          // to the standard error stream.
          if (count != primeCounts[i])
          {
            std::unique_lock<std::mutex> lock(mutex);
            printResult(threadId, threads, i, count, secsThread, primeCounts);
            std::exit(1);
          }
          else
          {
            // --timeout option
            if (opts.timeout)
            {
              std::chrono::duration<double> secsBeginning = t2 - timeBeginning;
              if (secsBeginning.count() >= (double) opts.timeout)
                return;
            }

            // --quiet option, no status output
            if (opts.quiet)
              continue;

            // We don't wait here. Keeping the CPU busy is more
            // important than printing status output.
            std::unique_lock<std::mutex> lock(mutex, std::try_to_lock);
            if (!lock.owns_lock())
              continue;

            // We slowly increase the status output delay (in seconds)
            // until it reaches 10 minutes. This way, long running
            // computations don't produce excessive logs.
            std::chrono::duration<double> secsStatus = t2 - lastStatusOutput;
            if (secsStatus.count() >= statusOutputDelay)
            {
              lastStatusOutput = t2;
              statusOutputDelay += 7;
              statusOutputDelay = std::min(statusOutputDelay, 600);
              printResult(threadId, threads, i, count, secsThread, primeCounts);
            }
          }
        }
      }
    }
    catch (const std::bad_alloc&)
    {
      std::cerr << "ERROR: failed to allocate memory!\n";
      std::exit(1);
    }
    catch (const std::exception& e)
    {
      std::ostringstream oss;
      oss << "ERROR: " << e.what() << "\n";
      std::cerr << oss.str();
      std::exit(1);
    }
  };

  stressTestInfo(opts, threads);

  using primesieve::Vector;
  Vector<std::thread> workerThreads;
  workerThreads.reserve(threads);

  for (int threadId = 1; threadId <= threads; threadId++)
  {
    if (opts.stressTestMode == "CPU")
      workerThreads.emplace_back(task, threadId, primeCounts_1e13);
    else // RAM stress test
      workerThreads.emplace_back(task, threadId, primeCounts_1e19);
  }

  for (auto& thread : workerThreads)
    thread.join();

  if (statusOutputDelay > 0)
    std::cout << std::endl;

  std::cout << "All tests passed successfully!" << std::endl;
}
