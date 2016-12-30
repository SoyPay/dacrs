// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "core.h"
#include "txmempool.h"
#include "database.h"
#include "main.h"

using namespace std;

CTxMemPoolEntry::CTxMemPoolEntry() {
	 m_llFee 		= 0;
	 m_unTxSize 	= 0;
	 m_llTime 		= 0;
	 m_dPriority 	= 0.0;
	 m_unHeight 	= 0;
}

CTxMemPoolEntry::CTxMemPoolEntry(CBaseTransaction *pTx, int64_t llFee, int64_t llTime, double dPriority,
		unsigned int unHeight) :
		m_llFee(llFee), m_llTime(llTime), m_dPriority(dPriority), m_unHeight(unHeight) {
	m_pTx = pTx->GetNewInstance();
	m_unTxSize = ::GetSerializeSize(*m_pTx, SER_NETWORK, g_sProtocolVersion);
}

CTxMemPoolEntry::CTxMemPoolEntry(const CTxMemPoolEntry& cOther) {
	this->m_dPriority 	= cOther.m_dPriority;
	this->m_llFee 		= cOther.m_llFee;
	this->m_unTxSize 	= cOther.m_unTxSize;
	this->m_llTime 		= cOther.m_llTime;
	this->m_dPriority 	= cOther.m_dPriority;
	this->m_unHeight 	= cOther.m_unHeight;
	this->m_pTx 		= cOther.m_pTx->GetNewInstance();
}

double CTxMemPoolEntry::GetPriority(unsigned int unCurrentHeight) const {
	double dResult = 0;
	dResult = m_llFee / m_unTxSize;
	return dResult;
}

CTxMemPool::CTxMemPool() {
	// Sanity checks off by default for performance, because otherwise
	// accepting transactions becomes O(N^2) where N is the number
	// of transactions in the pool
	m_bSanityCheck = false;
	m_unTransactionsUpdated = 0;
}

void CTxMemPool::SetAccountViewDB(CAccountViewCache *pAccountViewCacheIn) {
	m_pAccountViewCache = std::make_shared<CAccountViewCache>(*pAccountViewCacheIn, false);
}

void CTxMemPool::SetScriptDBViewDB(CScriptDBViewCache *pScriptDBViewCacheIn) {
	m_pScriptDBViewCache = std::make_shared<CScriptDBViewCache>(*pScriptDBViewCacheIn, false);
}

void CTxMemPool::ReScanMemPoolTx(CAccountViewCache *pAccountViewCacheIn, CScriptDBViewCache *pScriptDBViewCacheIn) {
	m_pAccountViewCache.reset(new CAccountViewCache(*pAccountViewCacheIn, true));
	m_pScriptDBViewCache.reset(new CScriptDBViewCache(*pScriptDBViewCacheIn, true));
	{
		LOCK(m_cs);
		CValidationState state;
		list<std::shared_ptr<CBaseTransaction> > removed;
		for (map<uint256, CTxMemPoolEntry>::iterator iterTx = m_mapTx.begin(); iterTx != m_mapTx.end();) {
			if (!CheckTxInMemPool(iterTx->first, iterTx->second, state, true)) {
				uint256 hash = iterTx->first;
				iterTx = m_mapTx.erase(iterTx++);
				g_cUIInterface.RemoveTransaction(hash);
				EraseTransaction(hash);
				continue;
			}
			++iterTx;
		}

	}
}

unsigned int CTxMemPool::GetTransactionsUpdated() const {
	LOCK(m_cs);
	return m_unTransactionsUpdated;
}

void CTxMemPool::AddTransactionsUpdated(unsigned int nUpdated) {
	LOCK(m_cs);
	m_unTransactionsUpdated += nUpdated;
}

