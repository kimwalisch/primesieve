/*
 * PrimeSieveProcess.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2010 Kim Walisch, <kim.walisch@gmail.com>
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

#include "PrimeSieveProcess.h"

#include <QStringList>
#include <QCoreApplication>

#if defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
#  include <windows.h>
#else
#  include <unistd.h>
#endif

PrimeSieveProcess::PrimeSieveProcess(QObject* parent = 0, int sharedMemoryId = 0) :
    QProcess(parent) {
  int processId = this->getProcessId();
  sharedMemory_.setParent(parent);
  sharedMemory_.setKey(QString::number(processId) +
                       QString::number(sharedMemoryId));
}

PrimeSieveProcess::~PrimeSieveProcess() {
  sharedMemory_.detach();
}

/**
 * Get the process ID of the current process. I tried to use
 * QProcess::pid() but got a lot of trouble on Windows and Mac OS X,
 * also it is not portable.
 */
int PrimeSieveProcess::getProcessId() {
#if defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
  return static_cast<int> (GetCurrentProcessId());
#else
  return static_cast<int> (getpid());
#endif
}

/**
 * Create a shared memory segement to which the process writes
 * its count results and its status.
 */
void PrimeSieveProcess::createSharedMemory(int flags) {
  // attach the shared memory segment
  if (!sharedMemory_.isAttached() && !sharedMemory_.create(sizeof(results_))) {
    throw std::runtime_error(
        "Interprocess communication error, could not allocate shared memory.");
  }
  // map the attached shared memory to the results_ structure
  results_ = static_cast<PrimeNumberFinder::Results*> (sharedMemory_.data());
  // initialize the count and status variables
  results_->reset(flags);
}

/**
 * Start a new process that sieves the prime numbers and k-tuplets
 * between startNumber and stopNumber.
 */
void PrimeSieveProcess::start(qulonglong startNumber, qulonglong stopNumber,
    int sieveSize, int flags) {
  this->createSharedMemory(flags);
  // path + file name of the aplication
  QString path = QCoreApplication::applicationFilePath();
  // build the arguments for primesieve
  QStringList args;
  args << QString::number(startNumber)
       << QString::number(stopNumber)
       << QString::number(sieveSize)
       << QString::number(flags)
       << sharedMemory_.key();
  // start a new process (for prime sieving)
  QProcess::start(path, args, QIODevice::ReadOnly);
}

/**
 * @return The count of prime numbers or prime k-tuplets between
 *         startNumber and stopNumber or -1 if the appropriate
 *         count flag is not set.
 *
 * @param  index 0 = Count of prime numbers
 *               1 = Count of twin primes
 *               2 = Count of prime triplets
 *               3 = Count of prime quadruplets
 *               4 = Count of prime quintuplets
 *               5 = Count of prime sextuplets
 *               6 = Count of prime septuplets
 */
qlonglong PrimeSieveProcess::getCounts(unsigned index) const {
  return results_->counts[index];
}

/**
 * @return The sieving status in percents.
 */
float PrimeSieveProcess::getStatus() const {
  return results_->status;
}
