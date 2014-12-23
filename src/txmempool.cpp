// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "core.h"
#include "txmempool.h"
#include "account.h"
#include "main.h"

using namespace std;

CTxMemPoolEntry::CTxMemPoolEntry() {
	nHeight = MEMPOOL_HEIGHT;
}

CTxMemPoolEntry::CTxMemPoolEntry(CBaseTransaction *pBaseTx, int64_t _nFee, int64_t _nTime, double _dPriority,
		unsigned int _nHeight) :
		nFee(_nFee), nTime(_nTime), dPriority(_dPriority), nHeight(_nHeight) {
	pTx = pBaseTx->GetNewInstance();
	nTxSize = ::GetSerializeSize(*pTx, SER_NETWORK, PROTOCOL_VERSION);
}

CTxMemPoolEntry::CTxMemPoolEntry(const CTxMemPoolEntry& other) {
	this->dPriority = other.dPriority;
	this->nFee = other.nFee;
	this->nTxSize = other.nTxSize;
	this->nTime = other.nTime;
	this->dPriority = other.dPriority;
	this->nHeight = other.nHeight;
	this->pTx = other.pTx->GetNewInstance();
}

double CTxMemPoolEntry::GetPriority(unsigned int currentHeight) const {
	double dResult = 0;
	dResult = nFee / nTxSize;
	return dResult;
}

CTxMemPool::CTxMemPool() {
	// Sanity checks off by default for performance, because otherwise
	// accepting transactions becomes O(N^2) where N is the number
	// of transactions in the pool
	fSanityCheck = false;
	nTransactionsUpdated = 0;
}

void CTxMemPool::SetAccountViewDB(CAccountViewCache *pAccountViewCacheIn) {
	pAccountViewCache = make_shared<CAccountViewCache>(*pAccountViewCacheIn, false);
}

void CTxMemPool::SetScriptDBViewDB(CScriptDBViewCache *pScriptDBViewCacheIn) {
	pScriptDBViewCache = make_shared<CScriptDBViewCache>(*pScriptDBViewCacheIn, false);
}

void CTxMemPool::ReScanMemPoolTx(const CBlock &block, CAccountViewCache *pAccountViewCacheIn, CScriptDBViewCache *pScriptDBViewCacheIn) {
	pAccountViewCache.reset(new CAccountViewCache(*pAccountViewCacheIn, false));
	pScriptDBViewCache.reset(new CScriptDBViewCache(*pScriptDBViewCacheIn, false));
	{
		LOCK(cs);
		for(auto &pTxItem : block.vptx){
			mapTx.erase(pTxItem->GetHash());
		}
		list<std::shared_ptr<CBaseTransaction> > removed;
		for(map<uint256, CTxMemPoolEntry >::iterator iterTx = mapTx.begin(); iterTx != mapTx.end(); ) {
			if (!CheckTxInMemPool(iterTx->first, iterTx->second)) {
				iterTx = mapTx.erase(iterTx++);
				continue;
			}
			++iterTx;
		}

	}
}

unsigned int CTxMemPool::GetTransactionsUpdated() const {
	LOCK(cs);
	return nTransactionsUpdated;
}

void CTxMemPool::AddTransactionsUpdated(unsigned int n) {
	LOCK(cs);
	nTransactionsUpdated += n;
}

void CTxMemPool::remove(CBaseTransaction *pBaseTx, list<std::shared_ptr<CBaseTransaction> >& removed, bool fRecursive) {
	// Remove transaction from memory pool
	{
		LOCK(cs);
		uint256 hash = pBaseTx->GetHash();
//        if(COMMON_TX == pTx->nTxType ){
//        	CTransaction *pNormalTx = (CTransaction *)pTx;//dynamic_pointer_cast<CTransaction>(pTx);
//			}
//        }
		if (mapTx.count(hash)) {
			removed.push_front(std::shared_ptr<CBaseTransaction>(mapTx[hash].GetTx()));
			mapTx.erase(hash);
			nTransactionsUpdated++;
		}
	}
}

bool CTxMemPool::CheckTxInMemPool(const uint256& hash, const CTxMemPoolEntry &entry) {
	CValidationState state;
	CTxUndo txundo;
	CTransactionDBCache txCacheTemp(*pTxCacheTip, true);
	if (!entry.GetTx()->UpdateAccount(0, *pAccountViewCache, state, txundo, chainActive.Tip()->nHeight + 1,
			txCacheTemp, *pScriptDBViewCache)) {
		return false;
	}
	return true;
}
bool CTxMemPool::addUnchecked(const uint256& hash, const CTxMemPoolEntry &entry) {
	// Add to memory pool without checking anything.
	// Used by main.cpp AcceptToMemoryPool(), which DOES do
	// all the appropriate checks.
	LOCK(cs);
	{
		if(!CheckTxInMemPool(hash, entry)) {
			return false;
		}
		mapTx.insert(make_pair(hash, entry));
		LogPrint("addtomempool", "add tx hash:%s time:%ld\n", hash.GetHex(), GetTime());
		nTransactionsUpdated++;
	}
	return true;
}

void CTxMemPool::removeConflicts(CBaseTransaction *pBaseTx, list<std::shared_ptr<CBaseTransaction> >& removed) {

}

void CTxMemPool::clear() {
	LOCK(cs);
	mapTx.clear();
	pAccountViewCache.reset(new CAccountViewCache(*pAccountViewTip, false));
	++nTransactionsUpdated;
}

void CTxMemPool::queryHashes(vector<uint256>& vtxid) {
	vtxid.clear();

	LOCK(cs);
	vtxid.reserve(mapTx.size());
	for (typename map<uint256, CTxMemPoolEntry>::iterator mi = mapTx.begin(); mi != mapTx.end(); ++mi)
		vtxid.push_back((*mi).first);
}

std::shared_ptr<CBaseTransaction> CTxMemPool::lookup(uint256 hash) const {
	LOCK(cs);
	typename map<uint256, CTxMemPoolEntry>::const_iterator i = mapTx.find(hash);
	if (i == mapTx.end())
		return std::shared_ptr<CBaseTransaction>();
	return i->second.GetTx();
}
