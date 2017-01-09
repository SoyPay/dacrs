// Copyright (c) 2009-2013 The DACRS developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "key.h"
#include "hash.h"
#include <openssl/bn.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/rand.h>
#include "base58.h"
// anonymous namespace with local implementation code (OpenSSL interaction)
namespace {

// Generate a private key from just the secret parameter
int EC_KEY_regenerate_key(EC_KEY *pEckey, BIGNUM *pPrivKey) {
	int nOk = 0;
	BN_CTX *pCtx = NULL;
	EC_POINT *pPubKey = NULL;

	if (!pEckey) {
		return 0;
	}

	const EC_GROUP *group = EC_KEY_get0_group(pEckey);

	if ((pCtx = BN_CTX_new()) == NULL) {
		goto err;
	}
	pPubKey = EC_POINT_new(group);

	if (pPubKey == NULL) {
		goto err;
	}
	if (!EC_POINT_mul(group, pPubKey, pPrivKey, NULL, NULL, pCtx)) {
		goto err;
	}
	EC_KEY_set_private_key(pEckey, pPrivKey);
	EC_KEY_set_public_key(pEckey, pPubKey);

	nOk = 1;

err:

	if (pPubKey) {
		EC_POINT_free(pPubKey);
	}
	if (pCtx != NULL) {
		BN_CTX_free(pCtx);
	}

	return (nOk);
}

// Perform ECDSA key recovery (see SEC1 4.1.6) for curves over (mod p)-fields
// recid selects which key is recovered
// if check is non-zero, additional checks are performed
int ECDSA_SIG_recover_key_GFp(EC_KEY *pEckey, ECDSA_SIG *pEcsig, const unsigned char *pMsg, int nMsglen, int nRecid, int nCheck)
{
    if (!pEckey) return 0;

    int nRet = 0;
    BN_CTX *ctx = NULL;

    BIGNUM *x = NULL;
    BIGNUM *e = NULL;
    BIGNUM *order = NULL;
    BIGNUM *sor = NULL;
    BIGNUM *eor = NULL;
    BIGNUM *field = NULL;
    EC_POINT *R = NULL;
    EC_POINT *O = NULL;
    EC_POINT *Q = NULL;
    BIGNUM *rr = NULL;
    BIGNUM *zero = NULL;
    int n = 0;
    int i = nRecid / 2;

    const EC_GROUP *group = EC_KEY_get0_group(pEckey);
    if ((ctx = BN_CTX_new()) == NULL) { nRet = -1; goto err; }
    BN_CTX_start(ctx);
    order = BN_CTX_get(ctx);
    if (!EC_GROUP_get_order(group, order, ctx)) { nRet = -2; goto err; }
    x = BN_CTX_get(ctx);
    if (!BN_copy(x, order)) { nRet=-1; goto err; }
    if (!BN_mul_word(x, i)) { nRet=-1; goto err; }
    if (!BN_add(x, x, pEcsig->r)) { nRet=-1; goto err; }
    field = BN_CTX_get(ctx);
    if (!EC_GROUP_get_curve_GFp(group, field, NULL, NULL, ctx)) { nRet=-2; goto err; }
    if (BN_cmp(x, field) >= 0) { nRet=0; goto err; }
    if ((R = EC_POINT_new(group)) == NULL) { nRet = -2; goto err; }
    if (!EC_POINT_set_compressed_coordinates_GFp(group, R, x, nRecid % 2, ctx)) { nRet=0; goto err; }
    if (nCheck)
    {
        if ((O = EC_POINT_new(group)) == NULL) { nRet = -2; goto err; }
        if (!EC_POINT_mul(group, O, NULL, R, order, ctx)) { nRet=-2; goto err; }
        if (!EC_POINT_is_at_infinity(group, O)) { nRet = 0; goto err; }
    }
    if ((Q = EC_POINT_new(group)) == NULL) { nRet = -2; goto err; }
    n = EC_GROUP_get_degree(group);
    e = BN_CTX_get(ctx);
    if (!BN_bin2bn(pMsg, nMsglen, e)) { nRet=-1; goto err; }
    if (8*nMsglen > n) BN_rshift(e, e, 8-(n & 7));
    zero = BN_CTX_get(ctx);
    if (!BN_zero(zero)) { nRet=-1; goto err; }
    if (!BN_mod_sub(e, zero, e, order, ctx)) { nRet=-1; goto err; }
    rr = BN_CTX_get(ctx);
    if (!BN_mod_inverse(rr, pEcsig->r, order, ctx)) { nRet=-1; goto err; }
    sor = BN_CTX_get(ctx);
    if (!BN_mod_mul(sor, pEcsig->s, rr, order, ctx)) { nRet=-1; goto err; }
    eor = BN_CTX_get(ctx);
    if (!BN_mod_mul(eor, e, rr, order, ctx)) { nRet=-1; goto err; }
    if (!EC_POINT_mul(group, Q, eor, R, sor, ctx)) { nRet=-2; goto err; }
    if (!EC_KEY_set_public_key(pEckey, Q)) { nRet=-2; goto err; }

    nRet = 1;

err:
    if (ctx) {
        BN_CTX_end(ctx);
        BN_CTX_free(ctx);
    }
    if (R != NULL) EC_POINT_free(R);
    if (O != NULL) EC_POINT_free(O);
    if (Q != NULL) EC_POINT_free(Q);
    return nRet;
}

// RAII Wrapper around OpenSSL's EC_KEY
class CECKey {
private:
    EC_KEY *pkey;

public:
    CECKey() {
        pkey = EC_KEY_new_by_curve_name(NID_secp256k1);
        assert(pkey != NULL);
    }

