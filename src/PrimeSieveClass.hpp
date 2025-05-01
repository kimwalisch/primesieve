///
/// @file   PrimeSieveClass.hpp
/// @brief  PrimeSieve is a high level class that manages prime
///         sieving. It is used for printing and counting primes
///         and for computing the nth prime.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMESIEVE_CLASS_HPP
#define PRIMESIEVE_CLASS_HPP

#include <primesieve/Vector.hpp>
#include <stdint.h>

namespace primesieve {

using counts_t = Array<uint64_t, 6>;
class ParallelSieve;

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
  PRINT_STATUS      = 1 << 12
};

class PrimeSieve
{
public:
  PrimeSieve();
  PrimeSieve(ParallelSieve*);
  virtual ~PrimeSieve() = default;
  // Getters
  uint64_t getStart() const;
  uint64_t getStop() const;
  uint64_t getDistance() const;
  int getSieveSize() const;
  double getSeconds() const;
  // Setters
  void setStart(uint64_t);
  void setStop(uint64_t);
  void updateStatus(uint64_t);
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
  uint64_t negativeNthPrime(int64_t, uint64_t);
  // Count
  counts_t& getCounts();
  uint64_t getCount(int) const;
  uint64_t countPrimes(uint64_t, uint64_t);

protected:
  /// Sieve primes >= start_
  uint64_t start_ = 0;
  /// Sieve primes <= stop_
  uint64_t stop_ = 0;
  /// Time elapsed of sieve()
  double seconds_ = 0;
  /// Sieving status in percent
  double percent_ = 0;
  /// Prime number and prime k-tuplet counts
  counts_t counts_;
  void reset();
  void setStatus(double);

private:
  uint64_t sievedDistance_ = 0;
  uint64_t updateDistance_ = 0;
  /// Default flags
  int flags_ = COUNT_PRIMES;
  /// Sieve size in KiB
  int sieveSize_ = 0;
  /// Status updates must be synchronized by main thread
  ParallelSieve* parent_ = nullptr;
  void processSmallPrimes();
  static void printStatus(double, double);
};

} // namespace

#endif
