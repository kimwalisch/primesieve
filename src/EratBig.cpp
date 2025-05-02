///
/// @file   EratBig.cpp
/// @brief  EratBig is a segmented sieve of Eratosthenes algorithm
///         optimized for big sieving primes that have very few
///         multiple occurrences per segment. EratBig is based on
///         Tomas Oliveira e Silva's cache-friendly bucket sieve algorithm:
///         http://www.ieeta.pt/~tos/software/prime_sieve.html
///         The idea is that for each segment we keep a list of
///         buckets which contain the sieving primes that have
///         multiple occurrence(s) in that segment. When we then cross
///         off the multiples from the current segment we avoid
///         processing sieving primes that do not have a multiple
///         occurrence in the current segment.
///
///         This algorithm is also very good at avoiding branch
///         mispredictions. Unlike the EratSmall and EratMedium
///         algorithms, in EratBig there is no branch misprediction
///         after the last multiple of each sieving prime is removed
///         from the sieve array.
///
/// Copyright (C) 2025 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "EratBig.hpp"
#include "Bucket.hpp"
#include "MemoryPool.hpp"

#include <primesieve/bits.hpp>
#include <primesieve/macros.hpp>
#include <primesieve/pmath.hpp>

#include <stdint.h>
#include <algorithm>

namespace {

/// The WheelElement data structure is used to skip multiples
/// of small primes using wheel factorization.
///
struct WheelElement
{
  /// Bitmask used to unset the bit corresponding to the
  /// current multiple of a SievingPrime object.
  uint8_t unsetBit;
  /// Factor used to calculate the next multiple of a sieving
  /// prime that is not divisible by any of the wheel factors.
  uint8_t nextMultipleFactor;
  /// Overflow needed to correct the next multiple index
  /// (due to sievingPrime = prime / 30)
  uint8_t correct;
  /// Used to get the next wheel index:
  /// wheelIndex = next;
  uint32_t next;
};

// The compiler must insert 1 byte of padding to WheelElement
// before the uint32_t WheelElement::next variable so that
// this variable is properly aligned to a 4-byte boundary and
// sizeof(WheelElement) is a power of 2. This improves
// performance by up to 15%.
static_assert(isPow2(sizeof(WheelElement)),
              "sizeof(WheelElement) must be a power of 2!");

/// Used to skip multiples of 2, 3, 5 and 7
const primesieve::Array<WheelElement, 8*48> wheel210 =
{{
  { BIT0, 10,  2,   1 }, { BIT3,  2,  0,   2 }, { BIT7,  4,  1,   3 }, { BIT6,  2,  1,   4 }, { BIT2,  4,  1,   5 }, { BIT1,  6,  1,   6 }, { BIT5,  2,  1,   7 }, { BIT0,  6,  1,   8 },
  { BIT4,  4,  1,   9 }, { BIT3,  2,  0,  10 }, { BIT7,  4,  1,  11 }, { BIT6,  6,  2,  12 }, { BIT1,  6,  1,  13 }, { BIT5,  2,  1,  14 }, { BIT0,  6,  1,  15 }, { BIT4,  4,  1,  16 },
  { BIT3,  2,  0,  17 }, { BIT7,  6,  2,  18 }, { BIT2,  4,  1,  19 }, { BIT1,  6,  1,  20 }, { BIT5,  8,  2,  21 }, { BIT4,  4,  1,  22 }, { BIT3,  2,  0,  23 }, { BIT7,  4,  1,  24 },
  { BIT6,  2,  1,  25 }, { BIT2,  4,  1,  26 }, { BIT1,  8,  2,  27 }, { BIT0,  6,  1,  28 }, { BIT4,  4,  1,  29 }, { BIT3,  6,  1,  30 }, { BIT6,  2,  1,  31 }, { BIT2,  4,  1,  32 },
  { BIT1,  6,  1,  33 }, { BIT5,  2,  1,  34 }, { BIT0,  6,  1,  35 }, { BIT4,  6,  1,  36 }, { BIT7,  4,  1,  37 }, { BIT6,  2,  1,  38 }, { BIT2,  4,  1,  39 }, { BIT1,  6,  1,  40 },
  { BIT5,  2,  1,  41 }, { BIT0,  6,  1,  42 }, { BIT4,  4,  1,  43 }, { BIT3,  2,  0,  44 }, { BIT7,  4,  1,  45 }, { BIT6,  2,  1,  46 }, { BIT2, 10,  2,  47 }, { BIT5,  2,  1,   0 },
  { BIT1, 10,  3,  49 }, { BIT7,  2,  1,  50 }, { BIT5,  4,  2,  51 }, { BIT0,  2,  0,  52 }, { BIT6,  4,  2,  53 }, { BIT2,  6,  2,  54 }, { BIT4,  2,  1,  55 }, { BIT1,  6,  2,  56 },
  { BIT3,  4,  1,  57 }, { BIT7,  2,  1,  58 }, { BIT5,  4,  2,  59 }, { BIT0,  6,  2,  60 }, { BIT2,  6,  2,  61 }, { BIT4,  2,  1,  62 }, { BIT1,  6,  2,  63 }, { BIT3,  4,  1,  64 },
  { BIT7,  2,  1,  65 }, { BIT5,  6,  2,  66 }, { BIT6,  4,  2,  67 }, { BIT2,  6,  2,  68 }, { BIT4,  8,  3,  69 }, { BIT3,  4,  1,  70 }, { BIT7,  2,  1,  71 }, { BIT5,  4,  2,  72 },
  { BIT0,  2,  0,  73 }, { BIT6,  4,  2,  74 }, { BIT2,  8,  3,  75 }, { BIT1,  6,  2,  76 }, { BIT3,  4,  1,  77 }, { BIT7,  6,  3,  78 }, { BIT0,  2,  0,  79 }, { BIT6,  4,  2,  80 },
  { BIT2,  6,  2,  81 }, { BIT4,  2,  1,  82 }, { BIT1,  6,  2,  83 }, { BIT3,  6,  2,  84 }, { BIT5,  4,  2,  85 }, { BIT0,  2,  0,  86 }, { BIT6,  4,  2,  87 }, { BIT2,  6,  2,  88 },
  { BIT4,  2,  1,  89 }, { BIT1,  6,  2,  90 }, { BIT3,  4,  1,  91 }, { BIT7,  2,  1,  92 }, { BIT5,  4,  2,  93 }, { BIT0,  2,  0,  94 }, { BIT6, 10,  4,  95 }, { BIT4,  2,  1,  48 },
  { BIT2, 10,  4,  97 }, { BIT5,  2,  1,  98 }, { BIT4,  4,  2,  99 }, { BIT1,  2,  1, 100 }, { BIT0,  4,  1, 101 }, { BIT6,  6,  3, 102 }, { BIT3,  2,  1, 103 }, { BIT2,  6,  2, 104 },
  { BIT7,  4,  2, 105 }, { BIT5,  2,  1, 106 }, { BIT4,  4,  2, 107 }, { BIT1,  6,  2, 108 }, { BIT6,  6,  3, 109 }, { BIT3,  2,  1, 110 }, { BIT2,  6,  2, 111 }, { BIT7,  4,  2, 112 },
  { BIT5,  2,  1, 113 }, { BIT4,  6,  3, 114 }, { BIT0,  4,  1, 115 }, { BIT6,  6,  3, 116 }, { BIT3,  8,  3, 117 }, { BIT7,  4,  2, 118 }, { BIT5,  2,  1, 119 }, { BIT4,  4,  2, 120 },
  { BIT1,  2,  1, 121 }, { BIT0,  4,  1, 122 }, { BIT6,  8,  4, 123 }, { BIT2,  6,  2, 124 }, { BIT7,  4,  2, 125 }, { BIT5,  6,  3, 126 }, { BIT1,  2,  1, 127 }, { BIT0,  4,  1, 128 },
  { BIT6,  6,  3, 129 }, { BIT3,  2,  1, 130 }, { BIT2,  6,  2, 131 }, { BIT7,  6,  3, 132 }, { BIT4,  4,  2, 133 }, { BIT1,  2,  1, 134 }, { BIT0,  4,  1, 135 }, { BIT6,  6,  3, 136 },
  { BIT3,  2,  1, 137 }, { BIT2,  6,  2, 138 }, { BIT7,  4,  2, 139 }, { BIT5,  2,  1, 140 }, { BIT4,  4,  2, 141 }, { BIT1,  2,  1, 142 }, { BIT0, 10,  4, 143 }, { BIT3,  2,  1,  96 },
  { BIT3, 10,  6, 145 }, { BIT0,  2,  1, 146 }, { BIT1,  4,  2, 147 }, { BIT4,  2,  1, 148 }, { BIT5,  4,  2, 149 }, { BIT7,  6,  4, 150 }, { BIT2,  2,  1, 151 }, { BIT3,  6,  3, 152 },
  { BIT6,  4,  3, 153 }, { BIT0,  2,  1, 154 }, { BIT1,  4,  2, 155 }, { BIT4,  6,  3, 156 }, { BIT7,  6,  4, 157 }, { BIT2,  2,  1, 158 }, { BIT3,  6,  3, 159 }, { BIT6,  4,  3, 160 },
  { BIT0,  2,  1, 161 }, { BIT1,  6,  3, 162 }, { BIT5,  4,  2, 163 }, { BIT7,  6,  4, 164 }, { BIT2,  8,  4, 165 }, { BIT6,  4,  3, 166 }, { BIT0,  2,  1, 167 }, { BIT1,  4,  2, 168 },
  { BIT4,  2,  1, 169 }, { BIT5,  4,  2, 170 }, { BIT7,  8,  5, 171 }, { BIT3,  6,  3, 172 }, { BIT6,  4,  3, 173 }, { BIT0,  6,  3, 174 }, { BIT4,  2,  1, 175 }, { BIT5,  4,  2, 176 },
  { BIT7,  6,  4, 177 }, { BIT2,  2,  1, 178 }, { BIT3,  6,  3, 179 }, { BIT6,  6,  4, 180 }, { BIT1,  4,  2, 181 }, { BIT4,  2,  1, 182 }, { BIT5,  4,  2, 183 }, { BIT7,  6,  4, 184 },
  { BIT2,  2,  1, 185 }, { BIT3,  6,  3, 186 }, { BIT6,  4,  3, 187 }, { BIT0,  2,  1, 188 }, { BIT1,  4,  2, 189 }, { BIT4,  2,  1, 190 }, { BIT5, 10,  6, 191 }, { BIT2,  2,  1, 144 },
  { BIT4, 10,  6, 193 }, { BIT6,  2,  2, 194 }, { BIT0,  4,  2, 195 }, { BIT5,  2,  1, 196 }, { BIT7,  4,  3, 197 }, { BIT3,  6,  4, 198 }, { BIT1,  2,  1, 199 }, { BIT4,  6,  4, 200 },
  { BIT2,  4,  2, 201 }, { BIT6,  2,  2, 202 }, { BIT0,  4,  2, 203 }, { BIT5,  6,  4, 204 }, { BIT3,  6,  4, 205 }, { BIT1,  2,  1, 206 }, { BIT4,  6,  4, 207 }, { BIT2,  4,  2, 208 },
  { BIT6,  2,  2, 209 }, { BIT0,  6,  3, 210 }, { BIT7,  4,  3, 211 }, { BIT3,  6,  4, 212 }, { BIT1,  8,  5, 213 }, { BIT2,  4,  2, 214 }, { BIT6,  2,  2, 215 }, { BIT0,  4,  2, 216 },
  { BIT5,  2,  1, 217 }, { BIT7,  4,  3, 218 }, { BIT3,  8,  5, 219 }, { BIT4,  6,  4, 220 }, { BIT2,  4,  2, 221 }, { BIT6,  6,  4, 222 }, { BIT5,  2,  1, 223 }, { BIT7,  4,  3, 224 },
  { BIT3,  6,  4, 225 }, { BIT1,  2,  1, 226 }, { BIT4,  6,  4, 227 }, { BIT2,  6,  4, 228 }, { BIT0,  4,  2, 229 }, { BIT5,  2,  1, 230 }, { BIT7,  4,  3, 231 }, { BIT3,  6,  4, 232 },
  { BIT1,  2,  1, 233 }, { BIT4,  6,  4, 234 }, { BIT2,  4,  2, 235 }, { BIT6,  2,  2, 236 }, { BIT0,  4,  2, 237 }, { BIT5,  2,  1, 238 }, { BIT7, 10,  7, 239 }, { BIT1,  2,  1, 192 },
  { BIT5, 10,  8, 241 }, { BIT2,  2,  1, 242 }, { BIT6,  4,  3, 243 }, { BIT7,  2,  2, 244 }, { BIT3,  4,  3, 245 }, { BIT4,  6,  5, 246 }, { BIT0,  2,  1, 247 }, { BIT5,  6,  5, 248 },
  { BIT1,  4,  3, 249 }, { BIT2,  2,  1, 250 }, { BIT6,  4,  3, 251 }, { BIT7,  6,  5, 252 }, { BIT4,  6,  5, 253 }, { BIT0,  2,  1, 254 }, { BIT5,  6,  5, 255 }, { BIT1,  4,  3, 256 },
  { BIT2,  2,  1, 257 }, { BIT6,  6,  5, 258 }, { BIT3,  4,  3, 259 }, { BIT4,  6,  5, 260 }, { BIT0,  8,  6, 261 }, { BIT1,  4,  3, 262 }, { BIT2,  2,  1, 263 }, { BIT6,  4,  3, 264 },
  { BIT7,  2,  2, 265 }, { BIT3,  4,  3, 266 }, { BIT4,  8,  6, 267 }, { BIT5,  6,  5, 268 }, { BIT1,  4,  3, 269 }, { BIT2,  6,  4, 270 }, { BIT7,  2,  2, 271 }, { BIT3,  4,  3, 272 },
  { BIT4,  6,  5, 273 }, { BIT0,  2,  1, 274 }, { BIT5,  6,  5, 275 }, { BIT1,  6,  4, 276 }, { BIT6,  4,  3, 277 }, { BIT7,  2,  2, 278 }, { BIT3,  4,  3, 279 }, { BIT4,  6,  5, 280 },
  { BIT0,  2,  1, 281 }, { BIT5,  6,  5, 282 }, { BIT1,  4,  3, 283 }, { BIT2,  2,  1, 284 }, { BIT6,  4,  3, 285 }, { BIT7,  2,  2, 286 }, { BIT3, 10,  8, 287 }, { BIT0,  2,  1, 240 },
  { BIT6, 10, 10, 289 }, { BIT4,  2,  2, 290 }, { BIT3,  4,  4, 291 }, { BIT2,  2,  2, 292 }, { BIT1,  4,  4, 293 }, { BIT0,  6,  5, 294 }, { BIT7,  2,  2, 295 }, { BIT6,  6,  6, 296 },
  { BIT5,  4,  4, 297 }, { BIT4,  2,  2, 298 }, { BIT3,  4,  4, 299 }, { BIT2,  6,  6, 300 }, { BIT0,  6,  5, 301 }, { BIT7,  2,  2, 302 }, { BIT6,  6,  6, 303 }, { BIT5,  4,  4, 304 },
  { BIT4,  2,  2, 305 }, { BIT3,  6,  6, 306 }, { BIT1,  4,  4, 307 }, { BIT0,  6,  5, 308 }, { BIT7,  8,  8, 309 }, { BIT5,  4,  4, 310 }, { BIT4,  2,  2, 311 }, { BIT3,  4,  4, 312 },
  { BIT2,  2,  2, 313 }, { BIT1,  4,  4, 314 }, { BIT0,  8,  7, 315 }, { BIT6,  6,  6, 316 }, { BIT5,  4,  4, 317 }, { BIT4,  6,  6, 318 }, { BIT2,  2,  2, 319 }, { BIT1,  4,  4, 320 },
  { BIT0,  6,  5, 321 }, { BIT7,  2,  2, 322 }, { BIT6,  6,  6, 323 }, { BIT5,  6,  6, 324 }, { BIT3,  4,  4, 325 }, { BIT2,  2,  2, 326 }, { BIT1,  4,  4, 327 }, { BIT0,  6,  5, 328 },
  { BIT7,  2,  2, 329 }, { BIT6,  6,  6, 330 }, { BIT5,  4,  4, 331 }, { BIT4,  2,  2, 332 }, { BIT3,  4,  4, 333 }, { BIT2,  2,  2, 334 }, { BIT1, 10,  9, 335 }, { BIT7,  2,  2, 288 },
  { BIT7, 10,  1, 337 }, { BIT1,  2,  0, 338 }, { BIT2,  4,  0, 339 }, { BIT3,  2,  0, 340 }, { BIT4,  4,  0, 341 }, { BIT5,  6,  0, 342 }, { BIT6,  2,  0, 343 }, { BIT7,  6,  1, 344 },
  { BIT0,  4,  0, 345 }, { BIT1,  2,  0, 346 }, { BIT2,  4,  0, 347 }, { BIT3,  6,  0, 348 }, { BIT5,  6,  0, 349 }, { BIT6,  2,  0, 350 }, { BIT7,  6,  1, 351 }, { BIT0,  4,  0, 352 },
  { BIT1,  2,  0, 353 }, { BIT2,  6,  0, 354 }, { BIT4,  4,  0, 355 }, { BIT5,  6,  0, 356 }, { BIT6,  8,  1, 357 }, { BIT0,  4,  0, 358 }, { BIT1,  2,  0, 359 }, { BIT2,  4,  0, 360 },
  { BIT3,  2,  0, 361 }, { BIT4,  4,  0, 362 }, { BIT5,  8,  0, 363 }, { BIT7,  6,  1, 364 }, { BIT0,  4,  0, 365 }, { BIT1,  6,  0, 366 }, { BIT3,  2,  0, 367 }, { BIT4,  4,  0, 368 },
  { BIT5,  6,  0, 369 }, { BIT6,  2,  0, 370 }, { BIT7,  6,  1, 371 }, { BIT0,  6,  0, 372 }, { BIT2,  4,  0, 373 }, { BIT3,  2,  0, 374 }, { BIT4,  4,  0, 375 }, { BIT5,  6,  0, 376 },
  { BIT6,  2,  0, 377 }, { BIT7,  6,  1, 378 }, { BIT0,  4,  0, 379 }, { BIT1,  2,  0, 380 }, { BIT2,  4,  0, 381 }, { BIT3,  2,  0, 382 }, { BIT4, 10,  0, 383 }, { BIT6,  2,  0, 336 }
}};

} // namespace

