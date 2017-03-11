/*
 * PrimeSieveGUI.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
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

#include "PrimeSieveGUI.hpp"
#include "ui_PrimeSieveGUI.h"
#include "PrimeSieveProcess.hpp"
#include "calculator.hpp"

#include <primesieve.hpp>
#include <primesieve/ParallelPrimeSieve.hpp>
#include <primesieve/pmath.hpp>

#if QT_VERSION >= 0x050000
  #include <QtGlobal>
  #include <QCoreApplication>
  #include <QByteArray>
  #include <QTextStream>
  #include <QFile>
  #include <QSize>
  #include <QtWidgets/QMessageBox>
  #include <QTextCursor>
  #include <stdexcept>
#else
  #include <QtGlobal>
  #include <QCoreApplication>
  #include <QByteArray>
  #include <QTextStream>
  #include <QFile>
  #include <QSize>
  #include <QMessageBox>
  #include <QTextCursor>
  #include <stdexcept>
#endif

int get_l1d_cache_size();

using namespace primesieve;

PrimeSieveGUI::PrimeSieveGUI(QWidget *parent) :
  QMainWindow(parent), ui(new Ui::PrimeSieveGUI), validator_(0),
  primeSieveProcess_(0), saveAct_(0), quitAct_(0), aboutAct_(0),
  alignmentGroup_(0) {
  ui->setupUi(this);
  primeText_.push_back("Prime numbers");
  primeText_.push_back("Twin primes");
  primeText_.push_back("Prime triplets");
  primeText_.push_back("Prime quadruplets");
  primeText_.push_back("Prime quintuplets");
  primeText_.push_back("Prime sextuplets");
  this->initGUI();
  this->initConnections();
}

PrimeSieveGUI::~PrimeSieveGUI() {
  this->cleanUp();
  delete validator_;
  delete saveAct_;
  delete quitAct_;
  delete aboutAct_;
  delete alignmentGroup_;
  for (; !countAct_.isEmpty(); countAct_.pop_back())
    delete countAct_.back();
  for (; !printAct_.isEmpty(); printAct_.pop_back())
    delete printAct_.back();
  // Qt code
  delete ui;
}

void PrimeSieveGUI::changeEvent(QEvent *e) {
  QMainWindow::changeEvent(e);
  switch (e->type()) {
  case QEvent::LanguageChange:
    ui->retranslateUi(this);
    break;
  default:
    break;
  }
}

void PrimeSieveGUI::initGUI() {
  this->setWindowTitle(APPLICATION_NAME + " " + PRIMESIEVE_VERSION);
  this->createMenu(primeText_);

  // fill the sieveSizeComboBox with power of 2 values <= "2048 KB"
  for (int i = MINIMUM_SIEVE_SIZE; i <= MAXIMUM_SIEVE_SIZE; i *= 2)
    ui->sieveSizeComboBox->addItem(QString::number(i) + " KB");

  int l1dCacheSize = get_l1d_cache_size();
  if (l1dCacheSize < 16 || l1dCacheSize > 1024)
    l1dCacheSize = DEFAULT_L1D_CACHE_SIZE;

  int defaultSieveSize = floorPowerOf2(l1dCacheSize);

  // default sieve size = CPU L1 data cache size
  this->setTo(ui->sieveSizeComboBox, QString::number(defaultSieveSize) + " KB");

  // fill the threadsComboBox with power of 2 values <= maxThreads_
  maxThreads_ = ParallelPrimeSieve::getMaxThreads();
  for (int i = 1; i < maxThreads_; i *= 2)
    ui->threadsComboBox->addItem(QString::number(i));
  ui->threadsComboBox->addItem(QString::number(maxThreads_));
  this->setTo(ui->threadsComboBox, "1");

  // set an ideal ComboBox width
  int width = ui->sieveSizeComboBox->minimumSizeHint().width();
  ui->sieveSizeComboBox->setFixedWidth(width);
  ui->threadsComboBox->setFixedWidth(width);

  // set a nice GUI size
  QSize size = this->sizeHint();
  size.setWidth(this->minimumSizeHint().width());
#if defined(Q_OS_WIN)
  size.setHeight(size.height() - size.height() / 10);
#endif
  this->resize(size);

  // limit input for arithmetic expressions
  QRegExp rx("[0-9\\+\\-\\*\\/\\%\\^\\(\\)\\e\\E]*");
  validator_ = new QRegExpValidator(rx, this);
  ui->lowerBoundLineEdit->setValidator(validator_);
  ui->upperBoundLineEdit->setValidator(validator_);
}

void PrimeSieveGUI::initConnections() {
  connect(&progressBarTimer_,     SIGNAL(timeout()),                    this, SLOT(advanceProgressBar()));
  connect(ui->lowerBoundLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(autoSetThreads()));
  connect(ui->upperBoundLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(autoSetThreads()));
  connect(ui->autoSetCheckBox,    SIGNAL(toggled(bool)),                this, SLOT(autoSetThreads()));
  connect(saveAct_,               SIGNAL(triggered()),                  this, SLOT(saveToFile()));
  connect(quitAct_,               SIGNAL(triggered()),                  qApp, SLOT(closeAllWindows()));
  connect(alignmentGroup_,        SIGNAL(triggered(QAction*)),          this, SLOT(printMenuClicked(QAction*)));
  connect(aboutAct_,              SIGNAL(triggered()),                  this, SLOT(showAboutDialog()));
}

/**
 * Get the sieve size in kilobytes from the sieveSizeComboBox.
 * @post sieveSize >= 1 && sieveSize <= 2048.
 */
