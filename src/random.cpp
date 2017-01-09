// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "random.h"

#include "support/cleanse.h"
#ifdef WIN32
#include "compat/compat.h" // for Windows API
#endif
#include "serialize.h"        // for begin_ptr(vec)
#include "util.h"             // for LogPrint()
//#include "utilstrencodings.h" // for GetTime()

#include <limits>

#ifndef WIN32
#include <sys/time.h>
#endif

#include <openssl/err.h>
#include <openssl/rand.h>

static inline int64_t GetPerformanceCounter() {
    int64_t llCounter = 0;
#ifdef WIN32
    QueryPerformanceCounter((LARGE_INTEGER*)&llCounter);
#else
    timeval t;
    gettimeofday(&t, NULL);
    llCounter = (int64_t)(t.tv_sec * 1000000 + t.tv_usec);
#endif
    return llCounter;
}

void RandAddSeed() {
	// Seed with CPU performance counter
	int64_t llCounter = GetPerformanceCounter();
	RAND_add(&llCounter, sizeof(llCounter), 1.5);
	memory_cleanse((void*) &llCounter, sizeof(llCounter));
}

void RandAddSeedPerfmon() {
    RandAddSeed();

#ifdef WIN32
    // Don't need this on Linux, OpenSSL automatically uses /dev/urandom
    // Seed with the entire set of perfmon data

    // This can take up to 2 seconds, so only do it every 10 minutes
    static int64_t sllLastPerfmon;
    if (GetTime() < sllLastPerfmon + 10 * 60) {
    	return;
    }
    sllLastPerfmon = GetTime();

    std::vector<unsigned char> vchData(250000, 0);
    long lRet = 0;
    unsigned long ulSize = 0;
    const size_t unMaxSize = 10000000; // Bail out at more than 10MB of performance data
	while (true) {
		ulSize = vchData.size();
		lRet = RegQueryValueExA(HKEY_PERFORMANCE_DATA, "Global", NULL, NULL, begin_ptr(vchData), &ulSize);
		if (lRet != ERROR_MORE_DATA || vchData.size() >= unMaxSize) {
			break;
		}
		vchData.resize(std::max((vchData.size() * 3) / 2, unMaxSize)); // Grow size of buffer exponentially
	}
    RegCloseKey(HKEY_PERFORMANCE_DATA);
    if (lRet == ERROR_SUCCESS) {
        RAND_add(begin_ptr(vchData), ulSize, ulSize / 100.0);
        memory_cleanse(begin_ptr(vchData), ulSize);
        LogPrint("rand", "%s: %lu bytes\n", __func__, ulSize);
    } else {
        static bool bWarned = false; // Warn only once
        if (!bWarned) {
            LogPrint("INFO", "%s: Warning: RegQueryValueExA(HKEY_PERFORMANCE_DATA) failed with code %i\n", __func__, lRet);
            bWarned = true;
        }
    }
#endif
}

void GetRandBytes(unsigned char* szBuf, int nNum) {
	if (RAND_bytes(szBuf, nNum) != 1) {
		LogPrint("INFO", "%s: OpenSSL RAND_bytes() failed with error: %s\n", __func__,
				ERR_error_string(ERR_get_error(), NULL));
		assert(false);
	}
}

uint64_t GetRand(uint64_t ullMax) {
	if (ullMax == 0) {
		return 0;
	}
	// The range of the random source must be a multiple of the modulus
	// to give every possible output value an equal possibility
	uint64_t ullRange = (std::numeric_limits<uint64_t>::max() / ullMax) * ullMax;
	uint64_t ullRand = 0;
	do {
		GetRandBytes((unsigned char*) &ullRand, sizeof(ullRand));
	} while (ullRand >= ullRange);
	return (ullRand % ullMax);
}

int GetRandInt(int nMax) {
	return GetRand(nMax);
}

uint256 GetRandHash() {
	uint256 cHash;
	GetRandBytes((unsigned char*) &cHash, sizeof(cHash));
	return cHash;
}

uint32_t g_ullInsecure_rand_Rz = 11;
uint32_t g_ullInsecure_rand_Rw = 11;

void seed_insecure_rand(bool bDeterministic) {
	// The seed values have some unlikely fixed points which we avoid.
	if (bDeterministic) {
		g_ullInsecure_rand_Rz = g_ullInsecure_rand_Rw = 11;
	} else {
		uint32_t uTmp;
		do {
			GetRandBytes((unsigned char*) &uTmp, 4);
		} while (uTmp == 0 || uTmp == 0x9068ffffU);
		g_ullInsecure_rand_Rz = uTmp;
		do {
			GetRandBytes((unsigned char*) &uTmp, 4);
		} while (uTmp == 0 || uTmp == 0x464fffffU);
		g_ullInsecure_rand_Rw = uTmp;
	}
}
