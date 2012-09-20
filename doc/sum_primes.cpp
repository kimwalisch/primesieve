#include <primesieve/soe/PrimeSieve.h>
#include <iostream>

unsigned int sum = 0;

void callback(unsigned int prime) {
  sum += prime;
}

int main() {
  PrimeSieve ps;
  ps.generatePrimes(2, 1000, callback);
  std::cout << "Sum of the primes below 1000 = " << sum << std::endl;
  return 0;
}
