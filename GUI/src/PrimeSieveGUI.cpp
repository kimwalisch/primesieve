/*
 * PrimeSieveGUI.cpp -- This file is part of primesieve
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

#include "PrimeSieveGUI.h"
#include "ui_PrimeSieveGUI.h"
#include "../src/pmath.h"

#include <QThread>
#include <QSize>
#include <QMessageBox>
#include <QTextCursor>
#include <cstdlib>
#include <stdexcept>

PrimeSieveGUI::PrimeSieveGUI(QWidget *parent) :
  QMainWindow(parent), ui(new Ui::PrimeSieveGUI), saveAct_(0), quitAct_(0),
      aboutAct_(0), alignmentGroup_(0), validator_(0), finishedProcesses_(0) {
  ui->setupUi(this);
  this->initMemberVariables();
  this->initGUI();
  this->initConnections();
}

PrimeSieveGUI::~PrimeSieveGUI() {
  // kill all processes
  this->cleanUp();
  // free all allocated memory
  if (saveAct_        != 0) delete saveAct_;
  if (quitAct_        != 0) delete quitAct_;
  if (aboutAct_       != 0) delete aboutAct_;
  if (alignmentGroup_ != 0) delete alignmentGroup_;
  if (validator_      != 0) delete validator_;
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

void PrimeSieveGUI::initMemberVariables() {
  primeText_.push_back("Prime numbers");
  primeText_.push_back("Twin primes");
  primeText_.push_back("Prime triplets");
  primeText_.push_back("Prime quadruplets");
  primeText_.push_back("Prime quintuplets");
  primeText_.push_back("Prime sextuplets");
  primeText_.push_back("Prime septuplets");

  // get the number of logical CPU cores
  int maxCpuCores = QThread::idealThreadCount();
  if (maxCpuCores > 0)
    isCpuDetected_ = true;
  else {
    isCpuDetected_ = false;
    // default value for undetected CPUs
    maxCpuCores = DEFAULT_MAX_CPU_CORES;
  }
  this->initCpuCoresComboBox(maxCpuCores);
}

void PrimeSieveGUI::initGUI() {
  // set the main window title
  this->setWindowTitle(APPLICATION_NAME + " " + APPLICATION_VERSION);
  // create the menu bar
  this->createMenu(primeText_);
  // fill with values
  this->initSieveSizeComboBox();

  // set an ideal ComboBox width
  int width = ui->sieveSizeComboBox->minimumSizeHint().width();
  ui->sieveSizeComboBox->setFixedWidth(width);
  ui->cpuCoresComboBox->setFixedWidth(width);

  if (!isCpuDetected_) {
    ui->autoSetCheckBox->setChecked(false);
    ui->autoSetCheckBox->setDisabled(true);
  }
  // limit input to 20 digits max
  QRegExp rx("[0-9]\\d{0,19}");
  validator_ = new QRegExpValidator(rx, this);
  ui->lowerBoundLineEdit->setValidator(validator_);
  ui->upperBoundLineEdit->setValidator(validator_);
  
  // set a nice GUI size
  int guiWidth  = this->minimumSizeHint().width();
  int guiHeight = this->sizeHint().height();
#if defined(Q_OS_WIN)
  guiHeight = static_cast<int> (guiHeight * 0.95);
#elif defined(Q_OS_MAC)
  guiHeight = static_cast<int> (guiHeight * 0.96);
#endif
  this->resize(QSize(guiWidth, guiHeight));
}

void PrimeSieveGUI::initConnections() {
  // advances the progress bar
  connect(&progressBarTimer_, SIGNAL(timeout()), this, SLOT(
      advanceProgressBar()));
  // autoSetCpuCores() connections
  if (isCpuDetected_) {
  connect(ui->lowerBoundLineEdit, SIGNAL(textChanged(const QString &)),
      this, SLOT(autoSetCpuCores()));
  connect(ui->upperBoundLineEdit, SIGNAL(textChanged(const QString &)),
      this, SLOT(autoSetCpuCores()));
  connect(ui->autoSetCheckBox, SIGNAL(toggled(bool)),
      this, SLOT(autoSetCpuCores()));
  }
}

/**
 * Fill the sieveSizeComboBox with power of 2 values from
 * 16 KB till 8192 KB.
 */
void PrimeSieveGUI::initSieveSizeComboBox() {
  for (int i = MINIMUM_SIEVE_SIZE; i <= MAXIMUM_SIEVE_SIZE; i *= 2)
    ui->sieveSizeComboBox->addItem(QString::number(i) + " KB");
  // set the default sieve size
  QString sieveSize = QString::number(DEFAULT_SIEVE_SIZE) + " KB";
  this->setComboBox(ui->sieveSizeComboBox, sieveSize);
}

