// Copyright (c) 2012 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bloom.h"

#include "core.h"
#include "hash.h"
//#include "script.h"

#include <math.h>
#include <stdlib.h>

#define LN2SQUARED 0.4804530139182014246671025263266649717305529515945455
#define LN2 0.6931471805599453094172321214581765680755001343602552

using namespace std;

CBloomFilter::CBloomFilter(unsigned int nElements, double nFPRate, unsigned int nTweakIn, unsigned char nFlagsIn) :
// The ideal size for a bloom filter with a given number of elements and false positive rate is:
// - nElements * log(fp rate) / ln(2)^2
// We ignore filter parameters which will create a bloom filter larger than the protocol limits
		m_vchData(min((unsigned int) (-1 / LN2SQUARED * nElements * log(nFPRate)), g_sMaxBloomFilterSize * 8) / 8),
// The ideal number of hash functions is filter size * ln(2) / number of elements
// Again, we ignore filter parameters which will create a bloom filter with more hash functions than the protocol limits
// See http://en.wikipedia.org/wiki/Bloom_filter for an explanation of these formulas
		m_bIsFull(false), m_bIsEmpty(false), m_unHashFuncs(
				min((unsigned int) (m_vchData.size() * 8 / nElements * LN2), g_sMaxHashFuncs)), m_unTweak(nTweakIn), m_uchFlags(
				nFlagsIn) {
}

inline unsigned int CBloomFilter::Hash(unsigned int nHashNum, const vector<unsigned char>& vDataToHash) const {
	// 0xFBA4C795 chosen as it guarantees a reasonable bit difference between nHashNum values.
	return MurmurHash3(nHashNum * 0xFBA4C795 + m_unTweak, vDataToHash) % (m_vchData.size() * 8);
}

void CBloomFilter::insert(const vector<unsigned char>& vKey) {
	if (m_bIsFull) {
		return;
	}

	for (unsigned int i = 0; i < m_unHashFuncs; i++) {
		unsigned int nIndex = Hash(i, vKey);
		// Sets bit nIndex of vData
		m_vchData[nIndex >> 3] |= (1 << (7 & nIndex));
	}

	m_bIsEmpty = false;
}

void CBloomFilter::insert(const uint256& hash) {
	vector<unsigned char> data(hash.begin(), hash.end());
	insert(data);
}

bool CBloomFilter::contains(const vector<unsigned char>& vKey) const {
	if (m_bIsFull) {
		return true;
	}

	if (m_bIsEmpty) {
		return false;
	}

	for (unsigned int i = 0; i < m_unHashFuncs; i++) {
		unsigned int nIndex = Hash(i, vKey);
		// Checks bit nIndex of vData
		if (!(m_vchData[nIndex >> 3] & (1 << (7 & nIndex)))) {
			return false;
		}
	}

	return true;
}


bool CBloomFilter::contains(const uint256& hash) const {
	vector<unsigned char> data(hash.begin(), hash.end());

	return contains(data);
}

bool CBloomFilter::IsWithinSizeConstraints() const {
	return m_vchData.size() <= g_sMaxBloomFilterSize && m_unHashFuncs <= g_sMaxHashFuncs;
}

bool CBloomFilter::IsRelevantAndUpdate(CBaseTransaction *pBaseTx, const uint256& hash) {
	// Match if the filter contains the hash of tx
	//  for finding tx when they appear in a block
	if (m_bIsFull) {
		return true;
	}

	if (m_bIsEmpty) {
		return false;
	}

//    if (contains(hash))
//        fFound = true;

//    for (unsigned int i = 0; i < tx.vout.size(); i++)
//    {
//        const CTxOut& txout = tx.vout[i];
//        // Match if the filter contains any arbitrary script data element in any scriptPubKey in tx
//        // If this matches, also add the specific output that was matched.
//        // This means clients don't have to update the filter themselves when a new relevant tx
//        // is discovered in order to find spending transactions, which avoids round-tripping and race conditions.
//        CScript::const_iterator pc = txout.scriptPubKey.begin();
//        vector<unsigned char> data;
//        while (pc < txout.scriptPubKey.end())
//        {
//            opcodetype opcode;
//            if (!txout.scriptPubKey.GetOp(pc, opcode, data))
//                break;
//            if (data.size() != 0 && contains(data))
//            {
//                fFound = true;
//                if ((nFlags & BLOOM_UPDATE_MASK) == BLOOM_UPDATE_ALL)
//                    insert(COutPoint(hash, i));
//                else if ((nFlags & BLOOM_UPDATE_MASK) == BLOOM_UPDATE_P2PUBKEY_ONLY)
//                {
//                    txnouttype type;
//                    vector<vector<unsigned char> > vSolutions;
//                    if (Solver(txout.scriptPubKey, type, vSolutions) &&
//                            (type == TX_PUBKEY || type == TX_MULTISIG))
//                        insert(COutPoint(hash, i));
//                }
//                break;
//            }
//        }
//    }
//
//    if (fFound)
//        return true;
//
//    BOOST_FOREACH(const CTxIn& txin, tx.vin)
//    {
//        // Match if the filter contains an outpoint tx spends
//        if (contains(txin.prevout))
//            return true;
//
//        // Match if the filter contains any arbitrary script data element in any scriptSig in tx
//        CScript::const_iterator pc = txin.scriptSig.begin();
//        vector<unsigned char> data;
//        while (pc < txin.scriptSig.end())
//        {
//            opcodetype opcode;
//            if (!txin.scriptSig.GetOp(pc, opcode, data))
//                break;
//            if (data.size() != 0 && contains(data))
//                return true;
//        }
//    }

	return false;
}

void CBloomFilter::UpdateEmptyFull() {
	bool bFull = true;
	bool bEmpty = true;

	for (unsigned int i = 0; i < m_vchData.size(); i++) {
		bFull &= m_vchData[i] == 0xff;
		bEmpty &= m_vchData[i] == 0;
	}

	m_bIsFull = bFull;
	m_bIsEmpty = bEmpty;
}
