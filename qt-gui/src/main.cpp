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
#include "../soe/ParallelPrimeSieve.h"

#include <QSharedMemory>
#include <stdint.h>
#include <stdexcept>
#include <cstdlib>
#include <iostream>

/**
 * The primesieve GUI interface is launched if the user launches the
 * application by mouse click, a PrimeSieveProcess is launched if 
 * "PrimeSieveProcess" and a shared memory identifier (process id) are
 * provided as arguments.
 * @see           PrimeSieveProcess.cpp
 * @param argv[1] ["PrimeSieveProcess"]
 * @param argv[2] [Shared memory identifier]
 */
int main(int argc, char *argv[]) {
  // PrimeSieveProcess
  if (argc == 3 && QString(argv[1]).compare("PrimeSieveProcess") == 0) {
    // open an existing and initialized shared memory
    QSharedMemory sharedMemory(argv[2]);
    if (!sharedMemory.attach()) {
      std::cerr << "Unable to attach shared memory " << argv[2] << std::endl;
      exit(EXIT_FAILURE);
    }
    // map the attached shared memory to the results structure
    ParallelPrimeSieve::SharedMemoryPPS* sharedMemoryPPS =
        static_cast<ParallelPrimeSieve::SharedMemoryPPS*> (sharedMemory.data());
    try {
      ParallelPrimeSieve primesieve;
      // initialize the ParallelPrimeSieve object with values from the
      // shared memory provided by the primesieve GUI
      primesieve.setSharedMemory(sharedMemoryPPS);
      // start sieving primes, the results are communicated back to
      // the primesieve GUI via the shared memory
      primesieve.sieve(sharedMemoryPPS->threads);
    }
    catch (std::exception& ex) {
      sharedMemory.detach();
      std::cerr << "ParallelPrimeSieve error: " << ex.what() << std::endl;
      exit(EXIT_FAILURE);
    }
    sharedMemory.detach();
    // exit success
    return 0;
  }
  else { // Qt GUI interface
    QApplication a(argc, argv);
    PrimeSieveGUI w;
    w.show();
    return a.exec();
  }
}
