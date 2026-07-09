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
/// Copyright (C) 2026 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "EratBig.hpp"
#include "Bucket.hpp"
#include "MemoryPool.hpp"

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
  { 0xfe, 10,  2,   1 }, { 0xf7,  2,  0,   2 }, { 0x7f,  4,  1,   3 }, { 0xbf,  2,  1,   4 }, { 0xfb,  4,  1,   5 }, { 0xfd,  6,  1,   6 }, { 0xdf,  2,  1,   7 }, { 0xfe,  6,  1,   8 },
  { 0xef,  4,  1,   9 }, { 0xf7,  2,  0,  10 }, { 0x7f,  4,  1,  11 }, { 0xbf,  6,  2,  12 }, { 0xfd,  6,  1,  13 }, { 0xdf,  2,  1,  14 }, { 0xfe,  6,  1,  15 }, { 0xef,  4,  1,  16 },
  { 0xf7,  2,  0,  17 }, { 0x7f,  6,  2,  18 }, { 0xfb,  4,  1,  19 }, { 0xfd,  6,  1,  20 }, { 0xdf,  8,  2,  21 }, { 0xef,  4,  1,  22 }, { 0xf7,  2,  0,  23 }, { 0x7f,  4,  1,  24 },
  { 0xbf,  2,  1,  25 }, { 0xfb,  4,  1,  26 }, { 0xfd,  8,  2,  27 }, { 0xfe,  6,  1,  28 }, { 0xef,  4,  1,  29 }, { 0xf7,  6,  1,  30 }, { 0xbf,  2,  1,  31 }, { 0xfb,  4,  1,  32 },
  { 0xfd,  6,  1,  33 }, { 0xdf,  2,  1,  34 }, { 0xfe,  6,  1,  35 }, { 0xef,  6,  1,  36 }, { 0x7f,  4,  1,  37 }, { 0xbf,  2,  1,  38 }, { 0xfb,  4,  1,  39 }, { 0xfd,  6,  1,  40 },
  { 0xdf,  2,  1,  41 }, { 0xfe,  6,  1,  42 }, { 0xef,  4,  1,  43 }, { 0xf7,  2,  0,  44 }, { 0x7f,  4,  1,  45 }, { 0xbf,  2,  1,  46 }, { 0xfb, 10,  2,  47 }, { 0xdf,  2,  1,   0 },
  { 0xfd, 10,  3,  49 }, { 0x7f,  2,  1,  50 }, { 0xdf,  4,  2,  51 }, { 0xfe,  2,  0,  52 }, { 0xbf,  4,  2,  53 }, { 0xfb,  6,  2,  54 }, { 0xef,  2,  1,  55 }, { 0xfd,  6,  2,  56 },
  { 0xf7,  4,  1,  57 }, { 0x7f,  2,  1,  58 }, { 0xdf,  4,  2,  59 }, { 0xfe,  6,  2,  60 }, { 0xfb,  6,  2,  61 }, { 0xef,  2,  1,  62 }, { 0xfd,  6,  2,  63 }, { 0xf7,  4,  1,  64 },
  { 0x7f,  2,  1,  65 }, { 0xdf,  6,  2,  66 }, { 0xbf,  4,  2,  67 }, { 0xfb,  6,  2,  68 }, { 0xef,  8,  3,  69 }, { 0xf7,  4,  1,  70 }, { 0x7f,  2,  1,  71 }, { 0xdf,  4,  2,  72 },
  { 0xfe,  2,  0,  73 }, { 0xbf,  4,  2,  74 }, { 0xfb,  8,  3,  75 }, { 0xfd,  6,  2,  76 }, { 0xf7,  4,  1,  77 }, { 0x7f,  6,  3,  78 }, { 0xfe,  2,  0,  79 }, { 0xbf,  4,  2,  80 },
  { 0xfb,  6,  2,  81 }, { 0xef,  2,  1,  82 }, { 0xfd,  6,  2,  83 }, { 0xf7,  6,  2,  84 }, { 0xdf,  4,  2,  85 }, { 0xfe,  2,  0,  86 }, { 0xbf,  4,  2,  87 }, { 0xfb,  6,  2,  88 },
  { 0xef,  2,  1,  89 }, { 0xfd,  6,  2,  90 }, { 0xf7,  4,  1,  91 }, { 0x7f,  2,  1,  92 }, { 0xdf,  4,  2,  93 }, { 0xfe,  2,  0,  94 }, { 0xbf, 10,  4,  95 }, { 0xef,  2,  1,  48 },
  { 0xfb, 10,  4,  97 }, { 0xdf,  2,  1,  98 }, { 0xef,  4,  2,  99 }, { 0xfd,  2,  1, 100 }, { 0xfe,  4,  1, 101 }, { 0xbf,  6,  3, 102 }, { 0xf7,  2,  1, 103 }, { 0xfb,  6,  2, 104 },
  { 0x7f,  4,  2, 105 }, { 0xdf,  2,  1, 106 }, { 0xef,  4,  2, 107 }, { 0xfd,  6,  2, 108 }, { 0xbf,  6,  3, 109 }, { 0xf7,  2,  1, 110 }, { 0xfb,  6,  2, 111 }, { 0x7f,  4,  2, 112 },
  { 0xdf,  2,  1, 113 }, { 0xef,  6,  3, 114 }, { 0xfe,  4,  1, 115 }, { 0xbf,  6,  3, 116 }, { 0xf7,  8,  3, 117 }, { 0x7f,  4,  2, 118 }, { 0xdf,  2,  1, 119 }, { 0xef,  4,  2, 120 },
  { 0xfd,  2,  1, 121 }, { 0xfe,  4,  1, 122 }, { 0xbf,  8,  4, 123 }, { 0xfb,  6,  2, 124 }, { 0x7f,  4,  2, 125 }, { 0xdf,  6,  3, 126 }, { 0xfd,  2,  1, 127 }, { 0xfe,  4,  1, 128 },
  { 0xbf,  6,  3, 129 }, { 0xf7,  2,  1, 130 }, { 0xfb,  6,  2, 131 }, { 0x7f,  6,  3, 132 }, { 0xef,  4,  2, 133 }, { 0xfd,  2,  1, 134 }, { 0xfe,  4,  1, 135 }, { 0xbf,  6,  3, 136 },
  { 0xf7,  2,  1, 137 }, { 0xfb,  6,  2, 138 }, { 0x7f,  4,  2, 139 }, { 0xdf,  2,  1, 140 }, { 0xef,  4,  2, 141 }, { 0xfd,  2,  1, 142 }, { 0xfe, 10,  4, 143 }, { 0xf7,  2,  1,  96 },
  { 0xf7, 10,  6, 145 }, { 0xfe,  2,  1, 146 }, { 0xfd,  4,  2, 147 }, { 0xef,  2,  1, 148 }, { 0xdf,  4,  2, 149 }, { 0x7f,  6,  4, 150 }, { 0xfb,  2,  1, 151 }, { 0xf7,  6,  3, 152 },
  { 0xbf,  4,  3, 153 }, { 0xfe,  2,  1, 154 }, { 0xfd,  4,  2, 155 }, { 0xef,  6,  3, 156 }, { 0x7f,  6,  4, 157 }, { 0xfb,  2,  1, 158 }, { 0xf7,  6,  3, 159 }, { 0xbf,  4,  3, 160 },
  { 0xfe,  2,  1, 161 }, { 0xfd,  6,  3, 162 }, { 0xdf,  4,  2, 163 }, { 0x7f,  6,  4, 164 }, { 0xfb,  8,  4, 165 }, { 0xbf,  4,  3, 166 }, { 0xfe,  2,  1, 167 }, { 0xfd,  4,  2, 168 },
  { 0xef,  2,  1, 169 }, { 0xdf,  4,  2, 170 }, { 0x7f,  8,  5, 171 }, { 0xf7,  6,  3, 172 }, { 0xbf,  4,  3, 173 }, { 0xfe,  6,  3, 174 }, { 0xef,  2,  1, 175 }, { 0xdf,  4,  2, 176 },
  { 0x7f,  6,  4, 177 }, { 0xfb,  2,  1, 178 }, { 0xf7,  6,  3, 179 }, { 0xbf,  6,  4, 180 }, { 0xfd,  4,  2, 181 }, { 0xef,  2,  1, 182 }, { 0xdf,  4,  2, 183 }, { 0x7f,  6,  4, 184 },
  { 0xfb,  2,  1, 185 }, { 0xf7,  6,  3, 186 }, { 0xbf,  4,  3, 187 }, { 0xfe,  2,  1, 188 }, { 0xfd,  4,  2, 189 }, { 0xef,  2,  1, 190 }, { 0xdf, 10,  6, 191 }, { 0xfb,  2,  1, 144 },
  { 0xef, 10,  6, 193 }, { 0xbf,  2,  2, 194 }, { 0xfe,  4,  2, 195 }, { 0xdf,  2,  1, 196 }, { 0x7f,  4,  3, 197 }, { 0xf7,  6,  4, 198 }, { 0xfd,  2,  1, 199 }, { 0xef,  6,  4, 200 },
  { 0xfb,  4,  2, 201 }, { 0xbf,  2,  2, 202 }, { 0xfe,  4,  2, 203 }, { 0xdf,  6,  4, 204 }, { 0xf7,  6,  4, 205 }, { 0xfd,  2,  1, 206 }, { 0xef,  6,  4, 207 }, { 0xfb,  4,  2, 208 },
  { 0xbf,  2,  2, 209 }, { 0xfe,  6,  3, 210 }, { 0x7f,  4,  3, 211 }, { 0xf7,  6,  4, 212 }, { 0xfd,  8,  5, 213 }, { 0xfb,  4,  2, 214 }, { 0xbf,  2,  2, 215 }, { 0xfe,  4,  2, 216 },
  { 0xdf,  2,  1, 217 }, { 0x7f,  4,  3, 218 }, { 0xf7,  8,  5, 219 }, { 0xef,  6,  4, 220 }, { 0xfb,  4,  2, 221 }, { 0xbf,  6,  4, 222 }, { 0xdf,  2,  1, 223 }, { 0x7f,  4,  3, 224 },
  { 0xf7,  6,  4, 225 }, { 0xfd,  2,  1, 226 }, { 0xef,  6,  4, 227 }, { 0xfb,  6,  4, 228 }, { 0xfe,  4,  2, 229 }, { 0xdf,  2,  1, 230 }, { 0x7f,  4,  3, 231 }, { 0xf7,  6,  4, 232 },
  { 0xfd,  2,  1, 233 }, { 0xef,  6,  4, 234 }, { 0xfb,  4,  2, 235 }, { 0xbf,  2,  2, 236 }, { 0xfe,  4,  2, 237 }, { 0xdf,  2,  1, 238 }, { 0x7f, 10,  7, 239 }, { 0xfd,  2,  1, 192 },
  { 0xdf, 10,  8, 241 }, { 0xfb,  2,  1, 242 }, { 0xbf,  4,  3, 243 }, { 0x7f,  2,  2, 244 }, { 0xf7,  4,  3, 245 }, { 0xef,  6,  5, 246 }, { 0xfe,  2,  1, 247 }, { 0xdf,  6,  5, 248 },
  { 0xfd,  4,  3, 249 }, { 0xfb,  2,  1, 250 }, { 0xbf,  4,  3, 251 }, { 0x7f,  6,  5, 252 }, { 0xef,  6,  5, 253 }, { 0xfe,  2,  1, 254 }, { 0xdf,  6,  5, 255 }, { 0xfd,  4,  3, 256 },
  { 0xfb,  2,  1, 257 }, { 0xbf,  6,  5, 258 }, { 0xf7,  4,  3, 259 }, { 0xef,  6,  5, 260 }, { 0xfe,  8,  6, 261 }, { 0xfd,  4,  3, 262 }, { 0xfb,  2,  1, 263 }, { 0xbf,  4,  3, 264 },
  { 0x7f,  2,  2, 265 }, { 0xf7,  4,  3, 266 }, { 0xef,  8,  6, 267 }, { 0xdf,  6,  5, 268 }, { 0xfd,  4,  3, 269 }, { 0xfb,  6,  4, 270 }, { 0x7f,  2,  2, 271 }, { 0xf7,  4,  3, 272 },
  { 0xef,  6,  5, 273 }, { 0xfe,  2,  1, 274 }, { 0xdf,  6,  5, 275 }, { 0xfd,  6,  4, 276 }, { 0xbf,  4,  3, 277 }, { 0x7f,  2,  2, 278 }, { 0xf7,  4,  3, 279 }, { 0xef,  6,  5, 280 },
  { 0xfe,  2,  1, 281 }, { 0xdf,  6,  5, 282 }, { 0xfd,  4,  3, 283 }, { 0xfb,  2,  1, 284 }, { 0xbf,  4,  3, 285 }, { 0x7f,  2,  2, 286 }, { 0xf7, 10,  8, 287 }, { 0xfe,  2,  1, 240 },
  { 0xbf, 10, 10, 289 }, { 0xef,  2,  2, 290 }, { 0xf7,  4,  4, 291 }, { 0xfb,  2,  2, 292 }, { 0xfd,  4,  4, 293 }, { 0xfe,  6,  5, 294 }, { 0x7f,  2,  2, 295 }, { 0xbf,  6,  6, 296 },
  { 0xdf,  4,  4, 297 }, { 0xef,  2,  2, 298 }, { 0xf7,  4,  4, 299 }, { 0xfb,  6,  6, 300 }, { 0xfe,  6,  5, 301 }, { 0x7f,  2,  2, 302 }, { 0xbf,  6,  6, 303 }, { 0xdf,  4,  4, 304 },
  { 0xef,  2,  2, 305 }, { 0xf7,  6,  6, 306 }, { 0xfd,  4,  4, 307 }, { 0xfe,  6,  5, 308 }, { 0x7f,  8,  8, 309 }, { 0xdf,  4,  4, 310 }, { 0xef,  2,  2, 311 }, { 0xf7,  4,  4, 312 },
  { 0xfb,  2,  2, 313 }, { 0xfd,  4,  4, 314 }, { 0xfe,  8,  7, 315 }, { 0xbf,  6,  6, 316 }, { 0xdf,  4,  4, 317 }, { 0xef,  6,  6, 318 }, { 0xfb,  2,  2, 319 }, { 0xfd,  4,  4, 320 },
  { 0xfe,  6,  5, 321 }, { 0x7f,  2,  2, 322 }, { 0xbf,  6,  6, 323 }, { 0xdf,  6,  6, 324 }, { 0xf7,  4,  4, 325 }, { 0xfb,  2,  2, 326 }, { 0xfd,  4,  4, 327 }, { 0xfe,  6,  5, 328 },
  { 0x7f,  2,  2, 329 }, { 0xbf,  6,  6, 330 }, { 0xdf,  4,  4, 331 }, { 0xef,  2,  2, 332 }, { 0xf7,  4,  4, 333 }, { 0xfb,  2,  2, 334 }, { 0xfd, 10,  9, 335 }, { 0x7f,  2,  2, 288 },
  { 0x7f, 10,  1, 337 }, { 0xfd,  2,  0, 338 }, { 0xfb,  4,  0, 339 }, { 0xf7,  2,  0, 340 }, { 0xef,  4,  0, 341 }, { 0xdf,  6,  0, 342 }, { 0xbf,  2,  0, 343 }, { 0x7f,  6,  1, 344 },
  { 0xfe,  4,  0, 345 }, { 0xfd,  2,  0, 346 }, { 0xfb,  4,  0, 347 }, { 0xf7,  6,  0, 348 }, { 0xdf,  6,  0, 349 }, { 0xbf,  2,  0, 350 }, { 0x7f,  6,  1, 351 }, { 0xfe,  4,  0, 352 },
  { 0xfd,  2,  0, 353 }, { 0xfb,  6,  0, 354 }, { 0xef,  4,  0, 355 }, { 0xdf,  6,  0, 356 }, { 0xbf,  8,  1, 357 }, { 0xfe,  4,  0, 358 }, { 0xfd,  2,  0, 359 }, { 0xfb,  4,  0, 360 },
  { 0xf7,  2,  0, 361 }, { 0xef,  4,  0, 362 }, { 0xdf,  8,  0, 363 }, { 0x7f,  6,  1, 364 }, { 0xfe,  4,  0, 365 }, { 0xfd,  6,  0, 366 }, { 0xf7,  2,  0, 367 }, { 0xef,  4,  0, 368 },
  { 0xdf,  6,  0, 369 }, { 0xbf,  2,  0, 370 }, { 0x7f,  6,  1, 371 }, { 0xfe,  6,  0, 372 }, { 0xfb,  4,  0, 373 }, { 0xf7,  2,  0, 374 }, { 0xef,  4,  0, 375 }, { 0xdf,  6,  0, 376 },
  { 0xbf,  2,  0, 377 }, { 0x7f,  6,  1, 378 }, { 0xfe,  4,  0, 379 }, { 0xfd,  2,  0, 380 }, { 0xfb,  4,  0, 381 }, { 0xf7,  2,  0, 382 }, { 0xef, 10,  0, 383 }, { 0xbf,  2,  0, 336 }
}};

} // namespace

