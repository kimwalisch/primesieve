////////////////////////////////////////////////////////////////////
// primesieve_error.cpp
// Catch primesieve_error exceptions, start must be <= stop.

#include <primesieve/soe/PrimeSieve.h>
#include <primesieve/soe/primesieve_error.h>
#include <iostream>

int main()
{
  int start = 100;
  int stop = 0;
  try {
    PrimeSieve ps;
    ps.printPrimes(start, stop);
  }
  catch (primesieve_error& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}