    ~CECKey() {
        EC_KEY_free(pkey);
    }

    void GetSecretBytes(unsigned char vch[32]) const {
        const BIGNUM *bn = EC_KEY_get0_private_key(pkey);
        assert(bn);
        int nBytes = BN_num_bytes(bn);
        int n=BN_bn2bin(bn,&vch[32 - nBytes]);
        assert(n == nBytes);
        memset(vch, 0, 32 - nBytes);
    }

    void SetSecretBytes(const unsigned char vch[32]) {
        bool bRet;
        BIGNUM tBn;
        BN_init(&tBn);
        bRet = BN_bin2bn(vch, 32, &tBn);
        assert(bRet);
        bRet = EC_KEY_regenerate_key(pkey, &tBn);
        assert(bRet);
        BN_clear_free(&tBn);
    }

    void GetPrivKey(CPrivKey &privkey, bool fCompressed) {
        EC_KEY_set_conv_form(pkey, fCompressed ? POINT_CONVERSION_COMPRESSED : POINT_CONVERSION_UNCOMPRESSED);
        int nSize = i2d_ECPrivateKey(pkey, NULL);
        assert(nSize);
        privkey.resize(nSize);
        unsigned char* pbegin = &privkey[0];
        int nSize2 = i2d_ECPrivateKey(pkey, &pbegin);
        assert(nSize == nSize2);
    }

    bool SetPrivKey(const CPrivKey &privkey, bool fSkipCheck=false) {
        const unsigned char* pbegin = &privkey[0];
        if (d2i_ECPrivateKey(&pkey, &pbegin, privkey.size())) {
            if(fSkipCheck){
                return true;
            }
            // d2i_ECPrivateKey returns true if parsing succeeds.
            // This doesn't necessarily mean the key is valid.
			if (EC_KEY_check_key(pkey)) {
				return true;
			}
        }
        return false;
    }

    void GetPubKey(CPubKey &pubkey, bool fCompressed) {
        EC_KEY_set_conv_form(pkey, fCompressed ? POINT_CONVERSION_COMPRESSED : POINT_CONVERSION_UNCOMPRESSED);
        int nSize = i2o_ECPublicKey(pkey, NULL);
        assert(nSize);
        assert(nSize <= 65);
        unsigned char c[65];
        unsigned char *pbegin = c;
        int nSize2 = i2o_ECPublicKey(pkey, &pbegin);
        assert(nSize == nSize2);
        pubkey.Set(&c[0], &c[nSize]);
    }