/**
 * Fill the cpuCoresComboBox with power of 2 values from 1 till
 * maxCpuCores.
 */
void PrimeSieveGUI::initCpuCoresComboBox(int maxCpuCores) {
  for (int i = 1; i < maxCpuCores; i *= 2)
    ui->cpuCoresComboBox->addItem(QString::number(i));
  ui->cpuCoresComboBox->addItem(QString::number(maxCpuCores));
  // default 1 CPU core
  this->setComboBox(ui->cpuCoresComboBox, "1");
}

/**
 * Get the users lower and upper bound for prime sieving.
 */
void PrimeSieveGUI::getBounds(qulonglong* lowerBound, qulonglong* upperBound) {
  QString lbound = ui->lowerBoundLineEdit->text();
  QString ubound = ui->upperBoundLineEdit->text();
  if (ubound.isEmpty() || lbound.isEmpty())
    throw std::invalid_argument("Missing input.");
  bool lOk = true;
  bool uOk = true;
  *lowerBound = lbound.toULongLong(&lOk, 10);
  *upperBound = ubound.toULongLong(&uOk, 10);
  if (!lOk || !uOk || *lowerBound >= UPPER_BOUND_LIMIT || *upperBound
      >= UPPER_BOUND_LIMIT)
    throw std::invalid_argument(
        "Please use numbers >= 0 and < (2^64-1) - (2^32-1) * 10.");
  if (*lowerBound > *upperBound)
    throw std::invalid_argument(
        "The lower bound must not be greater than the upper bound.");
}

/**
 * Get the sieve size (in KiloBytes) from the sieveSizeComboBox.
 * @post sieveSize >= 1 && sieveSize <= 8192.
 */
int PrimeSieveGUI::getSieveSize() {
  QString sieveSize(ui->sieveSizeComboBox->currentText());
  // remove " KB"
  sieveSize.chop(3);
  return sieveSize.toInt();
}

/**
 * Get the current number of CPU cores from the cpuCoresComboBox.
 */
int PrimeSieveGUI::getCpuCores() {
  return ui->cpuCoresComboBox->currentText().toInt();
}

/**
 * Get the maximum number of CPU cores from the cpuCoresComboBox.
 */
int PrimeSieveGUI::getMaxCpuCores() {
  int count = ui->cpuCoresComboBox->count();
  return ui->cpuCoresComboBox->itemText(count - 1).toInt();
}

/**
 * Show the text string in the ComboBox.
 */
void PrimeSieveGUI::setComboBox(QComboBox* comboBox, QString text) {
  int index = comboBox->findText(text);
  if (index < 0)
    QMessageBox::critical(this, APPLICATION_NAME,
        "Internal ComboBox error, please contact the developer.");
  comboBox->setCurrentIndex(index);
}

/**
 * The user has chosen a custom number of CPU cores.
 */
void PrimeSieveGUI::on_cpuCoresComboBox_activated() {
  // disable "Auto set"
  ui->autoSetCheckBox->setChecked(false);
}

/**
 * Calculate an ideal number of CPU cores for sieving.
 */
int PrimeSieveGUI::getIdealCpuCoreCount(qulonglong lowerBound,
    qulonglong upperBound, int maxCpuCores) {
  int cpuCores = -1;
  // I made some tests around 10^19 which showed that each CPU core
  // should at least sieve an interval of sqrt(upperBound) / 6 for a
  // performance benefit
  qulonglong interval = U32SQRT(upperBound) / 6;
  if (interval < MINIMUM_THREAD_INTERVAL)
    interval = MINIMUM_THREAD_INTERVAL;
  // use all CPU cores for big sieve intervals
  if (maxCpuCores * interval <= upperBound - lowerBound)
    cpuCores = maxCpuCores;
  else {
    // use less CPU cores for small sieve intervals
    cpuCores = static_cast<int> ((upperBound - lowerBound) / interval);
    // floor to the next power of 2 value
    cpuCores = 1 << floorLog2(cpuCores);
  }
  return cpuCores;
}

/**
 * Show the ideal CPU core number in the cpuCoresComboBox (if
 * "Auto set" is enabled).
 */
