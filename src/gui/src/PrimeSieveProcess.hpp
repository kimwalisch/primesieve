/*
 * PrimeSieveProcess.hpp -- This file is part of primesieve
 *
 * Copyright (C) 2019 Kim Walisch, <kim.walisch@gmail.com>
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

#ifndef PRIMESIEVEPROCESS_HPP
#define PRIMESIEVEPROCESS_HPP

#include <primesieve/ParallelSieve.hpp>

#include <QProcess>
#include <QSharedMemory>
#include <QtGlobal>
#include <QString>
#include <QVector>

/**
 * QProcess class used for prime sieving, using a separate process
 * for sieving allows to easily cancel a multi-threaded
 * ParallelSieve instance.
 */
class PrimeSieveProcess : public QProcess {
public:
  PrimeSieveProcess(QObject*);
  ~PrimeSieveProcess();
  void start(quint64, quint64, int, int, int);
  bool isFinished();
  quint64 getCount(unsigned int) const;
  double getPercent() const;
  double getSeconds() const;
private:
  /// Shared memory for interprocess communication between the
  /// Qt GUI process and the ParallelSieve process.
  QSharedMemory sharedMemory_;
  /// Contains the settings (start, stop, sieveSize, ...)
  /// for sieving, will be mapped to sharedMemory_
  primesieve::SharedMemory* shm_ = nullptr;
  void createSharedMemory();
  int getProcessId();
};

#endif // PRIMESIEVEPROCESS_H