    bool SetPubKey(const CPubKey &pubkey) {
        const unsigned char* pbegin = pubkey.begin();
        return o2i_ECPublicKey(&pkey, &pbegin, pubkey.size());
    }

    bool Sign(const uint256 &hash, vector<unsigned char>& vchSig) {
        vchSig.clear();
        ECDSA_SIG *pSig = ECDSA_do_sign((unsigned char*)&hash, sizeof(hash), pkey);
		if (pSig == NULL) {
			return false;
		}
        BN_CTX *ctx = BN_CTX_new();
        BN_CTX_start(ctx);
        const EC_GROUP *group = EC_KEY_get0_group(pkey);
        BIGNUM *pOrder = BN_CTX_get(ctx);
        BIGNUM *pHalforder = BN_CTX_get(ctx);
        EC_GROUP_get_order(group, pOrder, ctx);
        BN_rshift1(pHalforder, pOrder);
        if (BN_cmp(pSig->s, pHalforder) > 0) {
            // enforce low S values, by negating the value (modulo the pOrder) if above pOrder/2.
            BN_sub(pSig->s, pOrder, pSig->s);
        }
        BN_CTX_end(ctx);
        BN_CTX_free(ctx);
        unsigned int nSize = ECDSA_size(pkey);
        vchSig.resize(nSize); // Make sure it is big enough
        unsigned char *pos = &vchSig[0];
        nSize = i2d_ECDSA_SIG(pSig, &pos);
        ECDSA_SIG_free(pSig);
        vchSig.resize(nSize); // Shrink to fit actual size
        return true;
    }

    bool Verify(const uint256 &cHash, const vector<unsigned char>& vchSig) {
		if (vchSig.empty()) {
			return false;
		}
		// New versions of OpenSSL will reject non-canonical DER signatures. de/re-serialize first.
		unsigned char *pNormDer = NULL;
		ECDSA_SIG *pNormSig = ECDSA_SIG_new();
		const unsigned char* pSigptr = &vchSig[0];
		assert(pNormSig);
		if (d2i_ECDSA_SIG(&pNormSig, &pSigptr, vchSig.size()) == NULL) {
			/* As of OpenSSL 1.0.0p d2i_ECDSA_SIG frees and nulls the pointer on
			 * error. But OpenSSL's own use of this function redundantly frees the
			 * result. As ECDSA_SIG_free(NULL) is a no-op, and in the absence of a
			 * clear contract for the function behaving the same way is more
			 * conservative.
			 */
			ECDSA_SIG_free(pNormSig);
			return false;
		}
		int nDerlen = i2d_ECDSA_SIG(pNormSig, &pNormDer);
		ECDSA_SIG_free(pNormSig);
		if (nDerlen <= 0) {
			return false;
		}
		// -1 = error, 0 = bad sig, 1 = good
		bool bRet = ECDSA_verify(0, (unsigned char*)&cHash, sizeof(cHash), pNormDer, nDerlen, pkey) == 1;
		OPENSSL_free(pNormDer);
		return bRet;
    }

    bool SignCompact(const uint256 &cHash, unsigned char *p64, int &rec) {
        bool bOk = false;
        ECDSA_SIG *pSig = ECDSA_do_sign((unsigned char*)&cHash, sizeof(cHash), pkey);
		if (pSig == NULL) {
			return false;
		}
        memset(p64, 0, 64);
        int nBitsR = BN_num_bits(pSig->r);
        int nBitsS = BN_num_bits(pSig->s);
        if (nBitsR <= 256 && nBitsS <= 256) {
            CPubKey pubkey;
            GetPubKey(pubkey, true);
            for (int i=0; i<4; i++) {
                CECKey keyRec;
                if (ECDSA_SIG_recover_key_GFp(keyRec.pkey, pSig, (unsigned char*)&cHash, sizeof(cHash), i, 1) == 1) {
                    CPubKey cPubkeyRec;
                    keyRec.GetPubKey(cPubkeyRec, true);
                    if (cPubkeyRec == pubkey) {
                        rec = i;
                        bOk = true;
                        break;
                    }
                }
            }
            assert(bOk);
            BN_bn2bin(pSig->r,&p64[32-(nBitsR+7)/8]);
            BN_bn2bin(pSig->s,&p64[64-(nBitsS+7)/8]);
        }
        ECDSA_SIG_free(pSig);
        return bOk;
    }

