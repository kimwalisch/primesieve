/*
 * PrimeSieveGUI.cpp -- This file is part of primesieve
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
#include "../expr/ExpressionParser.h"

#include <QCoreApplication>
#include <QByteArray>
#include <QTextStream>
#include <QFile>
#include <QSize>
#include <QMessageBox>
#include <QTextCursor>
#include <stdexcept>

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
  primeText_.push_back("Prime septuplets");
  this->initGUI();
  this->initConnections();
}

PrimeSieveGUI::~PrimeSieveGUI() {
  // kill all processes
  this->cleanUp();
  // free all allocated memory
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

void PrimeSieveGUI::setComboBoxText(QComboBox* comboBox, QString text) {
  int index = comboBox->findText(text);
  if (index < 0)
    QMessageBox::critical(this, APPLICATION_NAME,
        "Internal ComboBox error, please contact the developer.");
  comboBox->setCurrentIndex(index);
}

void PrimeSieveGUI::initGUI() {
  this->setWindowTitle(APPLICATION_NAME + " " + APPLICATION_VERSION);
  this->createMenu(primeText_);

  // fill the sieveSizeComboBox with power of 2 values from
  // "16 KB" till "8192 KB"
  for (int i = MINIMUM_SIEVE_SIZE; i <= MAXIMUM_SIEVE_SIZE; i *= 2)
    ui->sieveSizeComboBox->addItem(QString::number(i) + " KB");
  QString defaultSieveSize = QString::number(DEFAULT_SIEVE_SIZE) + " KB";
  this->setComboBoxText(ui->sieveSizeComboBox, defaultSieveSize);

  // fill the threadsComboBox with power of 2 values from 1 till
  // maxThreads (number of logical CPU cores)
  int maxThreads = ParallelPrimeSieve::getMaxThreads();
  for (int i = 1; i < maxThreads; i *= 2)
    ui->threadsComboBox->addItem(QString::number(i));
  ui->threadsComboBox->addItem(QString::number(maxThreads));
  QString defaultThreads("1");
  this->setComboBoxText(ui->threadsComboBox, defaultThreads);

  // set an ideal ComboBox width
  int width = ui->sieveSizeComboBox->minimumSizeHint().width();
  ui->sieveSizeComboBox->setFixedWidth(width);
  ui->threadsComboBox->setFixedWidth(width);

  // set a nice GUI size
  QSize size = this->sizeHint();
  size.setWidth(this->minimumSizeHint().width());
#if defined(Q_OS_WIN)
  size.setHeight(size.height() - size.height() / 20);
#endif
  this->resize(size);

  // limit input for integer arithmetic expressions
  QRegExp rx("[0-9\\+\\-\\*\\/\\%\\^\\(\\)\\e\\E]*");
  validator_ = new QRegExpValidator(rx, this);
  ui->lowerBoundLineEdit->setValidator(validator_);
  ui->upperBoundLineEdit->setValidator(validator_);
}

void PrimeSieveGUI::initConnections() {
  // progress bar connection
  connect(&progressBarTimer_, SIGNAL(timeout()), this, SLOT(
      advanceProgressBar()));
  // autoSetThreads() connections
  connect(ui->lowerBoundLineEdit, SIGNAL(textChanged(const QString &)),
      this, SLOT(autoSetThreads()));
  connect(ui->upperBoundLineEdit, SIGNAL(textChanged(const QString &)),
      this, SLOT(autoSetThreads()));
  connect(ui->autoSetCheckBox, SIGNAL(toggled(bool)),
      this, SLOT(autoSetThreads()));
  // file menu connections
  connect(saveAct_, SIGNAL(triggered()), this, SLOT(saveToFile()));
  connect(quitAct_, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
  // print menu connection
  connect(alignmentGroup_, SIGNAL(triggered(QAction*)), this,
      SLOT(printMenuClicked(QAction*)));
  // about dialog connection
  connect(aboutAct_, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
}

/**
 * Get the sieve size in KiloBytes from the sieveSizeComboBox.
 * @post sieveSize >= 1 && sieveSize <= 8192.
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

/**
 * The user has chosen a custom number of threads, disable "Auto set".
 */
