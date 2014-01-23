/// @file   segmented_bit_sieve.cpp
/// @author Kim Walisch, <kim.walisch@gmail.com> 
/// @brief  This is an implementation of the segmented sieve of
///         Eratosthenes which uses a bit array with 16 numbers per
///         byte. It generates the primes below 10^10 in 8.21 seconds
///         on an Intel Core i7-4770 CPU (3.4 GHz) from 2013.
///         This is free software released into the public domain.

#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <stdint.h>

const int L1D_CACHE_SIZE = 32768;

/// Bitmasks needed to unset bits corresponding to multiples
const int unset_bit[16] =
{
  ~(1 << 0), ~(1 << 0),
  ~(1 << 1), ~(1 << 1),
  ~(1 << 2), ~(1 << 2),
  ~(1 << 3), ~(1 << 3),
  ~(1 << 4), ~(1 << 4),
  ~(1 << 5), ~(1 << 5),
  ~(1 << 6), ~(1 << 6),
  ~(1 << 7), ~(1 << 7)
};

const int popcnt[256] =
{
  0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

/// Generate primes using the segmented sieve of Eratosthenes.
/// This implementation uses a bit array with 16 numbers per byte.
/// @param limit       Sieve primes <= limit.
/// @param sieve_size  Size of the sieve array in bytes.
///
void segmented_bit_sieve(int64_t limit, int sieve_size = L1D_CACHE_SIZE)
{
  int segment_size = sieve_size * 16;
  int sqrt = (int) std::sqrt((double) limit);
  int64_t count = (limit == 1) ? -1 : 0;
  int64_t s = 2;
  int64_t n = 0;

  // vector used for sieving
  std::vector<uint8_t> sieve(sieve_size);

  // generate small primes <= sqrt
  std::vector<bool> is_prime(sqrt + 1, true);
  for (int i = 2; i * i <= sqrt; i++)
    if (is_prime[i])
      for (int j = i * i; j <= sqrt; j += i)
        is_prime[j] = false;

  std::vector<int> primes;
  std::vector<int> next;

  for (int64_t low = 0; low <= limit; low += segment_size)
  {
    std::fill(sieve.begin(), sieve.end(), 0xff);

    // unset bits > limit
    if (low + segment_size > limit)
      sieve[(limit + 1 - low) >> 4] &= ~(0xff << ((limit + 1) % 16 / 2));

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
    // segmented sieve of Eratosthenes
    for (std::size_t i = 1; i < primes.size(); i++)
    {
      int j = next[i];
      for (int k = primes[i] * 2; j < segment_size; j += k)
        sieve[j >> 4] &= unset_bit[j & 15];
      next[i] = j - segment_size;
    }

    for (; n <= high; n += 1 << 4)
      count += popcnt[sieve[(n - low) >> 4]];
  }

  std::cout << count << " primes found." << std::endl;
}

/// Usage: ./segmented_bit_sieve n size
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

  segmented_bit_sieve(limit, size);
  return 0;
}
