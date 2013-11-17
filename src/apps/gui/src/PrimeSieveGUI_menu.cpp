/*
 * PrimeSieveGUI_menu.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
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

#include "PrimeSieveGUI.hpp"
#include "ui_PrimeSieveGUI.h"

#include <primesieve.hpp>
#include <primesieve/ParallelPrimeSieve.hpp>

#if QT_VERSION >= 0x050000
  #include <QTextStream>
  #include <QDir>
  #include <QFile>
  #include <QtWidgets/QMessageBox>
  #include <QtWidgets/QFileDialog>
  #include <QtWidgets/QMenuBar>
#else
  #include <QMessageBox>
  #include <QDir>
  #include <QFile>
  #include <QTextStream>
  #include <QFileDialog>
#endif

/**
 * Initialize the menu items.
 */
void PrimeSieveGUI::createMenuActions(QVector<QString>& primeText) {
  // file actions
  saveAct_ = new QAction("&Save", this);
  saveAct_->setShortcut(tr("Ctrl+S"));
  quitAct_ = new QAction("&Quit", this);
  quitAct_->setShortcut(tr("Ctrl+Q"));

  // count actions
  countAct_.push_back(new QAction(primeText[0], this));
  countAct_.back()->setCheckable(true);
  countAct_.push_back(new QAction("Prime k-tuplets", this));
  countAct_.back()->setCheckable(true);
  // default count prime numbers
  countAct_.front()->setChecked(true);

  // radio button like behaviour for print actions
  alignmentGroup_ = new QActionGroup(this);
  alignmentGroup_->setExclusive(false);

  // print actions
  for (int i = 0; i < primeText.size(); i++) {
    printAct_.push_back(new QAction(primeText[i], this));
    printAct_.back()->setCheckable(true);
    alignmentGroup_->addAction(printAct_.back());
  }
  // about action
  aboutAct_ = new QAction("About", this);
}

/**
 * Create the menu bar with 'File', 'Count', 'Print' and 'Help'
 * menu options.
 */
void PrimeSieveGUI::createMenu(QVector<QString>& primeText) {
  this->createMenuActions(primeText);

  fileMenu_ = menuBar()->addMenu("&File");
  fileMenu_->addAction(saveAct_);
  fileMenu_->addAction(quitAct_);
  countMenu_ = menuBar()->addMenu("&Count");
  for (int i = 0; i < countAct_.size(); i++)
    countMenu_->addAction(countAct_[i]);
  printMenu_ = menuBar()->addMenu("&Print");
  for (int i = 0; i < printAct_.size(); i++)
    printMenu_->addAction(printAct_[i]);
  helpMenu_ = menuBar()->addMenu("&Help");
  helpMenu_->addAction(aboutAct_);
}

/**
 * Return the count and print menu settings as bit flags.
 */
int PrimeSieveGUI::getMenuSettings() {
  int flags = 0;
  // get count settings
  if (countAct_[0]->isChecked())
    flags |= COUNT_PRIMES;
  if (countAct_[1]->isChecked())
    flags |= COUNT_KTUPLETS;
  // get print settings
  for (int i = 0; i < printAct_.size(); i++)
    if (printAct_[i]->isChecked())
      flags |= PRINT_PRIMES << i;
  return flags;
}

/**
 * Disable the "Threads" ComboBox and the "Auto set" CheckBox and
 * set to 1 Threads for printing (else invert).
 */
void PrimeSieveGUI::printMenuClicked(QAction* qAct) {
  // disable other print options
  for (int i = 0; i < printAct_.size(); i++) {
    if (printAct_[i] != qAct)
      printAct_[i]->setChecked(false);
  }
  ui->autoSetCheckBox->setDisabled(qAct->isChecked());
  if (qAct->isChecked()) {
    ui->autoSetCheckBox->setChecked(true);
    ui->threadsComboBox->setCurrentIndex(0);
  }
  ui->threadsComboBox->setDisabled(qAct->isChecked());
  this->autoSetThreads();
}

/**
 * Save the content of the textEdit to a file.
 */
void PrimeSieveGUI::saveToFile() {
  // Qt uses '/' internally, also for Windows
  QString currentPath = QDir::currentPath() + "/Unsaved Document 1";
  QString fileName = QFileDialog::getSaveFileName(this, "Save As...", currentPath, "All Files (*)");
  QFile file(fileName);
  if (file.open(QFile::WriteOnly | QFile::Text)) {
    QTextStream textStream(&file);
    textStream << ui->textEdit->toPlainText();
  }
}

void PrimeSieveGUI::showAboutDialog() {
  QString title = "About " + APPLICATION_NAME;
  QString message = "<h2>" + APPLICATION_NAME + " " + PRIMESIEVE_VERSION + "</h2>"
      + APPLICATION_ABOUT +
      + "<a href=\"" + APPLICATION_HOMEPAGE + "\">" + APPLICATION_HOMEPAGE + "</a>";
  QMessageBox::about(this, title, message);
}
