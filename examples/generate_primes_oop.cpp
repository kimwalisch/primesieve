////////////////////////////////////////////////////////////////////
// generate_primes_oop.cpp
// This example shows how to use the generatePrimes()
// methods with classes, the code stores the primes below
// 1000 in the foo.primes_ vector.

#include <primesieve/soe/PrimeSieve.h>
#include <vector>

class Foo {
public:
  static void callback(unsigned int prime, void* foo)
  {
    Foo* f = (Foo*) foo;
    f->primes_.push_back(prime);
  }
private:
  std::vector<unsigned int> primes_;
};

int main()
{
  Foo foo;
  PrimeSieve ps;
  ps.generatePrimes(0, 1000, foo.callback, &foo);
  return 0;
}