    // reconstruct public key from a compact signature
    // This is only slightly more CPU intensive than just verifying it.
    // If this function succeeds, the recovered public key is guaranteed to be valid
    // (the signature is a valid signature of the given data for that key)
    bool Recover(const uint256 &cHash, const unsigned char *p64, int rec)
    {
		if (rec < 0 || rec >= 3) {
			return false;
		}
        ECDSA_SIG *pSig = ECDSA_SIG_new();
        BN_bin2bn(&p64[0],  32, pSig->r);
        BN_bin2bn(&p64[32], 32, pSig->s);
        bool bRet = ECDSA_SIG_recover_key_GFp(pkey, pSig, (unsigned char*)&cHash, sizeof(cHash), rec, 0) == 1;
        ECDSA_SIG_free(pSig);
        return bRet;
    }

    static bool TweakSecret(unsigned char vchSecretOut[32], const unsigned char vchSecretIn[32], const unsigned char vchTweak[32])
    {
        bool bRet = true;
        BN_CTX *pCtx = BN_CTX_new();
        BN_CTX_start(pCtx);
        BIGNUM *pBnSecret = BN_CTX_get(pCtx);
        BIGNUM *pBnTweak = BN_CTX_get(pCtx);
        BIGNUM *pBnOrder = BN_CTX_get(pCtx);
        EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_secp256k1);
        EC_GROUP_get_order(group, pBnOrder, pCtx); // what a grossly inefficient way to get the (constant) group pOrder...
        BN_bin2bn(vchTweak, 32, pBnTweak);
        if (BN_cmp(pBnTweak, pBnOrder) >= 0)
            bRet = false; // extremely unlikely
        BN_bin2bn(vchSecretIn, 32, pBnSecret);
        BN_add(pBnSecret, pBnSecret, pBnTweak);
        BN_nnmod(pBnSecret, pBnSecret, pBnOrder, pCtx);
        if (BN_is_zero(pBnSecret))
            bRet = false; // ridiculously unlikely
        int nBits = BN_num_bits(pBnSecret);
        memset(vchSecretOut, 0, 32);
        BN_bn2bin(pBnSecret, &vchSecretOut[32-(nBits+7)/8]);
        EC_GROUP_free(group);
        BN_CTX_end(pCtx);
        BN_CTX_free(pCtx);
        return bRet;
    }

    bool TweakPublic(const unsigned char vchTweak[32]) {
        bool bRet = true;
        BN_CTX *pCtx = BN_CTX_new();
        BN_CTX_start(pCtx);
        BIGNUM *pBnTweak = BN_CTX_get(pCtx);
        BIGNUM *pBnOrder = BN_CTX_get(pCtx);
        BIGNUM *pBnOne = BN_CTX_get(pCtx);
        const EC_GROUP *group = EC_KEY_get0_group(pkey);
        EC_GROUP_get_order(group, pBnOrder, pCtx); // what a grossly inefficient way to get the (constant) group pOrder...
        BN_bin2bn(vchTweak, 32, pBnTweak);
		if (BN_cmp(pBnTweak, pBnOrder) >= 0) {
			bRet = false; // extremely unlikely
		}
        EC_POINT *point = EC_POINT_dup(EC_KEY_get0_public_key(pkey), group);
        BN_one(pBnOne);
        EC_POINT_mul(group, point, pBnTweak, point, pBnOne, pCtx);
		if (EC_POINT_is_at_infinity(group, point)) {
			bRet = false; // ridiculously unlikely
		}
        EC_KEY_set_public_key(pkey, point);
        EC_POINT_free(point);
        BN_CTX_end(pCtx);
        BN_CTX_free(pCtx);
        return bRet;
    }
};

}; // end of anonymous namespace

