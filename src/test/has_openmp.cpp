#include <omp.h>
#include <iostream>

int main()
{
  std::cout << "OpenMP version (yyyymm) : " << _OPENMP               << std::endl
            << "Number of CPU cores     : " << omp_get_max_threads() << std::endl;
  return 0;
}