void PrimeSieveGUI::autoSetCpuCores() {
  if (ui->autoSetCheckBox->isEnabled() && 
      ui->autoSetCheckBox->isChecked()) {
    qulonglong lowerBound = 0;
    qulonglong upperBound = 0;
    QString cpuCores("1");
    try {
      // get the users lower and upper bound
      this->getBounds(&lowerBound, &upperBound);
      // get an ideal number of CPU cores
      cpuCores.setNum(this->getIdealCpuCoreCount(lowerBound, upperBound,
          this->getMaxCpuCores()));
    } catch (...) {
    }
    // set to the ideal CPU core number
    this->setComboBox(ui->cpuCoresComboBox, cpuCores);
  }
}

/**
 * Cancel sieving.
 */
void PrimeSieveGUI::on_cancelButton_clicked() {
  ui->cancelButton->setDisabled(true);
  // set to 0 percent
  ui->progressBar->setValue(0);
  // too late to abort
  if ((flags_ & PRINT_FLAGS) && processes_.front()->isFinished())
    return;
  // kill all running processes
  this->cleanUp();
}

void PrimeSieveGUI::advanceProgressBar() {
  // delay the timer after 60 sec
  if (progressBarTimer_.interval() < 100 && time_.elapsed() > 60000)
    progressBarTimer_.setInterval(100);
  // in percents
  float status = 0.0f;
  // combine the status of all processes
  for (int i = 0; i < processes_.size(); i++)
    status += processes_[i]->getStatus();
  status /= processes_.size();
  int permil = static_cast<int> (status * 10.0f);
  // advance the progressBar
  ui->progressBar->setValue(permil);
}

/**
 * Print the sieving results and clean up.
 */
void PrimeSieveGUI::printResults() {
  // add newline
  if (!ui->textEdit->toPlainText().isEmpty())
    ui->textEdit->appendPlainText("");
  QString align = this->getAlign();

  // combine the count results of all processes
  QVector<qlonglong> combinedCount(PrimeSieveProcess::COUNTS_SIZE, 0);
  for (int i = 0; i < processes_.size(); i++) {
    for (int j = 0; j < combinedCount.size(); j++)
      combinedCount[j] += processes_[i]->getCounts(j);
  }
  // print prime counts
  for (int i = 0; i < combinedCount.size(); i++) {
    if (combinedCount[i] >= 0)
      ui->textEdit->appendPlainText(primeText_[i] + align +
          QString::number(combinedCount[i]));
  }
  // add newline for prime k-tuplets
  if (flags_ & (COUNT_FLAGS - COUNT_PRIMES))
    ui->textEdit->appendPlainText("");

  // print time
  QString time("Elapsed time" + align);
  int milliSeconds = time_.elapsed();
  int hrs = (milliSeconds / 3600000);
  int min = (milliSeconds / 60000) % 60;
  if (hrs > 0)
    time.append(QString::number(hrs) + " hrs ");
  if (min > 0)
    time.append(QString::number(min) + " min ");
  double sec = (milliSeconds / 1000.0) - (hrs * 60 + min) * 60;
    time.append(QString::number(sec) + " sec");
  ui->textEdit->appendPlainText(time);
}

/**
 * Hack to get the count results aligned.
 * @return Align string.
 */
QString PrimeSieveGUI::getAlign() {
  // find the text with the largest width
  QString maxSizeText;
  for (int i = 0; i < PrimeSieveProcess::COUNTS_SIZE; i++) {
    if (flags_ & (COUNT_PRIMES << i))
      if (maxSizeText.size() < primeText_[i].size())
        maxSizeText = primeText_[i];
  }
  // print test string
  ui->textEdit->insertPlainText(maxSizeText + ": ");
  // get width in pixels
  int maxWidth = ui->textEdit->cursorRect().left();
  // remove test string
  ui->textEdit->moveCursor(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
  ui->textEdit->textCursor().removeSelectedText();
  // must be an error, do not use tabs
  if (maxWidth <= 20 || maxWidth >= 1024)
      return ": ";
  // set tab width
  ui->textEdit->setTabStopWidth(maxWidth);
  return ":\t";
}

/**
 * Clean up after sieving is finished or canceled (abort all
 * running processes).
 */
void PrimeSieveGUI::cleanUp() {
  // stop the timer first
  progressBarTimer_.stop();
  // kill all processes that are still running
  for (; !processes_.isEmpty(); processes_.pop_back())
    delete processes_.back();
  // reset
  finishedProcesses_ = 0;
  // invert buttons
  ui->cancelButton->setDisabled(true);
  ui->sieveButton->setEnabled(true);
  // force repainting widgets
  this->repaint();
}
