////////////////////////////////////////////////////////////////////
// generate_primes_oop.cpp
// OOP-style callback, generate the primes up to 1000 and
// store them into the private primes_ vector.

#include <primesieve/soe/PrimeSieve.h>
#include <iostream>
#include <vector>

class Foo {
public:
  void storePrimes(unsigned int limit)
  {
    PrimeSieve ps;
    ps.generatePrimes(0, limit, store, (void*) this);
    std::cout << primes_.size() << " primes stored!" << std::endl;
  }
private:
  std::vector<unsigned int> primes_;
  /// OOP-style callback
  static void store(unsigned int prime, void* obj)
  {
    Foo* f = (Foo*) obj;
    f->primes_.push_back(prime);
  }
};

int main()
{
  Foo foo;
  foo.storePrimes(1000);
  return 0;
}
