// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The DACRS developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_HASH_H_
#define DACRS_HASH_H_

#include "serialize.h"
#include "uint256.h"
#include "version.h"
#include "util.h"

#include <vector>

#include <openssl/ripemd.h>
#include <openssl/sha.h>

template<typename T1>
inline uint256 Hash(const T1 pbegin, const T1 pend)
{
    static unsigned char schblank[1];
    uint256 cHash1;
    SHA256((pbegin == pend ? schblank : (unsigned char*)&pbegin[0]), (pend - pbegin) * sizeof(pbegin[0]), (unsigned char*)&cHash1);
    uint256 cHash2;
    SHA256((unsigned char*)&cHash1, sizeof(cHash1), (unsigned char*)&cHash2);
    return std::move(cHash2);
}

class CHashWriter {
 public:
	void Init() {
		SHA256_Init(&m_CTx);
	}

	CHashWriter(int nTypeIn, int nVersionIn) :
		m_nType(nTypeIn), m_nVersion(nVersionIn) {
		Init();
	}

	CHashWriter& write(const char *pch, size_t unSize) {
		SHA256_Update(&m_CTx, pch, unSize);
		return (*this);
	}

	// invalidates the object
	uint256 GetHash() {
		uint256 cHash1;
		SHA256_Final((unsigned char*) &cHash1, &m_CTx);
		uint256 cHash2;
		SHA256((unsigned char*) &cHash1, sizeof(cHash1), (unsigned char*) &cHash2);
		return std::move(cHash2);
	}

	template<typename T>
	CHashWriter& operator<<(const T& obj) {
		// Serialize to this stream
		::Serialize(*this, obj, m_nType, m_nVersion);
		return (*this);
	}

	int m_nType;
	int m_nVersion;

 private:
 	SHA256_CTX m_CTx;
};

template<typename T1, typename T2>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end)
{
    static unsigned char pblank[1];
    uint256 hash1;
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, (p1begin == p1end ? pblank : (unsigned char*)&p1begin[0]), (p1end - p1begin) * sizeof(p1begin[0]));
    SHA256_Update(&ctx, (p2begin == p2end ? pblank : (unsigned char*)&p2begin[0]), (p2end - p2begin) * sizeof(p2begin[0]));
    SHA256_Final((unsigned char*)&hash1, &ctx);
    uint256 hash2;
    SHA256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return std::move(hash2);
}

template<typename T1, typename T2, typename T3>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end,
                    const T3 p3begin, const T3 p3end)
{
    static unsigned char pblank[1];
    uint256 hash1;
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, (p1begin == p1end ? pblank : (unsigned char*)&p1begin[0]), (p1end - p1begin) * sizeof(p1begin[0]));
    SHA256_Update(&ctx, (p2begin == p2end ? pblank : (unsigned char*)&p2begin[0]), (p2end - p2begin) * sizeof(p2begin[0]));
    SHA256_Update(&ctx, (p3begin == p3end ? pblank : (unsigned char*)&p3begin[0]), (p3end - p3begin) * sizeof(p3begin[0]));
    SHA256_Final((unsigned char*)&hash1, &ctx);
    uint256 hash2;
    SHA256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

template<typename T>
uint256 SerializeHash(const T& obj, int nType=SER_GETHASH, int nVersion=g_sProtocolVersion)
{
    CHashWriter ss(nType, nVersion);
    ss << obj;
    return std::move(ss.GetHash());
}

template<typename T1>
inline uint160 Hash160(const T1 pbegin, const T1 pend)
{
    static unsigned char pblank[1];
    uint256 hash1;
    SHA256((pbegin == pend ? pblank : (unsigned char*)&pbegin[0]), (pend - pbegin) * sizeof(pbegin[0]), (unsigned char*)&hash1);
    uint160 hash2;
    RIPEMD160((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

inline uint160 Hash160(const vector<unsigned char>& vch)
{
    return Hash160(vch.begin(), vch.end());
}

unsigned int MurmurHash3(unsigned int nHashSeed, const vector<unsigned char>& vDataToHash);

typedef struct
{
    SHA512_CTX ctxInner;
    SHA512_CTX ctxOuter;
} HMAC_SHA512_CTX;

int HMAC_SHA512_Init(HMAC_SHA512_CTX *pctx, const void *pkey, size_t len);
int HMAC_SHA512_Update(HMAC_SHA512_CTX *pctx, const void *pdata, size_t len);
int HMAC_SHA512_Final(unsigned char *pmd, HMAC_SHA512_CTX *pctx);

#endif