int PrimeSieveGUI::getSieveSize() {
  QString sieveSize(ui->sieveSizeComboBox->currentText());
  // remove " KB"
  sieveSize.chop(3);
  return sieveSize.toInt();
}

/**
 * Get the number of threads from the threadsComboBox.
 */
int PrimeSieveGUI::getThreads() {
  return ui->threadsComboBox->currentText().toInt();
}

quint64 PrimeSieveGUI::getNumber(const QString& str) {
  if (str.isEmpty())
    throw std::invalid_argument("Please enter a lower and upper bound for prime sieving.");

  quint64 result = 0;
  try {
    result = calculator::eval<quint64>(str.toLatin1().data());
  }
  catch (calculator::error& e) {
    throw std::invalid_argument(e.what());
  }

  int digits = str.count(QRegExp("[0-9]"));
  if (digits == str.size() && (
      digits >  UPPER_BOUND_STR.size() || (
      digits == UPPER_BOUND_STR.size() &&
      str    >  UPPER_BOUND_STR)))
    throw std::invalid_argument("primesieve is limited to primes < 2^64.");

  return result;
}

void PrimeSieveGUI::setTo(QComboBox* comboBox, const QString& text) {
  comboBox->setCurrentIndex(comboBox->findText(text));
}

/**
 * If "Auto set" is enabled set an ideal number of threads for the
 * current lower bound, upper bound in the threadsComboBox.
 */
void PrimeSieveGUI::autoSetThreads() {
  if (ui->autoSetCheckBox->isEnabled() && ui->autoSetCheckBox->isChecked()) {
    try {
      quint64 lowerBound = this->getNumber(ui->lowerBoundLineEdit->text());
      quint64 upperBound = this->getNumber(ui->upperBoundLineEdit->text());
      ParallelPrimeSieve pps;
      pps.setStart(lowerBound);
      pps.setStop(upperBound);
      int idealNumThreads = pps.idealNumThreads();
      if (idealNumThreads < maxThreads_) {
        // floor to the next power of 2 value
        int p = 1;
        for (; p <= idealNumThreads; p *= 2)
          ;
        idealNumThreads = p / 2;
      }
      this->setTo(ui->threadsComboBox, QString::number(idealNumThreads));
    } catch (...) {
      this->setTo(ui->threadsComboBox, "1");
    }
  }
}

/**
 * The user has chosen a custom number of threads, disable "Auto set".
 */
void PrimeSieveGUI::on_threadsComboBox_activated() {
  ui->autoSetCheckBox->setChecked(false);
}

/**
 * Start sieving primes.
 */
void PrimeSieveGUI::on_sieveButton_clicked() {
  // invert buttons, reset upon cleanUp()
  ui->sieveButton->setDisabled(true);
  ui->cancelButton->setEnabled(true);
  try {
    flags_ = this->getMenuSettings() | CALCULATE_STATUS;
    if ((flags_ & (COUNT_FLAGS | PRINT_FLAGS)) == 0)
      throw std::invalid_argument("Nothing to do, no count or print options selected.");

    quint64 lowerBound = this->getNumber(ui->lowerBoundLineEdit->text());
    quint64 upperBound = this->getNumber(ui->upperBoundLineEdit->text());
    if (lowerBound > upperBound)
      throw std::invalid_argument("The lower bound must not be greater than the upper bound.");

    // reset the GUI widgets
    ui->progressBar->setValue(ui->progressBar->minimum());
    ui->textEdit->clear();
    progressBarTimer_.start(25);

    // start a new process for sieving (avoids cancel
    // trouble with multiple threads)
    primeSieveProcess_ = new PrimeSieveProcess(this);
    if (flags_ & PRINT_FLAGS)
      connect(primeSieveProcess_, SIGNAL(readyReadStandardOutput()),
          this, SLOT(printProcessOutput()));
    connect(primeSieveProcess_, SIGNAL(finished(int, QProcess::ExitStatus)),
        this, SLOT(processFinished(int, QProcess::ExitStatus)));
    primeSieveProcess_->start(lowerBound, upperBound, this->getSieveSize(),
        flags_, this->getThreads());

  } catch (std::invalid_argument& ex) {
    this->cleanUp();
    QMessageBox::warning(this, APPLICATION_NAME, ex.what());
  } catch (std::exception& ex) {
    this->cleanUp();
    QMessageBox::critical(this, APPLICATION_NAME, ex.what());
  }
}

void PrimeSieveGUI::advanceProgressBar() {
  int permil = static_cast<int>(primeSieveProcess_->getStatus() * 10.0);
  ui->progressBar->setValue(permil);
}

