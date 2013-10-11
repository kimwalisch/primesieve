///
/// @file  PrimeFinder.h
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMEFINDER_H
#define PRIMEFINDER_H

#include "config.h"
#include "SieveOfEratosthenes.h"
#include "PrimeSieveCallback.h"

#include <stdint.h>
#include <vector>

class PrimeSieve;

namespace soe {

/// PrimeFinder is a SieveOfEratosthenes class that is used to
/// callback, print and count primes and prime k-tuplets
/// (twin primes, prime triplets, ...).
///
class PrimeFinder : public SieveOfEratosthenes {
public:
  PrimeFinder(PrimeSieve&);
private:
  enum { END = 0xff + 1 };
  static const uint_t kBitmasks_[7][5];
  /// Count lookup tables for prime k-tuplets
  std::vector<uint_t> kCounts_[7];
  /// Reference to the associated PrimeSieve object
  PrimeSieve& ps_;
  /// Reference ps_.counts_
  std::vector<uint64_t>& counts_;
  /// Copy ps_.threadNum_
  int threadNum_;
  /// Copy of ps_ callback functions and objects
  void (*callback_)(uint64_t);
  void (*callback_tn_)(uint64_t, int);
  PrimeSieveCallback<uint64_t>* psc_;
  PrimeSieveCallback<uint64_t, int>* psc_tn_;
  void init_kCounts();
  virtual void segmentFinished(const byte_t*, uint_t);
  void count(const byte_t*, uint_t);
  void print(const byte_t*, uint_t) const;
  void callback(const byte_t*, uint_t) const;
  void callback_tn(uint64_t) const;
  void callback_obj_tn(uint64_t) const;
  static void printPrime(uint64_t);
  DISALLOW_COPY_AND_ASSIGN(PrimeFinder);
};

} // namespace soe

#endif
