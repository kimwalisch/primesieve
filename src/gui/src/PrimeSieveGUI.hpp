/*
 * PrimeSieveGUI.hpp -- This file is part of primesieve
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

#ifndef PRIMESIEVEGUI_HPP
#define PRIMESIEVEGUI_HPP

#include "PrimeSieveGUI_const.hpp"
#include <primesieve/ParallelPrimeSieve.hpp>

#if QT_VERSION >= 0x050000
  #include <QtGlobal>
  #include <QtWidgets/QMainWindow>
  #include <QtWidgets/QMenu>
  #include <QtWidgets/QAction>
  #include <QtWidgets/QComboBox>
  #include <QProcess>
  #include <QVector>
  #include <QTime>
  #include <QTimer>
  #include <QValidator>
  #include <QString>
#else
  #include <QMainWindow>
  #include <QtGlobal>
  #include <QMenu>
  #include <QAction>
  #include <QProcess>
  #include <QVector>
  #include <QComboBox>
  #include <QTime>
  #include <QTimer>
  #include <QValidator>
  #include <QString>
#endif

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
  PrimeSieveGUI(QWidget* parent = 0);
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
    COUNT_PRIMES     = primesieve::ParallelPrimeSieve::COUNT_PRIMES,
    COUNT_KTUPLETS   = primesieve::ParallelPrimeSieve::COUNT_SEXTUPLETS * 2 - primesieve::ParallelPrimeSieve::COUNT_TWINS,
    COUNT_FLAGS      = primesieve::ParallelPrimeSieve::COUNT_SEXTUPLETS * 2 - primesieve::ParallelPrimeSieve::COUNT_PRIMES,
    PRINT_FLAGS      = primesieve::ParallelPrimeSieve::PRINT_SEXTUPLETS * 2 - primesieve::ParallelPrimeSieve::PRINT_PRIMES,
    PRINT_PRIMES     = primesieve::ParallelPrimeSieve::PRINT_PRIMES,
    CALCULATE_STATUS = primesieve::ParallelPrimeSieve::CALCULATE_STATUS
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
  QValidator* validator_;
  int maxThreads_;
  /// Settings (bit flags) for PrimeSieveProcess.
  int flags_;
  /// Timer for the progressBar.
  QTimer progressBarTimer_;
  /// Separate process used for sieving
  PrimeSieveProcess* primeSieveProcess_;

  /**
   * PrimeSieveGUI_menu.cpp & menu bar objects.
   */
  void createMenuActions(QVector<QString>&);
  void createMenu(QVector<QString>&);
  int getMenuSettings();

  QMenu* fileMenu_;
  QMenu* printMenu_;
  QMenu* countMenu_;
  QMenu* helpMenu_;

  /// Save textEdit content to file.
  QAction* saveAct_;
  /// Quit application.
  QAction* quitAct_;
  /// Show about dialog.
  QAction* aboutAct_;
  /// Use radio button like behaviour.
  QActionGroup* alignmentGroup_;

  /// Count settings for PrimeSieveProcess.
  QVector<QAction*> countAct_;
  /// Print settings for PrimeSieveProcess.
  QVector<QAction*> printAct_;
};

#endif // PRIMESIEVEGUI_H
