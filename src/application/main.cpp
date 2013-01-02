///
/// @file   main.cpp
/// @brief  This is the primesieve console (terminal) application.
///         Precompiled binaries are available at:
/// @see    https://code.google.com/p/primesieve/downloads/list
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the New BSD License. See the
/// LICENSE file in the top level directory.
///

#include "cmdoptions.h"
#include "../soe/ParallelPrimeSieve.h"

#include <iostream>
#include <exception>
#include <iomanip>
#include <algorithm>
#include <string>

using namespace std;

void printResults(const ParallelPrimeSieve&);

int main(int argc, char** argv)
{
  // process command-line options
  PrimeSieveSettings settings = processOptions(argc, argv);
  cout << left;

  try {
    ParallelPrimeSieve pps;

    // set the sieve interval to [start, stop]
    pps.setStart(settings.start());
    pps.setStop(settings.stop());

    if (settings.flags     != 0) pps.setFlags(settings.flags);
    if (settings.sieveSize != 0) pps.setSieveSize(settings.sieveSize);
    if (settings.preSieve  != 0) pps.setPreSieve(settings.preSieve);
    if (settings.threads   != 0) pps.setNumThreads(settings.threads);

    if (!settings.quiet) {
      cout << setw(10) << "Pre-sieve"  << " = " << pps.getPreSieve()                  << endl;
      cout << setw(10) << "Sieve size" << " = " << pps.getSieveSize() << " kilobytes" << endl;
      cout << setw(10) << "Threads"    << " = " << pps.getNumThreads()                << endl;
      if (!pps.isPrint())
        pps.addFlags(pps.PRINT_STATUS);
    }

    // ready to sieve
    pps.sieve();
    printResults(pps);

  } catch (exception& e) {
    cerr << "Error: " << e.what() << "."                    << endl
         << "Try `primesieve --help' for more information." << endl;
    return 1;
  }
  return 0;
}

void printResults(const ParallelPrimeSieve& pps)
{
  const string primeLabels[7] = {
    "Prime numbers",
    "Twin primes",
    "Prime triplets",
    "Prime quadruplets",
    "Prime quintuplets",
    "Prime sextuplets",
    "Prime septuplets"
  };
  
  // get maximum label size
  int size = 0;
  for (int i = 0; i < 7; i++) {
    if (pps.isCount(i))
      size = max(size, (int) primeLabels[i].size());
  }

  for (int i = 0; i < 7; i++) {
    if (pps.isCount(i))
      cout << setw(size) << primeLabels[i] << " : "
           << pps.getCount(i)
           << endl;
  }
  if (!pps.isPrint())
    cout << setw(size) << "Time elapsed" << " : "
         << pps.getSeconds() << " sec"
         << endl;
}