bool CKey::Check(const unsigned char *vch) {
    // Do not convert to OpenSSL's data structures for range-checking keys,
    // it's easy enough to do directly.
    static const unsigned char vchMax[32] = {
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
        0xBA,0xAE,0xDC,0xE6,0xAF,0x48,0xA0,0x3B,
        0xBF,0xD2,0x5E,0x8C,0xD0,0x36,0x41,0x40
    };
	bool bIsZero = true;
	for (int i = 0; i < 32 && bIsZero; i++)
		if (vch[i] != 0) {
			bIsZero = false;
		}
	if (bIsZero) {
		return false;
	}
	for (int i = 0; i < 32; i++) {
		if (vch[i] < vchMax[i]) {
			return true;
		}
		if (vch[i] > vchMax[i]) {
			return false;
		}
	}
	return true;
}

void CKey::MakeNewKey(bool bCompressedIn) {
	 RandAddSeedPerfmon();
    do {
        RAND_bytes(vch, sizeof(vch));
    } while (!Check(vch));
    fValid = true;
    assert(bCompressedIn == true);
    fCompressed = bCompressedIn;
}

bool CKey::SetPrivKey(const CPrivKey &privkey, bool bCompressedIn) {
    CECKey cKey;
	if (!cKey.SetPrivKey(privkey)) {
		return false;
	}
    cKey.GetSecretBytes(vch);
    fCompressed = bCompressedIn;
    fValid = true;
    return true;
}

CPrivKey CKey::GetPrivKey() const {
    assert(fValid);
    CECKey cKey;
    cKey.SetSecretBytes(vch);
    CPrivKey privkey;
    cKey.GetPrivKey(privkey, fCompressed);
    return privkey;
}

CPubKey CKey::GetPubKey() const {
    assert(fValid);
    CECKey cKey;
    cKey.SetSecretBytes(vch);
    CPubKey pubkey;
    cKey.GetPubKey(pubkey, fCompressed);
    return pubkey;
}

bool CKey::Sign(const uint256 &cHash, vector<unsigned char>& vchSig) const {
	if (!fValid) {
		return false;
	}
    CECKey cKey;
    cKey.SetSecretBytes(vch);
    return cKey.Sign(cHash, vchSig);
}

bool CKey::SignCompact(const uint256 &cHash, vector<unsigned char>& vchSig) const {
	if (!fValid) {
		return false;
	}
	CECKey cKey;
	cKey.SetSecretBytes(vch);
	vchSig.resize(65);
	int rec = -1;
	if (!cKey.SignCompact(cHash, &vchSig[1], rec)) {
		return false;
	}
	assert(rec != -1);
	vchSig[0] = 27 + rec + (fCompressed ? 4 : 0);
	return true;
}

bool CKey::Load(CPrivKey &privkey, CPubKey &vchPubKey, bool fSkipCheck = false) {
	CECKey cKey;
	if (!cKey.SetPrivKey(privkey, fSkipCheck)) {
		return false;
	}

	cKey.GetSecretBytes(vch);
	fCompressed = vchPubKey.IsCompressed();
	fValid = true;

	if (fSkipCheck) {
		return true;
	}

	if (GetPubKey() != vchPubKey) {
		return false;
	}

	return true;
}

CKeyID CPubKey::GetKeyID() const {
	return std::move(CKeyID(Hash160(vch, vch + size())));
}

uint256 CPubKey::GetHash() const {
	return Hash(vch, vch + size());
}

bool CPubKey::Verify(const uint256 &cHash, const vector<unsigned char>& vchSig) const {
	if (!IsValid()) {
		return false;
	}
	CECKey cKey;
	if (!cKey.SetPubKey(*this)) {
		return false;
	}
	if (!cKey.Verify(cHash, vchSig)) {
		return false;
	}
	return true;
}

