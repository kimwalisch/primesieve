/*
 * ParallelPrimeSieve.h -- This file is part of primesieve
 *
 * Copyright (C) 2011 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef PARALLELPRIMESIEVE_H
#define PARALLELPRIMESIEVE_H

#include "PrimeSieve.h"

/**
 * ParallelPrimeSieve uses multiple PrimeSieve objects and threads
 * (OpenMP) for parallel prime sieveing.
 * @see PrimeSieve.cpp
 */
class ParallelPrimeSieve: public PrimeSieve {
public:
  /**
   * Used in the Qt GUI version of primesieve to handle the
   * communication between the GUI process and the ParallelPrimeSieve
   * process.
   */
  struct SharedMemoryPPS {
    uint64_t startNumber;
    uint64_t stopNumber;
    uint32_t sieveSize;
    uint32_t flags;
    int threads;
    uint64_t counts[COUNTS_SIZE];
    double status;
    double timeElapsed;
  };
  ParallelPrimeSieve() : sharedMemoryPPS_(NULL) {};
  void setSharedMemory(SharedMemoryPPS*);
  static int getMaxThreads();
  int getIdealThreadCount() const;
  void sieve();
  void sieve(int);
protected:
  void doStatus(uint64_t);
private:
  SharedMemoryPPS *sharedMemoryPPS_;
  void validate(int);
};

#endif // PARALLELPRIMESIEVE_H
