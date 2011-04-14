/*
 * PrimeSieveProcess.cpp -- This file is part of primesieve
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

#include "PrimeSieveProcess.h"

#include <QStringList>
#include <QCoreApplication>
#if defined(Q_OS_WIN)
#  include <windows.h>
#else
#  include <unistd.h>
#endif

PrimeSieveProcess::PrimeSieveProcess(QObject* parent = 0) : QProcess(parent) {
  sharedMemory_.setParent(parent);
  sharedMemory_.setKey(QString::number(this->getProcessId()));
}

PrimeSieveProcess::~PrimeSieveProcess() {
  // disconnect all signals, must be used to avoid zombie processes
  this->disconnect();
  // kill() and terminate() = trouble, close() works fine
  this->close();
  sharedMemory_.detach();
}

/**
 * Get the process ID of the current process. I tried to use
 * QProcess::pid() but got a lot of trouble on Windows and Mac OS X,
 * also it is not portable.
 */
int PrimeSieveProcess::getProcessId() {
#if defined(Q_OS_WIN)
  return static_cast<int> (GetCurrentProcessId());
#else
  return static_cast<int> (getpid());
#endif
}

/**
 * Create a shared memory segement for communication with the
 * ParallelPrimeSieve process.
 */
void PrimeSieveProcess::createSharedMemory() {
  // attach the shared memory segment
  if (!sharedMemory_.isAttached() &&
      !sharedMemory_.create(sizeof(sharedMemoryPPS_))) {
    throw std::runtime_error(
        "Interprocess communication error, could not allocate shared memory.");
  }
  // map the attached shared memory to the sharedMemoryPPS_ structure
  sharedMemoryPPS_ = static_cast<ParallelPrimeSieve::SharedMemoryPPS*>
      (sharedMemory_.data());
}

/**
 * Start a new ParallelPrimeSieve process that sieves the
 * prime numbers and/or k-tuplets between startNumber and stopNumber.
 */
void PrimeSieveProcess::start(qulonglong startNumber, qulonglong stopNumber,
    int sieveSize, int flags, int threads) {
  this->createSharedMemory();
  // initialize the shared memory
  sharedMemoryPPS_->startNumber = startNumber;
  sharedMemoryPPS_->stopNumber  = stopNumber;
  sharedMemoryPPS_->sieveSize   = sieveSize;
  sharedMemoryPPS_->flags       = flags;
  sharedMemoryPPS_->threads     = threads;
  for (int i = 0; i < COUNTS_SIZE; i++)
    sharedMemoryPPS_->counts[i] = 0;
  sharedMemoryPPS_->status      = 0.0;
  sharedMemoryPPS_->timeElapsed = 0.0;
  // path + file name of the aplication
  QString path = QCoreApplication::applicationFilePath();
  // process arguments, see main.cpp
  QStringList args;
  args << "PrimeSieveProcess" << sharedMemory_.key();
  // start a new ParallelPrimeSieve process
  /// @see main.cpp
  QProcess::start(path, args, QIODevice::ReadOnly);
}

bool PrimeSieveProcess::isFinished() {
  return (sharedMemoryPPS_->status == 100.0);
}

/**
 * @return The count of prime numbers or prime k-tuplets between
 *         startNumber and stopNumber.
 * @param  index 0 = Count of prime numbers
 *               1 = Count of twin primes
 *               2 = Count of prime triplets
 *               3 = Count of prime quadruplets
 *               4 = Count of prime quintuplets
 *               5 = Count of prime sextuplets
 *               6 = Count of prime septuplets
 */
qlonglong PrimeSieveProcess::getCounts(unsigned index) const {
  return sharedMemoryPPS_->counts[index];
}

/**
 * @return The sieving status in percent.
 */
double PrimeSieveProcess::getStatus() const {
  return sharedMemoryPPS_->status;
}

/**
 * @return The time elapsed in seconds (if sieving is finished).
 */
double PrimeSieveProcess::getTimeElapsed() const {
  return sharedMemoryPPS_->timeElapsed;
}
