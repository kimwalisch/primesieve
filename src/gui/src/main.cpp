/*
 * main.cpp -- This file is part of primesieve
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

#include "PrimeSieveGUI.hpp"
#include <primesieve/ParallelPrimeSieve.hpp>

#if QT_VERSION >= 0x050000
  #include <QtWidgets/QApplication>
#else
  #include <QtGui/QApplication>
#endif

#include <QSharedMemory>
#include <stdexcept>
#include <cstdlib>
#include <iostream>

/**
 * The primesieve GUI is launched if the user starts the application
 * by mouse click or without arguments, a PrimeSieveProcess is
 * launched if process and a shared memory identifiers are provided as
 * arguments.
 * @param argv[1] [process identifier]
 * @param argv[2] [Shared memory identifier]
 * @see   PrimeSieveProcess.cpp
 */
int main(int argc, char *argv[])
{
  if (argc == 3)
  {
    QString psp("PrimeSieveProcess");
    QString parent(argv[1]);
    if (parent.compare(psp) == 0)
    {
      // open an existing and initialized shared memory
      QString id(argv[2]);
      QSharedMemory sharedMemory(id);
      if (!sharedMemory.attach())
      {
        std::cerr << "Unable to attach shared memory " << argv[2] << std::endl;
        exit(EXIT_FAILURE);
      }
      // map the attached shared memory to the shm segment
      primesieve::ParallelPrimeSieve::SharedMemory* shm =
          static_cast<primesieve::ParallelPrimeSieve::SharedMemory*>(sharedMemory.data());
      try
      {
        // initialize the ParallelPrimeSieve object with
        // values from the shared memory segment provided by
        // the primesieve GUI and start sieving
        if (!shm)
          throw std::runtime_error("sharedMemory.data() must not be NULL");
        primesieve::ParallelPrimeSieve pps;
        pps.init(*shm);
        pps.sieve();
      }
      catch (std::exception& e)
      {
        sharedMemory.detach();
        std::cerr << "ParallelPrimeSieve error: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
      }
      sharedMemory.detach();
      return 0;
    }
  }
  // Qt GUI interface
  QApplication a(argc, argv);
  PrimeSieveGUI w;
  w.show();
  return a.exec();
}
