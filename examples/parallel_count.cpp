////////////////////////////////////////////////////////////////////
// parallel_count.cpp
// Count the prime triplets within [10^14, 10^14+10^11]
// using all CPU cores (default).

#include <primesieve/soe/ParallelPrimeSieve.h>
#include <stdint.h>
#include <iostream>

int main()
{
  uint64_t start = (uint64_t) 1E14;
  uint64_t stop  = (uint64_t) (1E14+1E11);
  ParallelPrimeSieve pps;
  pps.countTriplets(start, stop);
  std::cout << "Prime triplets in [10^14, 10^14+10^11] = "
            << pps.getTripletCount()
            << std::endl;
  return 0;
}
