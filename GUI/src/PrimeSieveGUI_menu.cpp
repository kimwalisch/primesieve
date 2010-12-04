/*
 * PrimeSieveGUI_menu.cpp -- This file is part of primesieve
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

#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>

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

void PrimeSieveGUI::createMenuConnections() {
  // file menu connections
  connect(saveAct_, SIGNAL(triggered()), this, SLOT(saveToFile()));
  connect(quitAct_, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
  // print connection
  connect(alignmentGroup_, SIGNAL(triggered(QAction*)), this,
      SLOT(setPrint(QAction*)));
  // show an about dialog
  connect(aboutAct_, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
}

/**
 * Create the menu bar.
 */
void PrimeSieveGUI::createMenu(QVector<QString>& primeText) {
  // initialization
  this->createMenuActions(primeText);
  this->createMenuConnections();
  // create file menu
  fileMenu_ = menuBar()->addMenu("&File");
  fileMenu_->addAction(saveAct_);
  fileMenu_->addAction(quitAct_);
  // create count menu
  countMenu_ = menuBar()->addMenu("&Count");
  for (int i = 0; i < countAct_.size(); i++)
    countMenu_->addAction(countAct_[i]);
  // create print menu
  printMenu_ = menuBar()->addMenu("&Print");
  for (int i = 0; i < printAct_.size(); i++)
    printMenu_->addAction(printAct_[i]);
  // create help menu
  helpMenu_ = menuBar()->addMenu("&Help");
  helpMenu_->addAction(aboutAct_);
}

/**
 * Returns the count and print menu settings as bit flags.
 */
int PrimeSieveGUI::getMenuSettings() {
  int flags = 0;
  // get count settings
  if (countAct_[0]->isChecked())
    flags |= COUNT_PRIMES;
  if (countAct_[1]->isChecked())
    flags |= COUNT_FLAGS - COUNT_PRIMES;
  // get print settings
  for (int i = 0; i < printAct_.size(); i++) {
    if (printAct_[i]->isChecked()) {
      flags |= PRINT_PRIMES << i;
      break;
    }
  }
  return flags;
}

/**
 * Disable the "CPU cores" ComboBox and the "Auto set" CheckBox and
 * set to 1 CPU core for printing (else invert).
 */
void PrimeSieveGUI::setPrint(QAction* qAct) {
  bool isPrint = false;
  // disable other print options
  for (int i = 0; i < printAct_.size(); i++) {
    if (printAct_[i] != qAct)
      printAct_[i]->setChecked(false);
    else
      isPrint = printAct_[i]->isChecked();
  }
  if (isCpuDetected_)
    ui->autoSetCheckBox->setDisabled(isPrint);
  if (isPrint) {
    ui->autoSetCheckBox->setChecked(true);
    this->setComboBox(ui->cpuCoresComboBox, "1");
  }
  ui->cpuCoresComboBox->setDisabled(isPrint);
  this->autoSetCpuCores();
}

/**
 * Save the content of the textEdit to a file.
 */
void PrimeSieveGUI::saveToFile() {
  // Qt uses '/' internally, also for Windows
  QString currentPath = QDir::currentPath() + "/Unsaved Document 1";
  QString fileName = QFileDialog::getSaveFileName(this, "Save As...",
      currentPath, "All Files (*)");
  QFile file(fileName);
  // TODO the user does not know if I cannot save because the
  // file exists and is read only
  if (file.open(QFile::WriteOnly | QFile::Text)) {
    QTextStream textStream(&file);
    textStream << ui->textEdit->toPlainText();
  }
}

void PrimeSieveGUI::showAboutDialog() {
  QString title = "About " + APPLICATION_NAME;
  QString message = "<h2>" + APPLICATION_NAME + " " + APPLICATION_VERSION + "</h2>"
      + "<p>Copyright &copy; 2010 Kim Walisch</p>"
      + "<p>" + APPLICATION_ABOUT + "</p>"
      + "<a href=\"" + APPLICATION_HOMEPAGE + "\">" + APPLICATION_HOMEPAGE+ "</a>";
  QMessageBox::about(this, title, message);
}
