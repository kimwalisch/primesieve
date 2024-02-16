///
/// @file   stressTest.cpp
/// @brief  Run a stress test (--stress-test[=MODE] command-line
///         option) that puts maximum load on the CPU (default) or RAM.
///         The stress test keeps on running until either a
///         miscalculation occurs (due to a hardware issue) or the
///         timeout (--timeout=SECS option) expires.
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
/// the CPU's cache. Each thread uses about 3 MiB of memory.
/// This tests puts the highest load on the CPU, but not much
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
  /* Start number = */ 10000000000000ull,
  3340141707ull, 3339037770ull, 3337978139ull, 3336895789ull, 3335816088ull,
  3334786528ull, 3333711223ull, 3332674785ull, 3331678168ull, 3330629301ull,
  3329607166ull, 3328673627ull, 3327628347ull, 3326668678ull, 3325640524ull,
  3324742444ull, 3323791292ull, 3322806916ull, 3321871448ull, 3320978003ull,
  3320071119ull, 3319135499ull, 3318180524ull, 3317331622ull, 3316460192ull,
  3315535967ull, 3314685498ull, 3313824325ull, 3312975770ull, 3312115313ull,
  3311302346ull, 3310438260ull, 3309566639ull, 3308822830ull, 3307965666ull,
  3307206437ull, 3306366382ull, 3305523133ull, 3304756621ull, 3303985935ull,
  3303188494ull, 3302450534ull, 3301624455ull, 3300931434ull, 3300140636ull,
  3299387997ull, 3298659572ull, 3297919672ull, 3297202595ull, 3296420883ull,
  3295716204ull, 3294964942ull, 3294305835ull, 3293606447ull, 3292847935ull,
  3292190654ull, 3291459406ull, 3290784567ull, 3290083004ull, 3289386555ull,
  3288770253ull, 3288049408ull, 3287445692ull, 3286757785ull, 3286108293ull,
  3285403869ull, 3284758824ull, 3284148268ull, 3283516237ull, 3282842708ull,
  3282210028ull, 3281607239ull, 3280971749ull, 3280348811ull, 3279699440ull,
  3279124815ull, 3278501300ull, 3277898840ull, 3277282614ull, 3276682694ull,
  3276121352ull, 3275505636ull, 3274928897ull, 3274299689ull, 3273743021ull,
  3273135693ull, 3272563375ull, 3272020535ull, 3271457321ull, 3270889981ull,
  3270322147ull, 3269766399ull, 3269190820ull, 3268634444ull, 3268093100ull,
  3267530619ull, 3267004191ull, 3266440817ull, 3265923128ull
};

