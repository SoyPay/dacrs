// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_RANDOM_H_
#define DACRS_RANDOM_H_

#include "uint256.h"

#include <stdint.h>

/**
 * Seed OpenSSL PRNG with additional entropy data
 */
void RandAddSeed();
void RandAddSeedPerfmon();

/**
 * Functions to gather random data via the OpenSSL PRNG
 */
void GetRandBytes(unsigned char* szBuf, int nNum);
uint64_t GetRand(uint64_t ullMax);
int GetRandInt(int nMax);
uint256 GetRandHash();

/**
 * Seed insecure_rand using the random pool.
 * @param Deterministic Use a deterministic seed
 */
void seed_insecure_rand(bool bDeterministic = false);

/**
 * MWC RNG of George Marsaglia
 * This is intended to be fast. It has a period of 2^59.3, though the
 * least significant 16 bits only have a period of about 2^30.1.
 *
 * @return random value
 */
extern uint32_t g_ullInsecure_rand_Rz;
extern uint32_t g_ullInsecure_rand_Rw;
static inline uint32_t insecure_rand(void)
{
    g_ullInsecure_rand_Rz = 36969 * (g_ullInsecure_rand_Rz & 65535) + (g_ullInsecure_rand_Rz >> 16);
    g_ullInsecure_rand_Rw = 18000 * (g_ullInsecure_rand_Rw & 65535) + (g_ullInsecure_rand_Rw >> 16);
    return (g_ullInsecure_rand_Rw << 16) + g_ullInsecure_rand_Rz;
}

#endif // DACRS_RANDOM_H_