void PrimeSieveGUI::on_threadsComboBox_activated() {
  ui->autoSetCheckBox->setChecked(false);
}

/**
 * If "Auto set" is enabled set the ideal number of threads for the
 * current lower bound, upper bound and menu settings in the
 * threadsComboBox.
 */
void PrimeSieveGUI::autoSetThreads() {
  if (ui->autoSetCheckBox->isEnabled() && 
      ui->autoSetCheckBox->isChecked()) {
    QString threads("1");
    try {
      qulonglong lowerBound = 0;
      qulonglong upperBound = 0;
      this->getBounds(&lowerBound, &upperBound, false);
      ParallelPrimeSieve pps;
      pps.setStartNumber(lowerBound);
      pps.setStopNumber(upperBound);
      pps.setFlags(this->getMenuSettings());
      threads.setNum(pps.getNumThreads());
    } catch (...) { }
    this->setComboBoxText(ui->threadsComboBox, threads);
  }
}

/**
 * Get the users lower and upper bound for prime sieving.
 */
void PrimeSieveGUI::getBounds(qulonglong* lowerBound, qulonglong* upperBound,
    bool solveArithmeticExpression) {
  if (ui->lowerBoundLineEdit->text().isEmpty() ||
      ui->upperBoundLineEdit->text().isEmpty())
    throw std::invalid_argument("Missing number input.");

  QByteArray text = ui->lowerBoundLineEdit->text().toAscii();
  ExpressionParser<qulonglong> expr;
  if (!expr.eval(text.data()))
    throw std::invalid_argument(expr.getErrorMessage());
  *lowerBound = expr.getResult();
  if (solveArithmeticExpression)
    ui->lowerBoundLineEdit->setText(QString::number(expr.getResult()));
  text = ui->upperBoundLineEdit->text().toAscii();
  if (!expr.eval(text.data()))
    throw std::invalid_argument(expr.getErrorMessage());
  *upperBound = expr.getResult();
  if (solveArithmeticExpression)
    ui->upperBoundLineEdit->setText(QString::number(expr.getResult()));

  if (*lowerBound >= UPPER_BOUND_LIMIT ||
      *upperBound >= UPPER_BOUND_LIMIT)
    throw std::invalid_argument(
        "Please use numbers >= 0 and < (2^64-1) - (2^32-1) * 10.");
  if (*lowerBound > *upperBound)
    throw std::invalid_argument(
        "The lower bound must not be greater than the upper bound.");
}

/**
 * Start sieving prime numbers.
 */
void PrimeSieveGUI::on_sieveButton_clicked() {
  // invert buttons, reset upon cleanUp()
  ui->sieveButton->setDisabled(true);
  ui->cancelButton->setEnabled(true);

  try {
    flags_ = this->getMenuSettings();
    if ((flags_ & (ParallelPrimeSieve::COUNT_FLAGS | ParallelPrimeSieve::PRINT_FLAGS)) == 0)
      throw std::invalid_argument(
          "Nothing to do, no count or print options selected.");
    qulonglong lowerBound = 0;
    qulonglong upperBound = 0;
    this->getBounds(&lowerBound, &upperBound, true);
    int threads = this->getThreads();
    if (threads > 1 && (upperBound - lowerBound) / threads < 60)
      throw std::invalid_argument(
          "Use at least an interval of 60 for each thread");

    // reset the GUI widgets
    ui->progressBar->setValue(ui->progressBar->minimum());
    ui->textEdit->clear();
    progressBarTimer_.start(25);

    // start a separate process for sieving,
    // this allows to easily cancel the sieving process
    primeSieveProcess_ = new PrimeSieveProcess(this);
    connect(primeSieveProcess_, SIGNAL(finished(int, QProcess::ExitStatus)),
        this, SLOT(processFinished(int, QProcess::ExitStatus)));
    if (flags_ & ParallelPrimeSieve::PRINT_FLAGS)
      connect(primeSieveProcess_, SIGNAL(readyReadStandardOutput()), this,
          SLOT(printProcessOutput()));
    primeSieveProcess_->start(lowerBound, upperBound, this->getSieveSize(),
        flags_, threads);

  } catch (std::invalid_argument& ex) {
    // kill any running processes, free all memory
    this->cleanUp();
    QMessageBox::warning(this, APPLICATION_NAME, ex.what());
  } catch (std::exception& ex) {
    this->cleanUp();
    QMessageBox::critical(this, APPLICATION_NAME, ex.what());
  }
}

