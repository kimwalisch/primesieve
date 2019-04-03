/*
 * PrimeSieveGUI.hpp -- This file is part of primesieve
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

#ifndef PRIMESIEVEGUI_HPP
#define PRIMESIEVEGUI_HPP

#include <QtGlobal>
#include <QProcess>
#include <QVector>
#include <QString>
#include <QTime>
#include <QTimer>
#include <QValidator>

#include <primesieve/ParallelSieve.hpp>
#include <limits>
#include <stdint.h>

#if QT_VERSION >= 0x050000
  #include <QtWidgets/QMainWindow>
  #include <QtWidgets/QMenu>
  #include <QtWidgets/QAction>
  #include <QtWidgets/QComboBox>
#else
  #include <QMainWindow>
  #include <QMenu>
  #include <QAction>
  #include <QComboBox>
#endif

const int PRINT_BUFFER_SIZE = 1024;
const int MINIMUM_SIEVE_SIZE = 16;
const int MAXIMUM_SIEVE_SIZE = 4096;
const quint64 UPPER_BOUND_LIMIT = std::numeric_limits<uint64_t>::max();

const QString UPPER_BOUND_STR = QString::number(UPPER_BOUND_LIMIT);
const QString APPLICATION_NAME("primesieve");
const QString APPLICATION_HOMEPAGE("https://github.com/kimwalisch/primesieve");

const QString APPLICATION_ABOUT(
    "<p>Copyright &copy; 2019 Kim Walisch</p>"
    "<p>primesieve generates prime numbers and prime k-tuplets using a highly "
    "optimized implementation of the sieve of Eratosthenes."
    "<br><br>"
    "This is free software: you can redistribute it and/or modify "
    "it under the terms of the GNU General Public License as published by "
    "the Free Software Foundation; either version 3 of the License, or "
    "(at your option) any later version.</p>");

namespace Ui {
  class PrimeSieveGUI;
}

class PrimeSieveProcess;

/**
 * PrimeSieveGUI is a graphical user interface for primeSieve (highly
 * optimized sieve of Eratosthenes implementation).
 */
class PrimeSieveGUI : public QMainWindow {
Q_OBJECT
public:
  PrimeSieveGUI(QWidget* parent = nullptr);
  ~PrimeSieveGUI();
protected:
  void changeEvent(QEvent* e);
private slots:
  void autoSetThreads();
  void on_threadsComboBox_activated();
  void on_sieveButton_clicked();
  void on_cancelButton_clicked();
  void advanceProgressBar();
  void printProcessOutput();
  void processFinished(int, QProcess::ExitStatus);

  /// PrimeSieveGUI_menu.cpp
  void printMenuClicked(QAction*);
  void saveToFile();
  void showAboutDialog();
private:
  /// Qt GUI object
  Ui::PrimeSieveGUI* ui;

  enum {
    COUNT_PRIMES      = primesieve::COUNT_PRIMES,
    COUNT_KTUPLETS    = primesieve::COUNT_SEXTUPLETS * 2 - primesieve::COUNT_TWINS,
    COUNT_FLAGS       = primesieve::COUNT_SEXTUPLETS * 2 - primesieve::COUNT_PRIMES,
    PRINT_FLAGS       = primesieve::PRINT_SEXTUPLETS * 2 - primesieve::PRINT_PRIMES,
    PRINT_PRIMES      = primesieve::PRINT_PRIMES,
    UPDATE_GUI_STATUS = primesieve::UPDATE_GUI_STATUS
  };

  void initGUI();
  void initConnections();
  int getSieveSize();
  int getThreads();
  quint64 getNumber(const QString&);
  void setTo(QComboBox*, const QString&);
  void printResults();
  void cleanUp();

  QVector<QString> primeText_;
  /// Validates the input of the lower and upperBoundLineEdit.
  QValidator* validator_ = nullptr;
  int maxThreads_ = 0;
  /// Settings (bit flags) for PrimeSieveProcess.
  int flags_ = 0;
  /// Timer for the progressBar.
  QTimer progressBarTimer_;
  /// Separate process used for sieving
  PrimeSieveProcess* primeSieveProcess_ = nullptr;

  /**
   * PrimeSieveGUI_menu.cpp & menu bar objects.
   */
  void createMenuActions(QVector<QString>&);
  void createMenu(QVector<QString>&);
  int getMenuSettings();

  QMenu* fileMenu_ = nullptr;
  QMenu* printMenu_ = nullptr;
  QMenu* countMenu_ = nullptr;
  QMenu* helpMenu_ = nullptr;

  /// Save textEdit content to file.
  QAction* saveAct_ = nullptr;
  /// Quit application.
  QAction* quitAct_ = nullptr;
  /// Show about dialog.
  QAction* aboutAct_ = nullptr;
  /// Use radio button like behaviour.
  QActionGroup* alignmentGroup_ = nullptr;

  /// Count settings for PrimeSieveProcess.
  QVector<QAction*> countAct_;
  /// Print settings for PrimeSieveProcess.
  QVector<QAction*> printAct_;
};

#endif // PRIMESIEVEGUI_H
