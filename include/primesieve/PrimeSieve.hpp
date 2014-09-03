///
/// @file   PrimeSieve.hpp
/// @brief  The PrimeSieve class provides an easy API for prime
///         sieving (single-threaded).
///
/// Copyright (C) 2014 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMESIEVE_CLASS_HPP
#define PRIMESIEVE_CLASS_HPP

#include "Callback.hpp"

#include <stdint.h>
#include <string>
#include <vector>

namespace primesieve {

/// PrimeSieve is a highly optimized C++ implementation of the
/// segmented sieve of Eratosthenes that generates primes and prime
/// k-tuplets (twin primes, prime triplets, ...) in order up to 2^64
/// maximum. The README file describes the algorithms used in more
/// detail and doc/EXAMPLES contains source code examples.
///
class PrimeSieve
{
  friend class PrimeFinder;
  friend class LockGuard;
public:
  /// Public flags for use with setFlags(int)
  /// @pre flag < (1 << 20)
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
  PrimeSieve(PrimeSieve&, int);
  virtual ~PrimeSieve();
  // Getters
  uint64_t getStart() const;
  uint64_t getStop() const;
  int getSieveSize() const;
  int getFlags() const;
  double getStatus() const;
  double getSeconds() const;
  // Setters
  void setStart(uint64_t);
  void setStop(uint64_t);
  void setSieveSize(int);
  void setFlags(int);
  void addFlags(int);
  // Bool is*
  bool isFlag(int) const;
  bool isCallback() const;
  bool isCount() const;
  bool isCount(int) const;
  bool isPrint() const;
  bool isPrint(int) const;
  // Sieve
  virtual void sieve();
  void sieve(uint64_t, uint64_t);
  void sieve(uint64_t, uint64_t, int);
  // Callback
  void callbackPrimes(uint64_t, uint64_t, void (*)(uint64_t));
  void callbackPrimes(uint64_t, uint64_t, void (*)(uint64_t, int));
  void callbackPrimes(uint64_t, uint64_t, Callback<uint64_t>*);
  void callbackPrimes(uint64_t, uint64_t, Callback<uint64_t, int>*);
  void callbackPrimes_c(uint64_t, uint64_t, void (*)(uint64_t));
  void callbackPrimes_c(uint64_t, uint64_t, void (*)(uint64_t, int));
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
  uint64_t getPrimeCount() const;
  uint64_t getTwinCount() const;
  uint64_t getTripletCount() const;
  uint64_t getQuadrupletCount() const;
  uint64_t getQuintupletCount() const;
  uint64_t getSextupletCount() const;
  uint64_t getCount(int) const;
protected:
  /// Sieve primes >= start_
  uint64_t start_;
  /// Sieve primes <= stop_
  uint64_t stop_;
  /// Prime number and prime k-tuplet counts
  std::vector<uint64_t> counts_;
  /// Time elapsed of sieve()
  double seconds_;
  uint64_t getInterval() const;
  void reset();
  virtual double getWallTime() const;
  virtual void setLock();
  virtual void unsetLock();
  virtual bool updateStatus(uint64_t, bool);
private:
  struct SmallPrime
  {
    uint32_t firstPrime;
    uint32_t lastPrime;
    int index;
    std::string str;
  };
  static const SmallPrime smallPrimes_[8];
  /// Sum of all processed segments
  uint64_t processed_;
  /// Sum of processed segments that hasn't been updated yet
  uint64_t toUpdate_;
  /// Status of sieve() in percent
  double percent_;
  /// Sieve size in kilobytes
  int sieveSize_;
  /// Flags (settings) for PrimeSieve e.g. COUNT_PRIMES, PRINT_TWINS, ...
  int flags_;
  /// ParallelPrimeSieve thread number
  int threadNum_;
  /// Pointer to the parent ParallelPrimeSieve object
  PrimeSieve* parent_;
  /// Callbacks for use with *callbackPrimes()
  void (*callback_)(uint64_t);
  void (*callback_tn_)(uint64_t, int);
  Callback<uint64_t>* cb_;
  Callback<uint64_t, int>* cb_tn_;
  static void printStatus(double, double);
  bool isFlag(int, int) const;
  bool isValidFlags(int) const;
  bool isStatus() const;
  bool isParallelPrimeSieveChild() const;
  void doSmallPrime(const SmallPrime&);
  enum
  {
      INIT_STATUS = 0,
    FINISH_STATUS = 10
  };
  /// Private flags
  /// @pre flag >= (1 << 20)
  enum
  {
    CALLBACK_PRIMES        = 1 << 20,
    CALLBACK_PRIMES_TN     = 1 << 21,
    CALLBACK_PRIMES_OBJ    = 1 << 22,
    CALLBACK_PRIMES_OBJ_TN = 1 << 23,
    CALLBACK_PRIMES_C      = 1 << 24,
    CALLBACK_PRIMES_C_TN   = 1 << 25
  };
};

} // namespace primesieve

#endif
