// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_MINER_H_
#define DACRS_MINER_H_

#include <stdint.h>
#include <vector>
#include <map>
#include <set>

#include "uint256.h"
#include "key.h"
#include "boost/tuple/tuple.hpp"
#include <memory>

class CBlock;
class CBlockIndex;
struct CBlockTemplate;
class CWallet;
class CBaseTransaction;
class COrphan;
class CAccountViewCache;
class CTransactionDBCache;
class CScriptDBViewCache;

typedef boost::tuple<double, double, std::shared_ptr<CBaseTransaction> > TxPriority;
class TxPriorityCompare {
 public:
	TxPriorityCompare(bool bByFee) :
			m_bByFee(bByFee) {
	}

	bool operator()(const TxPriority& TxPriorityA, const TxPriority& TxPriorityB) {
		if (m_bByFee) {
			if (TxPriorityA.get<1>() == TxPriorityB.get<1>()) {
				return TxPriorityA.get<0>() < TxPriorityB.get<0>();
			}
			return TxPriorityA.get<1>() < TxPriorityB.get<1>();
		} else {
			if (TxPriorityA.get<0>() == TxPriorityB.get<0>()) {
				return TxPriorityA.get<1>() < TxPriorityB.get<1>();
			}
			return TxPriorityA.get<0>() < TxPriorityB.get<0>();
		}
	}

 private:
	bool m_bByFee;
};

/** Run the miner threads */
void GenerateDacrsBlock(bool bGenerate, CWallet* pWallet, int nThreads);

/** Generate a new block, without valid proof-of-work */
//CBlockTemplate* CreateNewBlock(const CScript& scriptPubKeyIn);
//CBlockTemplate* CreateNewBlockWithKey(CReserveKey& reservekey);
CBlockTemplate* CreateNewBlock(CAccountViewCache &cAccViewCache, CTransactionDBCache &cTxCache, CScriptDBViewCache &cScriptCache);

/** Modify the extranonce in a block */
void IncrementExtraNonce(CBlock* pBlock, CBlockIndex* pBlockIndexPrev, unsigned int& unExtraNonce);

/** Do mining precalculation */
void FormatHashBuffers(CBlock* pBlock, char* pMidstate, char* pData, char* pHash1);

bool CreatePosTx(const CBlockIndex *pPrevIndex, CBlock *pBlock, set<CKeyID>&setCreateKey, CAccountViewCache &cAccountViewCache,
		CTransactionDBCache &cTxCache, CScriptDBViewCache &cScriptCache);

bool VerifyPosTx(CAccountViewCache &cAccountViewCache, const CBlock *pBlock, CTransactionDBCache &cTxDBCache,
		CScriptDBViewCache &cScriptDBViewCache, bool bNeedRunTx = false);

/** Check mined block */
bool CheckWork(CBlock* pBlock, CWallet& cWallet);

/** Base sha256 mining transform */
void SHA256Transform(void* pState, void* pInput, const void* pInit);

/** Get burn element */
int GetElementForBurn(CBlockIndex *pBlockIndex);

void GetPriorityTx(vector<TxPriority> &vPriority, int nFuelRate);

extern uint256 CreateBlockWithAppointedAddr(CKeyID const &cKeyID);

#endif // DACRS_MINER_H_
