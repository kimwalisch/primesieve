////////////////////////////////////////////////////////////////////
// print_twins.cpp
// Print the twin primes up to 1000 to std::cout.
// Program output:
// (3, 5)
// (5, 7)
// ...

#include <primesieve/soe/PrimeSieve.h>

int main()
{
  PrimeSieve ps;
  ps.printTwins(0, 1000);
  return 0;
}
