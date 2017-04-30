///
/// @file   PrimeSieve.hpp
/// @brief  The PrimeSieve class is a high level class that
///         manages prime sieving using the PreSieve, SievingPrimes
///         and PrimeGenerator classes.
///
/// Copyright (C) 2017 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMESIEVE_CLASS_HPP
#define PRIMESIEVE_CLASS_HPP

#include <stdint.h>
#include <vector>

namespace primesieve {

class Store;

class PrimeSieve
{
public:
  enum
  {
    COUNT_PRIMES      = 1 << 0,
    COUNT_TWINS       = 1 << 1,
    COUNT_TRIPLETS    = 1 << 2,
    COUNT_QUADRUPLETS = 1 << 3,
    COUNT_QUINTUPLETS = 1 << 4,
    COUNT_SEXTUPLETS  = 1 << 5,
    PRINT_PRIMES      = 1 << 6,
    PRINT_TWINS       = 1 << 7,
    PRINT_TRIPLETS    = 1 << 8,
    PRINT_QUADRUPLETS = 1 << 9,
    PRINT_QUINTUPLETS = 1 << 10,
    PRINT_SEXTUPLETS  = 1 << 11,
    PRINT_STATUS      = 1 << 12,
    CALCULATE_STATUS  = 1 << 13
  };
  PrimeSieve();
  PrimeSieve(PrimeSieve*);
  virtual ~PrimeSieve();
  // Getters
  uint64_t getStart() const;
  uint64_t getStop() const;
  int getSieveSize() const;
  double getStatus() const;
  double getSeconds() const;
  Store& getStore();
  // Setters
  void setStart(uint64_t);
  void setStop(uint64_t);
  void setSieveSize(int);
  void setFlags(int);
  void addFlags(int);
  // Bool is*
  bool isCount() const;
  bool isCount(int) const;
  bool isPrint() const;
  bool isPrint(int) const;
  bool isFlag(int) const;
  bool isFlag(int, int) const;
  bool isStatus() const;
  bool isStore() const;
  // Sieve
  virtual void sieve();
  void sieve(uint64_t, uint64_t);
  void sieve(uint64_t, uint64_t, int);
  void storePrimes(uint64_t, uint64_t, Store*);
  // nth prime
  uint64_t nthPrime(uint64_t);
  uint64_t nthPrime(int64_t, uint64_t);
  // Print
  void printPrimes(uint64_t, uint64_t);
  void printTwins(uint64_t, uint64_t);
  void printTriplets(uint64_t, uint64_t);
  void printQuadruplets(uint64_t, uint64_t);
  void printQuintuplets(uint64_t, uint64_t);
  void printSextuplets(uint64_t, uint64_t);
  // Count
  uint64_t countPrimes(uint64_t, uint64_t);
  uint64_t countTwins(uint64_t, uint64_t);
  uint64_t countTriplets(uint64_t, uint64_t);
  uint64_t countQuadruplets(uint64_t, uint64_t);
  uint64_t countQuintuplets(uint64_t, uint64_t);
  uint64_t countSextuplets(uint64_t, uint64_t);
  // Count getters
  typedef std::vector<uint64_t> counts_t;
  counts_t& getCounts();
  uint64_t getPrimeCount() const;
  uint64_t getTwinCount() const;
  uint64_t getTripletCount() const;
  uint64_t getQuadrupletCount() const;
  uint64_t getQuintupletCount() const;
  uint64_t getSextupletCount() const;
  uint64_t getCount(int) const;
  virtual bool updateStatus(uint64_t, bool tryLock = true);
protected:
  /// Sieve primes >= start_
  uint64_t start_;
  /// Sieve primes <= stop_
  uint64_t stop_;
  /// Prime number and prime k-tuplet counts
  counts_t counts_;
  /// Time elapsed of sieve()
  double seconds_;
  uint64_t getDistance() const;
  void reset();
private:
  /// Sum of all processed segments
  uint64_t processed_;
  /// Sum of processed segments to update
  uint64_t toUpdate_;
  /// Status of sieve() in percent
  double percent_;
  /// Sieve size in kilobytes
  int sieveSize_;
  /// Setter methods set flags e.g. COUNT_PRIMES
  int flags_;
  /// parent ParallelPrimeSieve object
  PrimeSieve* parent_;
  Store* store_;
  static void printStatus(double, double);
  bool isParallelPrimeSieve() const;
  void processSmallPrimes();
};

} // namespace

#endif
