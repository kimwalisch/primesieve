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

/// @file   openmp_RAII.h
/// @brief  The OmpInitGuard and OmpGuard classes are RAII-style
///         wrappers for OpenMP locks.

#ifndef OPENMP_RAII_H
#define OPENMP_RAII_H

#include <omp.h>

namespace soe {

/// OpenMP RAII lock initialization and destroy
class OmpInitGuard {
public:
  OmpInitGuard(omp_lock_t&);
  ~OmpInitGuard();
private:
  omp_lock_t* lock_;
};

/// OpenMP RAII lock guard
class OmpGuard {
public:
  OmpGuard(omp_lock_t&, bool);
  ~OmpGuard();
  bool isSet() const;
private:
  omp_lock_t* lock_;
  bool isSet_;
};

OmpInitGuard::OmpInitGuard(omp_lock_t& lock) :
  lock_(&lock)
{
  omp_init_lock(lock_);
}

OmpInitGuard::~OmpInitGuard()
{
  omp_destroy_lock(lock_);
}

OmpGuard::OmpGuard(omp_lock_t& lock, bool noWait) :
  lock_(&lock)
{
  if (noWait)
    isSet_ = (omp_test_lock(lock_) != false);
  else {
    omp_set_lock(lock_);
    isSet_ = true;
  }
}

OmpGuard::~OmpGuard()
{
  if (isSet())
    omp_unset_lock(lock_);
}

bool OmpGuard::isSet() const
{
  return isSet_;
}

} // namespace soe

#endif