void CTxMemPool::remove(CBaseTransaction *pBaseTx, list<std::shared_ptr<CBaseTransaction> >& Removed, bool bRecursive) {
	// Remove transaction from memory pool
	{
		LOCK(m_cs);
		uint256 hash = pBaseTx->GetHash();

		if (m_mapTx.count(hash)) {
			Removed.push_front(std::shared_ptr<CBaseTransaction>(m_mapTx[hash].GetTx()));
			m_mapTx.erase(hash);
			g_cUIInterface.RemoveTransaction(hash);
			EraseTransaction(hash);
			m_unTransactionsUpdated++;
		}
	}
}

bool CTxMemPool::CheckTxInMemPool(const uint256& cHash, const CTxMemPoolEntry &cTxMemPoolEntry,
		CValidationState &cValidationState, bool bExcute) {
	CTxUndo cTxundo;
	CTransactionDBCache cTempTxCache(*g_pTxCacheTip, true);
	CAccountViewCache cTempAcctView(*m_pAccountViewCache, true);
	CScriptDBViewCache cTempScriptDBView(*m_pScriptDBViewCache, true);

	// is it already confirmed in block
	if (uint256() != g_pTxCacheTip->IsContainTx(cHash)) {
		return cValidationState.Invalid(ERRORMSG("CheckTxInMemPool() : tx hash %s has been confirmed", cHash.GetHex()),
						REJECT_INVALID, "tx-duplicate-confirmed");
	}
	// is it in valid height
	if (!cTxMemPoolEntry.GetTx()->IsValidHeight(g_cChainActive.Tip()->m_nHeight, SysCfg().GetTxCacheHeight())) {
		return cValidationState.Invalid(
				ERRORMSG("CheckTxInMemPool() : txhash=%s beyond the scope of valid height ", cHash.GetHex()),
				REJECT_INVALID, "tx-invalid-height");
	}
	if (EM_CONTRACT_TX == cTxMemPoolEntry.GetTx()->m_chTxType) {
		LogPrint("vm", "tx hash=%s CheckTxInMemPool run contract\n", cTxMemPoolEntry.GetTx()->GetHash().GetHex());
	}
	if (bExcute) {
		if (!cTxMemPoolEntry.GetTx()->ExecuteTx(0, cTempAcctView, cValidationState, cTxundo,
				g_cChainActive.Tip()->m_nHeight + 1, cTempTxCache, cTempScriptDBView)) {
			return false;
		}
	}
	assert(cTempAcctView.Flush() && cTempScriptDBView.Flush());
	return true;
}

bool CTxMemPool::addUnchecked(const uint256& cHash, const CTxMemPoolEntry &cTxMemPoolEntry, CValidationState &cValidationState) {
	// Add to memory pool without checking anything.
	// Used by main.cpp AcceptToMemoryPool(), which DOES do
	// all the appropriate checks.
	LOCK(m_cs);
	{
		if(!CheckTxInMemPool(cHash, cTxMemPoolEntry, cValidationState)) {
			return false;
		}
		m_mapTx.insert(make_pair(cHash, cTxMemPoolEntry));
		LogPrint("addtomempool", "add tx hash:%s time:%ld\n", cHash.GetHex(), GetTime());
		m_unTransactionsUpdated++;
	}
	return true;
}

void CTxMemPool::clear() {
	LOCK(m_cs);
	m_mapTx.clear();
	m_pAccountViewCache.reset(new CAccountViewCache(*g_pAccountViewTip, false));
	++m_unTransactionsUpdated;
}

void CTxMemPool::queryHashes(vector<uint256>& vctxid) {
	vctxid.clear();

	LOCK(m_cs);
	vctxid.reserve(m_mapTx.size());
	for (typename map<uint256, CTxMemPoolEntry>::iterator mi = m_mapTx.begin(); mi != m_mapTx.end(); ++mi) {
		vctxid.push_back((*mi).first);
	}
}

std::shared_ptr<CBaseTransaction> CTxMemPool::lookup(uint256 cHash) const {
	LOCK(m_cs);
	typename map<uint256, CTxMemPoolEntry>::const_iterator iter = m_mapTx.find(cHash);
	if (iter == m_mapTx.end()) {
		return std::shared_ptr<CBaseTransaction>();
	}
	return iter->second.GetTx();
}
