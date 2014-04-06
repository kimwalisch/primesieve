///
/// @file   main.cpp
/// @brief  This file contains the main() function of the primesieve
///         console (terminal) application.
///
/// Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primesieve/ParallelPrimeSieve.hpp>
#include "cmdoptions.hpp"

#include <iostream>
#include <exception>
#include <iomanip>
#include <algorithm>
#include <string>

using namespace std;
using primesieve::ParallelPrimeSieve;

void printResults(const ParallelPrimeSieve& pps)
{
  const string primeLabels[7] =
  {
    "Prime numbers",
    "Twin primes",
    "Prime triplets",
    "Prime quadruplets",
    "Prime quintuplets",
    "Prime sextuplets",
    "Prime septuplets"
  };

  int size = 0;
  for (int i = 0; i < 7; i++) {
    if (pps.isCount(i))
      size = max(size, static_cast<int>(primeLabels[i].size()));
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

int main(int argc, char** argv)
{
  PrimeSieveOptions options = parseOptions(argc, argv);
  ParallelPrimeSieve pps;
  cout << left;

  try
  {
    if (!options.nthPrime)
    {
      if (options.n.size() == 1)
        options.n.push_front(0);
      pps.setStart(options.n[0]);
      pps.setStop(options.n[1]);
    }

    if (options.flags     != 0) pps.setFlags(options.flags);
    if (options.sieveSize != 0) pps.setSieveSize(options.sieveSize);
    if (options.threads   != 0) pps.setNumThreads(options.threads);
    else if (pps.isPrint())     pps.setNumThreads(1);

    if (!options.quiet && !options.nthPrime)
    {
      cout << "Sieve size = " << pps.getSieveSize() << " kilobytes" << endl;
      cout << "Threads    = " << pps.getNumThreads() << endl;
      if (!pps.isPrint())
        pps.addFlags(pps.PRINT_STATUS);
    }

    if (options.nthPrime)
    {
      uint64_t start = (options.n.size() > 1) ? options.n[1] : 0;
      uint64_t nthPrime = pps.nthPrime(options.n[0], start);
      cout << "Nth prime    : " << nthPrime << endl;
      cout << "Time elapsed : " << pps.getSeconds() << " sec" << endl;
    }
    else
    {
      pps.sieve();
      printResults(pps);
    }
  }
  catch (exception& e)
  {
    cerr << "Error: " << e.what() << "." << endl
         << "Try `primesieve --help' for more information." << endl;
    return 1;
  }
  return 0;
}
