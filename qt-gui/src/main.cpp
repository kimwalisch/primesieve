/*
 * main.cpp -- This file is part of primesieve
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

#include <QtGui/QApplication>
#include "PrimeSieveGUI.h"
#include "../src/PrimeSieve.h"

#include <QString>
#include <QSharedMemory>
#include <stdint.h>
#include <stdexcept>
#include <cstdlib>
#include <iostream>

/**
 * The Qt GUI interface is launched if the user launches the
 * application by mouse click, a sieve of Eratosthenes process (no
 * GUI) is launched if exactly 5 arguments are provided.
 * @see createProcesses(...)
 *
 * @argv[1] start number < (2^64-1) - (2^32-1) * 10
 * @argv[2] stop number < (2^64-1) - (2^32-1) * 10
 * @argv[3] sieve size in KiloBytes, >= 1 && <= 8192
 * @argv[4] flags (settings for primesieve)
 * @argv[5] shared memory name
 */
int main(int argc, char *argv[]) {
  // GUI interface
  if (argc != 6) {
    QApplication a(argc, argv);
    PrimeSieveGUI w;
    w.show();
    return a.exec();
  }
  // PrimeSieveProcess
  else {
    QString str;
    str                = argv[1];
    bool ok            = true;
    uint64_t start     = str.toULongLong(&ok, 10);
    str                = argv[2];
    uint64_t stop      = str.toULongLong(&ok, 10);
    uint32_t sieveSize = std::strtol(argv[3], NULL, 10);
    uint32_t flags     = std::strtol(argv[4], NULL, 10);
    
    // open an existing and initialized shared memory
    QSharedMemory sharedMemory(argv[5]);
    if (!sharedMemory.attach()) {
      std::cerr << "Unable to attach shared memory " << argv[5] << std::endl;
      exit(EXIT_FAILURE);
    }
    // map the attached shared memory to the results structure
    PrimeNumberFinder::Results* results =
        static_cast<PrimeNumberFinder::Results*> (sharedMemory.data());
    try {
      // create a new PrimeSieve object that writes its sieving
      // results and status to the shared memory and prints prime
      // numbers or k-tuplets to stdout (if print flags are set)
      PrimeSieve primesieve;
      primesieve.setStartNumber(start);
      primesieve.setStopNumber(stop);
      primesieve.setSieveSize(sieveSize);
      primesieve.setFlags(flags);
      primesieve.setResults(results);
      // start sieving prime numbers and k-tuplets
      primesieve.sieve();
    }
    catch (std::exception& ex) {
      sharedMemory.detach();
      std::cerr << ex.what() << std::endl;
      exit(EXIT_FAILURE);
    }
    sharedMemory.detach();
  }
  // exit success
  return 0;
}