namespace primesieve {

/// @stop:      Upper bound for sieving
/// @sieveSize: Sieve size in bytes
/// @maxPrime:  Sieving primes <= maxPrime
///
void EratBig::init(uint64_t stop,
                   uint64_t sieveSize,
                   uint64_t maxPrime,
                   MemoryPool& memoryPool)
{
  // '>> log2SieveSize' requires power of 2 sieveSize
  ASSERT(isPow2(sieveSize));
  ASSERT(sieveSize <= SievingPrime::MAX_MULTIPLEINDEX + 1);

  stop_ = stop;
  maxPrime_ = maxPrime;
  log2SieveSize_ = ilog2(sieveSize);
  moduloSieveSize_ = sieveSize - 1;
  memoryPool_ = &memoryPool;

  uint64_t maxSievingPrime = maxPrime_ / 30;
  uint64_t maxNextMultiple = maxSievingPrime * getMaxFactor() + getMaxFactor();
  uint64_t maxMultipleIndex = sieveSize - 1 + maxNextMultiple;
  uint64_t maxSegmentIndex = maxMultipleIndex >> log2SieveSize_;
  uint64_t maxSize = maxSegmentIndex + 1;
  buckets_.reserve(maxSize);
}

/// Add a new sieving prime
void EratBig::storeSievingPrime(uint64_t prime,
                                uint64_t multipleIndex,
                                uint64_t wheelIndex)
{
  uint64_t sieveSize = 1ull << log2SieveSize_;
  uint64_t sievingPrime = prime / 30;
  uint64_t maxNextMultiple = sievingPrime * getMaxFactor() + getMaxFactor();
  uint64_t maxMultipleIndex = sieveSize - 1 + maxNextMultiple;
  uint64_t maxSegmentIndex = maxMultipleIndex >> log2SieveSize_;
  uint64_t newSize = maxSegmentIndex + 1;
  uint64_t segment = multipleIndex >> log2SieveSize_;
  multipleIndex &= moduloSieveSize_;

  while (buckets_.size() < newSize)
    buckets_.push_back(nullptr);

  ASSERT(prime <= maxPrime_);
  ASSERT(segment < buckets_.size());

  if (Bucket::isFull(buckets_[segment]))
    memoryPool_->addBucket(buckets_[segment]);

  buckets_[segment]++->set(sievingPrime, multipleIndex, wheelIndex);
}

void EratBig::crossOff(Vector<uint8_t>& sieve)
{
  while (buckets_[0])
  {
    Bucket* bucket = Bucket::get(buckets_[0]);
    bucket->setEnd(buckets_[0]);
    buckets_[0] = nullptr;

    // Iterate over the buckets related
    // to the current segment.
    while (bucket)
    {
      crossOff(sieve.data(), bucket->begin(), bucket->end());
      Bucket* processed = bucket;
      bucket = bucket->next();
      memoryPool_->freeBucket(processed);
    }
  }

  // Move the bucket related to the next segment
  // to the 1st position so that it will be used
  // when sieving the next segment.
  auto* bucket = buckets_[0];
  std::copy(buckets_.begin() + 1, buckets_.end(), buckets_.begin());
  buckets_.back() = bucket;
}

/// Removes the next multiple of each sieving prime from the
/// sieve array. After the next multiple of a sieving prime
/// has been removed we calculate its next multiple and
/// determine in which segment that multiple will occur. Then
/// we move the sieving prime to the bucket list related to
/// the previously computed segment.
///
void EratBig::crossOff(uint8_t* sieve,
                       SievingPrime* prime,
                       SievingPrime* end)
{
  auto buckets = buckets_.data();
  MemoryPool& memoryPool = *memoryPool_;
  std::size_t moduloSieveSize = moduloSieveSize_;
  std::size_t log2SieveSize = log2SieveSize_;

  for (; prime != end; prime++)
  {
    std::size_t multipleIndex = prime->getMultipleIndex();
    std::size_t wheelIndex    = prime->getWheelIndex();
    std::size_t sievingPrime  = prime->getSievingPrime();

    // Cross-off the current multiple (unset bit)
    // and calculate the next multiple.
    sieve[multipleIndex] &= wheel210[wheelIndex].unsetBit;
    multipleIndex += wheel210[wheelIndex].nextMultipleFactor * sievingPrime;
    multipleIndex += wheel210[wheelIndex].correct;
    wheelIndex = wheel210[wheelIndex].next;
    std::size_t segment = multipleIndex >> log2SieveSize;
    multipleIndex &= moduloSieveSize;

    if (Bucket::isFull(buckets[segment]))
      memoryPool.addBucket(buckets[segment]);

    buckets[segment]++->set(sievingPrime, multipleIndex, wheelIndex);
  }
}

} // namespace