bool CPubKey::RecoverCompact(const uint256 &cHash, const vector<unsigned char>& vchSig) {
	if (vchSig.size() != 65) {
		return false;
	}
	CECKey cKey;
	if (!cKey.Recover(cHash, &vchSig[1], (vchSig[0] - 27) & ~4)) {
		return false;
	}
	cKey.GetPubKey(*this, (vchSig[0] - 27) & 4);
	return true;
}

bool CPubKey::VerifyCompact(const uint256 &cHash, const vector<unsigned char>& vchSig) const {
	if (!IsValid()) {
		return false;
	}
	if (vchSig.size() != 65) {
		return false;
	}
	CECKey cKey;
	if (!cKey.Recover(cHash, &vchSig[1], (vchSig[0] - 27) & ~4)) {
		return false;
	}
	CPubKey pubkeyRec;
	cKey.GetPubKey(pubkeyRec, IsCompressed());
	if (*this != pubkeyRec) {
		return false;
	}
	return true;
}

bool CPubKey::IsFullyValid() const {
	if (!IsValid()) {
		return false;
	}
	CECKey cKey;
	if (!cKey.SetPubKey(*this)) {
		return false;
	}
	return true;
}

bool CPubKey::Decompress() {
	if (!IsValid()) {
		return false;
	}
	CECKey cKey;
	if (!cKey.SetPubKey(*this)) {
		return false;
	}
	cKey.GetPubKey(*this, false);
	return true;
}

void static BIP32Hash(const unsigned char chainCode[32], unsigned int nChild, unsigned char header, const unsigned char data[32], unsigned char output[64]) {
    unsigned char num[4];
    num[0] = (nChild >> 24) & 0xFF;
    num[1] = (nChild >> 16) & 0xFF;
    num[2] = (nChild >>  8) & 0xFF;
    num[3] = (nChild >>  0) & 0xFF;
    HMAC_SHA512_CTX ctx;
    HMAC_SHA512_Init(&ctx, chainCode, 32);
    HMAC_SHA512_Update(&ctx, &header, 1);
    HMAC_SHA512_Update(&ctx, data, 32);
    HMAC_SHA512_Update(&ctx, num, 4);
    HMAC_SHA512_Final(output, &ctx);
}

bool CKey::Derive(CKey& keyChild, unsigned char ccChild[32], unsigned int nChild, const unsigned char cc[32]) const {
    assert(IsValid());
    assert(IsCompressed());
    unsigned char out[64];
    LockObject(out);
    if ((nChild >> 31) == 0) {
        CPubKey pubkey = GetPubKey();
        assert(pubkey.begin() + 33 == pubkey.end());
        BIP32Hash(cc, nChild, *pubkey.begin(), pubkey.begin()+1, out);
    } else {
        assert(begin() + 32 == end());
        BIP32Hash(cc, nChild, 0, begin(), out);
    }
    memcpy(ccChild, out+32, 32);
    bool bRet = CECKey::TweakSecret((unsigned char*)keyChild.begin(), begin(), out);
    UnlockObject(out);
    keyChild.fCompressed = true;
    keyChild.fValid = bRet;
    return bRet;
}

bool CPubKey::Derive(CPubKey& pubkeyChild, unsigned char ccChild[32], unsigned int nChild, const unsigned char cc[32]) const {
    assert(IsValid());
    assert((nChild >> 31) == 0);
    assert(begin() + 33 == end());
    unsigned char out[64];
    BIP32Hash(cc, nChild, *begin(), begin()+1, out);
    memcpy(ccChild, out+32, 32);
    CECKey cKey;
    bool bRet = cKey.SetPubKey(*this);
    bRet &= cKey.TweakPublic(out);
    cKey.GetPubKey(pubkeyChild, true);
    return bRet;
}

bool CExtKey::Derive(CExtKey &out, unsigned int nChild) const {
    out.nDepth = nDepth + 1;
    CKeyID id = key.GetPubKey().GetKeyID();
    memcpy(&out.vchFingerprint[0], &id, 4);
    out.nChild = nChild;
    return key.Derive(out.key, out.vchChainCode, nChild, vchChainCode);
}

