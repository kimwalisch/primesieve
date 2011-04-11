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
 * The Qt GUI interface is launched if the user launches the
 * application by mouse click, a ParallelPrimeSieve process is
 * launched if exactly one argument (shared memory identifier) is
 * provided.
 * @see           createProcesses(...)
 * @param argv[1] Shared memory identifier
 */
int main(int argc, char *argv[]) {
  // Qt GUI interface
  if (argc != 2) {
    QApplication a(argc, argv);
    PrimeSieveGUI w;
    w.show();
    return a.exec();
  }
  // ParallelPrimeSieve process
  else {
    // open an existing and initialized shared memory
    QSharedMemory sharedMemory(argv[1]);
    if (!sharedMemory.attach()) {
      std::cerr << "Unable to attach shared memory " << argv[1] << std::endl;
      exit(EXIT_FAILURE);
    }
    // map the attached shared memory to the results structure
    ParallelPrimeSieve::SharedMemoryPPS* sharedMemoryPPS =
        static_cast<ParallelPrimeSieve::SharedMemoryPPS*> (sharedMemory.data());
    try {
      // create a new ParallelPrimeSieve object which is initialized
      // with values from the shared memory (provided by the Qt GUI),
      // upon completion ParallelPrimeSieve communicates its results
      // back to the Qt GUI through the shared memory segment.
      ParallelPrimeSieve primesieve;
      primesieve.setSharedMemory(sharedMemoryPPS);
      primesieve.sieve(sharedMemoryPPS->threads);
    }
    catch (std::exception& ex) {
      sharedMemory.detach();
      std::cerr << "ParallelPrimeSieve error: " << ex.what() << std::endl;
      exit(EXIT_FAILURE);
    }
    sharedMemory.detach();
  }
  // exit success
  return 0;
}
