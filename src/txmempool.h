// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef DACRS_TXMEMPOOL_H_
#define DACRS_TXMEMPOOL_H_

#include <list>
#include <map>

#include "core.h"
#include "sync.h"
#include <memory>

using namespace std;

class CAccountViewCache;
class CScriptDBViewCache;
class CValidationState;

/*
 * CTxMemPool stores these:
 */
class CTxMemPoolEntry {
 public:
	CTxMemPoolEntry(CBaseTransaction *pTx, int64_t llFee, int64_t llTime, double dPriority, unsigned int unHeight);
	CTxMemPoolEntry();
	CTxMemPoolEntry(const CTxMemPoolEntry& cOther);

	std::shared_ptr<CBaseTransaction> GetTx() const {
		return m_pTx;
	}
	double GetPriority(unsigned int unCurrentHeight) const;

	int64_t GetFee() const {
		return m_llFee;
	}
	size_t GetTxSize() const {
		return m_unTxSize;
	}
	int64_t GetTime() const {
		return m_llTime;
	}
	unsigned int GetHeight() const {
		return m_unHeight;
	}

 private:
	std::shared_ptr<CBaseTransaction> m_pTx;
	int64_t m_llFee; 								// Cached to avoid expensive parent-transaction lookups
	size_t 	m_unTxSize; 							// ... and avoid recomputing tx size
	int64_t m_llTime; 								// Local time when entering the mempool
	double 	m_dPriority; 							// Priority when entering the mempool
	unsigned int m_unHeight; 						// Chain height when entering the mempool
};

/*
 * CTxMemPool stores valid-according-to-the-current-best-chain
 * transactions that may be included in the next block.
 *
 * Transactions are added when they are seen on the network
 * (or created by the local node), but not all transactions seen
 * are added to the pool: if a new transaction double-spends
 * an input of a transaction in the pool, it is dropped,
 * as are non-standard transactions.
 */
class CTxMemPool {
 public:
	CTxMemPool();

	void setSanityCheck(bool bSanityCheck) {
		m_bSanityCheck = bSanityCheck;
	}

	bool addUnchecked(const uint256& cHash, const CTxMemPoolEntry &cTxMemPoolEntry, CValidationState &cValidationState);

	void remove(CBaseTransaction *pBaseTx, list<std::shared_ptr<CBaseTransaction> >& Removed, bool bRecursive = false);

	void clear();
	void queryHashes(vector<uint256>& vctxid);
	unsigned int GetTransactionsUpdated() const;
	void AddTransactionsUpdated(unsigned int nUpdated);

	unsigned long size() {
		LOCK(m_cs);
		return m_mapTx.size();
	}

	bool exists(uint256 hash) {
		LOCK(m_cs);
		return ((m_mapTx.count(hash) != 0));
	}

	std::shared_ptr<CBaseTransaction> lookup(uint256 cHash) const;

	void SetAccountViewDB(CAccountViewCache *pAccountViewCacheIn);

	void SetScriptDBViewDB(CScriptDBViewCache *pScriptDBViewCacheIn);

	bool CheckTxInMemPool(const uint256& cHash, const CTxMemPoolEntry &cTxMemPoolEntry, CValidationState &cValidationState, bool bExcute =
			true);

	void ReScanMemPoolTx(CAccountViewCache *pAccountViewCacheIn, CScriptDBViewCache *pScriptDBViewCacheIn);

	mutable CCriticalSection m_cs;
	map<uint256, CTxMemPoolEntry> m_mapTx;
	std::shared_ptr<CAccountViewCache> m_pAccountViewCache;
	std::shared_ptr<CScriptDBViewCache> m_pScriptDBViewCache;

 private:
 	bool m_bSanityCheck; 					// Normally false, true if -checkmempool or -regtest
 	unsigned int m_unTransactionsUpdated;  	//TODO meaning
};

#endif