/// Lookup table of correct prime count results.
/// primeCounts_1e16[i] = PrimePi(1e16+i*1e11) - PrimePi(1e16+(i-1)*1e11)
/// This test sieves near 10^16 where each thread uses about 47.3 MiB.
///
/// The table was generated using this bash program:
///
/// for i in {0..98};
/// do
///     res=$(primesieve 1e16+$i*1e11 -d1e11 -q);
///     printf "$((res))ull, ";
///     if [ $((($i+1) % 5)) -eq 0 ]; then printf "\n"; fi;
/// done
///
const Array<uint64_t, 100> primeCounts_1e16 =
{
  /* Start number = */ 10000000000000000ull,
  2714336584ull, 2714326790ull, 2714355257ull, 2714346750ull, 2714380169ull,
  2714367355ull, 2714315487ull, 2714346194ull, 2714339624ull, 2714291584ull,
  2714260506ull, 2714328048ull, 2714330822ull, 2714364692ull, 2714350998ull,
  2714321130ull, 2714300726ull, 2714295343ull, 2714311187ull, 2714326401ull,
  2714308833ull, 2714343112ull, 2714304377ull, 2714303764ull, 2714302043ull,
  2714310793ull, 2714286914ull, 2714323447ull, 2714288442ull, 2714286027ull,
  2714381328ull, 2714331996ull, 2714321106ull, 2714324408ull, 2714284821ull,
  2714304663ull, 2714356021ull, 2714350025ull, 2714312511ull, 2714291166ull,
  2714281777ull, 2714323140ull, 2714330999ull, 2714301448ull, 2714260108ull,
  2714331787ull, 2714317499ull, 2714307884ull, 2714277954ull, 2714288641ull,
  2714307481ull, 2714301755ull, 2714295022ull, 2714295584ull, 2714256920ull,
  2714304618ull, 2714317910ull, 2714304904ull, 2714219074ull, 2714294337ull,
  2714340626ull, 2714263462ull, 2714270120ull, 2714302754ull, 2714319589ull,
  2714285403ull, 2714279110ull, 2714291862ull, 2714300197ull, 2714319007ull,
  2714243950ull, 2714366813ull, 2714290168ull, 2714319835ull, 2714278666ull,
  2714229768ull, 2714246712ull, 2714284668ull, 2714332601ull, 2714320497ull,
  2714279010ull, 2714271560ull, 2714273265ull, 2714285220ull, 2714301566ull,
  2714308511ull, 2714248825ull, 2714250223ull, 2714324411ull, 2714272780ull,
  2714262114ull, 2714312851ull, 2714250307ull, 2714271837ull, 2714250326ull,
  2714299075ull, 2714278170ull, 2714242608ull, 2714262238ull
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
    int threads_1e16 = threads / 5;
    int threads_1e13 = threads - threads_1e16;
    double avgMiB = (threads_1e13 * 3.0 + threads_1e16 * 47.3) / threads;
    // Due to over-allocation and memory fragmentation
    // caused by the allocator (malloc), we increase
    // the expected memory usage by 2%.
    avgMiB *= 1.02;
    double avgGiB = avgMiB / 1024.0;

    if (threads * avgMiB < 1024)
      std::cout << std::fixed << std::setprecision(2) << avgMiB << " MiB = "
                << std::fixed << std::setprecision(2) << threads * avgMiB << " MiB.\n";
    else
      std::cout << std::fixed << std::setprecision(2) << avgMiB << " MiB = "
                << std::fixed << std::setprecision(2) << threads * avgGiB << " GiB.\n";
  }
  else // stressTestMode == "RAM"
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
  primesieve::PrimeSieve ps;
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
  int maxThreads = std::thread::hardware_concurrency();
  int threads = (opts.threads > 0) ? opts.threads : maxThreads;
  threads = inBetween(1, threads, maxThreads);
  auto timeBeginning = std::chrono::system_clock::now();
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
          uint64_t ChunkSize = (uint64_t) 1e11;
          uint64_t threadStart = start + ChunkSize * (i - 1);
          uint64_t threadStop = threadStart + ChunkSize;

          auto t1 = std::chrono::system_clock::now();
          uint64_t count = countPrimes(i, threadStart, threadStop);
          auto t2 = std::chrono::system_clock::now();
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
            // important then printing status output. 
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

  if (opts.stressTestMode == "RAM")
  {
    // Each thread uses about 1.16 GiB
    for (int threadId = 1; threadId <= threads; threadId++)
      workerThreads.emplace_back(task, threadId, primeCounts_1e19);
  }
  else // CPU stress test
  {
    for (int threadId = 1; threadId <= threads; threadId++)
    {
      // In CPU stress test mode we run 80% of the threads near 1e13
      // where all memory fits into the CPU's cache. And we run 20%
      // of the threads near 1e16 where not all memory fits into the
      // CPU's cache. Since Desktop PC CPUs frequently only have two
      // memory channels we don't want to use too many threads that
      // are sieving >= 1e16 otherwise the threads might become idle
      // due to the limited memory bandwidth.
      if (threadId % 5 != 0)
        workerThreads.emplace_back(task, threadId, primeCounts_1e13);
      else
        workerThreads.emplace_back(task, threadId, primeCounts_1e16);
    }
  }

  for (auto& thread : workerThreads)
    thread.join();

  if (statusOutputDelay > 0)
    std::cout << std::endl;

  std::cout << "All tests passed successfully!" << std::endl;
}
