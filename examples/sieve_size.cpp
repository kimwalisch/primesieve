////////////////////////////////////////////////////////////////////
// sieve_size.cpp
// Set a faster sieve size, for > 10^17 use L2 cache size.

#include <primesieve/soe/PrimeSieve.h>
#include <stdint.h>
#include <iostream>

int main()
{
  PrimeSieve ps;
  ps.setSieveSize(512);
  uint64_t start = (uint64_t) 1E19;
  uint64_t stop  = (uint64_t) (1E19+1E10);
  std::cout << "Twin primes in [10^19, 10^19+10^10] = "
            << ps.getTwinCount(start, stop)
            << std::endl;
  return 0;
}
