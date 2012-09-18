//
// Copyright (c) 2012 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of primesieve.
// Homepage: http://primesieve.googlecode.com
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials provided
//     with the distribution.
//   * Neither the name of the author nor the names of its
//     contributors may be used to endorse or promote products derived
//     from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PRIMESIEVE_H
#define PRIMESIEVE_H

#define PRIMESIEVE_VERSION "3.9"
#define PRIMESIEVE_MAJOR_VERSION 3
#define PRIMESIEVE_MINOR_VERSION 9
#define PRIMESIEVE_YEAR 2012

#include <stdint.h>
#include <stdexcept>
#include <string>
#include <vector>

/// PrimeSieve objects throw primesieve_error exceptions for
/// invalid arguments like start > stop.
///
class primesieve_error : public std::runtime_error {
public:
  primesieve_error(const std::string& message)
    : std::runtime_error(message)
  { }
};

namespace soe { class PrimeNumberFinder; }
using soe::PrimeNumberFinder;

/// PrimeSieve is a highly optimized C++ implementation of the
/// segmented sieve of Eratosthenes that generates primes and prime
/// k-tuplets (twin primes, prime triplets, ...) in order up to 2^64
/// maximum. The README file describes the algorithms used in more
/// detail and doc/USAGE_EXAMPLES contains source code examples.
///
class PrimeSieve {
  friend class PrimeNumberFinder;
public:
  /// Public flags for use with setFlags(int)
  /// @pre flag < (1 << 20)
  enum {
    COUNT_PRIMES      = 1 << 0,
    COUNT_TWINS       = 1 << 1,
    COUNT_TRIPLETS    = 1 << 2,
    COUNT_QUADRUPLETS = 1 << 3,
    COUNT_QUINTUPLETS = 1 << 4,
    COUNT_SEXTUPLETS  = 1 << 5,
    COUNT_SEPTUPLETS  = 1 << 6,
    PRINT_PRIMES      = 1 << 7,
    PRINT_TWINS       = 1 << 8,
    PRINT_TRIPLETS    = 1 << 9,
    PRINT_QUADRUPLETS = 1 << 10,
    PRINT_QUINTUPLETS = 1 << 11,
    PRINT_SEXTUPLETS  = 1 << 12,
    PRINT_SEPTUPLETS  = 1 << 13,
    PRINT_STATUS      = 1 << 14,
    CALCULATE_STATUS  = 1 << 15
  };
  PrimeSieve();
  PrimeSieve(PrimeSieve&, int);
  virtual ~PrimeSieve() { }
  uint64_t getStart() const;
  uint64_t getStop() const;
  double getStatus() const;
  double getSeconds() const;
  int getPreSieve() const;
  int getSieveSize() const;
  int getFlags() const;
  bool isFlag(int flag) const;
  bool isFlag(int first, int last) const;
  bool isGenerate() const;
  bool isCount() const;
  bool isCount(int index) const;
  bool isPrint() const;
  bool isPrint(int index) const;
  bool isStatus() const;
  void setStart(uint64_t);
  void setStop(uint64_t);
  void setSieveSize(int);
  void setPreSieve(int);
  void setFlags(int);
  void addFlags(int);
  virtual void sieve();
  void sieve(uint64_t, uint64_t);
  void sieve(uint64_t, uint64_t, int);
  void generatePrimes(uint32_t, uint32_t, void (*)(uint32_t));
  void generatePrimes(uint64_t, uint64_t, void (*)(uint64_t));
  void generatePrimes(uint32_t, uint32_t, void (*)(uint32_t, void*), void*);
  void generatePrimes(uint64_t, uint64_t, void (*)(uint64_t, void*), void*);
  void generatePrimes(uint64_t, uint64_t, void (*)(uint64_t, int));
  void printPrimes(uint64_t, uint64_t);
  void printTwins(uint64_t, uint64_t);
  void printTriplets(uint64_t, uint64_t);
  void printQuadruplets(uint64_t, uint64_t);
  void printQuintuplets(uint64_t, uint64_t);
  void printSextuplets(uint64_t, uint64_t);
  void printSeptuplets(uint64_t, uint64_t);
  uint64_t getPrimeCount(uint64_t, uint64_t);
  uint64_t getTwinCount(uint64_t, uint64_t);
  uint64_t getTripletCount(uint64_t, uint64_t);
  uint64_t getQuadrupletCount(uint64_t, uint64_t);
  uint64_t getQuintupletCount(uint64_t, uint64_t);
  uint64_t getSextupletCount(uint64_t, uint64_t);
  uint64_t getSeptupletCount(uint64_t, uint64_t);
  uint64_t getCounts(int) const;
  uint64_t getPrimeCount() const;
  uint64_t getTwinCount() const;
  uint64_t getTripletCount() const;
  uint64_t getQuadrupletCount() const;
  uint64_t getQuintupletCount() const;
  uint64_t getSextupletCount() const;
  uint64_t getSeptupletCount() const;
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
  virtual bool updateStatus(uint64_t, bool);
  virtual void set_lock();
  virtual void unset_lock();
private:
  /// Private flags
  /// @pre flag >= (1 << 20)
  enum {
    CALLBACK32     = 1 << 20,
    CALLBACK64     = 1 << 21,
    CALLBACK32_OBJ = 1 << 22,
    CALLBACK64_OBJ = 1 << 23,
    CALLBACK64_INT = 1 << 24
  };
  /// Synchronizes threads
  class LockGuard {
  public:
    LockGuard(PrimeSieve& ps) : ps_(ps) { ps_.set_lock(); }
    ~LockGuard()                        { ps_.unset_lock(); }
  private:
    PrimeSieve& ps_;
    LockGuard(const LockGuard&);
    LockGuard& operator=(const LockGuard&);
  };
  struct SmallPrime
  {
    unsigned int min;
    unsigned int max;
    int index;
    std::string str;
  };
  static const SmallPrime smallPrimes_[8];
  /// Pre-sieve multiples of small primes <= preSieve_
  int preSieve_;
  /// Sieve size in kilobytes
  int sieveSize_;
  /// Flags (settings) for PrimeSieve e.g. COUNT_PRIMES, PRINT_TWINS, ...
  int flags_;
  /// ParallelPrimeSieve thread number
  int threadNum_;
  /// Pointer to the parent ParallelPrimeSieve object
  PrimeSieve* parent_;
  /// Sum of all processed segments
  uint64_t processed_;
  /// Sum of recently processed segments
  uint64_t toUpdate_;
  /// Status in percent of sieve()
  double status_;
  /// Callback function for use with generatePrimes()
  void (*callback32_)(uint32_t);
  void (*callback64_)(uint64_t);
  void (*callback32_obj_)(uint32_t, void*);
  void (*callback64_obj_)(uint64_t, void*);
  void (*callback64_int_)(uint64_t, int);
  /// Callback object
  void* obj_;
  void doSmallPrime(const SmallPrime&);
  bool isPublicFlags(int) const;
};

#endif
