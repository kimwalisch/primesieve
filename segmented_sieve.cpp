/// @file   segmented_sieve.cpp
/// @author Kim Walisch, <kim.walisch@gmail.com> 
/// @brief  This is a simple implementation of the segmented sieve of
///         Eratosthenes with a few optimizations. It generates the
///         primes below 10^9 in 0.9 seconds (single-threaded) on an
///         Intel Core i7-4770 CPU (3.4 GHz) from 2013.
///         This is free software released into the public domain.

#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <stdint.h>

const int L1D_CACHE_SIZE = 32768;

/// Generate primes using the segmented sieve of Eratosthenes.
/// This algorithm uses O(n log log n) operations and O(sqrt(n)) space.
/// @param limit         Sieve primes <= limit.
/// @param segment_size  Size of the sieve array in bytes.
///
void segmented_sieve(int64_t limit, int segment_size = L1D_CACHE_SIZE)
{
  int sqrt = (int) std::sqrt((double) limit);
  int64_t count = (limit < 2) ? 0 : 1;
  int64_t s = 2;
  int64_t n = 3;

  // vector used for sieving
  std::vector<char> sieve(segment_size);

  // generate small primes <= sqrt
  std::vector<char> is_prime(sqrt + 1, 1);
  for (int i = 2; i * i <= sqrt; i++)
    if (is_prime[i])
      for (int j = i * i; j <= sqrt; j += i)
        is_prime[j] = 0;

  std::vector<int> primes;
  std::vector<int> next;

  for (int64_t low = 0; low <= limit; low += segment_size)
  {
    std::fill(sieve.begin(), sieve.end(), 1);

    // current segment = interval [low, high]
    int64_t high = std::min(low + segment_size - 1, limit);

    // store small primes needed to cross off multiples
    for (; s * s <= high; s++)
    {
      if (is_prime[s])
      {
        primes.push_back((int) s);
          next.push_back((int)(s * s - low));
      }
    }
    // sieve the current segment
    for (std::size_t i = 1; i < primes.size(); i++)
    {
      int j = next[i];
      for (int k = primes[i] * 2; j < segment_size; j += k)
        sieve[j] = 0;
      next[i] = j - segment_size;
    }

    for (; n <= high; n += 2)
      if (sieve[n - low]) // n is a prime
        count++;
  }

  std::cout << count << " primes found." << std::endl;
}

/// Usage: ./segmented_sieve n size
/// @param n     Sieve the primes up to n.
/// @param size  Size of the sieve array in bytes.
///
int main(int argc, char** argv)
{
  // generate the primes below this number
  int64_t limit = 100000000;
  if (argc >= 2)
    limit = atol(argv[1]);

  int size = L1D_CACHE_SIZE;
  if (argc >= 3)
    size = atoi(argv[2]);

  segmented_sieve(limit, size);
  return 0;
}
