///
/// @file   PrimeSieve.hpp
/// @brief  PrimeSieve is a high level class that manages prime
///         sieving. It is used for printing and counting primes
///         and for computing the nth prime.
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMESIEVE_CLASS_HPP
#define PRIMESIEVE_CLASS_HPP

#include "PreSieve.hpp"

#include <stdint.h>
#include <array>

namespace primesieve {

using counts_t = std::array<uint64_t, 6>;

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

class PrimeSieve
{
public:
  PrimeSieve();
  PrimeSieve(PrimeSieve*);
  virtual ~PrimeSieve();
  // Getters
  uint64_t getStart() const;
  uint64_t getStop() const;
  int getSieveSize() const;
  double getStatus() const;
  double getSeconds() const;
  PreSieve& getPreSieve();
  // Setters
  void setStart(uint64_t);
  void setStop(uint64_t);
  void setSieveSize(int);
  void setFlags(int);
  void addFlags(int);
  // Bool is*
  bool isCount(int) const;
  bool isCountPrimes() const;
  bool isCountkTuplets() const;
  bool isPrint() const;
  bool isPrint(int) const;
  bool isPrintPrimes() const;
  bool isPrintkTuplets() const;
  bool isFlag(int) const;
  bool isFlag(int, int) const;
  bool isStatus() const;
  // Sieve
  virtual void sieve();
  void sieve(uint64_t, uint64_t);
  void sieve(uint64_t, uint64_t, int);
  // nth prime
  uint64_t nthPrime(uint64_t);
  uint64_t nthPrime(int64_t, uint64_t);
  // Count
  counts_t& getCounts();
  uint64_t getCount(int) const;
  uint64_t countPrimes(uint64_t, uint64_t);
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
  /// Sieve size in KiB
  int sieveSize_;
  /// Setter methods set flags e.g. COUNT_PRIMES
  int flags_;
  /// parent ParallelSieve object
  PrimeSieve* parent_;
  PreSieve preSieve_;
  static void printStatus(double, double);
  bool isParallelSieve() const;
  void processSmallPrimes();
};

} // namespace

#endif