void PrimeSieveGUI::advanceProgressBar() {
  int permil = static_cast<int> (primeSieveProcess_->getStatus() * 10.0);
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
 * Is executed when the primeSieveProcess_ finishes, checks for
 * process errors and calls this->printResults().
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
      QTextStream out(&error_log);
      out << primeSieveProcess_->readAllStandardError();
      error_log.close();
    }
    this->cleanUp();
    QMessageBox::critical(
        this,
        APPLICATION_NAME,
        "The PrimeSieveProcess reported an error, sieving has been aborted. Please contact the developer.");
  }
  // the process has been interrupted by a signal (SIGTERM,
  // SIGKILL, ...) or a segmentation fault
  else if (exitStatus == QProcess::CrashExit) {
    this->cleanUp();
    QMessageBox::critical(this, APPLICATION_NAME,
        "The PrimeSieveProcess crashed, sieving has been aborted.");
  }
  // the PrimeSieveProcess has finished correctly
  ui->progressBar->setValue(ui->progressBar->maximum());
  // print results if not canceled lately
  if (ui->cancelButton->isEnabled())
    this->printResults();
  this->cleanUp();
}

/**
 * Print the sieving results.
 */
void PrimeSieveGUI::printResults() {
  if (!ui->textEdit->toPlainText().isEmpty())
    ui->textEdit->appendPlainText("");

  // hack to get the count results aligned (using tabs)
  QString maxSizeText;
  for (int i = 0; i < PrimeSieveProcess::COUNTS_SIZE; i++) {
    if (flags_ & (ParallelPrimeSieve::COUNT_PRIMES << i))
      if (maxSizeText.size() < primeText_[i].size())
        maxSizeText = primeText_[i];
  }
  ui->textEdit->insertPlainText(maxSizeText + ": ");
  int maxWidth = ui->textEdit->cursorRect().left();
  ui->textEdit->moveCursor(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
  ui->textEdit->textCursor().removeSelectedText();
  ui->textEdit->setTabStopWidth(maxWidth);

  // print prime counts & time elapsed
  for (int i = 0; i < PrimeSieveProcess::COUNTS_SIZE; i++) {
    if (flags_ & (ParallelPrimeSieve::COUNT_PRIMES << i))
      ui->textEdit->appendPlainText(primeText_[i] + ":\t" +
          QString::number(primeSieveProcess_->getCounts(i)));
  }
  if (flags_ & (ParallelPrimeSieve::COUNT_FLAGS - ParallelPrimeSieve::COUNT_PRIMES))
    ui->textEdit->appendPlainText("");
  QString time("Elapsed time:\t" +
               QString::number(primeSieveProcess_->getTimeElapsed(), 'f', 2) +
               " sec");
  ui->textEdit->appendPlainText(time);
}

/**
 * Cancel sieving.
 */
void PrimeSieveGUI::on_cancelButton_clicked() {
  ui->cancelButton->setDisabled(true);
  ui->progressBar->setValue(0);
  // too late to abort
  if ((flags_ & ParallelPrimeSieve::PRINT_FLAGS) && primeSieveProcess_->isFinished())
    return;
  // cancel, kill all running processes
  this->cleanUp();
}

/**
 * Clean up after sieving is finished or canceled (abort all
 * running processes).
 */
void PrimeSieveGUI::cleanUp() {
  progressBarTimer_.stop();
  // kill the sieving process if still running
  if (primeSieveProcess_ != 0)
    delete primeSieveProcess_;
  primeSieveProcess_ = 0;
  // invert buttons
  ui->cancelButton->setDisabled(true);
  ui->sieveButton->setEnabled(true);
  // force repainting widgets
  this->repaint();
}