void CExtKey::SetMaster(const unsigned char *seed, unsigned int nSeedLen) {
    static const char hashkey[] = {'B','i','t','c','o','i','n',' ','s','e','e','d'};
    HMAC_SHA512_CTX tCtx;
    HMAC_SHA512_Init(&tCtx, hashkey, sizeof(hashkey));
    HMAC_SHA512_Update(&tCtx, seed, nSeedLen);
    unsigned char out[64];
    LockObject(out);
    HMAC_SHA512_Final(out, &tCtx);
    key.Set(&out[0], &out[32], true);
    memcpy(vchChainCode, &out[32], 32);
    UnlockObject(out);
    nDepth = 0;
    nChild = 0;
    memset(vchFingerprint, 0, sizeof(vchFingerprint));
}

CExtPubKey CExtKey::Neuter() const {
    CExtPubKey cRet;
    cRet.nDepth = nDepth;
    memcpy(&cRet.vchFingerprint[0], &vchFingerprint[0], 4);
    cRet.nChild = nChild;
    cRet.pubkey = key.GetPubKey();
    memcpy(&cRet.vchChainCode[0], &vchChainCode[0], 32);
    return cRet;
}

void CExtKey::Encode(unsigned char code[74]) const {
    code[0] = nDepth;
    memcpy(code+1, vchFingerprint, 4);
    code[5] = (nChild >> 24) & 0xFF; code[6] = (nChild >> 16) & 0xFF;
    code[7] = (nChild >>  8) & 0xFF; code[8] = (nChild >>  0) & 0xFF;
    memcpy(code+9, vchChainCode, 32);
    code[41] = 0;
    assert(key.size() == 32);
    memcpy(code+42, key.begin(), 32);
}

void CExtKey::Decode(const unsigned char code[74]) {
    nDepth = code[0];
    memcpy(vchFingerprint, code+1, 4);
    nChild = (code[5] << 24) | (code[6] << 16) | (code[7] << 8) | code[8];
    memcpy(vchChainCode, code+9, 32);
    key.Set(code+42, code+74, true);
}

void CExtPubKey::Encode(unsigned char code[74]) const {
    code[0] = nDepth;
    memcpy(code+1, vchFingerprint, 4);
    code[5] = (nChild >> 24) & 0xFF; code[6] = (nChild >> 16) & 0xFF;
    code[7] = (nChild >>  8) & 0xFF; code[8] = (nChild >>  0) & 0xFF;
    memcpy(code+9, vchChainCode, 32);
    assert(pubkey.size() == 33);
    memcpy(code+41, pubkey.begin(), 33);
}

void CExtPubKey::Decode(const unsigned char code[74]) {
    nDepth = code[0];
    memcpy(vchFingerprint, code+1, 4);
    nChild = (code[5] << 24) | (code[6] << 16) | (code[7] << 8) | code[8];
    memcpy(vchChainCode, code+9, 32);
    pubkey.Set(code+41, code+74);
}

bool CExtPubKey::Derive(CExtPubKey &out, unsigned int nChild) const {
    out.nDepth = nDepth + 1;
    CKeyID id = pubkey.GetKeyID();
    memcpy(&out.vchFingerprint[0], &id, 4);
    out.nChild = nChild;
    return pubkey.Derive(out.pubkey, out.vchChainCode, nChild, vchChainCode);
}

string CPubKey::ToString() const {
	return HexStr(begin(),end());

}

string CKeyID::ToAddress() const {
	if (IsNull()) {
		return "";
	} else {
		return CDacrsAddress(*this).ToString();
	}
}

CKeyID::CKeyID(const string& strAddress) :
		uint160() {
	if (strAddress.length() == 40) {
		*this = uint160S(strAddress);
	} else {
		CDacrsAddress addr(strAddress);
		addr.GetKeyID(*this);
	}
}
