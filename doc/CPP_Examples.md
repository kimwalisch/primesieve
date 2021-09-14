# primesieve C++ examples

This is a short selection of C++ code snippets that use libprimesieve to generate prime numbers.
These examples cover the most frequently used functionality of libprimesieve. Arguably the most
useful feature provided by libprimesieve is the ```primesieve::iterator``` which lets you
iterate over primes using the ```next_prime()``` or ```prev_prime()``` methods.

For in-depth documentation please refer to the [C++ API Reference](https://kimwalisch.github.io/primesieve/api).

## ```primesieve::iterator::next_prime()```

By default ```primesieve::iterator::next_prime()``` generates primes > 0 i.e. 2, 3, 5, 7, ...

```C++
#include <primesieve.hpp>
#include <iostream>

int main()
{
  primesieve::iterator it;
  uint64_t prime = it.next_prime();
  uint64_t sum = 0;

  // iterate over the primes below 10^9
  for (; prime < 1000000000ull; prime = it.next_prime())
    sum += prime;

  std::cout << "Sum of the primes below 10^9 = " << sum << std::endl;

  return 0;
}
```

* [Build instructions](#how-to-compile)

## ```primesieve::iterator::skipto()```

This method changes the start number of the ```primesieve::iterator``` object. (By default
the start number is initialized to 0). Note that you can also specify the start number in
the constructor of the ```primesieve::iterator``` object.

```C++
#include <primesieve.hpp>
#include <iostream>

int main()
{
  primesieve::iterator it;

  // generate primes > 1000
  it.skipto(1000);
  prime = it.next_prime();

  // generate primes > 1000 and <= 1100
  for (; prime <= 1100; prime = it.next_prime())
    std::cout << prime << std::endl;

  return 0;
}
```

* [Build instructions](#how-to-compile)

The ```primesieve::iterator::skipto()``` method (and the ```primesieve::iterator```
constructor) take an optional ```stop_hint``` parameter for performance optimization.
If ```stop_hint``` is set ```primesieve::iterator``` will only buffer primes up to
this limit.

```C++
#include <primesieve.hpp>
#include <iostream>

int main()
{
  uint64_t start = 1000;
  uint64_t stop_hint = 1100;

  // generate primes > 1000 and <= 1100
  primesieve::iterator it(start, stop_hint);
  prime = it.next_prime();

  for (; prime <= 1100; prime = it.next_prime())
    std::cout << prime << std::endl;

  return 0;
}
```

* [Build instructions](#how-to-compile)

## ```primesieve::iterator::prev_prime()```

Before using ```primesieve::iterator::prev_prime()``` you must change the start number
(either in the constructor or using the ```skipto()``` method) as the start number is
initialized to 0 be default.

```C++
#include <primesieve.hpp>
#include <iostream>

int main()
{
  // generate primes < 1000
  primesieve::iterator it(1000);
  uint64_t prime = it.prev_prime();

  // iterate over primes from 1000 to 0
  for (; prime > 0;  prime = it.prev_prime())
    std::cout << prime << std::endl;

  return 0;
}
```

* [Build instructions](#how-to-compile)

## ```primesieve::generate_primes()```

Stores the primes inside [start, stop] in a ```std::vector```. If you are iterating over the same primes
many times in a loop you will likely get better performance if you store the primes in a vector
instead of using a ```primesieve::iterator``` (provided your system has enough memory).

```C++
#include <primesieve.hpp>
#include <vector>

int main()
{
  std::vector<int> primes;

  // Store primes <= 1000
  primesieve::generate_primes(1000, &primes);

  primes.clear();

  // Store primes inside [1000, 2000]
  primesieve::generate_primes(1000, 2000, &primes);

  return 0;
}
```

* [Build instructions](#how-to-compile)

## ```primesieve::generate_n_primes()```

Stores n primes in a ```std::vector```.

```C++
#include <primesieve.hpp>
#include <vector>

int main()
{
  std::vector<int> primes;

  // Store first 1000 primes
  primesieve::generate_n_primes(1000, &primes);

  primes.clear();

  // Store first 10 primes >= 1000
  primesieve::generate_n_primes(10, 1000, &primes);

  return 0;
}
```

* [Build instructions](#how-to-compile)

## ```primesieve::count_primes()```

Counts the primes inside [start, stop]. This method is multi-threaded and uses all
available CPU cores by default.

```C++
#include <primesieve.hpp>
#include <iostream>

int main()
{
  uint64_t count = primesieve::count_primes(0, 1000);
  std::cout << "Primes below 1000 = " << count << std::endl;

  return 0;
}
```

* [Build instructions](#how-to-compile)

## ```primesieve::nth_prime()```

This method finds the nth prime e.g. ```nth_prime(25) = 97```. This method is
multi-threaded and uses all available CPU cores by default.

```C++
#include <primesieve.hpp>
#include <iostream>

int main()
{
  uint64_t n = 25;
  uint64_t nth_prime = primesieve::nth_prime(n);
  std::cout << n << "th prime = " << nth_prime << std::endl;

  return 0;
}
```

* [Build instructions](#how-to-compile)

# How to compile

### Unix-like OSes

If [libprimesieve is installed](https://github.com/kimwalisch/primesieve#installation)
on your system, then you can compile any of the C++ example programs above using:

```sh
c++ -O2 primes.cpp -o primes -lprimesieve
```

If you have [built libprimesieve yourself](BUILD.md#primesieve-build-instructions)
then the default installation path is usually ```/usr/local/lib``` which is not
part of ```LD_LIBRARY_PATH``` on many OSes. Hence you may need to export some
environment variables:

```sh
export LIBRARY_PATH=/usr/local/lib:$LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
export CPLUS_INCLUDE_PATH=/usr/local/include:$CPLUS_INCLUDE_PATH
```

### Microsoft Visual C++

```sh
cl /O2 /EHsc /MD primes.cpp /I "path\to\primesieve\include" /link "path\to\primesieve.lib"
```

# CMake support

If you are using the CMake build system to compile your program and libprimesieve has been
[installed](https://github.com/kimwalisch/primesieve#installation) on your system, then you
can add the following two lines to your ```CMakeLists.txt``` to link your program against
libprimesieve.

```CMake
find_package(primesieve REQUIRED)
target_link_libraries(your_program primesieve::primesieve)
```

To link against the static libprimesieve use:

```CMake
find_package(primesieve REQUIRED static)
target_link_libraries(your_program primesieve::primesieve)
```

# Minimal CMake project file

If you wish to build your C++ program (named ```primes.cpp```) using CMake, then you can use
the minimal ```CMakeLists.txt``` below. Note that this requires that libprimesieve has been
[installed](https://github.com/kimwalisch/primesieve#installation) on your system. Using
CMake has the advantage that you don't need to specify the libprimesieve include path and
the ```-lprimesieve``` linker option when building your project.

```CMake
# File: CMakeLists.txt
cmake_minimum_required(VERSION 3.4...3.19)
project(primes CXX)
find_package(primesieve REQUIRED)
add_executable(primes primes.cpp)
target_link_libraries(primes primesieve::primesieve)
```

Put the ```CMakeLists.txt``` file from above into the same directory as your ```primes.cpp``` file.<br/>
Then open a terminal, cd into that directory and build your project using:

```
cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Using the MSVC compiler (Windows) the build instructions are slightly different. First you should link
against the static libprimesieve in your ```CMakeLists.txt``` using:
```find_package(primesieve REQUIRED static)```. Next open a Visual Studio Command Prompt, cd into your
project's directory and build your project using:

```
cmake -G "Visual Studio 16 2019" .
cmake --build . --config Release
```
