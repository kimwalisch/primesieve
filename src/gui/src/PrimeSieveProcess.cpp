/*
 * PrimeSieveProcess.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2012 Kim Walisch, <kim.walisch@gmail.com>
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

#include "PrimeSieveProcess.hpp"
#include <primesieve/ParallelPrimeSieve.hpp>

#include <QtGlobal>
#include <QStringList>
#include <QCoreApplication>
#include <stdexcept>

#if defined(Q_OS_WIN)
  #include <windows.h>
#else
  #include <unistd.h>
#endif

PrimeSieveProcess::PrimeSieveProcess(QObject* parent) :
  QProcess(parent) {
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
  return static_cast<int>(GetCurrentProcessId());
#else
  return static_cast<int>(getpid());
#endif
}

/**
 * Create a shared memory segement for communication with the
 * ParallelPrimeSieve process.
 */
void PrimeSieveProcess::createSharedMemory() {
  // attach the shared memory
  if (!sharedMemory_.isAttached() &&
      !sharedMemory_.create(sizeof(shm_))) {
    throw std::runtime_error(
        "Interprocess communication error, could not allocate shared memory.");
  }
  // map the attached shared memory to the shm_ segment
  shm_ = static_cast<primesieve::ParallelPrimeSieve::SharedMemory*>(sharedMemory_.data());
}

/**
 * Start a new ParallelPrimeSieve process that sieves
 * the primes within [start, stop].
 */
void PrimeSieveProcess::start(quint64 start, quint64 stop,
    int sieveSize, int flags, int threads) {
  this->createSharedMemory();
  // initialize the shared memory segment
  shm_->start = start;
  shm_->stop = stop;
  shm_->sieveSize = sieveSize;
  shm_->flags = flags;
  shm_->threads = threads;
  shm_->status = 0.0;
  shm_->seconds = 0.0;
  for (int i = 0; i < 6; i++)
    shm_->counts[i] = 0;
  // path + file name of the aplication
  QString path = QCoreApplication::applicationFilePath();
  // process arguments, see main.cpp
  QStringList args;
  args << "PrimeSieveProcess" << sharedMemory_.key();
  /// start a new ParallelPrimeSieve process
  /// @see main.cpp
  QProcess::start(path, args, QIODevice::ReadOnly);
}

bool PrimeSieveProcess::isFinished() {
  return (static_cast<int>(shm_->status) == 100);
}

/**
 * @return The count of primes/k-tuplets within [start, stop].
 * @pre index < 6
 */
quint64 PrimeSieveProcess::getCount(unsigned int index) const {
  return shm_->counts[index];
}

/**
 * @return The sieving status in percent.
 */
double PrimeSieveProcess::getStatus() const {
  return shm_->status;
}

/**
 * @return The time elapsed in seconds (if sieving is finished).
 */
double PrimeSieveProcess::getSeconds() const {
  return shm_->seconds;
}
