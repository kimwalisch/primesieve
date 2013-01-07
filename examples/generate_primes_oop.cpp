////////////////////////////////////////////////////////////////////
// generate_primes_oop.cpp
// This example shows how to use the generatePrimes() methods
// with classes. Your class must inherit from
// PrimeSieveCallback<T> and define a callback method.

#include <primesieve/soe/PrimeSieve.h>
#include <primesieve/soe/PrimeSieveCallback.h>
#include <iostream>

class Foo : public PrimeSieveCallback<unsigned int> {
public:
  void callback(unsigned int prime)
  {
    std::cout << prime << '\n';
  }
};

int main()
{
  Foo foo;
  PrimeSieve ps;
  ps.generatePrimes(0, 1000, &foo);
  return 0;
}
