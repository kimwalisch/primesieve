/*
 * PrimeSieveGUI_sieve.cpp -- This file is part of primesieve
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

#include "PrimeSieveGUI.h"
#include "ui_PrimeSieveGUI.h"

#include <QMessageBox>
#include <QCoreApplication>
#include <QByteArray>
#include <QTextStream>
#include <QFile>
#include <stdexcept>

/**
 * Start sieving prime numbers.
 */
void PrimeSieveGUI::on_sieveButton_clicked() {
  // disable the Sieve button and enable the Cancel button,
  // will be inverted again if cleanUp() is called
  ui->sieveButton->setDisabled(true);
  ui->cancelButton->setEnabled(true);
  try {
    qulonglong lowerBound = 0;
    qulonglong upperBound = 0;
    // get the users lower and upper bound
    this->getBounds(&lowerBound, &upperBound);
    // get the settings
    flags_ = this->getMenuSettings() | STORE_STATUS;
    if ((flags_ & (COUNT_FLAGS | PRINT_FLAGS)) == 0)
      throw std::invalid_argument(
          "Nothing to do, no count or print options selected.");
    // reset the GUI widgets
    ui->progressBar->setValue(ui->progressBar->minimum());
    ui->textEdit->clear();
    // start advancing the progress bar, use 40 fps for a
    // smooth progress bar
    progressBarTimer_.start(25);
    // start measuring time
    time_.start();
    // use multiple processes (if appropriate) for sieving
    this->createProcesses(lowerBound, upperBound, this->getSieveSize(),
        flags_, this->getCpuCores());
  } catch (std::invalid_argument& ex) {
    // kill any running processes, free all memory
    this->cleanUp();
    QMessageBox::warning(this, APPLICATION_NAME, ex.what());
  } catch (std::exception& ex) {
    this->cleanUp();
    QMessageBox::critical(this, APPLICATION_NAME, ex.what());
  }
}

/**
 * Subdivide the entire sieve interval into smaller subintervals and
 * create a PrimeSieveProcess for each subinterval.
 * @see main.cpp
 */
void PrimeSieveGUI::createProcesses(qulonglong lowerBound,
    qulonglong upperBound, int sieveSize, int flags, int processCount) {
  /// calculate a sieve interval for each process
  qulonglong interval = (upperBound - lowerBound) / processCount;
  if (processCount > 1 && interval < 60)
    throw std::invalid_argument(
        "Use at least an interval of 60 for each process.");
  // interval must be a multiple of 30
  if (interval % 30 != 0)
    interval += 30 - (interval % 30);

  /// calculate the start and stop number of the first process
  qulonglong startNumber = lowerBound;
  qulonglong stopNumber = lowerBound + interval;
  // ceil to the next multiple of 30 value
  if (stopNumber % 30 != 0)
    stopNumber += 30 - (stopNumber % 30);
  // stop numbers must always be of type n * 30 + 1
  stopNumber += 1;

  /// create new PrimeSieveProcesses
  for (int i = 0; i < processCount; i++) {
    // correct the stop number of the last process
    if (i + 1 == processCount)
      stopNumber = upperBound;
    processes_.push_back(new PrimeSieveProcess(this, i));
    // connection to detect end of process
    connect(processes_.back(), SIGNAL(finished(int, QProcess::ExitStatus)), this,
        SLOT(processFinished(int, QProcess::ExitStatus)));
    if (flags & PRINT_FLAGS)
      connect(processes_.back(), SIGNAL(readyReadStandardOutput()), this,
          SLOT(printProcessOutput()));
    // start sieving primes
    processes_.back()->start(startNumber, stopNumber, sieveSize, flags);
    // set for the next process
    startNumber = stopNumber + 1;
    stopNumber += interval;
  }
}

/**
 * Print the standard output (prime numbers or prime k-tuplets) of
 * the sieving process to the TextEdit.
 */
void PrimeSieveGUI::printProcessOutput() {
  QByteArray buffer;
  buffer.reserve(PRINT_BUFFER_SIZE + 256);
  while (ui->cancelButton->isEnabled() && processes_.front()->canReadLine()) {
    buffer.clear();
    while (processes_.front()->canReadLine() && buffer.size() < PRINT_BUFFER_SIZE)
      buffer.append(processes_.front()->readLine(256));
    // remove "\r\n" or '\n', '\r' at the back
    while (buffer.endsWith('\n') ||
           buffer.endsWith('\r'))
      buffer.chop(1);
    if (!buffer.isEmpty())
      ui->textEdit->appendPlainText(buffer);
/// Keep the GUI responsive.
/// @warning QApplication::processEvents() must not be used on
///          operating systems that use signal recursion (like Linux
///          X11) otherwise the stack will explode!
#if defined(Q_OS_WIN) || defined(Q_OS_MAC) 
    QApplication::processEvents();
#else
    ui->textEdit->repaint();
#endif
  }
}

/**
 * Is executed each time a PrimeSieveProcess finishes. Checks for
 * process errors and calls this->printResults() once all
 * processes have finished sieving.
 */
void PrimeSieveGUI::processFinished(int exitCode,
    QProcess::ExitStatus exitStatus) {
  // the process did not exit normally, i.e threw and exception,
  // exit(EXIT_FAILURE) ...
  if (exitCode != 0) {
    // Qt uses '/' internally, even for Windows
    QString path = QCoreApplication::applicationDirPath() + "/"
        + APPLICATION_NAME + "_error.txt";
    QFile error_log(path);
    if (error_log.open(QIODevice::WriteOnly | QIODevice::Append
        | QIODevice::Text)) {
      // TODO I do not know which process crashed, thus I have to
      // read stderr of all processes (find a better solution,
      // setStandardErrorFile() is not an option)
      QTextStream out(&error_log);
      for (int i = 0; i < processes_.size(); i++)
        out << processes_[i]->readAllStandardError();
      error_log.close();
    }
    this->cleanUp();
    QMessageBox::critical(
        this,
        APPLICATION_NAME,
        "One of the processes reported an error, sieving has been aborted. Please contact the developer.");
  }
  // the process has been interrupted by a signal (SIGTERM,
  // SIGKILL, ...) or a segmentation fault
  else if (exitStatus == QProcess::CrashExit) {
    this->cleanUp();
    QMessageBox::critical(this, APPLICATION_NAME,
        "One of the processes crashed, sieving has been aborted.");
  }
  // all processes have finished sieving without errors
  else if (++finishedProcesses_ == processes_.size()) {
    // set to 100 percent
    ui->progressBar->setValue(ui->progressBar->maximum());
    // print results if not canceled lately
    if (ui->cancelButton->isEnabled())
      this->printResults();
    this->cleanUp();
  }
}