/**
 * Redirects the standard output (prime numbers or prime k-tuplets) of
 * the primeSieveProcess_ to the TextEdit.
 */
void PrimeSieveGUI::printProcessOutput() {
  QByteArray buffer;
  buffer.reserve(PRINT_BUFFER_SIZE + 256);
  while (ui->cancelButton->isEnabled() && primeSieveProcess_->canReadLine()) {
    buffer.clear();
    while (primeSieveProcess_->canReadLine() && buffer.size() < PRINT_BUFFER_SIZE)
      buffer.append(primeSieveProcess_->readLine(256));
    // remove "\r\n" or '\n', '\r' at the back
    while (buffer.endsWith('\n') ||
           buffer.endsWith('\r'))
      buffer.chop(1);
    if (!buffer.isEmpty())
      ui->textEdit->appendPlainText(buffer);
/// @brief   Keep the GUI responsive.
/// @bug     processEvents() crashes on Windows with MSVC 2010 and Qt 5 beta.
/// @warning QApplication::processEvents() must not be used on
///          operating systems that use signal recursion (like Linux
///          X11) otherwise the stack will explode!
#if defined(Q_OS_WIN) || defined(Q_OS_MAC) || defined(Q_OS_IOS)
    QApplication::processEvents();
#else
    ui->textEdit->repaint();
#endif
  }
}

/**
 * Is executed when the primeSieveProcess_ finishes, checks for
 * process errors and calls this->printResults().
 */
void PrimeSieveGUI::processFinished(int exitCode,
    QProcess::ExitStatus exitStatus) {
  // the process did not exit normally, i.e. threw and exception
  if (exitCode != 0) {
    // Qt uses '/' internally, even for Windows
    QString path = QCoreApplication::applicationDirPath() + "/" + APPLICATION_NAME + "_error.txt";
    QFile error_log(path);
    if (error_log.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
      QTextStream out(&error_log);
      out << primeSieveProcess_->readAllStandardError();
      error_log.close();
    }
    this->cleanUp();
    QMessageBox::critical(this, APPLICATION_NAME,
        "The PrimeSieveProcess reported an error (see primesieve_error.txt), sieving has been aborted.");
  }
  // the PrimeSieveProcess has been interrupted by a signal (SIGTERM,
  // SIGKILL, ...) or a segmentation fault
  else if (exitStatus == QProcess::CrashExit) {
    this->cleanUp();
    QMessageBox::critical(this, APPLICATION_NAME, "The PrimeSieveProcess crashed, sieving has been aborted.");
  }
  // the PrimeSieveProcess has finished correctly
  else {
    ui->progressBar->setValue(ui->progressBar->maximum());
    // print results if not canceled lately
    if (ui->cancelButton->isEnabled())
      this->printResults();
    this->cleanUp();
  }
}

/**
 * Print the sieving results.
 */
void PrimeSieveGUI::printResults() {
  if (!ui->textEdit->toPlainText().isEmpty())
    ui->textEdit->appendPlainText("");

  // hack to get the count results aligned using tabs
  QString maxSizeText;
  for (int i = 0; i < primeText_.size(); i++) {
    if ((flags_ & (COUNT_PRIMES << i)) && maxSizeText.size() < primeText_[i].size())
      maxSizeText = primeText_[i];
  }
  ui->textEdit->insertPlainText(maxSizeText + ": ");
  int maxWidth = ui->textEdit->cursorRect().left();
  ui->textEdit->moveCursor(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
  ui->textEdit->textCursor().removeSelectedText();
  ui->textEdit->setTabStopWidth(maxWidth);

  // print prime counts & time elapsed
  for (int i = 0; i < primeText_.size(); i++) {
    if (flags_ & (COUNT_PRIMES << i))
      ui->textEdit->appendPlainText(primeText_[i] + ":\t" + QString::number(primeSieveProcess_->getCount(i)));
  }
  if (flags_ & COUNT_KTUPLETS)
    ui->textEdit->appendPlainText("");
  QString time("Elapsed time:\t" + QString::number(primeSieveProcess_->getSeconds(), 'f', 2) + " sec");
  ui->textEdit->appendPlainText(time);
}

/**
 * Cancel sieving.
 */
void PrimeSieveGUI::on_cancelButton_clicked() {
  ui->cancelButton->setDisabled(true);
  ui->progressBar->setValue(0);
  // too late to abort
  if ((flags_ & PRINT_FLAGS) && primeSieveProcess_->isFinished())
    return;
  this->cleanUp();
}

/**
 * Clean up after sieving is finished or canceled (abort the
 * PrimeSieveProcess if still running).
 */
void PrimeSieveGUI::cleanUp() {
  progressBarTimer_.stop();
  if (primeSieveProcess_ != 0)
    delete primeSieveProcess_;
  primeSieveProcess_ = 0;
  // invert buttons
  ui->cancelButton->setDisabled(true);
  ui->sieveButton->setEnabled(true);
  // force repainting widgets
  this->repaint();
}
