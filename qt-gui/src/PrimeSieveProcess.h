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

#include "../soe/ParallelPrimeSieve.h"

#include <QProcess>
#include <QSharedMemory>
#include <QtGlobal>
#include <QString>
#include <QVector>

/**
 * QProcess object used for prime sieving. Using a separate process
 * for prime sieving allows to easily cancel a multi-threaded
 * ParallelPrimeSieve instance.
 */
class PrimeSieveProcess : public QProcess {
public:
  enum {
    COUNTS_SIZE = ParallelPrimeSieve::COUNTS_SIZE
  };
  PrimeSieveProcess(QObject*);
  ~PrimeSieveProcess();
  void start(qulonglong, qulonglong, int, int, int);
  bool isFinished();
  qlonglong getCounts(unsigned int) const;
  double getStatus() const;
  double getTimeElapsed() const;
private:
  /// Shared memory for interprocess communication between the
  /// Qt GUI process and the ParallelPrimeSieve process.
  QSharedMemory sharedMemory_;
  /// Contains the settings (startNumber, stopNumber, sieveSize, ...)
  /// for sieving, will be mapped to sharedMemory_
  ParallelPrimeSieve::SharedMemory* shm_;
  void createSharedMemory();
  int getProcessId();
};

#endif // PRIMESIEVEPROCESS_H
