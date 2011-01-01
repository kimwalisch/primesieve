/*
 * PrimeSieveProcess.h -- This file is part of primesieve
 *
 * Copyright (C) 2011 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef PRIMESIEVEPROCESS_H
#define PRIMESIEVEPROCESS_H

#include "../src/PrimeNumberFinder.h"

#include <QProcess>
#include <QSharedMemory>
#include <QtGlobal>
#include <QString>
#include <QVector>

/**
 * Process used to sieve primes, multiple PrimeSieveProcesses may
 * be used for multi-core prime sieving.
 */
class PrimeSieveProcess : public QProcess {
public:
  enum {
    COUNTS_SIZE = PrimeNumberFinder::Results::COUNTS_SIZE
  };
  PrimeSieveProcess(QObject*, int);
  ~PrimeSieveProcess();
  void start(qulonglong, qulonglong, int, int);
  bool isFinished();
  qlonglong getCounts(unsigned int) const;
  float getStatus() const;
private:
  /// Shared memory for interprocess communication.
  QSharedMemory sharedMemory_;
  /// Contains the prime count results and the status of the process.
  PrimeNumberFinder::Results* results_;
  void createSharedMemory(int);
  int getProcessId();
};

#endif // PRIMESIEVEPROCESS_H
