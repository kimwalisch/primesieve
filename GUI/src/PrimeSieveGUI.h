/*
 * PrimeSieveGUI.h -- This file is part of primesieve
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

#ifndef PRIMESIEVEGUI_H
#define PRIMESIEVEGUI_H

#include "PrimeSieveGUI_const.h"
#include "PrimeSieveProcess.h"
#include "../src/PrimeSieve.h"
#include "../src/PrimeNumberFinder.h"

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

namespace Ui {
  class PrimeSieveGUI;
}

/**
 * PrimeSieveGUI is an easy to use Graphical User Interface with
 * multi-core support for PrimeSieve (highly optimized implementation
 * of the sieve of Eratosthenes).
 */
class PrimeSieveGUI: public QMainWindow {
Q_OBJECT
public:
  PrimeSieveGUI(QWidget* parent = 0);
  ~PrimeSieveGUI();
protected:
  void changeEvent(QEvent* e);
private slots:
  void on_cpuCoresComboBox_activated();
  void on_cancelButton_clicked();
  void on_sieveButton_clicked();
  void setPrint(QAction*);
  void saveToFile();
  void showAboutDialog();
  void autoSetCpuCores();
  void advanceProgressBar();
  void printProcessOutput();
  void processFinished(int, QProcess::ExitStatus);
private:
  void initMemberVariables();
  void initGUI();
  void initConnections();
  void initSieveSizeComboBox();
  void initCpuCoresComboBox(int);
  void createMenuActions(QVector<QString>&);
  void createMenuConnections();
  void createMenu(QVector<QString>&);
  int getMenuSettings();
  int getSieveSize();
  int getCpuCores();
  int getMaxCpuCores();
  int getIdealCpuCoreCount(qulonglong, qulonglong, int);
  void getBounds(qulonglong*, qulonglong*);
  void setComboBox(QComboBox*, QString);
  void createProcesses(qulonglong, qulonglong, int, int, int);
  void printResults();
  QString getAlign();
  void cleanUp();

  /// Qt GUI object
  Ui::PrimeSieveGUI* ui;

  /**
   * Menu bar objects.
   */
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
  /**
   * Other member variables.
   */
  QVector<QString> primeText_;
  /// Validates the input of the lower and upperBoundLineEdit.
  QValidator* validator_;
  /// true if the CPU has been detected.
  bool isCpuDetected_;
  /// Settings (bit flags) for PrimeSieveProcess.
  int flags_;
  /// Number of finished processes in the current sieving session.
  int finishedProcesses_;
  /// Timer for the progressBar.
  QTimer progressBarTimer_;
  /// Used to mesure the sieving time.
  QTime time_;
  /// Array used for multi-process sieving.
  QVector<PrimeSieveProcess*> processes_;
};

#endif // PRIMESIEVEGUI_H