namespace primesieve {

/// @stop:     Upper bound for sieving
/// @maxPrime: Sieving primes <= maxPrime
///
void EratBig::init(uint64_t stop,
                   uint64_t maxPrime,
                   const Vector<uint64_t>& sieve,
                   MemoryPool& memoryPool)
{
  uint64_t sieveBytes = sieve.size() * sizeof(uint64_t);

  // '>> log2SieveBytes' requires power of 2 sieveBytes
  ASSERT(isPow2(sieveBytes));
  ASSERT(sieveBytes <= SievingPrime::MAX_MULTIPLEINDEX + 1);

  stop_ = stop;
  maxPrime_ = maxPrime;
  log2SieveBytes_ = ilog2(sieveBytes);
  moduloSieveBytes_ = sieveBytes - 1;
  memoryPool_ = &memoryPool;

  uint64_t maxSievingPrime = maxPrime_ / 30;
  uint64_t maxNextMultiple = maxSievingPrime * getMaxFactor() + getMaxFactor();
  uint64_t maxMultipleIndex = sieveBytes - 1 + maxNextMultiple;
  uint64_t maxSegmentIndex = maxMultipleIndex >> log2SieveBytes_;
  uint64_t maxSize = maxSegmentIndex + 1;
  buckets_.reserve(maxSize);
}

/// Add a new sieving prime
void EratBig::storeSievingPrime(uint64_t prime,
                                uint64_t multipleIndex,
                                uint64_t wheelIndex)
{
  uint64_t sieveBytes = 1ull << log2SieveBytes_;
  uint64_t sievingPrime = prime / 30;
  uint64_t maxNextMultiple = sievingPrime * getMaxFactor() + getMaxFactor();
  uint64_t maxMultipleIndex = sieveBytes - 1 + maxNextMultiple;
  uint64_t maxSegmentIndex = maxMultipleIndex >> log2SieveBytes_;
  uint64_t newSize = maxSegmentIndex + 1;
  uint64_t segment = multipleIndex >> log2SieveBytes_;
  multipleIndex &= moduloSieveBytes_;

  while (buckets_.size() < newSize)
    buckets_.push_back(nullptr);

  ASSERT(prime <= maxPrime_);
  ASSERT(segment < buckets_.size());

  if (Bucket::isFull(buckets_[segment]))
    memoryPool_->addBucket(buckets_[segment]);

  buckets_[segment]++->set(sievingPrime, multipleIndex, wheelIndex);
}

void EratBig::crossOff(Vector<uint64_t>& sieve)
{
  uint8_t* sieve8 = (uint8_t*) sieve.data();

  while (buckets_[0])
  {
    Bucket* bucket = Bucket::get(buckets_[0]);
    bucket->setEnd(buckets_[0]);
    buckets_[0] = nullptr;

    // Iterate over the buckets related
    // to the current segment.
    while (bucket)
    {
      crossOff(sieve8, bucket->begin(), bucket->end());
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
  std::size_t moduloSieveBytes = moduloSieveBytes_;
  std::size_t log2SieveBytes = log2SieveBytes_;

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
    std::size_t segment = multipleIndex >> log2SieveBytes;
    multipleIndex &= moduloSieveBytes;

    if (Bucket::isFull(buckets[segment]))
      memoryPool.addBucket(buckets[segment]);

    buckets[segment]++->set(sievingPrime, multipleIndex, wheelIndex);
  }
}

} // namespace
