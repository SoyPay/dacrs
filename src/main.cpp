// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "main.h"

#include "addrman.h"
#include "alert.h"
#include "chainparams.h"
#include "checkpoints.h"
#include "checkqueue.h"
#include "init.h"
#include "net.h"
#include "txdb.h"
#include "txmempool.h"
#include "ui_interface.h"
#include "util.h"
#include "miner.h"
#include "tx.h"
#include "syncdatadb.h"
#include "vm/vmrunevn.h"
#include <sstream>

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <memory>
#include <algorithm>

#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_writer_template.h"
using namespace json_spirit;

using namespace std;
using namespace boost;

#if defined(NDEBUG)
# error "Dacrs cannot be compiled without assertions."
#endif

//
// Global state
//

CCriticalSection g_cs_main;

CTxMemPool g_cTxMemPool;  //存放收到的未被执行,未收集到块的交易
//static const unsigned int nStakeTargetSpacing = 60;  // 60 sec block spacing
map<uint256, CBlockIndex*> g_mapBlockIndex;
CChain g_cChainActive;
CChain g_cChainMostWork;
int g_nSyncTipHeight(0);  //同步时 ,chainActive.Tip()->nHeight

map<uint256, std::tuple<std::shared_ptr<CAccountViewCache>, std::shared_ptr<CTransactionDBCache>,
				std::shared_ptr<CScriptDBViewCache> > > mapCache;

CSignatureCache g_cSignatureCache;

/** Fees smaller than this (in satoshi) are considered zero fee (for transaction creation) */
uint64_t CBaseTransaction::m_sMinTxFee = 10000;  				// Override with -mintxfee
/** Fees smaller than this (in satoshi) are considered zero fee (for relaying and mining) */
int64_t CBaseTransaction::m_sMinRelayTxFee = 1000;

static CMedianFilter<int> g_scPeerBlockCounts(8, 0); 			// Amount of blocks that other nodes claim to have

struct ST_OrphanBlock {
    uint256 hashBlock;
    uint256 hashPrev;
    int     height;
    vector<unsigned char> vchBlock;
};
map<uint256, ST_OrphanBlock*> g_mapOrphanBlocks;  				//存放因网络延迟等原因，收到的孤儿块
multimap<uint256, ST_OrphanBlock*> g_mapOrphanBlocksByPrev;  	//存放孤儿块的上一个块Hash及块(pblock2->hashPrev, pblock2)

map<uint256, std::shared_ptr<CBaseTransaction> > g_mapOrphanTransactions;
map<uint256, set<uint256> > g_mapOrphanTransactionsByPrev;

//map<string, CContractScript> mapScript;
// Constant stuff for coinbase transactions we create:
//CScript COINBASE_FLAGS;

const string g_strMessageMagic = "Dacrs Signed Message:\n";

// Internal stuff
namespace {
struct CBlockIndexWorkComparator {
	bool operator()(CBlockIndex *pBlockA, CBlockIndex *pBlockB) {
		// First sort by most total work, ...
		if (pBlockA->m_cChainWork > pBlockB->m_cChainWork) {
			return false;
		}
		if (pBlockA->m_cChainWork < pBlockB->m_cChainWork) {
			return true;
		}

		// ... then by earliest time received, ...
		if (pBlockA->m_uSequenceId < pBlockB->m_uSequenceId) {
			return false;
		}
		if (pBlockA->m_uSequenceId > pBlockB->m_uSequenceId) {
			return true;
		}

		// Use pointer address as tie breaker (should only happen with blocks
		// loaded from disk, as those all have id 0).
		if (pBlockA < pBlockB) {
			return false;
		}
		if (pBlockA > pBlockB) {
			return true;
		}

		// Identical blocks.
		return false;
	}
};

CBlockIndex *g_pIndexBestInvalid;
// may contain all CBlockIndex*'s that have validness >=BLOCK_VALID_TRANSACTIONS, and must contain those who aren't failed
set<CBlockIndex*, CBlockIndexWorkComparator> g_setBlockIndexValid;		//根据高度排序的有序集合

struct ST_OrphanBlockComparator {
	bool operator()(ST_OrphanBlock* pBlcokA, ST_OrphanBlock* pBlockB) {
		if (pBlcokA->height > pBlockB->height) {
			return false;
		}
		if (pBlcokA->height < pBlockB->height) {
			return true;
		}
		return false;
	}
};
    set<ST_OrphanBlock*, ST_OrphanBlockComparator> setOrphanBlock; //存的孤立块

    CCriticalSection g_cs_LastBlockFile;
    CBlockFileInfo g_InfoLastBlockFile;
    int g_nLastBlockFile = 0;

    // Every received block is assigned a unique and increasing identifier, so we
    // know which one to give priority in case of a fork.
    CCriticalSection g_cs_nBlockSequenceId;
    // Blocks loaded from disk are assigned id 0, so start the counter at 1.
    uint32_t g_ullBlockSequenceId = 1;

    // Sources of received blocks, to be able to send them reject messages or ban
    // them, if processing happens afterwards. Protected by cs_main.
    map<uint256, NodeId> g_mapBlockSource;  // Remember who we got this block from.

    // Blocks that are in flight, and that are in the queue to be downloaded.
    // Protected by cs_main.
    struct ST_QueuedBlock {
        uint256 hash;
        int64_t nTime;  // Time of "getdata" request in microseconds.
        int nQueuedBefore;  // Number of blocks in flight at the time of request.
    };

    map<uint256, pair<NodeId, list<ST_QueuedBlock>::iterator> > g_mapBlocksInFlight;
    map<uint256, pair<NodeId, list<uint256>::iterator> > g_mapBlocksToDownload; //存放待下载到的块，下载后执行erase

}

//////////////////////////////////////////////////////////////////////////////
//
// dispatching functions
//

// These functions dispatch to one or all registered wallets

namespace {
struct CMainSignals {
	// Notifies listeners of updated transaction data (passing hash, transaction, and optionally the block it is found in.
	boost::signals2::signal<void(const uint256 &, CBaseTransaction *, const CBlock *)> SyncTransaction;
	// Notifies listeners of an erased transaction (currently disabled, requires transaction replacement).
	boost::signals2::signal<void(const uint256 &)> EraseTransaction;
	// Notifies listeners of an updated transaction without new data (for now: a coinbase potentially becoming visible).
	boost::signals2::signal<void(const uint256 &)> UpdatedTransaction;
	// Notifies listeners of a new active block chain.
	boost::signals2::signal<void(const ST_BlockLocator &)> SetBestChain;
	// Notifies listeners about an inventory item being seen on the network.
	boost::signals2::signal<void(const uint256 &)> Inventory;
	// Tells listeners to broadcast their data.
	boost::signals2::signal<void()> Broadcast;
} g_signals;
}

bool WriteBlockLog(bool bFalg, string strSuffix) {
	if (NULL == g_cChainActive.Tip()) {
		return false;
	}
	char splitChar;
	#ifdef WIN32
		splitChar = '\\';
	#else
		splitChar = '/';
	#endif

	boost::filesystem::path LogDirpath = GetDataDir() / "BlockLog";
	if(!bFalg){
		LogDirpath = GetDataDir() / "BlockLog1";
	}
	if (!boost::filesystem::exists(LogDirpath)) {
		boost::filesystem::create_directory(LogDirpath);
	}

	ofstream file;
	int high = g_cChainActive.Height();
	string strLogFilePath = LogDirpath.string();
	strLogFilePath += splitChar + strprintf("%d_",high) +g_cChainActive.Tip()->GetBlockHash().ToString();

	string strScriptLog = strLogFilePath +"_scriptDB_"+ strSuffix +".txt";
	file.open(strScriptLog);
	if (!file.is_open()) {
		return false;
	}

	file << write_string(Value(g_pScriptDBTip->ToJosnObj()), true);
	file.close();

	string strAccountViewLog = strLogFilePath +"_AccountView_"+ strSuffix +".txt";
	file.open(strAccountViewLog);
	if (!file.is_open()) {
		return false;
	}
	file << write_string(Value(g_pAccountViewTip->ToJosnObj()), true);
	file.close();

	string strCacheLog = strLogFilePath + "_Cache_" + strSuffix +".txt";
	file.open(strCacheLog);
	if (!file.is_open()) {
		return false;
	}
	file << write_string(Value(g_pTxCacheTip->ToJosnObj()), true);
	file.close();

	string strundoLog = strLogFilePath +"_undo.txt";
	file.open(strundoLog);
	if (!file.is_open()) {
		return false;
	}
    CBlockUndo blockUndo;
    ST_DiskBlockPos pos =g_cChainActive.Tip()->GetUndoPos();
    if (!pos.IsNull()){
    	 if (blockUndo.ReadFromDisk(pos, g_cChainActive.Tip()->m_pPrevBlockIndex->GetBlockHash())) {
    		 file << blockUndo.ToString();
    	 }
    }

	file.close();
	return true;
}


void RegisterWallet(CWalletInterface* pWalletIn) {
    g_signals.SyncTransaction.connect(boost::bind(&CWalletInterface::SyncTransaction, pWalletIn, _1, _2, _3));
    g_signals.EraseTransaction.connect(boost::bind(&CWalletInterface::EraseFromWallet, pWalletIn, _1));
    g_signals.UpdatedTransaction.connect(boost::bind(&CWalletInterface::UpdatedTransaction, pWalletIn, _1));
    g_signals.SetBestChain.connect(boost::bind(&CWalletInterface::SetBestChain, pWalletIn, _1));
    g_signals.Broadcast.connect(boost::bind(&CWalletInterface::ResendWalletTransactions, pWalletIn));
}

void UnregisterWallet(CWalletInterface* pWalletIn) {
    g_signals.Broadcast.disconnect(boost::bind(&CWalletInterface::ResendWalletTransactions, pWalletIn));
    g_signals.SetBestChain.disconnect(boost::bind(&CWalletInterface::SetBestChain, pWalletIn, _1));
    g_signals.UpdatedTransaction.disconnect(boost::bind(&CWalletInterface::UpdatedTransaction, pWalletIn, _1));
    g_signals.EraseTransaction.disconnect(boost::bind(&CWalletInterface::EraseFromWallet, pWalletIn, _1));
    g_signals.SyncTransaction.disconnect(boost::bind(&CWalletInterface::SyncTransaction, pWalletIn, _1, _2, _3));
}

void UnregisterAllWallets() {
    g_signals.Broadcast.disconnect_all_slots();
    g_signals.Inventory.disconnect_all_slots();
    g_signals.SetBestChain.disconnect_all_slots();
    g_signals.UpdatedTransaction.disconnect_all_slots();
    g_signals.EraseTransaction.disconnect_all_slots();
    g_signals.SyncTransaction.disconnect_all_slots();
}

void SyncWithWallets(const uint256 &cHash, CBaseTransaction *pBaseTx, const CBlock* pBlock) {
    g_signals.SyncTransaction(cHash, pBaseTx, pBlock);
}

void EraseTransaction(const uint256 &cHash) {
	g_signals.EraseTransaction(cHash);
}

//////////////////////////////////////////////////////////////////////////////
//
// Registration of network node signals.
//

namespace {

struct ST_BlockReject {
    unsigned char chRejectCode;
    string strRejectReason;
    uint256 hashBlock;
};

// Maintain validation-specific state about nodes, protected by cs_main, instead
// by CNode's own locks. This simplifies asynchronous operation, where
// processing of incoming data is done after the ProcessMessage call returns,
// and we're no longer holding the node's locks.
struct ST_NodeState {
    // Accumulated misbehaviour score for this peer.
    int nMisbehavior;
    // Whether this peer should be disconnected and banned.
    bool fShouldBan;
    // String name of this peer (debugging/logging purposes).
    string name;
    // List of asynchronously-determined block rejections to notify this peer about.
    vector<ST_BlockReject> rejects;
    list<ST_QueuedBlock> vBlocksInFlight;
    int nBlocksInFlight;              	//每个节点,单独能下载的最大块数量   MAX_BLOCKS_IN_TRANSIT_PER_PEER
    list<uint256> vBlocksToDownload;  	//待下载的块
    int nBlocksToDownload;            	//待下载的块个数
    int64_t nLastBlockReceive;   		//上一次收到块的时间
    int64_t nLastBlockProcess;   		//收到块，处理消息时的时间

    ST_NodeState() {
        nMisbehavior = 0;
        fShouldBan = false;
        nBlocksToDownload = 0;
        nBlocksInFlight = 0;
        nLastBlockReceive = 0;
        nLastBlockProcess = 0;
    }
};

// Map maintaining per-node state. Requires cs_main.
map<NodeId, ST_NodeState> g_mapNodeState;

// Requires cs_main.
ST_NodeState *State(NodeId pNode) {
    map<NodeId, ST_NodeState>::iterator it = g_mapNodeState.find(pNode);
    if (it == g_mapNodeState.end()) {
    	return NULL;
    }
    return &it->second;
}

int GetHeight() {
	LOCK(g_cs_main);
	return g_cChainActive.Height();
}

void InitializeNode(NodeId cNodeId, const CNode *pNode) {
	LOCK(g_cs_main);
	ST_NodeState &tNodeState = g_mapNodeState.insert(make_pair(cNodeId, ST_NodeState())).first->second;
	tNodeState.name = pNode->m_strAddrName;
}

void FinalizeNode(NodeId cNodeId) {
	LOCK(g_cs_main);
	ST_NodeState *tNodeState = State(cNodeId);

	for (const auto& entry : tNodeState->vBlocksInFlight) {
		g_mapBlocksInFlight.erase(entry.hash);
	}
	for (const auto& hash : tNodeState->vBlocksToDownload) {
		g_mapBlocksToDownload.erase(hash);
	}

	g_mapNodeState.erase(cNodeId);
}

// Requires cs_main.
void MarkBlockAsReceived(const uint256 &cHash, NodeId cNodeFrom = -1) {
	map<uint256, pair<NodeId, list<uint256>::iterator> >::iterator itToDownload = g_mapBlocksToDownload.find(cHash);
	if (itToDownload != g_mapBlocksToDownload.end()) {
		ST_NodeState *pState = State(itToDownload->second.first);
		pState->vBlocksToDownload.erase(itToDownload->second.second);
		pState->nBlocksToDownload--;
		g_mapBlocksToDownload.erase(itToDownload);
	}

	map<uint256, pair<NodeId, list<ST_QueuedBlock>::iterator> >::iterator itInFlight = g_mapBlocksInFlight.find(cHash);
	if (itInFlight != g_mapBlocksInFlight.end()) {
		ST_NodeState *state = State(itInFlight->second.first);
		state->vBlocksInFlight.erase(itInFlight->second.second);
		state->nBlocksInFlight--;
		if (itInFlight->second.first == cNodeFrom) {
			state->nLastBlockReceive = GetTimeMicros();
		}
		g_mapBlocksInFlight.erase(itInFlight);
	}
}

// Requires cs_main.
bool AddBlockToQueue(NodeId cNodeId, const uint256 &cHash) {
    if (g_mapBlocksToDownload.count(cHash) || g_mapBlocksInFlight.count(cHash)) {
    	return false;
    }

    ST_NodeState *pState = State(cNodeId);
    if (pState == NULL) {
    	return false;
    }

    list<uint256>::iterator it = pState->vBlocksToDownload.insert(pState->vBlocksToDownload.end(), cHash);
    pState->nBlocksToDownload++;
    if (pState->nBlocksToDownload > 5000) {
    	LogPrint("INFO", "Misbehaving,AddBlockToQueue download to many times, nMisbehavior add 10\n");
    	Misbehaving(cNodeId, 10);
    }
    g_mapBlocksToDownload[cHash] = make_pair(cNodeId, it);
    return true;
}

// Requires cs_main.
void MarkBlockAsInFlight(NodeId cNodeId, const uint256 &cHash) {
    ST_NodeState *pState = State(cNodeId);
    assert(pState != NULL);

    // Make sure it's not listed somewhere already.
    MarkBlockAsReceived(cHash);

    ST_QueuedBlock tNewEntry = {cHash, GetTimeMicros(), pState->nBlocksInFlight};
    if (pState->nBlocksInFlight == 0) {
    	pState->nLastBlockReceive = tNewEntry.nTime; // Reset when a first request is sent.
    }
    list<ST_QueuedBlock>::iterator it = pState->vBlocksInFlight.insert(pState->vBlocksInFlight.end(), tNewEntry);
    pState->nBlocksInFlight++;
    g_mapBlocksInFlight[cHash] = make_pair(cNodeId, it);
}
}

bool GetNodeStateStats(NodeId nNodeId, ST_NodeStateStats &NodeStateStats) {
    LOCK(g_cs_main);
    ST_NodeState *pState = State(nNodeId);
    if (pState == NULL) {
    	return false;
    }

    NodeStateStats.nMisbehavior = pState->nMisbehavior;
    return true;
}

void RegisterNodeSignals(ST_NodeSignals& tNodeSignals) {
	tNodeSignals.GetHeight.connect(&GetHeight);
	tNodeSignals.ProcessMessages.connect(&ProcessMessages);
	tNodeSignals.SendMessages.connect(&SendMessages);
	tNodeSignals.InitializeNode.connect(&InitializeNode);
	tNodeSignals.FinalizeNode.connect(&FinalizeNode);
}

void UnregisterNodeSignals(ST_NodeSignals& tNodeSignals) {
	tNodeSignals.GetHeight.disconnect(&GetHeight);
	tNodeSignals.ProcessMessages.disconnect(&ProcessMessages);
	tNodeSignals.SendMessages.disconnect(&SendMessages);
	tNodeSignals.InitializeNode.disconnect(&InitializeNode);
	tNodeSignals.FinalizeNode.disconnect(&FinalizeNode);
}

//////////////////////////////////////////////////////////////////////////////
	//
// CChain implementation
//

CBlockIndex *CChain::SetTip(CBlockIndex *pIndex) {
    if (pIndex == NULL) {
        m_vcChain.clear();
        return NULL;
    }
    m_vcChain.resize(pIndex->m_nHeight + 1);
    while (pIndex && m_vcChain[pIndex->m_nHeight] != pIndex) {
        m_vcChain[pIndex->m_nHeight] = pIndex;
        pIndex = pIndex->m_pPrevBlockIndex;
    }
    return pIndex;
}

ST_BlockLocator CChain::GetLocator(const CBlockIndex *pIndex) const {
    int nStep = 1;
    vector<uint256> vHave;
    vHave.reserve(32);

    if (!pIndex) {
    	pIndex = Tip();
    }

    while (pIndex) {
        vHave.push_back(pIndex->GetBlockHash());
        // Stop when we have added the genesis block.
        if (pIndex->m_nHeight == 0) {
        	break;
        }
        // Exponentially larger steps back, plus the genesis block.
        int nHeight = max(pIndex->m_nHeight - nStep, 0);
        // In case pindex is not in this chain, iterate pindex->pprev to find blocks.
        while (pIndex->m_nHeight > nHeight && !Contains(pIndex)) {
        	pIndex = pIndex->m_pPrevBlockIndex;
        }
        // If pindex is in this chain, use direct height-based access.
        if (pIndex->m_nHeight > nHeight) {
        	pIndex = (*this)[nHeight];
        }
        if (vHave.size() > 10) {
        	nStep *= 2;
        }
    }
    return ST_BlockLocator(vHave);
}

CBlockIndex *CChain::FindFork(const ST_BlockLocator &tLocator) const {
	// Find the first block the caller has in the main chain
	for (const auto& hash : tLocator.vcHave) {
		map<uint256, CBlockIndex*>::iterator mi = g_mapBlockIndex.find(hash);
		if (mi != g_mapBlockIndex.end()) {
			CBlockIndex* pindex = (*mi).second;
			if (pindex && Contains(pindex)) {
				return pindex;
			}
		}
	}
	return Genesis();
}

CAccountViewDB *g_pAccountViewDB = NULL;
CBlockTreeDB *g_pblocktree = NULL;
CAccountViewCache *g_pAccountViewTip = NULL;
CTransactionDB *g_pTxCacheDB = NULL;
CTransactionDBCache *g_pTxCacheTip = NULL;
CScriptDB *g_pScriptDB = NULL;
CScriptDBViewCache *g_pScriptDBTip = NULL;

unsigned int LimitOrphanTxSize(unsigned int nMaxOrphans) {
	unsigned int nEvicted = 0;
	while (g_mapOrphanTransactions.size() > nMaxOrphans) {
		// Evict a random orphan:
		uint256 randomhash = GetRandHash();
		map<uint256, std::shared_ptr<CBaseTransaction> >::iterator it = g_mapOrphanTransactions.lower_bound(randomhash);
		if (it == g_mapOrphanTransactions.end()) {
			it = g_mapOrphanTransactions.begin();
		}
		g_mapOrphanTransactions.erase(it->first);
		++nEvicted;
	}
	return nEvicted;
}

bool IsStandardTx(CBaseTransaction *pBaseTx, string& strReason) {
	AssertLockHeld(g_cs_main);
	if (pBaseTx->m_nVersion > CTransaction::m_sCurrentVersion || pBaseTx->m_nVersion < 1) {
		strReason = "version";
		return false;
	}

	// Extremely large transactions with lots of inputs can cost the network
	// almost as much to process as they cost the sender in fees, because
	// computing signature hashes is O(ninputs*txsize). Limiting transactions
	// to MAX_STANDARD_TX_SIZE mitigates CPU exhaustion attacks.
	unsigned int sz = ::GetSerializeSize(pBaseTx->GetNewInstance(), SER_NETWORK, CTransaction::m_sCurrentVersion);
	if (sz >= MAX_STANDARD_TX_SIZE) {
		strReason = "tx-size";
		return false;
	}

	return true;
}

bool IsFinalTx(CBaseTransaction *ptx, int nBlockHeight, int64_t llBlockTime) {
	AssertLockHeld(g_cs_main);
	return true;
}

int CMerkleTx::SetMerkleBranch(const CBlock* pBlock) {
	AssertLockHeld(g_cs_main);
	CBlock cBlockTmp;

	if (pBlock) {
		// Update the tx's hashBlock
		m_cHashBlock = pBlock->GetHash();

		// Locate the transaction
		for (m_nIndex = 0; m_nIndex < (int) pBlock->vptx.size(); m_nIndex++) {
			if ((pBlock->vptx[m_nIndex])->GetHash() == m_ptrTx->GetHash()) {
				break;
			}
		}

		if (m_nIndex == (int) pBlock->vptx.size()) {
			m_vcMerkleBranch.clear();
			m_nIndex = -1;
			LogPrint("INFO", "ERROR: SetMerkleBranch() : couldn't find tx in block\n");
			return 0;
		}

		// Fill in merkle branch
		m_vcMerkleBranch = pBlock->GetMerkleBranch(m_nIndex);
	}

	// Is the tx in a block that's in the main chain
	map<uint256, CBlockIndex*>::iterator mi = g_mapBlockIndex.find(m_cHashBlock);
	if (mi == g_mapBlockIndex.end()) {
		return 0;
	}
	CBlockIndex* pIndex = (*mi).second;
	if (!pIndex || !g_cChainActive.Contains(pIndex)) {
		return 0;
	}

	return g_cChainActive.Height() - pIndex->m_nHeight + 1;
}

bool CheckSignScript(const uint256 & cSigHash, const std::vector<unsigned char> vchSignature, const CPubKey cPubKey) {
	if (g_cSignatureCache.Get(cSigHash, vchSignature, cPubKey)) {
		return true;
	}
	if (!cPubKey.Verify(cSigHash, vchSignature)) {
		return false;
	}
	g_cSignatureCache.Set(cSigHash, vchSignature, cPubKey);
	return true;
}

bool CheckTransaction(CBaseTransaction *pBaseTx, CValidationState &cValidationState,
		CAccountViewCache &cAccountViewCache, CScriptDBViewCache &cScriptDBViewCache) {
	if (EM_REWARD_TX == pBaseTx->m_chTxType) {
		return true;
	}
	// check version
	if (pBaseTx->m_nValidHeight > g_sUpdateTxVersion2Height) {
		if (pBaseTx->m_nVersion != g_sTxVersion2) {
			return cValidationState.DoS(100,
					ERRORMSG(
							"CheckTransaction() : CheckTransction,tx version is not equal current version, (tx version %d: vs current %d)",
							pBaseTx->m_nVersion, g_sTxVersion2));
		}
	}

	// Size limits
	if (::GetSerializeSize(pBaseTx->GetNewInstance(), SER_NETWORK, g_sProtocolVersion) > MAX_BLOCK_SIZE) {
		return cValidationState.DoS(100, ERRORMSG("CheckTransaction() : size limits failed"), REJECT_INVALID,
						"bad-txns-oversize");
	}
	if (!pBaseTx->CheckTransction(cValidationState, cAccountViewCache, cScriptDBViewCache)) {
		return false;
	}

	return true;
}

int64_t GetMinFee(const CBaseTransaction *pBaseTx, unsigned int unBytes, bool bAllowFree, enum GetMinFee_mode mode) {
	// Base fee is either nMinTxFee or nMinRelayTxFee
	int64_t nBaseFee = (mode == GMF_RELAY) ? pBaseTx->m_sMinRelayTxFee : pBaseTx->m_sMinTxFee;
	int64_t nMinFee = (1 + (int64_t) unBytes / 1000) * nBaseFee;

	if (bAllowFree) {
		// There is a free transaction area in blocks created by most miners,
		// * If we are relaying we allow transactions up to DEFAULT_BLOCK_PRIORITY_SIZE - 1000
		//   to be considered to fall into this category. We don't want to encourage sending
		//   multiple transactions instead of one big transaction to avoid fees.
		// * If we are creating a transaction we allow transactions up to 1,000 bytes
		//   to be considered safe and assume they can likely make it into this section.
		if (unBytes < (mode == GMF_SEND ? 1000 : (DEFAULT_BLOCK_PRIORITY_SIZE - 1000)))
			nMinFee = 0;
	}

	// This code can be removed after enough miners have upgraded to version 0.9.
	// Until then, be safe when sending and require a fee if any output
	// is less than CENT:
	if (nMinFee < nBaseFee && mode == GMF_SEND) {
		// BOOST_FOREACH(const CTxOut& txout, tx.vout)
		// if (txout.nValue < CENT)
		// nMinFee = nBaseFee;
	}

	if (!MoneyRange(nMinFee)) {
		nMinFee = GetMaxMoney();
	}

	return nMinFee;
}

bool AcceptToMemoryPool(CTxMemPool& cTxMemPool, CValidationState &cValidationState, CBaseTransaction *pBaseTx,
		bool bLimitFree, bool bRejectInsaneFee) {
	AssertLockHeld(g_cs_main);

	// is it already in the memory pool?
	uint256 cHash = pBaseTx->GetHash();
	if (cTxMemPool.exists(cHash)) {
		return false;
	}

	// is it already confirmed in block
	if (uint256() != g_pTxCacheTip->IsContainTx(cHash)) {
		return cValidationState.Invalid(
						ERRORMSG("AcceptToMemoryPool() : tx hash %s has been confirmed\n", cHash.GetHex()), REJECT_INVALID,
						"tx-duplicate-confirmed");
	}

	if (pBaseTx->IsCoinBase()) {
		return cValidationState.Invalid(
						ERRORMSG("AcceptToMemoryPool() : tx hash %s is coin base tx,can't put into mempool", cHash.GetHex()),
						REJECT_INVALID, "tx-coinbase-to-mempool");
	}

	// is it in valid height
	if (!pBaseTx->IsValidHeight(g_cChainActive.Tip()->m_nHeight, SysCfg().GetTxCacheHeight())) {
		return cValidationState.Invalid(
				ERRORMSG("AcceptToMemoryPool() : txhash=%s beyond the scope of valid height\n ", cHash.GetHex()),
				REJECT_INVALID, "tx-invalid-height");
	}

	// CAccountViewCache view(*pAccountViewTip, true);
	if (!CheckTransaction(pBaseTx, cValidationState, *cTxMemPool.m_pAccountViewCache, *cTxMemPool.m_pScriptDBViewCache)) {
		return ERRORMSG("AcceptToMemoryPool: : CheckTransaction failed");
	}

	// Rather not work on nonstandard transactions (unless -testnet/-regtest)
	string strReason;
	if (SysCfg().NetworkID() == CBaseParams::EM_MAIN && !IsStandardTx(pBaseTx, strReason)) {
		return cValidationState.DoS(0, ERRORMSG("AcceptToMemoryPool : nonstandard transaction: %s", strReason),
						REJECT_NONSTANDARD, strReason);
	}

	{
		double dPriority = pBaseTx->GetPriority();
		int64_t llFees = pBaseTx->GetFee();

		CTxMemPoolEntry cEntry(pBaseTx, llFees, GetTime(), dPriority, g_cChainActive.Height());
		unsigned int nSize = cEntry.GetTxSize();

		if (pBaseTx->m_chTxType == EM_COMMON_TX) {
			CTransaction *pTx = (CTransaction *) pBaseTx;
			if (pTx->m_ullValues < CBaseTransaction::m_sMinTxFee) {
				return cValidationState.DoS(0,
						ERRORMSG("AcceptToMemoryPool : tx %d transfer amount(%d) very small, you must send a min (%d)",
								cHash.ToString(), pTx->m_ullValues, CBaseTransaction::m_sMinTxFee), REJECT_DUST,
						"dust amount");
			}
		}

		// Don't accept it if it can't get into a block
		int64_t llTxMinFee = GetMinFee(pBaseTx, nSize, true, GMF_RELAY);
		if (bLimitFree && llFees < llTxMinFee)
			return cValidationState.DoS(0,
					ERRORMSG("AcceptToMemoryPool : not enough fees %s, %d < %d", cHash.ToString(), llFees, llTxMinFee),
					REJECT_INSUFFICIENTFEE, "insufficient fee");

		// Continuously rate-limit free transactions
		// This mitigates 'penny-flooding' -- sending thousands of free transactions just to
		// be annoying or make others' transactions take longer to confirm.
		if (bLimitFree && llFees < CTransaction::m_sMinRelayTxFee) {
			static CCriticalSection csFreeLimiter;
			static double dFreeCount;
			static int64_t llLastTime;
			int64_t llNow = GetTime();

			LOCK(csFreeLimiter);
			// Use an exponentially decaying ~10-minute window:
			dFreeCount *= pow(1.0 - 1.0 / 600.0, (double) (llNow - llLastTime));
			llLastTime = llNow;
			// -limitfreerelay unit is thousand-bytes-per-minute
			// At default rate it would take over a month to fill 1GB
			if (dFreeCount >= SysCfg().GetArg("-limitfreerelay", 15) * 10 * 1000) {
				return cValidationState.DoS(0,
										ERRORMSG("AcceptToMemoryPool : free transaction rejected by rate limiter"),
										REJECT_INSUFFICIENTFEE, "insufficient priority");
			}

			LogPrint("INFO", "Rate limit dFreeCount: %g => %g\n", dFreeCount, dFreeCount + nSize);
			dFreeCount += nSize;
		}

		if (bRejectInsaneFee && llFees > SysCfg().GetMaxFee()) {
			return ERRORMSG("AcceptToMemoryPool: : insane fees %s, %d > %d", cHash.ToString(), llFees,
								SysCfg().GetMaxFee());
		}

		// Store transaction in memory
		if (!cTxMemPool.addUnchecked(cHash, cEntry, cValidationState)) {
			return ERRORMSG("AcceptToMemoryPool: : addUnchecked failed cHash:%s \r\n", cHash.ToString());
		}
	}

	g_signals.SyncTransaction(cHash, pBaseTx, NULL);

	return true;
}


int CMerkleTx::GetDepthInMainChainINTERNAL(CBlockIndex* &pBlockIndexRet) const {
	if (m_cHashBlock.IsNull() || m_nIndex == -1) {
		return 0;
	}

	AssertLockHeld(g_cs_main);

	// Find the block it claims to be in
	map<uint256, CBlockIndex*>::iterator mi = g_mapBlockIndex.find(m_cHashBlock);
	if (mi == g_mapBlockIndex.end()) {
		return 0;
	}

	CBlockIndex* pIndex = (*mi).second;
	if (!pIndex || !g_cChainActive.Contains(pIndex)) {
		return 0;
	}

	// Make sure the merkle branch connects to this block
	if (!m_bMerkleVerified) {
		if (CBlock::CheckMerkleBranch(m_ptrTx->GetHash(), m_vcMerkleBranch, m_nIndex) != pIndex->m_cHashMerkleRoot) {
			return 0;
		}

		m_bMerkleVerified = true;
	}

	pBlockIndexRet = pIndex;
	return g_cChainActive.Height() - pIndex->m_nHeight + 1;
}

int CMerkleTx::GetDepthInMainChain(CBlockIndex* &pBlockIndexRet) const {
	AssertLockHeld(g_cs_main);
	int nResult = GetDepthInMainChainINTERNAL(pBlockIndexRet);
	if (nResult == 0 && !g_cTxMemPool.exists(m_ptrTx->GetHash())) {
		return -1; // Not in chain, not in mempool
	}

	return nResult;
}

int CMerkleTx::GetBlocksToMaturity() const {
	if (!m_ptrTx->IsCoinBase()) {
		return 0;
	}

	return max(0, (COINBASE_MATURITY + 1) - GetDepthInMainChain());
}


bool CMerkleTx::AcceptToMemoryPool(bool bLimitFree) {
	CValidationState state;
	return ::AcceptToMemoryPool(g_cTxMemPool, state, m_ptrTx.get(), bLimitFree, NULL);
}

int GetTxComfirmHigh(const uint256 &cHash, CScriptDBViewCache &cScriptDBCache) {
	if (SysCfg().IsTxIndex()) {
		ST_DiskTxPos tPostx;
		if (cScriptDBCache.ReadTxIndex(cHash, tPostx)) {
			CAutoFile file(OpenBlockFile(tPostx, true), SER_DISK, g_sClientVersion);
			CBlockHeader cBlockHeader;
			try {
				file >> cBlockHeader;

			} catch (std::exception &e) {
				ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
				return -1;
			}
			return cBlockHeader.GetHeight();
		}
	}
	return -1;
}

// Return transaction in tx, and if it was found inside a block, its hash is placed in hashBlock
bool GetTransaction(std::shared_ptr<CBaseTransaction> &pBaseTx, const uint256 &cHash,
		CScriptDBViewCache& cScriptDBCache, bool bSearchMemPool) {
	{
		LOCK(g_cs_main);
		{
			if (bSearchMemPool == true) {
				pBaseTx = g_cTxMemPool.lookup(cHash);
				if (pBaseTx.get()) {
					return true;
				}
			}
		}

		if (SysCfg().IsTxIndex()) {
			ST_DiskTxPos postx;
			if (cScriptDBCache.ReadTxIndex(cHash, postx)) {
				CAutoFile file(OpenBlockFile(postx, true), SER_DISK, g_sClientVersion);
				CBlockHeader header;
				try {
					file >> header;
					fseek(file, postx.m_unTxOffset, SEEK_CUR);
					file >> pBaseTx;
				} catch (std::exception &e) {
					return ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
				}
				return true;
			}
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////
//
// CBlock and CBlockIndex
//

bool WriteBlockToDisk(CBlock& cBlock, ST_DiskBlockPos& tDiskBlockPos) {
	// Open history file to append
	CAutoFile fileout = CAutoFile(OpenBlockFile(tDiskBlockPos), SER_DISK, g_sClientVersion);
	if (!fileout) {
		return ERRORMSG("WriteBlockToDisk : OpenBlockFile failed");
	}

	// Write index header
	unsigned int nSize = fileout.GetSerializeSize(cBlock);
	fileout << FLATDATA(SysCfg().MessageStart()) << nSize;

	// Write block
	long lFileOutPos = ftell(fileout);
	if (lFileOutPos < 0) {
		return ERRORMSG("WriteBlockToDisk : ftell failed");
	}

	tDiskBlockPos.unPos = (unsigned int) lFileOutPos;
	fileout << cBlock;

	// Flush stdio buffers and commit to disk before returning
	fflush(fileout);
	if (!IsInitialBlockDownload()) {
		FileCommit(fileout);
	}

	return true;
}

bool ReadBlockFromDisk(CBlock& cBlock, const ST_DiskBlockPos& tDiskBlockPos) {
	cBlock.SetNull();

	// Open history file to read
	CAutoFile cFilein = CAutoFile(OpenBlockFile(tDiskBlockPos, true), SER_DISK, g_sClientVersion);
	if (!cFilein) {
		return ERRORMSG("ReadBlockFromDisk : OpenBlockFile failed");
	}

	// Read block
	try {
		cFilein >> cBlock;
	} catch (std::exception &e) {
		return ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
	}

	// Check the header
	if (!CheckProofOfWork(cBlock.GetHash(), cBlock.GetBits())) {
		return ERRORMSG("ReadBlockFromDisk : Errors in block header");
	}

	return true;
}

bool ReadBlockFromDisk(CBlock& cBlock, const CBlockIndex* pBlockIndex) {
	if (!ReadBlockFromDisk(cBlock, pBlockIndex->GetBlockPos())) {
		return false;
	}
	if (cBlock.GetHash() != pBlockIndex->GetBlockHash()) {
		return ERRORMSG("ReadBlockFromDisk(CBlock&, CBlockIndex*) : GetHash() doesn't match index");
	}

	return true;
}

uint256 static GetOrphanRoot(const uint256& cHash) {
	map<uint256, ST_OrphanBlock*>::iterator it = g_mapOrphanBlocks.find(cHash);
	if (it == g_mapOrphanBlocks.end()) {
		return cHash;
	}
	// Work back to the first block in the orphan chain
	do {
		map<uint256, ST_OrphanBlock*>::iterator it2 = g_mapOrphanBlocks.find(it->second->hashPrev);
		if (it2 == g_mapOrphanBlocks.end()) {
			return it->first;
		}
		it = it2;
	} while (true);
}

// Remove a random orphan block (which does not have any dependent orphans).
bool static PruneOrphanBlocks(int nHeight) {
	if (g_mapOrphanBlocksByPrev.size() <= MAX_ORPHAN_BLOCKS) {
		return true;
	}

	// Pick a random orphan block.
	//    int pos = insecure_rand() % mapOrphanBlocksByPrev.size();
	//    multimap<uint256, COrphanBlock*>::iterator it = mapOrphanBlocksByPrev.begin();
	//    while (pos--) it++;
	//
	//    // As long as this block has other orphans depending on it, move to one of those successors.
	//    do {
	//        multimap<uint256, COrphanBlock*>::iterator it2 = mapOrphanBlocksByPrev.find(it->second->hashBlock);
	//        if (it2 == mapOrphanBlocksByPrev.end())
	//            break;
	//        it = it2;
	//    } while(1);
	//for debug test, release version drop it 
	LogPrint("OrphanBlock", "\n\n\n mapOrphanBlocks size:%d\n", g_mapOrphanBlocks.size());
	LogPrint("OrphanBlock", "OrphanBlock set size:%d\n", setOrphanBlock.size());
	set<ST_OrphanBlock*, ST_OrphanBlockComparator>::reverse_iterator it1 = setOrphanBlock.rbegin();
	int nSize = setOrphanBlock.size();
	for (; it1 != setOrphanBlock.rend(); ++it1) {
		LogPrint("OrphanBlock", "OrphanBlock %d height=%d hash=%s\n", nSize--, (*it1)->height,
				(*it1)->hashBlock.GetHex());
	}

	set<ST_OrphanBlock*, ST_OrphanBlockComparator>::reverse_iterator it = setOrphanBlock.rbegin();
	ST_OrphanBlock *pOrphanBlock = *it;
	if (pOrphanBlock->height <= nHeight) {
		return false;
	}

	LogPrint("INFO", " Update OrphanBlock height=%d hash=%s\n", (*it)->height, (*it)->hashBlock.GetHex());
	LogPrint("OrphanBlock", " Update OrphanBlock height=%d hash=%s\n", (*it)->height, (*it)->hashBlock.GetHex());
	uint256 cHash = pOrphanBlock->hashBlock;
	uint256 cPrevHash = pOrphanBlock->hashPrev;
	setOrphanBlock.erase(pOrphanBlock);
	multimap<uint256, ST_OrphanBlock*>::iterator beg = g_mapOrphanBlocksByPrev.lower_bound(cPrevHash);
	multimap<uint256, ST_OrphanBlock*>::iterator end = g_mapOrphanBlocksByPrev.upper_bound(cPrevHash);
	while (beg != end) {
		if (beg->second->hashBlock == cHash) {
			LogPrint("INFO", " Update PreOrphanBlockMap key=%s value=%s\n", beg->first.GetHex(),
					beg->second->hashBlock.GetHex());
			LogPrint("OrphanBlock", " Update PreOrphanBlockMap key=%s value=%s\n", beg->first.GetHex(),
					beg->second->hashBlock.GetHex());
			g_mapOrphanBlocksByPrev.erase(beg);
			break;
		}
		++beg;
	}
	g_mapOrphanBlocks.erase(cHash);
	delete pOrphanBlock;
	return true;
}

int64_t GetBlockValue(int nHeight, int64_t llFees) {
	int64_t llSubsidy = 50 * COIN;
	int halvings = nHeight / SysCfg().SubsidyHalvingInterval();

	// Force block reward to zero when right shift is undefined.
	if (halvings >= 64) {
		return llFees;
	}

	// Subsidy is cut in half every 210,000 blocks which will occur approximately every 4 years.
	llSubsidy >>= halvings;

	return llSubsidy + llFees;
}


//
// minimum amount of work that could possibly be required nTime after
// minimum work required was nBase
//
unsigned int ComputeMinWork(unsigned int unBase, int64_t llTime) {
	arith_uint256 bnLimit = SysCfg().ProofOfWorkLimit();
//	LogPrint("INFO", "bnLimit:%s\n", bnLimit.getuint256().GetHex());
	bool bNegative;
	bool bOverflow;

	arith_uint256 bnResult;
	bnResult.SetCompact(unBase, &bNegative, &bOverflow);
	bnResult *= 2;
	while (llTime > 0 && bnResult < bnLimit) {
		bnResult *= 2;
		llTime -= 24 * 60 * 60;
	}
	if (bNegative || bnResult == 0 || bOverflow || bnResult > bnLimit) {
		bnResult = bnLimit;
	}

	return bnResult.GetCompact();
}

int64_t GetAverageSpaceTime(const CBlockIndex* pindexLast, int64_t nInterval) {
	int64_t llMedian[nInterval];
	int64_t* pBegin = &llMedian[nInterval];
	int64_t* pEnd = &llMedian[nInterval];

	const CBlockIndex* pindex = pindexLast;
	const CBlockIndex* pPreIndex = pindexLast->m_pPrevBlockIndex;

	string strSelects;
	for (int i = 0; i < nInterval && pindex && pPreIndex;
			i++, pindex = pPreIndex, pPreIndex = pPreIndex->m_pPrevBlockIndex) {
		*(--pBegin) = pindex->GetBlockTime() - pPreIndex->GetBlockTime();
		strSelects += strprintf(" %lld", *(pBegin));
	}

	sort(pBegin, pEnd);

	int64_t llThreeQuarters = pBegin[(pEnd - pBegin) * 3 / 4];
	int64_t llQuarter = pBegin[(pEnd - pBegin) / 4];
	int64_t llUpBound = llThreeQuarters + (llThreeQuarters - llQuarter) * 1.5;
	int64_t llLowBound = llQuarter - (llThreeQuarters - llQuarter) * 1.5;
	int64_t* pBeginCopy = pBegin;
	int nCount = 0;
	int64_t llTotalSpace = 0;
	for (; pBeginCopy != pEnd; ++pBeginCopy) {
		if (*pBeginCopy <= llUpBound && *pBeginCopy >= llLowBound) {
			llTotalSpace += *pBeginCopy;
			++nCount;
		}
	}
	int64_t llAverageSpacing = llTotalSpace / nCount;
	return llAverageSpacing;
}

double CaculateDifficulty(unsigned int unBits) {
	int nShift = (unBits >> 24) & 0xff;

	double dDiff = (double) 0x0000ffff / (double) (unBits & 0x00ffffff);

	while (nShift < 29) {
		dDiff *= 256.0;
		nShift++;
	}
	while (nShift > 29) {
		dDiff /= 256.0;
		nShift--;
	}

	return dDiff;
}

unsigned int GetNextWorkRequired(const CBlockIndex* pLastBlockIndex, const CBlockHeader *pBlockHeader) {
	if (pLastBlockIndex == NULL) {
		return SysCfg().ProofOfWorkLimit().GetCompact(); // genesis block
	}

	const CBlockIndex* pIndexPrev = pLastBlockIndex;
	if (pIndexPrev->m_pPrevBlockIndex == NULL) {
		return SysCfg().ProofOfWorkLimit().GetCompact(); // first block
	}

	const CBlockIndex* pindexPrevPrev = pIndexPrev->m_pPrevBlockIndex;
	if (pindexPrevPrev->m_pPrevBlockIndex == NULL) {
		return SysCfg().ProofOfWorkLimit().GetCompact(); // second block
	}

	int64_t nTargetSpacing = SysCfg().GetTargetSpacing();  //nStakeTargetSpacing;
	int64_t nInterval = SysCfg().GetTargetTimespan() / nTargetSpacing;

	if (pLastBlockIndex->m_nHeight > 85000) {
		arith_uint256 bnNew;
		bnNew.SetCompact(pIndexPrev->m_unBits);
		int64_t nActualSpacing = pIndexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
		int64_t nTotalSpacing = ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
		if (nTotalSpacing < 0) {
			return bnNew.GetCompact();
		}
		bnNew *= nTotalSpacing;
		bnNew /= ((nInterval + 1) * nTargetSpacing);
		if (bnNew > SysCfg().ProofOfWorkLimit()) {
			LogPrint("INFO", "bnNew:%s\n", bnNew.GetHex());
			bnNew = SysCfg().ProofOfWorkLimit();
		}
		return bnNew.GetCompact();
	} else {
		arith_uint256 bnNew;
		bnNew.SetCompact(pIndexPrev->m_unBits);
		int64_t nAverageSpacing = GetAverageSpaceTime(pLastBlockIndex, nInterval);
		bnNew *= ((nInterval - 1) * nTargetSpacing + nAverageSpacing + nAverageSpacing);
		bnNew /= ((nInterval + 1) * nTargetSpacing);
		if (bnNew > SysCfg().ProofOfWorkLimit() || bnNew < 0) {
			LogPrint("INFO", "bnNew:%s\n", bnNew.GetHex());
			bnNew = SysCfg().ProofOfWorkLimit();
		}
		return bnNew.GetCompact();
	}
}

bool CheckProofOfWork(uint256 cHash, unsigned int unBits) {
	return true;
}

// Return maximum amount of blocks that other nodes claim to have
int GetNumBlocksOfPeers() {
	return max(g_scPeerBlockCounts.median(), Checkpoints::GetTotalBlocksEstimate());
}

bool IsInitialBlockDownload() {
	LOCK(g_cs_main);
	if (SysCfg().IsImporting() || SysCfg().IsReindex()
			|| g_cChainActive.Height() < Checkpoints::GetTotalBlocksEstimate()) {
		return true;
	}

	static int64_t nLastUpdate;
	static CBlockIndex* pindexLastBest;
	if (g_cChainActive.Tip() != pindexLastBest) {
		pindexLastBest = g_cChainActive.Tip();
		nLastUpdate = GetTime();
	}
	return (GetTime() - nLastUpdate < 10 && g_cChainActive.Tip()->GetBlockTime() < GetTime() - 24 * 60 * 60);
}

arith_uint256 GetBlockProof(const CBlockIndex& block) {
	arith_uint256 bnTarget;
	bool fNegative;
	bool fOverflow;
	bnTarget.SetCompact(block.m_unBits, &fNegative, &fOverflow);
	if (fNegative || fOverflow || bnTarget == 0)
		return 0;
	// We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
	// as it's too large for a arith_uint256. However, as 2**256 is at least as large
	// as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
	// or ~bnTarget / (nTarget+1) + 1.
	return (~bnTarget / (bnTarget + 1)) + 1;
}

arith_uint256 GetBlockChainWork(CBlock& block) {
	CBlockIndex* pindexNew = new CBlockIndex(block);
	map<uint256, CBlockIndex*>::iterator miPrev = g_mapBlockIndex.find(block.GetHashPrevBlock());
	if (miPrev != g_mapBlockIndex.end()) {
		pindexNew->m_pPrevBlockIndex = (*miPrev).second;
	}

	arith_uint256 nChainWork = (pindexNew->m_pPrevBlockIndex ? pindexNew->m_pPrevBlockIndex->m_cChainWork : 0)
			+ GetBlockProof(*pindexNew);

	delete pindexNew;

	return nChainWork;
}

bool fLargeWorkForkFound = false;
bool fLargeWorkInvalidChainFound = false;
CBlockIndex *pindexBestForkTip = NULL, *pindexBestForkBase = NULL;

void CheckForkWarningConditions()
{
    AssertLockHeld(g_cs_main);
    // Before we get past initial download, we cannot reliably alert about forks
    // (we assume we don't get stuck on a fork before the last checkpoint)
    if (IsInitialBlockDownload())
        return;

    // If our best fork is no longer within 72 blocks (+/- 12 hours if no one mines it)
    // of our head, drop it
    if (pindexBestForkTip && g_cChainActive.Height() - pindexBestForkTip->m_nHeight >= 72)
        pindexBestForkTip = NULL;

    if (pindexBestForkTip || (g_pIndexBestInvalid && g_pIndexBestInvalid->m_cChainWork > g_cChainActive.Tip()->m_cChainWork + (GetBlockProof(*g_cChainActive.Tip()) * 6)))
    {
        if (!fLargeWorkForkFound)
        {
            string strCmd = SysCfg().GetArg("-alertnotify", "");
            if (!strCmd.empty())
            {
                string warning = string("'Warning: Large-work fork detected, forking after block ") +
                                      pindexBestForkBase->m_pHashBlock->ToString() + string("'");
                boost::replace_all(strCmd, "%s", warning);
                boost::thread t(runCommand, strCmd); // thread runs free
            }
        }
        if (pindexBestForkTip)
        {
            LogPrint("INFO","CheckForkWarningConditions: Warning: Large valid fork found\n  forking the chain at height %d (%s)\n  lasting to height %d (%s).\nChain state database corruption likely.\n",
                   pindexBestForkBase->m_nHeight, pindexBestForkBase->m_pHashBlock->ToString(),
                   pindexBestForkTip->m_nHeight, pindexBestForkTip->m_pHashBlock->ToString());
            fLargeWorkForkFound = true;
        }
        else
        {
            LogPrint("INFO","CheckForkWarningConditions: Warning: Found invalid chain at least ~6 blocks longer than our best chain.\nChain state database corruption likely.\n");
            fLargeWorkInvalidChainFound = true;
        }
    }
    else
    {
        fLargeWorkForkFound = false;
        fLargeWorkInvalidChainFound = false;
    }
}

void CheckForkWarningConditionsOnNewFork(CBlockIndex* pindexNewForkTip)
{
    AssertLockHeld(g_cs_main);
    // If we are on a fork that is sufficiently large, set a warning flag
    CBlockIndex* pfork = pindexNewForkTip;
    CBlockIndex* plonger = g_cChainActive.Tip();
    while (pfork && pfork != plonger)
    {
        while (plonger && plonger->m_nHeight > pfork->m_nHeight)
            plonger = plonger->m_pPrevBlockIndex;
        if (pfork == plonger)
            break;
        pfork = pfork->m_pPrevBlockIndex;
    }

    // We define a condition which we should warn the user about as a fork of at least 7 blocks
    // who's tip is within 72 blocks (+/- 12 hours if no one mines it) of ours
    // We use 7 blocks rather arbitrarily as it represents just under 10% of sustained network
    // hash rate operating on the fork.
    // or a chain that is entirely longer than ours and invalid (note that this should be detected by both)
    // We define it this way because it allows us to only store the highest fork tip (+ base) which meets
    // the 7-block condition and from this always have the most-likely-to-cause-warning fork
    if (pfork && (!pindexBestForkTip || (pindexBestForkTip && pindexNewForkTip->m_nHeight > pindexBestForkTip->m_nHeight)) &&
            pindexNewForkTip->m_cChainWork - pfork->m_cChainWork > (GetBlockProof(*pfork) * 7) &&
            g_cChainActive.Height() - pindexNewForkTip->m_nHeight < 72)
    {
        pindexBestForkTip = pindexNewForkTip;
        pindexBestForkBase = pfork;
    }

    CheckForkWarningConditions();
}

// Requires cs_main.
void Misbehaving(NodeId nNodeId, int nHowMuch)
{
    if (nHowMuch == 0)
        return;

    ST_NodeState *state = State(nNodeId);
    if (state == NULL)
        return;

    state->nMisbehavior += nHowMuch;
    if (state->nMisbehavior >= SysCfg().GetArg("-banscore", 100))
    {
        LogPrint("INFO","Misbehaving: %s (%d -> %d) BAN THRESHOLD EXCEEDED\n", state->name, state->nMisbehavior-nHowMuch, state->nMisbehavior);
        state->fShouldBan = true;
    } else
        LogPrint("INFO","Misbehaving: %s (%d -> %d)\n", state->name, state->nMisbehavior-nHowMuch, state->nMisbehavior);
}

void static InvalidChainFound(CBlockIndex* pindexNew)
{
	if (!g_pIndexBestInvalid || pindexNew->m_cChainWork > g_pIndexBestInvalid->m_cChainWork) {
		g_pIndexBestInvalid = pindexNew;
		// The current code doesn't actually read the BestInvalidWork entry in
		// the block database anymore, as it is derived from the flags in block
		// index entry. We only write it for backward compatibility.
		g_pblocktree->WriteBestInvalidWork(ArithToUint256(g_pIndexBestInvalid->m_cChainWork));
		g_cUIInterface.NotifyBlocksChanged(pindexNew->GetBlockTime(),g_cChainActive.Height(),g_cChainActive.Tip()->GetBlockHash());
	}
    LogPrint("INFO","InvalidChainFound: invalid block=%s  height=%d  log2_work=%.8g  date=%s\n",
      pindexNew->GetBlockHash().ToString(), pindexNew->m_nHeight,
      log(pindexNew->m_cChainWork.getdouble())/log(2.0), DateTimeStrFormat("%Y-%m-%d %H:%M:%S",
      pindexNew->GetBlockTime()));
    LogPrint("INFO","InvalidChainFound:  current best=%s  height=%d  log2_work=%.8g  date=%s\n",
    		g_cChainActive.Tip()->GetBlockHash().ToString(), g_cChainActive.Height(), log(g_cChainActive.Tip()->m_cChainWork.getdouble())/log(2.0),
      DateTimeStrFormat("%Y-%m-%d %H:%M:%S", g_cChainActive.Tip()->GetBlockTime()));
    CheckForkWarningConditions();
}

void static InvalidBlockFound(CBlockIndex *pindex, const CValidationState &state) {
    int nDoS = 0;
    if (state.IsInvalid(nDoS)) {
        map<uint256, NodeId>::iterator it = g_mapBlockSource.find(pindex->GetBlockHash());
        if (it != g_mapBlockSource.end() && State(it->second)) {
            ST_BlockReject reject = {state.GetRejectCode(), state.GetRejectReason(), pindex->GetBlockHash()};
            State(it->second)->rejects.push_back(reject);
            if (nDoS > 0) {
            	LogPrint("INFO", "Misebehaving, find invalid block, hash:%s, Misbehavior add %d", it->first.GetHex(), nDoS);
                Misbehaving(it->second, nDoS);
            }
        }
    }
    if (!state.CorruptionPossible()) {
        pindex->m_unStatus |= BLOCK_FAILED_VALID;
        g_pblocktree->WriteBlockIndex(CDiskBlockIndex(pindex));
        g_setBlockIndexValid.erase(pindex);
        InvalidChainFound(pindex);
    }
}

void UpdateTime(CBlockHeader& cBlockHeader, const CBlockIndex* pPrevBlockIndex)
{
	cBlockHeader.SetTime(max(pPrevBlockIndex->GetMedianTimePast()+1, GetAdjustedTime()));
    // Updating time can change work required on testnet:
    if (TestNet())
    	cBlockHeader.SetBits(GetNextWorkRequired(pPrevBlockIndex, &cBlockHeader));
}

bool DisconnectBlock(CBlock& cBlock, CValidationState& cValidationState, CAccountViewCache &cAccountViewCache,
		CBlockIndex* pBlockIndex, CTransactionDBCache &cTxCache, CScriptDBViewCache &cScriptCache,
		bool* pbClean)
{
    assert(pBlockIndex->GetBlockHash() == cAccountViewCache.GetBestBlock());

    if (pbClean)
        *pbClean = false;

    bool fClean = true;

    CBlockUndo blockUndo;
    ST_DiskBlockPos pos = pBlockIndex->GetUndoPos();
    if (pos.IsNull())
        return ERRORMSG("DisconnectBlock() : no undo data available");
    if (!blockUndo.ReadFromDisk(pos, pBlockIndex->m_pPrevBlockIndex->GetBlockHash()))
        return ERRORMSG("DisconnectBlock() : failure reading undo data");

    if ((blockUndo.m_vcTxUndo.size() != cBlock.vptx.size()) && (blockUndo.m_vcTxUndo.size() != (cBlock.vptx.size()+1)))
        return ERRORMSG("DisconnectBlock() : block and undo data inconsistent");

//    LogPrint("INFO","height= %d\n,%s", pindex->nHeight,blockUndo.ToString());

//    int64_t llTime = GetTimeMillis();
    CTxUndo txundo;
    if(pBlockIndex->m_nHeight - COINBASE_MATURITY > 0) {
		//undo mature reward tx
		txundo = blockUndo.m_vcTxUndo.back();
		blockUndo.m_vcTxUndo.pop_back();
		//CBlockIndex *pMatureIndex = chainActive[pindex->nHeight - COINBASE_MATURITY];
		CBlockIndex *pMatureIndex = pBlockIndex;
		for(int i=0; i<COINBASE_MATURITY; ++i) {
			pMatureIndex = pMatureIndex->m_pPrevBlockIndex;
		}
		if (NULL != pMatureIndex) {
			CBlock matureBlock;
			if (!ReadBlockFromDisk(matureBlock, pMatureIndex)) {
				return cValidationState.DoS(100, ERRORMSG("ConnectBlock() : read mature block error"), REJECT_INVALID,
						"bad-read-block");
			}
			if (!matureBlock.vptx[0]->UndoExecuteTx(-1, cAccountViewCache, cValidationState, txundo, pBlockIndex->m_nHeight, cTxCache, cScriptCache))
				return ERRORMSG("ConnectBlock() : execure mature block reward tx error!");
		}
    }

    //undo reward tx
    std::shared_ptr<CBaseTransaction> pBaseTx = cBlock.vptx[0];
	txundo = blockUndo.m_vcTxUndo.back();
	LogPrint("undo_account", "tx Hash:%s\n", pBaseTx->GetHash().ToString());
	if(!pBaseTx->UndoExecuteTx(0, cAccountViewCache, cValidationState, txundo, pBlockIndex->m_nHeight, cTxCache, cScriptCache))
		return false;
//	LogPrint("INFO", "reward tx undo elapse:%lld ms\n", GetTimeMillis() - llTime);

    // undo transactions in reverse order
    for (int i = cBlock.vptx.size() - 1; i >= 1; i--) {
//    	llTime = GetTimeMillis();
        std::shared_ptr<CBaseTransaction> pBaseTx = cBlock.vptx[i];
        CTxUndo txundo = blockUndo.m_vcTxUndo[i-1];
        LogPrint("undo_account", "tx Hash:%s\n", pBaseTx->GetHash().ToString());
        if(!pBaseTx->UndoExecuteTx(i, cAccountViewCache, cValidationState, txundo, pBlockIndex->m_nHeight, cTxCache, cScriptCache))
        	return false;
      //  LogPrint("INFO", "tx type:%d,undo elapse:%lld ms\n", pBaseTx->nTxType, GetTimeMillis() - llTime);
    }


    // move best block pointer to prevout block
    cAccountViewCache.SetBestBlock(pBlockIndex->m_pPrevBlockIndex->GetBlockHash());


	if (!cTxCache.DeleteBlockFromCache(cBlock))
		return cValidationState.Abort(_("Disconnect tip block failed to delete tx from txcache"));

	//load a block tx into cache transaction
	CBlockIndex *pReLoadBlockIndex = pBlockIndex;
	if(pBlockIndex->m_nHeight - SysCfg().GetTxCacheHeight()>0) {
		CChain chainTemp;
		chainTemp.SetTip(pBlockIndex->m_pPrevBlockIndex);
		pReLoadBlockIndex = chainTemp[pBlockIndex->m_nHeight - SysCfg().GetTxCacheHeight()];
		CBlock reLoadblock;
		if (!ReadBlockFromDisk(reLoadblock, pReLoadBlockIndex))
			return cValidationState.Abort(_("Failed to read block"));
		if (!cTxCache.AddBlockToCache(reLoadblock))
				return cValidationState.Abort(_("Disconnect tip block reload preblock tx to txcache"));
	}

    if (pbClean) {
        *pbClean = fClean;
        return true;
    } else {
        return fClean;
    }
}

void static FlushBlockFile(bool fFinalize = false)
{
    LOCK(g_cs_LastBlockFile);

    ST_DiskBlockPos posOld(g_nLastBlockFile, 0);

    FILE *fileOld = OpenBlockFile(posOld);
    if (fileOld) {
        if (fFinalize)
            TruncateFile(fileOld, g_InfoLastBlockFile.m_unSize);
        FileCommit(fileOld);
        fclose(fileOld);
    }

    fileOld = OpenUndoFile(posOld);
    if (fileOld) {
        if (fFinalize)
            TruncateFile(fileOld, g_InfoLastBlockFile.m_unUndoSize);
        FileCommit(fileOld);
        fclose(fileOld);
    }
}

bool FindUndoPos(CValidationState &state, int nFile, ST_DiskBlockPos &pos, unsigned int nAddSize);

//static CCheckQueue<CScriptCheck> scriptcheckqueue(128);
bool ConnectBlock(CBlock& cBlock, CValidationState& cValidationState, CAccountViewCache &cAccountViewCache,
		CBlockIndex* pBlockIndex, CTransactionDBCache &cTxCache, CScriptDBViewCache &cScriptCache, bool bJustCheck) {
	AssertLockHeld(g_cs_main);
	// Check it again in case a previous version let a bad block in
	if (!CheckBlock(cBlock, cValidationState, cAccountViewCache, cScriptCache, !bJustCheck, !bJustCheck))
		return false;

	if (!bJustCheck) {
		// verify that the view's current state corresponds to the previous block
		uint256 hashPrevBlock =
				pBlockIndex->m_pPrevBlockIndex == NULL ? uint256() : pBlockIndex->m_pPrevBlockIndex->GetBlockHash();
		if (hashPrevBlock != cAccountViewCache.GetBestBlock()) {
			LogPrint("INFO", "hashPrevBlock=%s, bestblock=%s\n", hashPrevBlock.GetHex(),
					cAccountViewCache.GetBestBlock().GetHex());
			assert(hashPrevBlock == cAccountViewCache.GetBestBlock());
		}
	}

	// Special case for the genesis block, skipping connection of its transactions
	// (its coinbase is unspendable)
	if (cBlock.GetHash() == SysCfg().HashGenesisBlock()) {
		cAccountViewCache.SetBestBlock(pBlockIndex->GetBlockHash());
		for (unsigned int i = 0; i < cBlock.vptx.size(); i++) {
			std::shared_ptr<CRewardTransaction> pRewardTx = dynamic_pointer_cast<CRewardTransaction>(cBlock.vptx[i]);
			CAccount sourceAccount;
			CRegID accountId(pBlockIndex->m_nHeight, i);
			CPubKey pubKey = boost::get<CPubKey>(pRewardTx->m_cAccount);
			CKeyID keyId = pubKey.GetKeyID();
			sourceAccount.m_cKeyID = keyId;
			sourceAccount.SetRegId(accountId);
			sourceAccount.m_cPublicKey = pubKey;
			sourceAccount.m_ullValues = pRewardTx->m_ullRewardValue;
			assert(cAccountViewCache.SaveAccountInfo(accountId.GetVec6(), keyId, sourceAccount));
		}
		return true;
	}

	CBlockUndo blockundo;

	int64_t nStart = GetTimeMicros();
	ST_DiskTxPos pos(pBlockIndex->GetBlockPos(), GetSizeOfCompactSize(cBlock.vptx.size()));
	std::vector<pair<uint256, ST_DiskTxPos> > vPos;
	vPos.reserve(cBlock.vptx.size());

	//push reward pos
	vPos.push_back(make_pair(cBlock.GetTxHash(0), pos));
	pos.m_unTxOffset += ::GetSerializeSize(cBlock.vptx[0], SER_DISK, g_sClientVersion);

	LogPrint("op_account", "block height:%d block hash:%s\n", cBlock.GetHeight(), cBlock.GetHash().GetHex());
	uint64_t nTotalRunStep(0);
	int64_t nTotalFuel(0);
	if (cBlock.vptx.size() > 1) {
		for (unsigned int i = 1; i < cBlock.vptx.size(); i++) {
			std::shared_ptr<CBaseTransaction> pBaseTx = cBlock.vptx[i];
			if (uint256() != cTxCache.IsContainTx((pBaseTx->GetHash()))) {
				return cValidationState.DoS(100,
						ERRORMSG("ConnectBlock() : the TxHash %s the confirm duplicate", pBaseTx->GetHash().GetHex()),
						REJECT_INVALID, "bad-cb-amount");
			}
			assert(g_mapBlockIndex.count(cAccountViewCache.GetBestBlock()));
			if (!pBaseTx->IsValidHeight(g_mapBlockIndex[cAccountViewCache.GetBestBlock()]->m_nHeight,
					SysCfg().GetTxCacheHeight())) {
				return cValidationState.DoS(100,
						ERRORMSG("ConnectBlock() : txhash=%s beyond the scope of valid height",
								pBaseTx->GetHash().GetHex()), REJECT_INVALID, "tx-invalid-height");
			}

			if (EM_CONTRACT_TX == pBaseTx->m_chTxType) {
				LogPrint("vm", "tx hash=%s ConnectBlock run contract\n", pBaseTx->GetHash().GetHex());
			}
			LogPrint("op_account", "tx index:%d tx hash:%s\n", i, pBaseTx->GetHash().GetHex());
			CTxUndo txundo;
			pBaseTx->m_nFuelRate = cBlock.GetFuelRate();
			if (!pBaseTx->ExecuteTx(i, cAccountViewCache, cValidationState, txundo, pBlockIndex->m_nHeight, cTxCache,
					cScriptCache)) {
				return false;
			}

			nTotalRunStep += pBaseTx->m_ullRunStep;
			if (nTotalRunStep > MAX_BLOCK_RUN_STEP) {
				return cValidationState.DoS(100,
						ERRORMSG("block hash=%s total run steps exceed max run step", cBlock.GetHash().GetHex()),
						REJECT_INVALID, "exeed-max_step");
			}
			uint64_t llFuel = ceil(pBaseTx->m_ullRunStep / 100.f) * cBlock.GetFuelRate();
			if (EM_REG_APP_TX == pBaseTx->m_chTxType) {
				if (g_cChainActive.Tip()->m_nHeight > g_sRegAppFuel2FeeForkHeight) {
					llFuel = 0;
				} else {
					if (llFuel < 1 * COIN) {
						llFuel = 1 * COIN;
					}
				}

			}
			nTotalFuel += llFuel;
			LogPrint("fuel", "connect block total fuel:%d, tx fuel:%d runStep:%d fuelRate:%d txhash:%s \n", nTotalFuel,
					llFuel, pBaseTx->m_ullRunStep, cBlock.GetFuelRate(), pBaseTx->GetHash().GetHex());
			vPos.push_back(make_pair(cBlock.GetTxHash(i), pos));
			pos.m_unTxOffset += ::GetSerializeSize(pBaseTx, SER_DISK, g_sClientVersion);
			blockundo.m_vcTxUndo.push_back(txundo);
		}

		if (nTotalFuel != cBlock.GetFuel()) {
			return ERRORMSG("fuel value at block header calculate error(actual fuel:%ld vs block fuel:%ld)", nTotalFuel,
					cBlock.GetFuel());
		}
	}

	if (!VerifyPosTx(cAccountViewCache, &cBlock, cTxCache, cScriptCache, false)) {
		return cValidationState.DoS(100,
				ERRORMSG("ConnectBlock() : the block Hash=%s check pos tx error", cBlock.GetHash().GetHex()),
				REJECT_INVALID, "bad-pos-tx");
	}

	std::shared_ptr<CRewardTransaction> pRewardTx = dynamic_pointer_cast<CRewardTransaction>(cBlock.vptx[0]);

	//校验coinday
	CAccount account;
	if (cAccountViewCache.GetAccount(pRewardTx->m_cAccount, account)) {
		if (account.GetAccountPos(pBlockIndex->m_nHeight) <= 0 || !account.IsMiner(pBlockIndex->m_nHeight))
			return cValidationState.DoS(100,
					ERRORMSG("coindays of account dismatch, can't be miner, account info:%s", account.ToString()),
					REJECT_INVALID, "bad-coinday-miner");
	}

	//校验reward
	uint64_t llValidReward = cBlock.GetFee() - cBlock.GetFuel() + POS_REWARD;
	if (pRewardTx->m_ullRewardValue != llValidReward) {
		LogPrint("INFO", "block fee:%lld, block fuel:%lld\n", cBlock.GetFee(), cBlock.GetFuel());
		return cValidationState.DoS(100,
				ERRORMSG("ConnectBlock() : coinbase pays too much (actual=%d vs limit=%d)", pRewardTx->m_ullRewardValue,
						llValidReward), REJECT_INVALID, "bad-cb-amount");
	}
	//deal reward tx
	LogPrint("op_account", "tx index:%d tx hash:%s\n", 0, cBlock.vptx[0]->GetHash().GetHex());
	CTxUndo txundo;
	if (!cBlock.vptx[0]->ExecuteTx(0, cAccountViewCache, cValidationState, txundo, pBlockIndex->m_nHeight, cTxCache,
			cScriptCache))
		return ERRORMSG("ConnectBlock() : execure reward tx error!");
	blockundo.m_vcTxUndo.push_back(txundo);

	if (pBlockIndex->m_nHeight - COINBASE_MATURITY > 0) {
		//deal mature reward tx
		//CBlockIndex *pMatureIndex = chainActive[pindex->nHeight - COINBASE_MATURITY];
		CBlockIndex * pMatureIndex = pBlockIndex;
		for (int i = 0; i < COINBASE_MATURITY; ++i) {
			pMatureIndex = pMatureIndex->m_pPrevBlockIndex;
		}
		if (NULL != pMatureIndex) {
			CBlock matureBlock;
			if (!ReadBlockFromDisk(matureBlock, pMatureIndex)) {
				return cValidationState.DoS(100, ERRORMSG("ConnectBlock() : read mature block error"), REJECT_INVALID,
						"bad-read-block");
			}
			if (!matureBlock.vptx[0]->ExecuteTx(-1, cAccountViewCache, cValidationState, txundo, pBlockIndex->m_nHeight,
					cTxCache, cScriptCache))
				return ERRORMSG("ConnectBlock() : execure mature block reward tx error!");
		}
		blockundo.m_vcTxUndo.push_back(txundo);
	}
	int64_t nTime = GetTimeMicros() - nStart;
	if (SysCfg().IsBenchmark())
		LogPrint("INFO", "- Connect %u transactions: %.2fms (%.3fms/tx)\n", (unsigned )cBlock.vptx.size(),
				0.001 * nTime, 0.001 * nTime / cBlock.vptx.size());

	if (bJustCheck)
		return true;

	if (SysCfg().IsTxIndex()) {
		LogPrint("txindex", " add tx index, block hash:%s\n", pBlockIndex->GetBlockHash().GetHex());
		vector<CScriptDBOperLog> vTxIndexOperDB;
		if (!cScriptCache.WriteTxIndex(vPos, vTxIndexOperDB))
			return cValidationState.Abort(_("Failed to write transaction index"));
		auto itTxUndo = blockundo.m_vcTxUndo.rbegin();
		itTxUndo->m_vcScriptOperLog.insert(itTxUndo->m_vcScriptOperLog.begin(), vTxIndexOperDB.begin(),
				vTxIndexOperDB.end());
	}

	// Write undo information to disk
	if (pBlockIndex->GetUndoPos().IsNull() || (pBlockIndex->m_unStatus & BLOCK_VALID_MASK) < BLOCK_VALID_SCRIPTS) {
		if (pBlockIndex->GetUndoPos().IsNull()) {
			ST_DiskBlockPos pos;
			if (!FindUndoPos(cValidationState, pBlockIndex->m_nFile, pos,
					::GetSerializeSize(blockundo, SER_DISK, g_sClientVersion) + 40))
				return ERRORMSG("ConnectBlock() : FindUndoPos failed");
			if (!blockundo.WriteToDisk(pos, pBlockIndex->m_pPrevBlockIndex->GetBlockHash()))
				return cValidationState.Abort(_("Failed to write undo data"));

			// update nUndoPos in block index
			pBlockIndex->m_nUndoPos = pos.unPos;
			pBlockIndex->m_unStatus |= BLOCK_HAVE_UNDO;
		}

		pBlockIndex->m_unStatus = (pBlockIndex->m_unStatus & ~BLOCK_VALID_MASK) | BLOCK_VALID_SCRIPTS;

		CDiskBlockIndex blockindex(pBlockIndex);
		if (!g_pblocktree->WriteBlockIndex(blockindex))
			return cValidationState.Abort(_("Failed to write block index"));
	}

	if (!cTxCache.AddBlockToCache(cBlock))
		return cValidationState.Abort(_("Connect tip block failed add block tx to txcache"));
	if (pBlockIndex->m_nHeight - SysCfg().GetTxCacheHeight() > 0) {
		CChain chainTemp;
		chainTemp.SetTip(pBlockIndex);
		CBlockIndex *pDeleteBlockIndex = chainTemp[pBlockIndex->m_nHeight - SysCfg().GetTxCacheHeight()];
		CBlock deleteBlock;
		if (!ReadBlockFromDisk(deleteBlock, pDeleteBlockIndex))
			return cValidationState.Abort(_("Failed to read block"));
		if (!cTxCache.DeleteBlockFromCache(deleteBlock))
			return cValidationState.Abort(_("Connect tip block failed delete block tx to txcache"));
	}

	// add this block to the view's block chain
	assert(cAccountViewCache.SetBestBlock(pBlockIndex->GetBlockHash()));
	return true;
}

// Update the on-disk chain state.
bool static WriteChainState(CValidationState &state) {
	static int64_t nLastWrite = 0;
	unsigned int cachesize = g_pAccountViewTip->GetCacheSize() + g_pScriptDBTip->GetCacheSize();
	if (!IsInitialBlockDownload() || cachesize > SysCfg().GetViewCacheSize()
			|| GetTimeMicros() > nLastWrite + 600 * 1000000) {
		// Typical CCoins structures on disk are around 100 bytes in size.
		// Pushing a new one to the database can cause it to be written
		// twice (once in the log, and once in the tables). This is already
		// an overestimation, as most will delete an existing entry or
		// overwrite one. Still, use a conservative safety factor of 2.
		if (!CheckDiskSpace(cachesize))
			return state.Error("out of disk space");

		FlushBlockFile();
		g_pblocktree->Sync();
		if (!g_pAccountViewTip->Flush())
			return state.Abort(_("Failed to write to account database"));
		if (!g_pTxCacheTip->Flush())
			return state.Abort(_("Failed to write to tx cache database"));
		if (!g_pScriptDBTip->Flush())
			return state.Abort(_("Failed to write to script db database"));
		mapCache.clear();
		nLastWrite = GetTimeMicros();
	}
	return true;
}

// Update chainActive and related internal data structures.
void static UpdateTip(CBlockIndex *pindexNew, const CBlock &block) {
	g_cChainActive.SetTip(pindexNew);

	SyncWithWallets(uint256(), NULL, &block);

	// Update best block in wallet (so we can detect restored wallets)
	bool fIsInitialDownload = IsInitialBlockDownload();
	if ((g_cChainActive.Height() % 20160) == 0 || (!fIsInitialDownload && (g_cChainActive.Height() % 144) == 0))
		g_signals.SetBestChain(g_cChainActive.GetLocator());

	// New best block
	SysCfg().SetBestRecvTime(GetTime());
	g_cTxMemPool.AddTransactionsUpdated(1);
	LogPrint("INFO",
			"UpdateTip: new best=%s  height=%d  log2_work=%.8g  tx=%lu  date=%s progress=%f txnumber=%d dFeePerKb=%lf nFuelRate=%d\n",
			g_cChainActive.Tip()->GetBlockHash().ToString(), g_cChainActive.Height(),
			log(g_cChainActive.Tip()->m_cChainWork.getdouble()) / log(2.0),
			(unsigned long )g_cChainActive.Tip()->m_unChainTx,
			DateTimeStrFormat("%Y-%m-%d %H:%M:%S", g_cChainActive.Tip()->GetBlockTime()),
			Checkpoints::GuessVerificationProgress(g_cChainActive.Tip()), block.vptx.size(),
			g_cChainActive.Tip()->m_dFeePerKb, g_cChainActive.Tip()->m_nFuelRate);
	LogPrint("updatetip",
			"UpdateTip: new best=%s  height=%d  log2_work=%.8g  tx=%lu  date=%s progress=%f txnumber=%d dFeePerKb=%lf nFuelRate=%d difficulty=%.8lf\n",
			g_cChainActive.Tip()->GetBlockHash().ToString(), g_cChainActive.Height(),
			log(g_cChainActive.Tip()->m_cChainWork.getdouble()) / log(2.0),
			(unsigned long )g_cChainActive.Tip()->m_unChainTx,
			DateTimeStrFormat("%Y-%m-%d %H:%M:%S", g_cChainActive.Tip()->GetBlockTime()),
			Checkpoints::GuessVerificationProgress(g_cChainActive.Tip()), block.vptx.size(),
			g_cChainActive.Tip()->m_dFeePerKb, g_cChainActive.Tip()->m_nFuelRate,
			CaculateDifficulty(g_cChainActive.Tip()->m_unBits));
	// Check the version of the last 100 blocks to see if we need to upgrade:
	if (!fIsInitialDownload) {
		int nUpgraded = 0;
		const CBlockIndex* pindex = g_cChainActive.Tip();
		for (int i = 0; i < 100 && pindex != NULL; i++) {
			if (pindex->m_nVersion > CBlock::m_sCurrentVersion)
				++nUpgraded;
			pindex = pindex->m_pPrevBlockIndex;
		}
		if (nUpgraded > 0)
			LogPrint("INFO", "SetBestChain: %d of last 100 blocks above version %d\n", nUpgraded,
					(int )CBlock::m_sCurrentVersion);
		if (nUpgraded > 100 / 2)
			// strMiscWarning is read by GetWarnings(), called by Qt and the JSON-RPC code to warn the user:
			g_strMiscWarning = _("Warning: This version is obsolete, upgrade required!");
	}
}

// Disconnect chainActive's tip.
bool static DisconnectTip(CValidationState &state) {
	CBlockIndex *pindexDelete = g_cChainActive.Tip();
	assert(pindexDelete);
	// Read block from disk.
	CBlock block;
	if (!ReadBlockFromDisk(block, pindexDelete))
		return state.Abort(_("Failed to read block"));
	// Apply the block atomically to the chain state.
	int64_t nStart = GetTimeMicros();
	{
		CAccountViewCache view(*g_pAccountViewTip, true);
		CScriptDBViewCache scriptDBView(*g_pScriptDBTip, true);
		if (!DisconnectBlock(block, state, view, pindexDelete, *g_pTxCacheTip, scriptDBView, NULL))
			return ERRORMSG("DisconnectTip() : DisconnectBlock %s failed", pindexDelete->GetBlockHash().ToString());
		assert(view.Flush() && scriptDBView.Flush());
	}
	if (SysCfg().IsBenchmark())
		LogPrint("INFO", "- Disconnect: %.2fms\n", (GetTimeMicros() - nStart) * 0.001);
	// Write the chain state to disk, if necessary.
	if (!WriteChainState(state))
		return false;

//    if (!pTxCacheTip->DeleteBlockFromCache(block))
//    	return state.Abort(_("Disconnect tip block failed to delete tx from txcache"));
//
//    //load a block tx into cache transaction
//	CBlockIndex *pReLoadBlockIndex = pindexDelete;
//	if(pindexDelete->nHeight - SysCfg().GetTxCacheHeight()>0) {
//		pReLoadBlockIndex = chainActive[pindexDelete->nHeight - SysCfg().GetTxCacheHeight()];
//		CBlock reLoadblock;
//		if (!ReadBlockFromDisk(reLoadblock, pindexDelete))
//			return state.Abort(_("Failed to read block"));
//		if (!pTxCacheTip->AddBlockToCache(reLoadblock))
//				return state.Abort(_("Disconnect tip block reload preblock tx to txcache"));
//	}

	// Update chainActive and related variables.
	UpdateTip(pindexDelete->m_pPrevBlockIndex, block);
	// Resurrect mempool transactions from the disconnected block.
	for (const auto &ptx : block.vptx) {
		list<std::shared_ptr<CBaseTransaction> > removed;
		CValidationState stateDummy;
		if (!ptx->IsCoinBase()) {
			if (!AcceptToMemoryPool(g_cTxMemPool, stateDummy, ptx.get(), false, NULL)) {
				g_cTxMemPool.remove(ptx.get(), removed, true);
			} else
				g_cUIInterface.ReleaseTransaction(ptx->GetHash());
		} else {
			g_cUIInterface.RemoveTransaction(ptx->GetHash());
			EraseTransaction(ptx->GetHash());
		}

	}

	if (SysCfg().GetArg("-blocklog", 0) != 0) {
		if (g_cChainActive.Height() % SysCfg().GetArg("-blocklog", 0) == 0) {
			if (!g_pAccountViewTip->Flush())
				return state.Abort(_("Failed to write to account database"));
			if (!g_pTxCacheTip->Flush())
				return state.Abort(_("Failed to write to tx cache database"));
			if (!g_pScriptDBTip->Flush())
				return state.Abort(_("Failed to write to script db database"));
			WriteBlockLog(true, "DisConnectTip");
		}
	}
	return true;
}

void PrintInfo(const uint256 &hash, const int &nCurHeight, CScriptDBViewCache &scriptDBView, const string &scriptId) {
	vector<unsigned char> vScriptKey;
	vector<unsigned char> vScriptData;
	int nHeight;
	set<CScriptDBOperLog> setOperLog;
	CRegID regId(scriptId);
	int nCount(0);
	scriptDBView.GetScriptDataCount(scriptId, nCount);
	bool ret = scriptDBView.GetScriptData(nCurHeight, regId, 0, vScriptKey, vScriptData);
	LogPrint("scriptdbview", "\n\n\n");
	LogPrint("scriptdbview", "blockhash=%s,curHeight=%d\n", hash.GetHex(), nCurHeight);
	LogPrint("scriptdbview", "sriptid ID:%s key:%s value:%s height:%d, nCount:%d\n", scriptId.c_str(),
			HexStr(vScriptKey), HexStr(vScriptData), nHeight, nCount);
	while (ret) {
		ret = scriptDBView.GetScriptData(nCurHeight, regId, 1, vScriptKey, vScriptData);
		scriptDBView.GetScriptDataCount(scriptId, nCount);
		if (ret)
			LogPrint("scriptdbview", "sriptid ID:%s key:%s value:%s height:%d, nCount:%d\n", scriptId.c_str(),
					HexStr(vScriptKey), HexStr(vScriptData), nHeight, nCount);
	}
}

// Connect a new block to chainActive.
bool static ConnectTip(CValidationState &state, CBlockIndex *pindexNew) {
	assert(pindexNew->m_pPrevBlockIndex == g_cChainActive.Tip());
	// Read block from disk.
	CBlock block;
	if (!ReadBlockFromDisk(block, pindexNew))
		return state.Abort(strprintf("Failed to read block hash:%s\n", pindexNew->GetBlockHash().GetHex()));
		// Apply the block atomically to the chain state.
	int64_t nStart = GetTimeMicros();
	{
		CInv inv(MSG_BLOCK, pindexNew->GetBlockHash());
		CAccountViewCache view(*g_pAccountViewTip, true);
		CScriptDBViewCache scriptDBView(*g_pScriptDBTip, true);
		if (!ConnectBlock(block, state, view, pindexNew, *g_pTxCacheTip, scriptDBView)) {
			if (state.IsInvalid())
				InvalidBlockFound(pindexNew, state);
			return ERRORMSG("ConnectTip() : ConnectBlock %s failed", pindexNew->GetBlockHash().ToString());
		}
		g_mapBlockSource.erase(inv.m_cHash);
		assert(view.Flush() && scriptDBView.Flush());
		CAccountViewCache viewtemp(*g_pAccountViewTip, true);
		uint256 uBestblockHash = viewtemp.GetBestBlock();
		LogPrint("INFO", "uBestBlockHash: %s\n", uBestblockHash.GetHex());
	}
	if (SysCfg().IsBenchmark())
		LogPrint("INFO", "- Connect: %.2fms\n", (GetTimeMicros() - nStart) * 0.001);

	// Write the chain state to disk, if necessary.
	if (!WriteChainState(state))
		return false;
	// Remove conflicting transactions from the mempool.
//  list<std::shared_ptr<CBaseTransaction> > txConflicted;
//	for (const auto &ptx : block.vptx) {
//		list<std::shared_ptr<CBaseTransaction> > unused;
//		mempool.remove(ptx.get(), unused);
//	}

//    if (!pTxCacheTip->AddBlockToCache(block))
//    		return state.Abort(_("Connect tip block failed add block tx to txcache"));
//    if(pindexNew->nHeight-SysCfg().GetTxCacheHeight() > 0) {
//    	CBlockIndex *pDeleteBlockIndex = chainActive[pindexNew->nHeight - SysCfg().GetTxCacheHeight()];
//		CBlock deleteBlock;
//		if (!ReadBlockFromDisk(deleteBlock, pDeleteBlockIndex))
//			return state.Abort(_("Failed to read block"));
//		if(!pTxCacheTip->DeleteBlockFromCache(deleteBlock))
//			return state.Abort(_("Connect tip block failed delete block tx to txcache"));
//    }
	// Update chainActive & related variables.
	UpdateTip(pindexNew, block);

	// Write new block info to log, if necessary.
	if (SysCfg().GetArg("-blocklog", 0) != 0) {
		if (g_cChainActive.Height() % SysCfg().GetArg("-blocklog", 0) == 0) {
			if (!g_pAccountViewTip->Flush())
				return state.Abort(_("Failed to write to account database"));
			if (!g_pTxCacheTip->Flush())
				return state.Abort(_("Failed to write to tx cache database"));
			if (!g_pScriptDBTip->Flush())
				return state.Abort(_("Failed to write to script db database"));
			WriteBlockLog(true, "ConnectTip");
		}
	}

	for (auto &pTxItem : block.vptx) {
		g_cTxMemPool.m_mapTx.erase(pTxItem->GetHash());
	}
//  mempool.ReScanMemPoolTx(pAccountViewTip, pScriptDBTip);
	return true;
}

// Make chainMostWork correspond to the chain with the most work in it, that isn't
// known to be invalid (it's however far from certain to be valid).
void static FindMostWorkChain() {
	CBlockIndex *pindexNew = NULL;

	// In case the current best is invalid, do not consider it.
	while (g_cChainMostWork.Tip() && (g_cChainMostWork.Tip()->m_unStatus & BLOCK_FAILED_MASK)) {
		g_setBlockIndexValid.erase(g_cChainMostWork.Tip());
		g_cChainMostWork.SetTip(g_cChainMostWork.Tip()->m_pPrevBlockIndex);
	}

	do {
		// Find the best candidate header.
		{
			set<CBlockIndex*, CBlockIndexWorkComparator>::reverse_iterator it = g_setBlockIndexValid.rbegin();
			if (it == g_setBlockIndexValid.rend())
				return;
			pindexNew = *it;
		}

		// Check whether all blocks on the path between the currently active chain and the candidate are valid.
		// Just going until the active chain is an optimization, as we know all blocks in it are valid already.
		CBlockIndex *pindexTest = pindexNew;
		bool fInvalidAncestor = false;
		while (pindexTest && !g_cChainActive.Contains(pindexTest)) {
			if (pindexTest->m_unStatus & BLOCK_FAILED_MASK) { // pindexTest->nStatus is BLOCK_FAILED_VALID or BLOCK_FAILED_CHILD
				// Candidate has an invalid ancestor, remove entire chain from the set.
				if (g_pIndexBestInvalid == NULL || pindexNew->m_cChainWork > g_pIndexBestInvalid->m_cChainWork)
					g_pIndexBestInvalid = pindexNew;
				CBlockIndex *pindexFailed = pindexNew;
				while (pindexTest != pindexFailed) {
					pindexFailed->m_unStatus |= BLOCK_FAILED_CHILD;
					g_setBlockIndexValid.erase(pindexFailed);
					pindexFailed = pindexFailed->m_pPrevBlockIndex;
				}
				fInvalidAncestor = true;
				break;
			}
			pindexTest = pindexTest->m_pPrevBlockIndex;
		}
		if (fInvalidAncestor)
			continue;

		break;
	} while (true);

	// Check whether it's actually an improvement.
	if (g_cChainMostWork.Tip() && !CBlockIndexWorkComparator()(g_cChainMostWork.Tip(), pindexNew))
		return;

	// We have a new best.
	g_cChainMostWork.SetTip(pindexNew);
}

// Try to activate to the most-work chain (thereby connecting it).
bool ActivateBestChain(CValidationState &cValidationState) {
    LOCK(g_cs_main);
    CBlockIndex *pindexOldTip = g_cChainActive.Tip();
    bool fComplete = false;
    while (!fComplete) {
        FindMostWorkChain();
        fComplete = true;

        // Check whether we have something to do.
        if (g_cChainMostWork.Tip() == NULL) break;

        // Disconnect active blocks which are no longer in the best chain.
        while (g_cChainActive.Tip() && !g_cChainMostWork.Contains(g_cChainActive.Tip())) {
            if (!DisconnectTip(cValidationState))
                return false;
            if(g_cChainActive.Tip() && g_cChainMostWork.Contains(g_cChainActive.Tip())) {
            	g_cTxMemPool.ReScanMemPoolTx(g_pAccountViewTip, g_pScriptDBTip);
            }
        }

        // Connect new blocks.
        while (!g_cChainActive.Contains(g_cChainMostWork.Tip())) {
            CBlockIndex *pindexConnect = g_cChainMostWork[g_cChainActive.Height() + 1];
            if (!ConnectTip(cValidationState, pindexConnect)) {
                if (cValidationState.IsInvalid()) {
                    // The block violates a consensus rule.
                    if (!cValidationState.CorruptionPossible())
                        InvalidChainFound(g_cChainMostWork.Tip());
                    fComplete = false;
                    cValidationState = CValidationState();
                    break;
                } else {
                    // A system error occurred (disk space, database error, ...).
                    return false;
                }
            }
            
            if(g_cChainActive.Contains(g_cChainMostWork.Tip())) {
				g_cTxMemPool.ReScanMemPoolTx(g_pAccountViewTip, g_pScriptDBTip);
            }
        }

    }

    if (g_cChainActive.Tip() != pindexOldTip) {
        string strCmd = SysCfg().GetArg("-blocknotify", "");
        if (!IsInitialBlockDownload() && !strCmd.empty())
        {
            boost::replace_all(strCmd, "%s", g_cChainActive.Tip()->GetBlockHash().GetHex());
            boost::thread t(runCommand, strCmd); // thread runs free
        }
    }

    return true;
}

bool AddToBlockIndex(CBlock& cBlock, CValidationState& cValidationState, const ST_DiskBlockPos& tDiskBlockPos) { // add  new blockindex to   mapBlockIndex,setBlockIndexValid,pblocktree;
																												 // Check for duplicate
	uint256 hash = cBlock.GetHash();
	if (g_mapBlockIndex.count(hash))
		return cValidationState.Invalid(ERRORMSG("AddToBlockIndex() : %s already exists", hash.ToString()), 0,
				"duplicate");

	// Construct new block index object
	CBlockIndex* pindexNew = new CBlockIndex(cBlock);
	assert(pindexNew);
	{
		LOCK(g_cs_nBlockSequenceId);
		pindexNew->m_uSequenceId = g_ullBlockSequenceId++;
	}
	map<uint256, CBlockIndex*>::iterator mi = g_mapBlockIndex.insert(make_pair(hash, pindexNew)).first;
//  LogPrint("INFO", "in map hash:%s map size:%d\n", hash.GetHex(), mapBlockIndex.size());
	pindexNew->m_pHashBlock = &((*mi).first);
	map<uint256, CBlockIndex*>::iterator miPrev = g_mapBlockIndex.find(cBlock.GetHashPrevBlock());
	if (miPrev != g_mapBlockIndex.end()) {
		pindexNew->m_pPrevBlockIndex = (*miPrev).second;
		pindexNew->m_nHeight = pindexNew->m_pPrevBlockIndex->m_nHeight + 1;
	}
	pindexNew->m_unTx = cBlock.vptx.size();
	pindexNew->m_cChainWork = (pindexNew->m_pPrevBlockIndex ? pindexNew->m_pPrevBlockIndex->m_cChainWork : 0)
			+ GetBlockProof(*pindexNew);
	pindexNew->m_unChainTx = (pindexNew->m_pPrevBlockIndex ? pindexNew->m_pPrevBlockIndex->m_unChainTx : 0)
			+ pindexNew->m_unTx;
	pindexNew->m_nFile = tDiskBlockPos.nFile;
	pindexNew->m_nDataPos = tDiskBlockPos.unPos;
	pindexNew->m_nUndoPos = 0;
	pindexNew->m_unStatus = BLOCK_VALID_TRANSACTIONS | BLOCK_HAVE_DATA;
	g_setBlockIndexValid.insert(pindexNew);

	if (!g_pblocktree->WriteBlockIndex(CDiskBlockIndex(pindexNew)))
		return cValidationState.Abort(_("Failed to write block index"));
	int64_t tempTime = GetTimeMillis();
	// New best?
	if (!ActivateBestChain(cValidationState)) {
		LogPrint("INFO", "ActivateBestChain() elapse time:%lld ms\n", GetTimeMillis() - tempTime);
		return false;
	}
//    LogPrint("INFO", "ActivateBestChain() elapse time:%lld ms\n", GetTimeMillis() - tempTime);
	LOCK(g_cs_main);
	if (pindexNew == g_cChainActive.Tip()) {
		// Clear fork warning if its no longer applicable
		CheckForkWarningConditions();
		// Notify UI to display prev block's coinbase if it was ours
		static uint256 hashPrevBestCoinBase;
		g_signals.UpdatedTransaction(hashPrevBestCoinBase);
		hashPrevBestCoinBase = cBlock.GetTxHash(0);
	} else
		CheckForkWarningConditionsOnNewFork(pindexNew);

	if (!g_pblocktree->Flush())
		return cValidationState.Abort(_("Failed to sync block index"));

	if (g_cChainActive.Tip()->m_nHeight > g_nSyncTipHeight)
		g_nSyncTipHeight = g_cChainActive.Tip()->m_nHeight;
	g_cUIInterface.NotifyBlocksChanged(pindexNew->GetBlockTime(), g_cChainActive.Height(),
			g_cChainActive.Tip()->GetBlockHash());
	return true;
}

bool FindBlockPos(CValidationState &state, ST_DiskBlockPos &pos, unsigned int nAddSize, unsigned int nHeight,
		uint64_t nTime, bool fKnown = false) {
	bool fUpdatedLast = false;

	LOCK(g_cs_LastBlockFile);

	if (fKnown) {
		if (g_nLastBlockFile != pos.nFile) {
			g_nLastBlockFile = pos.nFile;
			g_InfoLastBlockFile.SetNull();
			g_pblocktree->ReadBlockFileInfo(g_nLastBlockFile, g_InfoLastBlockFile);
			fUpdatedLast = true;
		}
	} else {
		while (g_InfoLastBlockFile.m_unSize + nAddSize >= MAX_BLOCKFILE_SIZE) {
			LogPrint("INFO", "Leaving block file %i: %s\n", g_nLastBlockFile, g_InfoLastBlockFile.ToString());
			FlushBlockFile(true);
			g_nLastBlockFile++;
			g_InfoLastBlockFile.SetNull();
			g_pblocktree->ReadBlockFileInfo(g_nLastBlockFile, g_InfoLastBlockFile); // check whether data for the new file somehow already exist; can fail just fine
			fUpdatedLast = true;
		}
		pos.nFile = g_nLastBlockFile;
		pos.unPos = g_InfoLastBlockFile.m_unSize;
	}

	g_InfoLastBlockFile.m_unSize += nAddSize;
	g_InfoLastBlockFile.AddBlock(nHeight, nTime);

	if (!fKnown) {
		unsigned int nOldChunks = (pos.unPos + BLOCKFILE_CHUNK_SIZE - 1) / BLOCKFILE_CHUNK_SIZE;
		unsigned int nNewChunks = (g_InfoLastBlockFile.m_unSize + BLOCKFILE_CHUNK_SIZE - 1) / BLOCKFILE_CHUNK_SIZE;
		if (nNewChunks > nOldChunks) {
			if (CheckDiskSpace(nNewChunks * BLOCKFILE_CHUNK_SIZE - pos.unPos)) {
				FILE *file = OpenBlockFile(pos);
				if (file) {
					LogPrint("INFO", "Pre-allocating up to position 0x%x in blk%05u.dat\n",
							nNewChunks * BLOCKFILE_CHUNK_SIZE, pos.nFile);
					AllocateFileRange(file, pos.unPos, nNewChunks * BLOCKFILE_CHUNK_SIZE - pos.unPos);
					fclose(file);
				}
			} else
				return state.Error("out of disk space");
		}
	}

	if (!g_pblocktree->WriteBlockFileInfo(g_nLastBlockFile, g_InfoLastBlockFile))
		return state.Abort(_("Failed to write file info"));
	if (fUpdatedLast)
		g_pblocktree->WriteLastBlockFile(g_nLastBlockFile);

	return true;
}

bool FindUndoPos(CValidationState &state, int nFile, ST_DiskBlockPos &pos, unsigned int nAddSize) {
	pos.nFile = nFile;

	LOCK(g_cs_LastBlockFile);

	unsigned int nNewSize;
	if (nFile == g_nLastBlockFile) {
		pos.unPos = g_InfoLastBlockFile.m_unUndoSize;
		nNewSize = (g_InfoLastBlockFile.m_unUndoSize += nAddSize);
		if (!g_pblocktree->WriteBlockFileInfo(g_nLastBlockFile, g_InfoLastBlockFile))
			return state.Abort(_("Failed to write block info"));
	} else {
		CBlockFileInfo info;
		if (!g_pblocktree->ReadBlockFileInfo(nFile, info))
			return state.Abort(_("Failed to read block info"));
		pos.unPos = info.m_unUndoSize;
		nNewSize = (info.m_unUndoSize += nAddSize);
		if (!g_pblocktree->WriteBlockFileInfo(nFile, info))
			return state.Abort(_("Failed to write block info"));
	}

	unsigned int nOldChunks = (pos.unPos + UNDOFILE_CHUNK_SIZE - 1) / UNDOFILE_CHUNK_SIZE;
	unsigned int nNewChunks = (nNewSize + UNDOFILE_CHUNK_SIZE - 1) / UNDOFILE_CHUNK_SIZE;
	if (nNewChunks > nOldChunks) {
		if (CheckDiskSpace(nNewChunks * UNDOFILE_CHUNK_SIZE - pos.unPos)) {
			FILE *file = OpenUndoFile(pos);
			if (file) {
				LogPrint("INFO", "Pre-allocating up to position 0x%x in rev%05u.dat\n",
						nNewChunks * UNDOFILE_CHUNK_SIZE, pos.nFile);
				AllocateFileRange(file, pos.unPos, nNewChunks * UNDOFILE_CHUNK_SIZE - pos.unPos);
				fclose(file);
			}
		} else
			return state.Error("out of disk space");
	}

	return true;
}

bool CheckBlockProofWorkWithCoinDay(const CBlock& cBlock, CBlockIndex *pPreBlockIndex,
		CValidationState& cValidationState) {

	std::shared_ptr<CAccountViewCache> pForkAcctViewCache;
	std::shared_ptr<CTransactionDBCache> pForkTxCache;
	std::shared_ptr<CScriptDBViewCache> pForkScriptDBCache;

	std::shared_ptr<CAccountViewCache> pAcctViewCache = std::make_shared<CAccountViewCache>(*g_pAccountViewDB, true);
	pAcctViewCache->m_mapCacheAccounts = g_pAccountViewTip->m_mapCacheAccounts;
	pAcctViewCache->m_mapCacheKeyIds = g_pAccountViewTip->m_mapCacheKeyIds;
	pAcctViewCache->m_cHashBlock = g_pAccountViewTip->m_cHashBlock;

	std::shared_ptr<CTransactionDBCache> pTxCache = std::make_shared<CTransactionDBCache>(*g_pTxCacheDB, true);
	pTxCache->SetCacheMap(g_pTxCacheTip->GetCacheMap());

	std::shared_ptr<CScriptDBViewCache> pScriptDBCache = std::make_shared<CScriptDBViewCache>(*g_pScriptDB, true);
	pScriptDBCache->m_mapDatas = g_pScriptDBTip->m_mapDatas;

	uint256 preBlockHash;
	bool bFindForkChainTip(false);
	vector<CBlock> vPreBlocks;
	if (pPreBlockIndex->GetBlockHash() != g_cChainActive.Tip()->GetBlockHash()
			&& pPreBlockIndex->m_cChainWork > g_cChainActive.Tip()->m_cChainWork) {
		while (!g_cChainActive.Contains(pPreBlockIndex)) {
			if (mapCache.count(pPreBlockIndex->GetBlockHash()) > 0 && !bFindForkChainTip) {
				preBlockHash = pPreBlockIndex->GetBlockHash();
				LogPrint("INFO", "ForkChainTip hash=%s, height=%d\n", pPreBlockIndex->GetBlockHash().GetHex(),
						pPreBlockIndex->m_nHeight);
				bFindForkChainTip = true;
			}
			if (!bFindForkChainTip) {
				CBlock block;
				if (!ReadBlockFromDisk(block, pPreBlockIndex))
					return cValidationState.Abort(_("Failed to read block"));
				vPreBlocks.push_back(block);                   //将支链的block保存起来
			}
			pPreBlockIndex = pPreBlockIndex->m_pPrevBlockIndex;
			if (g_cChainActive.Tip()->m_nHeight - pPreBlockIndex->m_nHeight > SysCfg().GetIntervalPos()) {
				return cValidationState.DoS(100,
						ERRORMSG(
								"CheckBlockProofWorkWithCoinDay() : block at fork chain too earlier than tip block hash=%s block height=%d\n",
								cBlock.GetHash().GetHex(), cBlock.GetHeight()));
			}
			map<uint256, CBlockIndex*>::iterator mi = g_mapBlockIndex.find(pPreBlockIndex->GetBlockHash());
			if (mi == g_mapBlockIndex.end())
				return cValidationState.DoS(10, ERRORMSG("CheckBlockProofWorkWithCoinDay() : prev block not found"), 0,
						"bad-prevblk");
		}                   //如果进来的preblock hash不为tip的hash,找到主链中分叉处

		int64_t tempTime = GetTimeMillis();
		if (mapCache.count(pPreBlockIndex->GetBlockHash()) > 0) {
			LogPrint("INFO", "hash=%s, height=%d\n", pPreBlockIndex->GetBlockHash().GetHex(),
					pPreBlockIndex->m_nHeight);
			pAcctViewCache = std::get<0>(mapCache[pPreBlockIndex->GetBlockHash()]);
			pTxCache = std::get<1>(mapCache[pPreBlockIndex->GetBlockHash()]);
			pScriptDBCache = std::get<2>(mapCache[pPreBlockIndex->GetBlockHash()]);
		} else {
			CBlockIndex *pBlockIndex = g_cChainActive.Tip();
			while (pPreBlockIndex != pBlockIndex) {       //数据库状态回滚到主链分叉处
				LogPrint("INFO", "CheckBlockProofWorkWithCoinDay() DisconnectBlock block nHieght=%d hash=%s\n",
						pBlockIndex->m_nHeight, pBlockIndex->GetBlockHash().GetHex());
				CBlock block;
				if (!ReadBlockFromDisk(block, pBlockIndex))
					return cValidationState.Abort(_("Failed to read block"));
				bool bfClean = true;
				if (!DisconnectBlock(block, cValidationState, *pAcctViewCache, pBlockIndex, *pTxCache, *pScriptDBCache,
						&bfClean)) {
					return ERRORMSG("CheckBlockProofWorkWithCoinDay() : DisconnectBlock %s failed",
							pBlockIndex->GetBlockHash().ToString());
				}
				pBlockIndex = pBlockIndex->m_pPrevBlockIndex;
			}
			std::tuple<std::shared_ptr<CAccountViewCache>, std::shared_ptr<CTransactionDBCache>,
					std::shared_ptr<CScriptDBViewCache> > forkCache = std::make_tuple(pAcctViewCache, pTxCache,
					pScriptDBCache);
			LogPrint("INFO", "add mapCache Key:%s height:%d\n", pPreBlockIndex->GetBlockHash().GetHex(),
					pPreBlockIndex->m_nHeight);
			LogPrint("INFO", "add pAcctViewCache:%x \n", pAcctViewCache.get());
			LogPrint("INFO", "view best block hash:%s \n", pAcctViewCache->GetBestBlock().GetHex());
			mapCache[pPreBlockIndex->GetBlockHash()] = forkCache;
		}

		LogPrint("INFO", "CheckBlockProofWorkWithCoinDay() DisconnectBlock elapse :%lld ms\n",
				GetTimeMillis() - tempTime);
		if (bFindForkChainTip) {
			pForkAcctViewCache = std::get<0>(mapCache[preBlockHash]);
			pForkTxCache = std::get<1>(mapCache[preBlockHash]);
			pForkScriptDBCache = std::get<2>(mapCache[preBlockHash]);
			pForkAcctViewCache->SetBaseData(pAcctViewCache.get());
			pForkTxCache->SetBaseData(pTxCache.get());
			pForkScriptDBCache->SetBaseData(pScriptDBCache.get());
		} else {
			pForkAcctViewCache.reset(new CAccountViewCache(*pAcctViewCache, true));
			pForkTxCache.reset(new CTransactionDBCache(*pTxCache, true));
			pForkScriptDBCache.reset(new CScriptDBViewCache(*pScriptDBCache, true));
		}

		LogPrint("INFO", "pForkAcctView:%x\n", pForkAcctViewCache.get());
		LogPrint("INFO", "view best block hash:%s height:%d\n", pForkAcctViewCache->GetBestBlock().GetHex(),
				g_mapBlockIndex[pForkAcctViewCache->GetBestBlock()]->m_nHeight);

		vector<CBlock>::reverse_iterator rIter = vPreBlocks.rbegin();
		for (; rIter != vPreBlocks.rend(); ++rIter) { //连接支链的block
			LogPrint("INFO", "CheckBlockProofWorkWithCoinDay() ConnectBlock block nHieght=%d hash=%s\n",
					rIter->GetHeight(), rIter->GetHash().GetHex());
			if (!ConnectBlock(*rIter, cValidationState, *pForkAcctViewCache, g_mapBlockIndex[rIter->GetHash()],
					*pForkTxCache, *pForkScriptDBCache, false))
				return ERRORMSG("CheckBlockProofWorkWithCoinDay() : ConnectBlock %s failed",
						rIter->GetHash().ToString());
			CBlockIndex *pConnBlockIndex = g_mapBlockIndex[rIter->GetHash()];
			if (pConnBlockIndex->m_unStatus | BLOCK_FAILED_MASK)
				pConnBlockIndex->m_unStatus = BLOCK_VALID_TRANSACTIONS | BLOCK_HAVE_DATA;
		}

		//校验pos交易
		if (!VerifyPosTx(*pForkAcctViewCache, &cBlock, *pForkTxCache, *pForkScriptDBCache, true)) {
			return cValidationState.DoS(100,
					ERRORMSG("CheckBlockProofWorkWithCoinDay() : the block Hash=%s check pos tx error",
							cBlock.GetHash().GetHex()), REJECT_INVALID, "bad-pos-tx");
		}

		//校验利息是否正常
		std::shared_ptr<CRewardTransaction> pRewardTx = dynamic_pointer_cast<CRewardTransaction>(cBlock.vptx[0]);
		uint64_t llValidReward = cBlock.GetFee() - cBlock.GetFuel() + POS_REWARD;
		if (pRewardTx->m_ullRewardValue != llValidReward)
			return cValidationState.DoS(100,
					ERRORMSG("CheckBlockProofWorkWithCoinDay() : coinbase pays too much (actual=%d vs limit=%d)",
							pRewardTx->m_ullRewardValue, llValidReward), REJECT_INVALID, "bad-cb-amount");

		for (auto & item : cBlock.vptx) {
			//校验交易是否在有效高度
			if (!item->IsValidHeight(g_mapBlockIndex[pForkAcctViewCache->GetBestBlock()]->m_nHeight,
					SysCfg().GetTxCacheHeight())) {
				return cValidationState.DoS(100,
						ERRORMSG("CheckBlockProofWorkWithCoinDay() : txhash=%s beyond the scope of valid height\n ",
								item->GetHash().GetHex()), REJECT_INVALID, "tx-invalid-height");
			}
			//校验是否有重复确认交易
			if (uint256() != pForkTxCache->IsContainTx(item->GetHash()))
				return cValidationState.DoS(100,
						ERRORMSG("CheckBlockProofWorkWithCoinDay() : tx hash %s has been confirmed\n",
								item->GetHash().GetHex()), REJECT_INVALID, "bad-txns-oversize");
		}

		if (!vPreBlocks.empty()) {
			vector<CBlock>::iterator iterBlock = vPreBlocks.begin();
			if (bFindForkChainTip) {
				LogPrint("INFO", "delete mapCache Key:%s\n", preBlockHash.GetHex());
				mapCache.erase(preBlockHash);
			}
			std::tuple<std::shared_ptr<CAccountViewCache>, std::shared_ptr<CTransactionDBCache>,
					std::shared_ptr<CScriptDBViewCache> > cache = std::make_tuple(pForkAcctViewCache, pForkTxCache,
					pForkScriptDBCache);
			LogPrint("INFO", "add mapCache Key:%s\n", iterBlock->GetHash().GetHex());
			mapCache[iterBlock->GetHash()] = cache;
		}
	} else {
		return true;
	}
	return true;

}

typedef struct tagBlockPair {
	CBlock blk;
	CBlockIndex* bi;
} BlockPair;

bool CheckBlockProofWorkWithCoinDayEx(const CBlock& block, CBlockIndex *pPreBlockIndex, CValidationState& state) {
	bool bFindForkChainTip(false);
	vector<BlockPair> vPreBlocks;

	if (pPreBlockIndex->GetBlockHash() != g_cChainActive.Tip()->GetBlockHash()
			&& GetBlockChainWork((CBlock&) block) >= g_cChainActive.Tip()->m_cChainWork) {

		BlockPair bp;
		bp.blk = block;
		bp.bi = pPreBlockIndex;
		vPreBlocks.push_back(bp);
		while (!g_cChainActive.Contains(pPreBlockIndex)) {
			if (mapCache.count(pPreBlockIndex->GetBlockHash()) > 0 && !bFindForkChainTip) {
				bFindForkChainTip = true;
			}

			if (!bFindForkChainTip) {
				if (!ReadBlockFromDisk(bp.blk, pPreBlockIndex))
					return state.Abort(_("Failed to read block"));
			}
			pPreBlockIndex = pPreBlockIndex->m_pPrevBlockIndex;
			if (g_cChainActive.Tip()->m_nHeight - pPreBlockIndex->m_nHeight > SysCfg().GetIntervalPos()) {
				return state.DoS(100,
						ERRORMSG("block at fork chain too earlier than tip block hash=%s block height=%d\n",
								bp.blk.GetHash().GetHex(), bp.blk.GetHeight()));
			}
			map<uint256, CBlockIndex*>::iterator mi = g_mapBlockIndex.find(pPreBlockIndex->GetBlockHash());
			if (mi == g_mapBlockIndex.end())
				return state.DoS(10, ERRORMSG("prev block not found"), 0, "bad-prevblk");

			if (!bFindForkChainTip) {
				bp.bi = pPreBlockIndex;
				vPreBlocks.push_back(bp);            //将支链的block保存起来
			}
		}                   //如果进来的preblock hash不为tip的hash,找到主链中分叉处

		vector<BlockPair>::reverse_iterator r_iter;
		for (r_iter = vPreBlocks.rbegin(); r_iter != vPreBlocks.rend(); ++r_iter) {
			BlockPair tmpBP = *r_iter;
			if (!CheckBlockProofWorkWithCoinDay(tmpBP.blk, tmpBP.bi, state)) {
				return state.DoS(100, ERRORMSG("CheckBlockProofWorkWithCoinDayEx() : check proof of pos tx"),
						REJECT_INVALID, "bad-pos-tx");
			}
		}
	}

	return true;
}

bool CheckBlock(const CBlock& cBlock, CValidationState& cValidationState, CAccountViewCache &cAccountViewCache,
		CScriptDBViewCache &cScriptDBCache, bool bCheckTx, bool bCheckMerkleRoot) {
	// These are checks that are independent of context
	// that can be verified before saving an orphan block.

	unsigned int nBlockSize = ::GetSerializeSize(cBlock, SER_NETWORK, g_sProtocolVersion);
	// Size limits
	if (cBlock.vptx.empty() || cBlock.vptx.size() > MAX_BLOCK_SIZE || nBlockSize > MAX_BLOCK_SIZE)
		return cValidationState.DoS(100, ERRORMSG("CheckBlock() : size limits failed"), REJECT_INVALID,
				"bad-blk-length");

	if ((cBlock.GetHeight() >= g_sUpdateBlockVersionHeight)
			&& (cBlock.GetVersion() != CBlockHeader::m_sCurrentVersion)) {
		return cValidationState.Invalid(ERRORMSG("CheckBlock() : block version must be set 3"), REJECT_INVALID,
				"block-version-error");
	}

	// Check timestamp 12minutes limits
	if (cBlock.GetHeight() > g_sTwelveForwardLimits && cBlock.GetBlockTime() > GetAdjustedTime() + 12 * 60)
		return cValidationState.Invalid(ERRORMSG("CheckBlock() : block timestamp too far in the future"),
				REJECT_INVALID, "time-too-new");

	// First transaction must be coinbase, the rest must not be
	if (cBlock.vptx.empty() || !cBlock.vptx[0]->IsCoinBase())
		return cValidationState.DoS(100, ERRORMSG("CheckBlock() : first tx is not coinbase"), REJECT_INVALID,
				"bad-cb-missing");

	// Build the merkle tree already. We need it anyway later, and it makes the
	// block cache the transaction hashes, which means they don't need to be
	// recalculated many times during this block's validation.
	cBlock.BuildMerkleTree();

	// Check transactions
//    CAccountViewCache view(*pAccountViewTip, true);
//    CScriptDBViewCache scriptDBCache(*pScriptDBTip, true);
	// Check for duplicate txids. This is caught by ConnectInputs(),
	// but catching it earlier avoids a potential DoS attack:
	set<uint256> uniqueTx;
	for (unsigned int i = 0; i < cBlock.vptx.size(); i++) {
		uniqueTx.insert(cBlock.GetTxHash(i));

		if (bCheckTx && !CheckTransaction(cBlock.vptx[i].get(), cValidationState, cAccountViewCache, cScriptDBCache))
			return ERRORMSG("CheckBlock() :tx hash:%s CheckTransaction failed", cBlock.vptx[i]->GetHash().GetHex());
		if (cBlock.GetHash() != SysCfg().HashGenesisBlock()) {
			if (0 != i && cBlock.vptx[i]->IsCoinBase())
				return cValidationState.DoS(100, ERRORMSG("CheckBlock() : more than one coinbase"), REJECT_INVALID,
						"bad-cb-multiple");
		}
	}

	if (uniqueTx.size() != cBlock.vptx.size())
		return cValidationState.DoS(100, ERRORMSG("CheckBlock() : duplicate transaction"), REJECT_INVALID,
				"bad-txns-duplicate", true);

	// Check merkle root
	if (bCheckMerkleRoot && cBlock.GetHashMerkleRoot() != cBlock.vMerkleTree.back())
		return cValidationState.DoS(100,
				ERRORMSG("CheckBlock() : hashMerkleRoot mismatch, block.hashMerkleRoot=%s, block.vMerkleTree.back()=%s",
						cBlock.GetHashMerkleRoot().ToString(), cBlock.vMerkleTree.back().ToString()), REJECT_INVALID,
				"bad-txnmrklroot", true);

	//check nonce
	uint64_t maxNonce = SysCfg().GetBlockMaxNonce(); //cacul times
	if (cBlock.GetNonce() > maxNonce) {
		return cValidationState.Invalid(ERRORMSG("CheckBlock() : Nonce is larger than maxNonce"), REJECT_INVALID,
				"Nonce-too-large");
	}

	return true;
}

bool AcceptBlock(CBlock& cBlock, CValidationState& cValidationState, ST_DiskBlockPos* pDiskBlockPos) {
	AssertLockHeld(g_cs_main);
	// Check for duplicate
	uint256 hash = cBlock.GetHash();
	LogPrint("INFO", "AcceptBlcok hash:%s height:%d\n", hash.GetHex(), cBlock.GetHeight());
	if (g_mapBlockIndex.count(hash))
		return cValidationState.Invalid(ERRORMSG("AcceptBlock() : block already in mapBlockIndex"), 0, "duplicate");

	assert(cBlock.GetHash() == SysCfg().HashGenesisBlock() || g_mapBlockIndex.count(cBlock.GetHashPrevBlock()));
	if (cBlock.GetHash() != SysCfg().HashGenesisBlock()
			&& cBlock.GetFuelRate() != GetElementForBurn(g_mapBlockIndex[cBlock.GetHashPrevBlock()]))
		return cValidationState.DoS(100, ERRORMSG("CheckBlock() : block nfuelrate dismatched"), REJECT_INVALID,
				"fuelrate-dismatch");

	// Get prev block index
	CBlockIndex* pindexPrev = NULL;
	int nHeight = 0;
	if (hash != SysCfg().HashGenesisBlock()) {
		map<uint256, CBlockIndex*>::iterator mi = g_mapBlockIndex.find(cBlock.GetHashPrevBlock());
		if (mi == g_mapBlockIndex.end())
			return cValidationState.DoS(10, ERRORMSG("AcceptBlock() : prev block not found"), 0, "bad-prevblk");
		pindexPrev = (*mi).second;
		nHeight = pindexPrev->m_nHeight + 1;

		if (cBlock.GetHeight() != (unsigned int) nHeight)
			return cValidationState.DoS(100,
					ERRORMSG("AcceptBlock() : height in block claimed dismatched it's actual height"), REJECT_INVALID,
					"incorrect-height");

		int64_t tempTime = GetTimeMillis();

		// Check timestamp against prev
		if (cBlock.GetBlockTime() <= pindexPrev->GetMedianTimePast())
			return cValidationState.Invalid(ERRORMSG("AcceptBlock() : block's timestamp is too early"), REJECT_INVALID,
					"time-too-old");

		// Check that the block chain matches the known block chain up to a checkpoint
		if (!Checkpoints::CheckBlock(nHeight, hash))
			return cValidationState.DoS(100, ERRORMSG("AcceptBlock() : rejected by checkpoint lock-in at %d", nHeight),
					REJECT_CHECKPOINT, "checkpoint mismatch");

		// Don't accept any forks from the main chain prior to last checkpoint
		CBlockIndex* pcheckpoint = Checkpoints::GetLastCheckpoint(g_mapBlockIndex);
		if (pcheckpoint && nHeight < pcheckpoint->m_nHeight)
			return cValidationState.DoS(100,
					ERRORMSG("AcceptBlock() : forked chain older than last checkpoint (height %d)", nHeight));

		// Check proof of work, before height 35001 don't check proofwork, fixed CBigNum adjust difficult bug.
		if (cBlock.GetHeight() > g_sFixedDifficulty && cBlock.GetBits() != GetNextWorkRequired(pindexPrev, &cBlock))
			return cValidationState.DoS(100,
					ERRORMSG("AcceptBlock() : incorrect proof of work actual vs need(%d vs %d)", cBlock.GetBits(),
							GetNextWorkRequired(pindexPrev, &cBlock)), REJECT_INVALID, "bad-diffbits");

		//Check proof of pos tx
		if (!CheckBlockProofWorkWithCoinDayEx(cBlock, pindexPrev, cValidationState)) {
			LogPrint("INFO", "CheckBlockProofWorkWithCoinDayEx() end:%lld ms\n", GetTimeMillis() - tempTime);
			return cValidationState.DoS(100, ERRORMSG("AcceptBlock() : check proof of pos tx"), REJECT_INVALID,
					"bad-pos-tx");
		}

		// Reject block.nVersion=1 blocks when 95% (75% on testnet) of the network has upgraded:
		if (cBlock.GetVersion() < 2) {
			if ((!TestNet() && CBlockIndex::IsSuperMajority(2, pindexPrev, 950, 1000))
					|| (TestNet() && CBlockIndex::IsSuperMajority(2, pindexPrev, 75, 100))) {
				return cValidationState.Invalid(ERRORMSG("AcceptBlock() : rejected nVersion=1 block"), REJECT_OBSOLETE,
						"bad-version");
			}
		}

	}

	// Write block to history file
	try {
		unsigned int nBlockSize = ::GetSerializeSize(cBlock, SER_DISK, g_sClientVersion);
		ST_DiskBlockPos blockPos;
		if (pDiskBlockPos != NULL)
			blockPos = *pDiskBlockPos;
		if (!FindBlockPos(cValidationState, blockPos, nBlockSize + 8, nHeight, cBlock.GetTime(), pDiskBlockPos != NULL))
			return ERRORMSG("AcceptBlock() : FindBlockPos failed");
		if (pDiskBlockPos == NULL)
			if (!WriteBlockToDisk(cBlock, blockPos))
				return cValidationState.Abort(_("Failed to write block"));
		if (!AddToBlockIndex(cBlock, cValidationState, blockPos))
			return ERRORMSG("AcceptBlock() : AddToBlockIndex failed");
	} catch (std::runtime_error &e) {
		return cValidationState.Abort(_("System error: ") + e.what());
	}

	// Relay inventory, but don't relay old inventory during initial block download
	int nBlockEstimate = Checkpoints::GetTotalBlocksEstimate();
	if (g_cChainActive.Tip()->GetBlockHash() == hash) {
		LOCK(g_cs_vNodes);
		for (auto pnode : g_vNodes)
			if (g_cChainActive.Height()
					> (pnode->m_nStartingHeight != -1 ? pnode->m_nStartingHeight - 2000 : nBlockEstimate))
				pnode->PushInventory(CInv(MSG_BLOCK, hash));
	}
	return true;
}

bool CBlockIndex::IsSuperMajority(int nMinVersion, const CBlockIndex* pStartBlockIndex, unsigned int unRequired,
		unsigned int unToCheck) {
	unsigned int nFound = 0;
	for (unsigned int i = 0; i < unToCheck && nFound < unRequired && pStartBlockIndex != NULL; i++) {
		if (pStartBlockIndex->m_nVersion >= nMinVersion)
			++nFound;
		pStartBlockIndex = pStartBlockIndex->m_pPrevBlockIndex;
	}
	return (nFound >= unRequired);
}

int64_t CBlockIndex::GetMedianTime() const {
	AssertLockHeld(g_cs_main);
	const CBlockIndex* pindex = this;
	for (int i = 0; i < nMedianTimeSpan / 2; i++) {
		if (!g_cChainActive.Next(pindex))
			return GetBlockTime();
		pindex = g_cChainActive.Next(pindex);
	}
	return pindex->GetMedianTimePast();
}

void PushGetBlocks(CNode* pNode, CBlockIndex* pBlockIndexBegin, uint256 cHashEnd) { // Ask this guy to fill in what we're missing ,要求从网络上同步，从pindexBegin 开始,hashEnd值结束的块
	AssertLockHeld(g_cs_main);
	// Filter out duplicate requests
	if (pBlockIndexBegin == pNode->m_pIndexLastGetBlocksBegin && cHashEnd == pNode->m_cHashLastGetBlocksEnd) {
		LogPrint("GetLocator", "filter the same GetLocator\n");
		return;
	}
	pNode->m_pIndexLastGetBlocksBegin = pBlockIndexBegin;
	pNode->m_cHashLastGetBlocksEnd = cHashEnd;
	ST_BlockLocator blockLocator = g_cChainActive.GetLocator(pBlockIndexBegin);
	for (auto & blockHash : blockLocator.vcHave) {
		LogPrint("net", "GetLocator block hash:%s\n", blockHash.GetHex());
	}
	pNode->PushMessage("getblocks", blockLocator, cHashEnd);
	LogPrint("net", "getblocks from peer %s, hashEnd:%s\n", pNode->m_cAddress.ToString(), cHashEnd.GetHex());
}

bool ProcessBlock(CValidationState &cValidationState, CNode* pFromNode, CBlock* pBlock,
		ST_DiskBlockPos *pDiskBlockPos) {
//	int64_t llBeginTime = GetTimeMillis();
//	LogPrint("INFO", "ProcessBlock() enter:%lld\n", llBeginTime);
	AssertLockHeld(g_cs_main);
	// Check for duplicate
	uint256 hash = pBlock->GetHash();
	if (g_mapBlockIndex.count(hash))
		return cValidationState.Invalid(
				ERRORMSG("ProcessBlock() : already have block %d %s", g_mapBlockIndex[hash]->m_nHeight,
						hash.ToString()), 0, "duplicate");
	if (g_mapOrphanBlocks.count(hash))
		return cValidationState.Invalid(ERRORMSG("ProcessBlock() : already have block (orphan) %s", hash.ToString()), 0,
				"duplicate");

	int64_t llBeginCheckBlockTime = GetTimeMillis();
	CAccountViewCache view(*g_pAccountViewTip, true);
	CScriptDBViewCache scriptDBCache(*g_pScriptDBTip, true);
	// Preliminary checks
	if (!CheckBlock(*pBlock, cValidationState, view, scriptDBCache, false)) {
		LogPrint("INFO", "CheckBlock() id: %d elapse time:%lld ms\n", g_cChainActive.Height(),
				GetTimeMillis() - llBeginCheckBlockTime);
		return ERRORMSG("ProcessBlock() :block hash:%s CheckBlock FAILED", pBlock->GetHash().GetHex());
	}
//    LogPrint("INFO", "CheckBlock() elapse time:%lld ms\n", GetTimeMillis() - llBeginCheckBlockTime);
//    CBlockIndex* pcheckpoint = Checkpoints::GetLastCheckpoint(mapBlockIndex);
//    if (pcheckpoint && pblock->hashPrevBlock != (chainActive.Tip() ? chainActive.Tip()->GetBlockHash() : uint256(0)))
//    {
//        // Extra checks to prevent "fill up memory by spamming with bogus blocks"
//        int64_t deltaTime = pblock->GetBlockTime() - pcheckpoint->nTime;
//        if (deltaTime < 0)
//        {
//            return state.DoS(100, ERRORMSG("ProcessBlock() : block with timestamp before last checkpoint"),
//                             REJECT_CHECKPOINT, "time-too-old");
//        }
//        CBigNum bnNewBlock;
//        bnNewBlock.SetCompact(pblock->nBits);
//        CBigNum bnRequired;
//        bnRequired.SetCompact(ComputeMinWork(pcheckpoint->nBits, deltaTime));
//        if (bnNewBlock > bnRequired)
//        {
//            return state.DoS(100, ERRORMSG("ProcessBlock() : block with too little proof-of-work\n"
//            		" bnNewBlock:%s \n bnRequired:%s \n hash:%s \n prevHash:%s", bnNewBlock.getuint256().GetHex(), bnRequired.getuint256().GetHex(), pblock->GetHash().GetHex(), pblock->hashPrevBlock.GetHex()),
//                             REJECT_INVALID, "bad-diffbits");
//        }
//    }

	// If we don't already have its previous block, shunt it off to holding area until we get it
	if (!pBlock->GetHashPrevBlock().IsNull() && !g_mapBlockIndex.count(pBlock->GetHashPrevBlock())) { /* 网络有延迟,会存在*/
//      LogPrint("INFO","ProcessBlock: ORPHAN BLOCK %lu height=%d hash=%s, prev=%s\n", (unsigned long)mapOrphanBlocks.size(), pblock->nHeight, pblock->GetHash().GetHex(), pblock->hashPrevBlock.ToString());

		if (pBlock->GetHeight() > (unsigned int) g_nSyncTipHeight)
			g_nSyncTipHeight = pBlock->GetHeight();
		// Accept orphans as long as there is a node to request its parents from
		if (pFromNode) {
			if (PruneOrphanBlocks(pBlock->GetHeight())) {
				ST_OrphanBlock* pblock2 = new ST_OrphanBlock();
				{
					CDataStream ss(SER_DISK, g_sClientVersion);
					ss << *pBlock;
					pblock2->vchBlock = vector<unsigned char>(ss.begin(), ss.end());
				}
				pblock2->hashBlock = hash;
				pblock2->hashPrev = pBlock->GetHashPrevBlock();
				pblock2->height = pBlock->GetHeight();
				g_mapOrphanBlocks.insert(make_pair(hash, pblock2));                 //保存因网络延迟，收到的孤立块
				g_mapOrphanBlocksByPrev.insert(make_pair(pblock2->hashPrev, pblock2));
				setOrphanBlock.insert(pblock2);
				LogPrint("INFO", "ProcessBlock: ORPHAN BLOCK %lu insert height=%d hash=%s, prev=%s\n",
						(unsigned long )g_mapOrphanBlocks.size(), pBlock->GetHeight(), pBlock->GetHash().GetHex(),
						pBlock->GetHashPrevBlock().ToString());
			} else {
				LogPrint("INFO", "ProcessBlock: ORPHAN BLOCK %lu abandon height=%d hash=%s, prev=%s\n",
						(unsigned long )g_mapOrphanBlocks.size(), pBlock->GetHeight(), pBlock->GetHash().GetHex(),
						pBlock->GetHashPrevBlock().ToString());
			}
			// Ask this guy to fill in what we're missing
			LogPrint("net", "receive orphanblocks heignt=%d hash=%s lead to getblocks\n", pBlock->GetHeight(),
					pBlock->GetHash().GetHex());
			PushGetBlocks(pFromNode, g_cChainActive.Tip(), GetOrphanRoot(hash));
		}
		return true;
	}
	int64_t llAcceptBlockTime = GetTimeMillis();
	// Store to disk
	if (!AcceptBlock(*pBlock, cValidationState, pDiskBlockPos)) {
		LogPrint("INFO", "AcceptBlock() elapse time:%lld ms\n", GetTimeMillis() - llAcceptBlockTime);
		return ERRORMSG("ProcessBlock() : AcceptBlock FAILED");
	}
//    LogPrint("INFO", "AcceptBlock() elapse time:%lld ms\n", GetTimeMillis() - llAcceptBlockTime);

	// Recursively process any orphan blocks that depended on this one  递归处理
	vector<uint256> vWorkQueue;
	vWorkQueue.push_back(hash);
	for (unsigned int i = 0; i < vWorkQueue.size(); i++) {
		uint256 hashPrev = vWorkQueue[i];
		for (multimap<uint256, ST_OrphanBlock*>::iterator mi = g_mapOrphanBlocksByPrev.lower_bound(hashPrev);
				mi != g_mapOrphanBlocksByPrev.upper_bound(hashPrev); ++mi) {
			CBlock block;
			{
				CDataStream ss(mi->second->vchBlock, SER_DISK, g_sClientVersion);
				ss >> block;
			}
			block.BuildMerkleTree();
			// Use a dummy CValidationState so someone can't setup nodes to counter-DoS based on orphan resolution (that is, feeding people an invalid block based on LegitBlockX in order to get anyone relaying LegitBlockX banned)
			CValidationState stateDummy;
			if (AcceptBlock(block, stateDummy))
				vWorkQueue.push_back(mi->second->hashBlock);
			setOrphanBlock.erase(mi->second);
			g_mapOrphanBlocks.erase(mi->second->hashBlock);
			delete mi->second;
		}
		g_mapOrphanBlocksByPrev.erase(hashPrev);
	}

//  LogPrint("INFO", "ProcessBlock() elapse time:%lld ms\n", GetTimeMillis() - llBeginTime);
//  LogPrint("INFO","ProcessBlock: ACCEPTED\n");
	return true;
}

bool CheckActiveChain(int nHeight, uint256 cHash) {

	LogPrint("CHECKPOINT", "CheckActiveChain Enter====\n");
	LogPrint("CHECKPOINT", "check point hash:%s\n", cHash.ToString());
	if (nHeight < 1) {
		return true;
	}
	LOCK(g_cs_main);
	CBlockIndex *pindexOldTip = g_cChainActive.Tip();
	LogPrint("CHECKPOINT", "Current tip block:\n");
	LogPrint("CHECKPOINT", pindexOldTip->ToString().c_str());

	//Find the active chain dismatch checkpoint
	if (NULL == g_cChainActive[nHeight] || cHash != g_cChainActive[nHeight]->GetBlockHash()) {
		CBlockIndex* pcheckpoint = Checkpoints::GetLastCheckpoint(g_mapBlockIndex);
		LogPrint("CHECKPOINT", "Get Last check point:\n");
		if (pcheckpoint) {
			if (NULL == g_cChainActive[nHeight] && g_cChainActive.Contains(pcheckpoint)) {
				return true;
			}
			pcheckpoint->print();
			g_cChainMostWork.SetTip(pcheckpoint);
			bool bInvalidBlock = false;
			std::set<CBlockIndex*, CBlockIndexWorkComparator>::reverse_iterator it = g_setBlockIndexValid.rbegin();
			for (; (it != g_setBlockIndexValid.rend()) && !g_cChainMostWork.Contains(*it);) {
				bInvalidBlock = false;
				CBlockIndex *pIndexTest = *it;
				LogPrint("CHECKPOINT", "iterator:height=%d, hash=%s\n", pIndexTest->m_nHeight,
						pIndexTest->GetBlockHash().GetHex());
				if (pcheckpoint->m_nHeight < nHeight) {
					if (pIndexTest->m_nHeight >= nHeight) {
						LogPrint("CHECKPOINT", "CheckActiveChain delete blockindex:%s\n",
								pIndexTest->GetBlockHash().GetHex());
						g_setBlockIndexValid.erase(pIndexTest);
						it = g_setBlockIndexValid.rbegin();
						bInvalidBlock = true;
					}

				} else {
//					if (!chainMostWork.Contains(pIndexTest))
					{
						CBlockIndex *pIndexCheck = pIndexTest->m_pPrevBlockIndex;
						while (pIndexCheck && !g_cChainMostWork.Contains(pIndexCheck)) {
							pIndexCheck = pIndexCheck->m_pPrevBlockIndex;
						}
						if (NULL == pIndexCheck || pIndexCheck->m_nHeight < pcheckpoint->m_nHeight) {
							CBlockIndex *pIndexFailed = pIndexCheck;
							while (pIndexTest != pIndexFailed) {
								LogPrint("CHECKPOINT", "CheckActiveChain delete blockindex height=%d hash=%s\n",
										pIndexTest->m_nHeight, pIndexTest->GetBlockHash().GetHex());
								g_setBlockIndexValid.erase(pIndexTest);
								it = g_setBlockIndexValid.rbegin();
								bInvalidBlock = true;
								pIndexTest = pIndexTest->m_pPrevBlockIndex;
							}
						}
						if (g_cChainMostWork.Contains(pIndexCheck)
								&& g_cChainMostWork.Height() == pIndexCheck->m_nHeight
								&& pIndexTest->m_cChainWork > g_cChainMostWork.Tip()->m_cChainWork) {
							g_cChainMostWork.SetTip(pIndexTest);
							LogPrint("CHECKPOINT", "chainMostWork tip:height=%d, hash=%s\n", pIndexTest->m_nHeight,
									pIndexTest->GetBlockHash().GetHex());
						}

					}
				}
				if (!bInvalidBlock)
					++it;
			}

		} else {
			if (NULL == g_cChainActive[nHeight])
				return true;
			bool bInvalidBlock = false;
//			CBlockIndex * pInvalidBlockIndex = chainActive[nHeight];
			std::set<CBlockIndex*, CBlockIndexWorkComparator>::reverse_iterator it = g_setBlockIndexValid.rbegin();
			for (; it != g_setBlockIndexValid.rend();) {
				bInvalidBlock = false;
				CBlockIndex *pBlockTest = *it;
				while (pBlockTest->m_nHeight > nHeight) {
					pBlockTest = pBlockTest->m_pPrevBlockIndex;
				}
				if (pBlockTest->GetBlockHash() != cHash) {
					CBlockIndex *pBlockIndexFailed = *it;
					while (pBlockIndexFailed != pBlockTest) {
						LogPrint("CHECKPOINT", "CheckActiveChain delete blockindex height=%d hash=%s\n",
								pBlockIndexFailed->m_nHeight, pBlockIndexFailed->GetBlockHash().GetHex());
						g_setBlockIndexValid.erase(pBlockIndexFailed);
						pBlockIndexFailed = pBlockIndexFailed->m_pPrevBlockIndex;
						it = g_setBlockIndexValid.rbegin();
						bInvalidBlock = true;
						LogPrint("CHECKPOINT", "g_setBlockIndexValid size:%d\n", g_setBlockIndexValid.size());
					}
				}
				if (!bInvalidBlock)
					++it;
			} Assert(g_cChainActive[nHeight - 1]);
			g_cChainMostWork.SetTip(g_cChainActive[nHeight - 1]);
		}

		// Check whether we have something to do. sync chainMostWork to chainActive;disconnect block or connect block;
		if (g_cChainMostWork.Tip() == NULL)
			return false;
		CValidationState state;
		while (g_cChainActive.Tip() && !g_cChainMostWork.Contains(g_cChainActive.Tip())) {
			if (!DisconnectTip(state))
				return false;
		}
		while (NULL != g_cChainMostWork[g_cChainActive.Height() + 1]) {
			CBlockIndex *pindexConnect = g_cChainMostWork[g_cChainActive.Height() + 1];
			if (!ConnectTip(state, pindexConnect)) {
				if (state.IsInvalid()) {
					// The block violates a consensus rule.
					if (!state.CorruptionPossible())
						InvalidChainFound(g_cChainMostWork.Tip());
					state = CValidationState();
					break;
				} else {
					// A system error occurred (disk space, database error, ...).
					return false;
				}
			}
			g_setBlockIndexValid.insert(pindexConnect);
		}

	}

	if (g_cChainActive.Tip() != pindexOldTip) {
		std::string strCmd = SysCfg().GetArg("-blocknotify", "");
		if (!IsInitialBlockDownload() && !strCmd.empty()) {
			boost::replace_all(strCmd, "%s", g_cChainActive.Tip()->GetBlockHash().GetHex());
			boost::thread t(runCommand, strCmd); // thread runs free
		}
	}
	LogPrint("CHECKPOINT", "CheckActiveChain End====\n");
	return true;
}

CMerkleBlock::CMerkleBlock(const CBlock& cBlock, CBloomFilter& cBloomFilter) {
	m_cBlockHeader = cBlock.GetBlockHeader();

	vector<bool> vMatch;
	vector<uint256> vHashes;

	vMatch.reserve(cBlock.vptx.size());
	vHashes.reserve(cBlock.vptx.size());

	for (unsigned int i = 0; i < cBlock.vptx.size(); i++) {
		uint256 hash = cBlock.vptx[i]->GetHash();
		if (cBloomFilter.contains(cBlock.vptx[i]->GetHash())) {
			vMatch.push_back(true);
			m_vMatchedTxn.push_back(make_pair(i, hash));
		} else
			vMatch.push_back(false);
		vHashes.push_back(hash);
	}

	m_cTxNum = CPartialMerkleTree(vHashes, vMatch);
}


uint256 CPartialMerkleTree::CalcHash(int nHeight, unsigned int nPos, const vector<uint256> &vcTxid) {
	if (nHeight == 0) {
		// hash at height 0 is the txids themself
		return vcTxid[nPos];
	} else {
		// calculate left hash
		uint256 left = CalcHash(nHeight - 1, nPos * 2, vcTxid), right;
		// calculate right hash if not beyong the end of the array - copy left hash otherwise1
		if (nPos * 2 + 1 < CalcTreeWidth(nHeight - 1))
			right = CalcHash(nHeight - 1, nPos * 2 + 1, vcTxid);
		else
			right = left;
		// combine subhashes
		return Hash(BEGIN(left), END(left), BEGIN(right), END(right));
	}
}

void CPartialMerkleTree::TraverseAndBuild(int nHeight, unsigned int nPos, const vector<uint256> &vcTxid,
		const vector<bool> &vbMatch) {
	// determine whether this node is the parent of at least one matched txid
	bool fParentOfMatch = false;
	for (unsigned int p = nPos << nHeight; p < (nPos + 1) << nHeight && p < m_unTransactions; p++)
		fParentOfMatch |= vbMatch[p];
	// store as flag bit
	m_vbBits.push_back(fParentOfMatch);
	if (nHeight == 0 || !fParentOfMatch) {
		// if at height 0, or nothing interesting below, store hash and stop
		m_vcHash.push_back(CalcHash(nHeight, nPos, vcTxid));
	} else {
		// otherwise, don't store any hash, but descend into the subtrees
		TraverseAndBuild(nHeight - 1, nPos * 2, vcTxid, vbMatch);
		if (nPos * 2 + 1 < CalcTreeWidth(nHeight - 1))
			TraverseAndBuild(nHeight - 1, nPos * 2 + 1, vcTxid, vbMatch);
	}
}

uint256 CPartialMerkleTree::TraverseAndExtract(int nHeight, unsigned int nPos, unsigned int &unBitsUsed,
		unsigned int &unHashUsed, vector<uint256> &vcMatch) {
	if (unBitsUsed >= m_vbBits.size()) {
		// overflowed the bits array - failure
		m_bBad = true;
		return uint256();
	}
	bool fParentOfMatch = m_vbBits[unBitsUsed++];
	if (nHeight == 0 || !fParentOfMatch) {
		// if at height 0, or nothing interesting below, use stored hash and do not descend
		if (unBitsUsed >= m_vcHash.size()) {
			// overflowed the hash array - failure
			m_bBad = true;
			return uint256();
		}
		const uint256 &hash = m_vcHash[unBitsUsed++];
		if (nHeight == 0 && fParentOfMatch) // in case of height 0, we have a matched txid
			vcMatch.push_back(hash);
		return hash;
	} else {
		// otherwise, descend into the subtrees to extract matched txids and hashes
		uint256 left = TraverseAndExtract(nHeight - 1, nPos * 2, unBitsUsed, unHashUsed, vcMatch), right;
		if (nPos * 2 + 1 < CalcTreeWidth(nHeight - 1))
			right = TraverseAndExtract(nHeight - 1, nPos * 2 + 1, unBitsUsed, unHashUsed, vcMatch);
		else
			right = left;
		// and combine them before returning
		return Hash(BEGIN(left), END(left), BEGIN(right), END(right));
	}
}

CPartialMerkleTree::CPartialMerkleTree(const vector<uint256> &vcTxid, const vector<bool> &vbMatch) :
		m_unTransactions(vcTxid.size()), m_bBad(false) {
	// reset state
	m_vbBits.clear();
	m_vcHash.clear();

	// calculate height of tree
	int nHeight = 0;
	while (CalcTreeWidth(nHeight) > 1)
		nHeight++;

	// traverse the partial tree
	TraverseAndBuild(nHeight, 0, vcTxid, vbMatch);
}

CPartialMerkleTree::CPartialMerkleTree() :
		m_unTransactions(0), m_bBad(true) {
}

uint256 CPartialMerkleTree::ExtractMatches(vector<uint256> &vcMatch) {
	vcMatch.clear();
	// An empty set will not work
	if (m_unTransactions == 0) {
		return uint256();
	}
	// check for excessively high numbers of transactions
	// 60 is the lower bound for the size of a serialized CTransaction
	if (m_unTransactions > MAX_BLOCK_SIZE / 60) {
		return uint256();
	}
	// there can never be more hashes provided than one for every txid
	if (m_vcHash.size() > m_unTransactions) {
		return uint256();
	}
	// there must be at least one bit per node in the partial tree, and at least one node per hash
	if (m_vbBits.size() < m_vcHash.size()) {
		return uint256();
	}
	// calculate height of tree
	int nHeight = 0;
	while (CalcTreeWidth(nHeight) > 1) {
		nHeight++;
	}
	// traverse the partial tree
	unsigned int unBitsUsed = 0, unHashUsed = 0;
	uint256 hashMerkleRoot = TraverseAndExtract(nHeight, 0, unBitsUsed, unHashUsed, vcMatch);
	// verify that no problems occured during the tree traversal
	if (m_bBad) {
		return uint256();
	}
	// verify that all bits were consumed (except for the padding caused by serializing it as a byte sequence)
	if ((unBitsUsed + 7) / 8 != (m_vbBits.size() + 7) / 8) {
		return uint256();
	}
	// verify that all hashes were consumed
	if (unHashUsed != m_vcHash.size()) {
		return uint256();
	}

	return hashMerkleRoot;
}

bool AbortNode(const string &strMsg) {
    g_strMiscWarning = strMsg;
    LogPrint("INFO","*** %s\n", strMsg);
    g_cUIInterface.ThreadSafeMessageBox(strMsg, "", CClientUIInterface::MSG_ERROR);
    StartShutdown();
    return false;
}

bool CheckDiskSpace(uint64_t ullAdditionalBytes) {
	uint64_t nFreeBytesAvailable = filesystem::space(GetDataDir()).available;

	// Check for nMinDiskSpace bytes (currently 50MB)
	if (nFreeBytesAvailable < g_sMinDiskSpace + ullAdditionalBytes)
		return AbortNode(_("Error: Disk space is low!"));

	return true;
}

FILE* OpenDiskFile(const ST_DiskBlockPos &pos, const char *prefix, bool fReadOnly) {
	if (pos.IsNull())
		return NULL;
	boost::filesystem::path path = GetDataDir() / "blocks" / strprintf("%s%05u.dat", prefix, pos.nFile);
	boost::filesystem::create_directories(path.parent_path());
	FILE* file = fopen(path.string().c_str(), "rb+");
	if (!file && !fReadOnly)
		file = fopen(path.string().c_str(), "wb+");
	if (!file) {
		LogPrint("INFO", "Unable to open file %s\n", path.string());
		return NULL;
	}
	if (pos.unPos) {
		if (fseek(file, pos.unPos, SEEK_SET)) {
			LogPrint("INFO", "Unable to seek to position %u of %s\n", pos.unPos, path.string());
			fclose(file);
			return NULL;
		}
	}
	return file;
}

FILE* OpenBlockFile(const ST_DiskBlockPos &cDiskBlockPos, bool bReadOnly) {
    return OpenDiskFile(cDiskBlockPos, "blk", bReadOnly);
}

FILE* OpenUndoFile(const ST_DiskBlockPos &cDiskBlockPos, bool bReadOnly) {
    return OpenDiskFile(cDiskBlockPos, "rev", bReadOnly);
}

CBlockIndex* InsertBlockIndex(uint256 cHash) {
	if (cHash.IsNull())
		return NULL;

	// Return existing
	map<uint256, CBlockIndex*>::iterator mi = g_mapBlockIndex.find(cHash);
	if (mi != g_mapBlockIndex.end())
		return (*mi).second;

	// Create new
	CBlockIndex* pindexNew = new CBlockIndex();
	if (!pindexNew)
		throw runtime_error("LoadBlockIndex() : new CBlockIndex failed");
	mi = g_mapBlockIndex.insert(make_pair(cHash, pindexNew)).first;
	pindexNew->m_pHashBlock = &((*mi).first);

	return pindexNew;
}

bool static LoadBlockIndexDB() {
	if (!g_pblocktree->LoadBlockIndexGuts())
		return false;

	boost::this_thread::interruption_point();

	// Calculate nChainWork
	vector<pair<int, CBlockIndex*> > vSortedByHeight;
	vSortedByHeight.reserve(g_mapBlockIndex.size());
	for (const auto& item : g_mapBlockIndex) {
		CBlockIndex* pindex = item.second;
		vSortedByHeight.push_back(make_pair(pindex->m_nHeight, pindex));
	}
	sort(vSortedByHeight.begin(), vSortedByHeight.end());
	for (const auto& item : vSortedByHeight) {
		CBlockIndex* pindex = item.second;
		pindex->m_cChainWork = (pindex->m_pPrevBlockIndex ? pindex->m_pPrevBlockIndex->m_cChainWork : 0)
				+ GetBlockProof(*pindex);
		pindex->m_unChainTx = (pindex->m_pPrevBlockIndex ? pindex->m_pPrevBlockIndex->m_unChainTx : 0) + pindex->m_unTx;
		if ((pindex->m_unStatus & BLOCK_VALID_MASK) >= BLOCK_VALID_TRANSACTIONS
				&& !(pindex->m_unStatus & BLOCK_FAILED_MASK))
			g_setBlockIndexValid.insert(pindex);
		if (pindex->m_unStatus & BLOCK_FAILED_MASK
				&& (!g_pIndexBestInvalid || pindex->m_cChainWork > g_pIndexBestInvalid->m_cChainWork))
			g_pIndexBestInvalid = pindex;
	}

	// Load block file info
	g_pblocktree->ReadLastBlockFile(g_nLastBlockFile);
	LogPrint("INFO", "LoadBlockIndexDB(): last block file = %i\n", g_nLastBlockFile);
	if (g_pblocktree->ReadBlockFileInfo(g_nLastBlockFile, g_InfoLastBlockFile))
		LogPrint("INFO", "LoadBlockIndexDB(): last block file info: %s\n", g_InfoLastBlockFile.ToString());

	// Check whether we need to continue reindexing
	bool fReindexing = false;
	g_pblocktree->ReadReindexing(fReindexing);

	bool fcurReinx = SysCfg().IsReindex();
	SysCfg().SetReIndex(fcurReinx |= fReindexing);

	// Check whether we have a transaction index
	bool bTxIndex = SysCfg().IsTxIndex();
	g_pblocktree->ReadFlag("txindex", bTxIndex);
	SysCfg().SetTxIndex(bTxIndex);
	LogPrint("INFO", "LoadBlockIndexDB(): transaction index %s\n", bTxIndex ? "enabled" : "disabled");

	// Load pointer to end of best chain
	map<uint256, CBlockIndex*>::iterator it = g_mapBlockIndex.find(g_pAccountViewTip->GetBestBlock());

//    for(auto &item : mapBlockIndex)
//    	LogPrint("INFO", "block hash:%s\n", item.first.GetHex());
	LogPrint("INFO", "best block hash:%s\n", g_pAccountViewTip->GetBestBlock().GetHex());
	if (it == g_mapBlockIndex.end())
		return true;
	g_cChainActive.SetTip(it->second);
	LogPrint("INFO", "LoadBlockIndexDB(): hashBestChain=%s height=%d date=%s progress=%f\n",
			g_cChainActive.Tip()->GetBlockHash().ToString(), g_cChainActive.Height(),
			DateTimeStrFormat("%Y-%m-%d %H:%M:%S", g_cChainActive.Tip()->GetBlockTime()),
			Checkpoints::GuessVerificationProgress(g_cChainActive.Tip()));

	return true;
}

bool VerifyDB(int nCheckLevel, int nCheckDepth) {
	LOCK(g_cs_main);
	if (g_cChainActive.Tip() == NULL || g_cChainActive.Tip()->m_pPrevBlockIndex == NULL)
		return true;

	// Verify blocks in the best chain
	if (nCheckDepth <= 0)
		nCheckDepth = 1000000000; // suffices until the year 19000
	if (nCheckDepth > g_cChainActive.Height())
		nCheckDepth = g_cChainActive.Height();
	nCheckLevel = max(0, min(4, nCheckLevel));
	LogPrint("INFO", "Verifying last %i blocks at level %i\n", nCheckDepth, nCheckLevel);
	CAccountViewCache view(*g_pAccountViewTip, true);
	CTransactionDBCache txCacheTemp(*g_pTxCacheTip, true);
	CScriptDBViewCache scriptDBCache(*g_pScriptDBTip, true);
	CBlockIndex* pindexState = g_cChainActive.Tip();
	CBlockIndex* pindexFailure = NULL;
	int nGoodTransactions = 0;
	CValidationState state;
	//   int64_t llTime = 0;
	for (CBlockIndex* pindex = g_cChainActive.Tip(); pindex && pindex->m_pPrevBlockIndex;
			pindex = pindex->m_pPrevBlockIndex) {
//    	llTime = GetTimeMillis();
		boost::this_thread::interruption_point();
		if (pindex->m_nHeight < g_cChainActive.Height() - nCheckDepth)
			break;
		CBlock block;
		//       LogPrint("INFO", "block hash:%s", pindex->GetBlockHash().ToString());
		// check level 0: read from disk
		if (!ReadBlockFromDisk(block, pindex))
			return ERRORMSG("VerifyDB() : *** ReadBlockFromDisk failed at %d, hash=%s", pindex->m_nHeight,
					pindex->GetBlockHash().ToString());
		// check level 1: verify block validity
		if (nCheckLevel >= 1 && !CheckBlock(block, state, view, scriptDBCache))
			return ERRORMSG("VerifyDB() : *** found bad block at %d, hash=%s\n", pindex->m_nHeight,
					pindex->GetBlockHash().ToString());
		// check level 2: verify undo validity
		if (nCheckLevel >= 2 && pindex) {
			CBlockUndo undo;
			ST_DiskBlockPos pos = pindex->GetUndoPos();
			if (!pos.IsNull()) {
				if (!undo.ReadFromDisk(pos, pindex->m_pPrevBlockIndex->GetBlockHash()))
					return ERRORMSG("VerifyDB() : *** found bad undo data at %d, hash=%s\n", pindex->m_nHeight,
							pindex->GetBlockHash().ToString());
			}
		}
		// check level 3: check for inconsistencies during memory-only disconnect of tip blocks
		if (nCheckLevel >= 3
				&& pindex
						== pindexState /*&& (coins.GetCacheSize() + pcoinsTip->GetCacheSize()) <= 2*nCoinCacheSize + 32000*/) {
			bool fClean = true;

			if (!DisconnectBlock(block, state, view, pindex, txCacheTemp, scriptDBCache, &fClean))
				return ERRORMSG("VerifyDB() : *** irrecoverable inconsistency in block data at %d, hash=%s",
						pindex->m_nHeight, pindex->GetBlockHash().ToString());
			pindexState = pindex->m_pPrevBlockIndex;
			if (!fClean) {
				nGoodTransactions = 0;
				pindexFailure = pindex;
			} else
				nGoodTransactions += block.vptx.size();
		}
//        LogPrint("INFO", "VerifyDB block height:%d, hash:%s ,elapse time:%lld ms\n", pindex->nHeight, pindex->GetBlockHash().GetHex(), GetTimeMillis() - llTime);
	}
	if (pindexFailure)
		return ERRORMSG(
				"VerifyDB() : *** coin database inconsistencies found (last %i blocks, %i good transactions before that)\n",
				g_cChainActive.Height() - pindexFailure->m_nHeight + 1, nGoodTransactions);

	// check level 4: try reconnecting blocks
	if (nCheckLevel >= 4) {
		CBlockIndex *pindex = pindexState;
		while (pindex != g_cChainActive.Tip()) {
			boost::this_thread::interruption_point();
			pindex = g_cChainActive.Next(pindex);
			CBlock block;
			if (!ReadBlockFromDisk(block, pindex))
				return ERRORMSG("VerifyDB() : *** ReadBlockFromDisk failed at %d, hash=%s", pindex->m_nHeight,
						pindex->GetBlockHash().ToString());
			if (!ConnectBlock(block, state, view, pindex, txCacheTemp, scriptDBCache, false))
				return ERRORMSG("VerifyDB() : *** found unconnectable block at %d, hash=%s", pindex->m_nHeight,
						pindex->GetBlockHash().ToString());
		}
	}

	LogPrint("INFO", "No coin database inconsistencies in last %i blocks (%i transactions)\n",
			g_cChainActive.Height() - pindexState->m_nHeight, nGoodTransactions);

	return true;
}

void UnloadBlockIndex() {
	g_mapBlockIndex.clear();
	g_setBlockIndexValid.clear();
	g_cChainActive.SetTip(NULL);
	g_pIndexBestInvalid = NULL;
}

bool LoadBlockIndex() {
	// Load block index from databases
	if (!SysCfg().IsReindex() && !LoadBlockIndexDB())
		return false;
	return true;
}

bool InitBlockIndex() {
	LOCK(g_cs_main);
	// Check whether we're already initialized
	if (g_cChainActive.Genesis() != NULL)
		return true;

	// Use the provided setting for -txindex in the new database
	SysCfg().SetTxIndex(SysCfg().GetBoolArg("-txindex", true));
	g_pblocktree->WriteFlag("txindex", SysCfg().IsTxIndex());
	LogPrint("INFO", "Initializing databases...\n");

	// Only add the genesis block if not reindexing (in which case we reuse the one already on disk)
	if (!SysCfg().IsReindex()) {
		try {
			CBlock &block = const_cast<CBlock&>(SysCfg().GenesisBlock());
			// Start new block file
			unsigned int nBlockSize = ::GetSerializeSize(block, SER_DISK, g_sClientVersion);
			ST_DiskBlockPos blockPos;
			CValidationState state;
			if (!FindBlockPos(state, blockPos, nBlockSize + 8, 0, block.GetTime()))
				return ERRORMSG("LoadBlockIndex() : FindBlockPos failed");
			if (!WriteBlockToDisk(block, blockPos))
				return ERRORMSG("LoadBlockIndex() : writing genesis block to disk failed");
			if (!AddToBlockIndex(block, state, blockPos))
				return ERRORMSG("LoadBlockIndex() : genesis block not accepted");
		} catch (runtime_error &e) {
			return ERRORMSG("LoadBlockIndex() : failed to initialize block database: %s", e.what());
		}
	}

	return true;
}

void PrintBlockTree() {
	AssertLockHeld(g_cs_main);
	// pre-compute tree structure
	map<CBlockIndex*, vector<CBlockIndex*> > mapNext;
	for (map<uint256, CBlockIndex*>::iterator mi = g_mapBlockIndex.begin(); mi != g_mapBlockIndex.end(); ++mi) {
		CBlockIndex* pindex = (*mi).second;
		mapNext[pindex->m_pPrevBlockIndex].push_back(pindex);
		// test
		//while (rand() % 3 == 0)
		//    mapNext[pindex->pprev].push_back(pindex);
	}

	vector<pair<int, CBlockIndex*> > vStack;
	vStack.push_back(make_pair(0, g_cChainActive.Genesis()));

	int nPrevCol = 0;
	while (!vStack.empty()) {
		int nCol = vStack.back().first;
		CBlockIndex* pindex = vStack.back().second;
		vStack.pop_back();

		// print split or gap
		if (nCol > nPrevCol) {
			for (int i = 0; i < nCol - 1; i++)
				LogPrint("INFO", "| ");
			LogPrint("INFO", "|\\\n");
		} else if (nCol < nPrevCol) {
			for (int i = 0; i < nCol; i++)
				LogPrint("INFO", "| ");
			LogPrint("INFO", "|\n");
		}
		nPrevCol = nCol;

		// print columns
		for (int i = 0; i < nCol; i++)
			LogPrint("INFO", "| ");

		// print item
		CBlock block;
		ReadBlockFromDisk(block, pindex);
		LogPrint("INFO", "%d (blk%05u.dat:0x%x)  %s  tx %u\n", pindex->m_nHeight, pindex->GetBlockPos().nFile,
				pindex->GetBlockPos().unPos, DateTimeStrFormat("%Y-%m-%d %H:%M:%S", block.GetBlockTime()),
				block.vptx.size());

		// put the main time-chain first
		vector<CBlockIndex*>& vNext = mapNext[pindex];
		for (unsigned int i = 0; i < vNext.size(); i++) {
			if (g_cChainActive.Next(vNext[i])) {
				swap(vNext[0], vNext[i]);
				break;
			}
		}

		// iterate children
		for (unsigned int i = 0; i < vNext.size(); i++)
			vStack.push_back(make_pair(nCol + i, vNext[i]));
	}
}

bool LoadExternalBlockFile(FILE* pFileIn, ST_DiskBlockPos *pDiskBlockPos) {
	int64_t nStart = GetTimeMillis();

	int nLoaded = 0;
	try {
		CBufferedFile blkdat(pFileIn, 2 * MAX_BLOCK_SIZE, MAX_BLOCK_SIZE + 8, SER_DISK, g_sClientVersion);
		uint64_t nStartByte = 0;
		if (pDiskBlockPos) {
			// (try to) skip already indexed part
			CBlockFileInfo info;
			if (g_pblocktree->ReadBlockFileInfo(pDiskBlockPos->nFile, info)) {
				nStartByte = info.m_unSize;
				blkdat.Seek(info.m_unSize);
			}
		}
		uint64_t nRewind = blkdat.GetPos();
		while (blkdat.good() && !blkdat.eof()) {
			boost::this_thread::interruption_point();

			blkdat.SetPos(nRewind);
			nRewind++; // start one byte further next time, in case of failure
			blkdat.SetLimit(); // remove former limit
			unsigned int nSize = 0;
			try {
				// locate a header
				unsigned char buf[MESSAGE_START_SIZE];
				blkdat.FindByte(SysCfg().MessageStart()[0]);
				nRewind = blkdat.GetPos() + 1;
				blkdat >> FLATDATA(buf);
				if (memcmp(buf, SysCfg().MessageStart(), MESSAGE_START_SIZE))
					continue;
				// read size
				blkdat >> nSize;
				if (nSize < 80 || nSize > MAX_BLOCK_SIZE)
					continue;
			} catch (std::exception &e) {
				// no valid block header found; don't complain
				break;
			}
			try {
				// read block
				uint64_t nBlockPos = blkdat.GetPos();
				blkdat.SetLimit(nBlockPos + nSize);
				CBlock block;
				blkdat >> block;
				nRewind = blkdat.GetPos();

				// process block
				if (nBlockPos >= nStartByte) {
					LOCK(g_cs_main);
					if (pDiskBlockPos)
						pDiskBlockPos->unPos = nBlockPos;
					CValidationState state;
					if (ProcessBlock(state, NULL, &block, pDiskBlockPos))
						nLoaded++;
					if (state.IsError())
						break;
				}
			} catch (std::exception &e) {
				LogPrint("INFO", "%s : Deserialize or I/O error - %s", __func__, e.what());
			}
		}
		fclose(pFileIn);
	} catch (runtime_error &e) {
		AbortNode(_("Error: system error: ") + e.what());
	}
	if (nLoaded > 0)
		LogPrint("INFO", "Loaded %i blocks from external file in %dms\n", nLoaded, GetTimeMillis() - nStart);
	return nLoaded > 0;
}

//////////////////////////////////////////////////////////////////////////////
//
// CAlert
//

string GetWarnings(string strFor) {
	int nPriority = 0;
	string strStatusBar;
	string strRPC;

	if (SysCfg().GetBoolArg("-testsafemode", false))
		strRPC = "test";

	if (!CLIENT_VERSION_IS_RELEASE)
		strStatusBar =
				_(
						"This is a pre-release test build - use at your own risk - do not use for mining or merchant applications");

	// Misc warnings like out of disk space and clock is wrong
	if (g_strMiscWarning != "") {
		nPriority = 1000;
		strStatusBar = g_strMiscWarning;
	}

	if (fLargeWorkForkFound) {
		nPriority = 2000;
		strStatusBar = strRPC = _(
				"Warning: The network does not appear to fully agree! Some miners appear to be experiencing issues.");
	} else if (fLargeWorkInvalidChainFound) {
		nPriority = 2000;
		strStatusBar =
				strRPC =
						_(
								"Warning: We do not appear to fully agree with our peers! You may need to upgrade, or other nodes may need to upgrade.");
	}

	// Alerts
	{
		LOCK(g_cs_mapAlerts);
		for (auto& item : g_mapAlerts) {
			const CAlert& alert = item.second;
			if (alert.AppliesToMe() && alert.m_nPriority > nPriority) {
				nPriority = alert.m_nPriority;
				strStatusBar = alert.m_strStatusBar;
			}
		}
	}

	if (strFor == "statusbar")
		return strStatusBar;
	else if (strFor == "rpc")
		return strRPC;
	assert(!"GetWarnings() : invalid parameter");
	return "error";
}

//////////////////////////////////////////////////////////////////////////////
//
// Messages
//


bool static AlreadyHave(const CInv& inv)
{
    switch (inv.m_nType)
    {
    case MSG_TX:
        {
            bool txInMap = false;
            txInMap = g_cTxMemPool.exists(inv.m_cHash);
            return txInMap || g_mapOrphanTransactions.count(inv.m_cHash);
        }
    case MSG_BLOCK:
        return g_mapBlockIndex.count(inv.m_cHash) ||
               g_mapOrphanBlocks.count(inv.m_cHash);
    }
    // Don't know what it is, just say we already got one
    return true;
}

void static ProcessGetData(CNode* pfrom) {
	deque<CInv>::iterator it = pfrom->m_vRecvGetData.begin();

	vector<CInv> vNotFound;

	LOCK(g_cs_main);

	while (it != pfrom->m_vRecvGetData.end()) {
		// Don't bother if send buffer is too full to respond anyway
		if (pfrom->m_unSendSize >= SendBufferSize())
			break;

		const CInv &inv = *it;
		{
			boost::this_thread::interruption_point();
			it++;

			if (inv.m_nType == MSG_BLOCK || inv.m_nType == MSG_FILTERED_BLOCK) {
				bool send = false;
				map<uint256, CBlockIndex*>::iterator mi = g_mapBlockIndex.find(inv.m_cHash);
				if (mi != g_mapBlockIndex.end()) {
					// If the requested block is at a height below our last
					// checkpoint, only serve it if it's in the checkpointed chain
					int nHeight = mi->second->m_nHeight;
					CBlockIndex* pcheckpoint = Checkpoints::GetLastCheckpoint(g_mapBlockIndex);
					if (pcheckpoint && nHeight < pcheckpoint->m_nHeight) {
						if (!g_cChainActive.Contains(mi->second)) {
							LogPrint("INFO",
									"ProcessGetData(): ignoring request for old block that isn't in the main chain\n");
						} else {
							send = true;
						}
					} else {
						send = true;
					}
				}
				if (send) {
					// Send block from disk
					CBlock block;
					ReadBlockFromDisk(block, (*mi).second);
					if (inv.m_nType == MSG_BLOCK)
						pfrom->PushMessage("block", block);
					else // MSG_FILTERED_BLOCK)
					{
						LOCK(pfrom->m_cs_filter);
						if (pfrom->m_pBloomFilter) {
							CMerkleBlock merkleBlock(block, *pfrom->m_pBloomFilter);
							pfrom->PushMessage("merkleblock", merkleBlock);
							// CMerkleBlock just contains hashes, so also push any transactions in the block the client did not see
							// This avoids hurting performance by pointlessly requiring a round-trip
							// Note that there is currently no way for a node to request any single transactions we didnt send here -
							// they must either disconnect and retry or request the full block.
							// Thus, the protocol spec specified allows for us to provide duplicate txn here,
							// however we MUST always provide at least what the remote peer needs
//                            typedef pair<unsigned int, uint256> PairType;
							for (auto& pair : merkleBlock.m_vMatchedTxn)
								if (!pfrom->m_setInventoryKnown.count(CInv(MSG_TX, pair.second)))
									pfrom->PushMessage("tx", block.vptx[pair.first]);
						}
						// else
						// no response
					}

					// Trigger them to send a getblocks request for the next batch of inventory
					if (inv.m_cHash == pfrom->m_cHashContinue) {
						// Bypass PushInventory, this must send even if redundant,
						// and we want it right after the last block so they don't
						// wait for other stuff first.
						vector<CInv> vInv;
						vInv.push_back(CInv(MSG_BLOCK, g_cChainActive.Tip()->GetBlockHash()));
						pfrom->PushMessage("inv", vInv);
						pfrom->m_cHashContinue.SetNull();
						LogPrint("net", "reset node hashcontinue\n");
					}
				}
			} else if (inv.IsKnownType()) {
				// Send stream from relay memory
				bool pushed = false;
				{
					LOCK(g_cs_mapRelay);
					map<CInv, CDataStream>::iterator mi = g_mapRelay.find(inv);
					if (mi != g_mapRelay.end()) {
						pfrom->PushMessage(inv.GetCommand(), (*mi).second);
						pushed = true;
					}
				}
				if (!pushed && inv.m_nType == MSG_TX) {
					std::shared_ptr<CBaseTransaction> pBaseTx = g_cTxMemPool.lookup(inv.m_cHash);
					if (pBaseTx.get()) {
						CDataStream ss(SER_NETWORK, g_sProtocolVersion);
						ss.reserve(1000);
						if (EM_COMMON_TX == pBaseTx->m_chTxType || EM_CONTRACT_TX == pBaseTx->m_chTxType) {
							ss << *((CTransaction *) (pBaseTx.get()));
						} else if (EM_REG_ACCT_TX == pBaseTx->m_chTxType) {
							ss << *((CRegisterAccountTx *) pBaseTx.get());
						} else if (EM_REG_APP_TX == pBaseTx->m_chTxType) {
							ss << *((CRegisterAppTx *) pBaseTx.get());
						}
						pfrom->PushMessage("tx", ss);
						pushed = true;
					}
				}
				if (!pushed) {
					vNotFound.push_back(inv);
				}
			}

			// Track requests for our stuff.
			g_signals.Inventory(inv.m_cHash);

			if (inv.m_nType == MSG_BLOCK || inv.m_nType == MSG_FILTERED_BLOCK)
				break;
		}
	}

	pfrom->m_vRecvGetData.erase(pfrom->m_vRecvGetData.begin(), it);

	if (!vNotFound.empty()) {
		// Let the peer know that we didn't find what it asked for, so it doesn't
		// have to wait around forever. Currently only SPV clients actually care
		// about this message: it's needed when they are recursively walking the
		// dependencies of relevant unconfirmed transactions. SPV clients want to
		// do that because they want to know about (and store and rebroadcast and
		// risk analyze) the dependencies of transactions relevant to them, without
		// having to download the entire memory pool.
		pfrom->PushMessage("notfound", vNotFound);
	}
}

bool static ProcessMessage(CNode* pfrom, string strCommand, CDataStream& vRecv) {
	RandAddSeedPerfmon();
	LogPrint("net", "received: %s (%u bytes)\n", strCommand, vRecv.size());
//  if (GetRand(atoi(SysCfg().GetArg("-dropmessagestest", "0"))) == 0)
//  {
//      LogPrint("INFO","dropmessagestest DROPPING RECV MESSAGE\n");
//      return true;
//  }

	{
		LOCK(g_cs_main);
		State(pfrom->GetId())->nLastBlockProcess = GetTimeMicros();
	}

	if (strCommand == "version") {
		// Each connection can only send one version message
		if (pfrom->m_nVersion != 0) {
			pfrom->PushMessage("reject", strCommand, REJECT_DUPLICATE, string("Duplicate version message"));
			LogPrint("INFO", "Misbehaving, Duplicate version message, nMisbehavior add 1\n");
			Misbehaving(pfrom->GetId(), 1);
			return false;
		}

		int64_t nTime;
		CAddress addrMe;
		CAddress addrFrom;
		uint64_t nNonce = 1;
		vRecv >> pfrom->m_nVersion >> pfrom->m_ullServices >> nTime >> addrMe;
		if (pfrom->m_nVersion < g_sMinPeerProtoVersion) {
			// disconnect from peers older than this proto version
			LogPrint("INFO", "partner %s using obsolete version %i; disconnecting\n", pfrom->m_cAddress.ToString(),
					pfrom->m_nVersion);
			pfrom->PushMessage("reject", strCommand, REJECT_OBSOLETE,
			strprintf("Version must be %d or greater", g_sMinPeerProtoVersion));
			pfrom->m_bDisconnect = true;
			return false;
		}

		if (pfrom->m_nVersion == 10300)
			pfrom->m_nVersion = 300;
		if (!vRecv.empty())
			vRecv >> addrFrom >> nNonce;
		if (!vRecv.empty()) {
			vRecv >> pfrom->m_strSubVer;
			pfrom->m_strCleanSubVer = SanitizeString(pfrom->m_strSubVer);
		}
		if (!vRecv.empty())
			vRecv >> pfrom->m_nStartingHeight;
		if (!vRecv.empty())
			vRecv >> pfrom->m_bRelayTxes; // set to true after we get the first filter* message
		else
			pfrom->m_bRelayTxes = true;

		if (pfrom->m_bInbound && addrMe.IsRoutable()) {
			pfrom->m_strAddrLocal = addrMe;
			SeenLocal(addrMe);
		}

		// Disconnect if we connected to ourself
		if (nNonce == g_ullLocalHostNonce && nNonce > 1) {
			LogPrint("INFO", "connected to self at %s, disconnecting\n", pfrom->m_cAddress.ToString());
			pfrom->m_bDisconnect = true;
			return true;
		}

		// Be shy and don't send version until we hear
		if (pfrom->m_bInbound)
			pfrom->PushVersion();

		pfrom->m_bClient = !(pfrom->m_ullServices & NODE_NETWORK);

		// Change version
		pfrom->PushMessage("verack");
		pfrom->m_cDSSend.SetVersion(min(pfrom->m_nVersion, g_sProtocolVersion));

		if (!pfrom->m_bInbound) {
			// Advertise our address
			if (!g_bNoListen && !IsInitialBlockDownload()) {
				CAddress addr = GetLocalAddress(&pfrom->m_cAddress);
				if (addr.IsRoutable())
					pfrom->PushAddress(addr);
			}

			// Get recent addresses
			if (pfrom->m_bOneShot || /*pfrom->nVersion >= CADDR_TIME_VERSION || */g_cAddrman.size() < 1000) {
				pfrom->PushMessage("getaddr");
				pfrom->m_bGetAddr = true;
			}
			g_cAddrman.Good(pfrom->m_cAddress);
		} else {
			if (((CNetAddr) pfrom->m_cAddress) == (CNetAddr) addrFrom) {
				g_cAddrman.Add(addrFrom, addrFrom);
				g_cAddrman.Good(addrFrom);
			}
		}

		// Relay alerts
		{
			LOCK(g_cs_mapAlerts);
			for (const auto& item : g_mapAlerts)
				item.second.RelayTo(pfrom);
		}

		pfrom->m_bSuccessfullyConnected = true;

		LogPrint("INFO", "receive version message: %s: version %d, blocks=%d, us=%s, them=%s, peer=%s\n",
				pfrom->m_strCleanSubVer, pfrom->m_nVersion, pfrom->m_nStartingHeight, addrMe.ToString(),
				addrFrom.ToString(), pfrom->m_cAddress.ToString());

		AddTimeData(pfrom->m_cAddress, nTime);

		LOCK(g_cs_main);
		g_scPeerBlockCounts.input(pfrom->m_nStartingHeight);

		if (pfrom->m_nStartingHeight > Checkpoints::GetTotalBlocksEstimate()) {
			pfrom->PushMessage("getcheck", Checkpoints::GetTotalBlocksEstimate());
		}
	}

	else if (pfrom->m_nVersion == 0) {
		// Must have a version message before anything else
		Misbehaving(pfrom->GetId(), 1);
		return false;
	}

	else if (strCommand == "verack") {
		pfrom->SetRecvVersion(min(pfrom->m_nVersion, g_sProtocolVersion));
	}

	else if (strCommand == "addr") {
		vector<CAddress> vAddr;
		vRecv >> vAddr;

		// Don't want addr from older versions unless seeding
//        if (pfrom->nVersion < CADDR_TIME_VERSION && addrman.size() > 1000)
//            return true;
		if (vAddr.size() > 1000) {
			Misbehaving(pfrom->GetId(), 20);
			return ERRORMSG("message addr size() = %u", vAddr.size());
		}

		// Store the new addresses
		vector<CAddress> vAddrOk;
		int64_t nNow = GetAdjustedTime();
		int64_t nSince = nNow - 10 * 60;
		for (auto& addr : vAddr) {
			boost::this_thread::interruption_point();

			if (addr.m_ullTime <= 100000000 || addr.m_ullTime > nNow + 10 * 60)
				addr.m_ullTime = nNow - 5 * 24 * 60 * 60;
			pfrom->AddAddressKnown(addr);
			bool fReachable = IsReachable(addr);
			if (addr.m_ullTime > nSince && !pfrom->m_bGetAddr && vAddr.size() <= 10 && addr.IsRoutable()) {
				// Relay to a limited number of other nodes
				{
					LOCK(g_cs_vNodes);
					// Use deterministic randomness to send to the same nodes for 24 hours
					// at a time so the setAddrKnowns of the chosen nodes prevent repeats
					static uint256 hashSalt;
					if (hashSalt.IsNull())
						hashSalt = GetRandHash();
					uint64_t hashAddr = addr.GetHash();
					uint256 hashRand = ArithToUint256(
							UintToArith256(hashSalt) ^ (hashAddr << 32) ^ ((GetTime() + hashAddr) / (24 * 60 * 60)));
					hashRand = Hash(BEGIN(hashRand), END(hashRand));
					multimap<uint256, CNode*> mapMix;
					for (auto pnode : g_vNodes) {
//                        if (pnode->nVersion < CADDR_TIME_VERSION)
//                            continue;
						unsigned int nPointer;
						memcpy(&nPointer, &pnode, sizeof(nPointer));
						uint256 hashKey = ArithToUint256(UintToArith256(hashRand) ^ nPointer);
						hashKey = Hash(BEGIN(hashKey), END(hashKey));
						mapMix.insert(make_pair(hashKey, pnode));
					}
					int nRelayNodes = fReachable ? 2 : 1; // limited relaying of addresses outside our network(s)
					for (multimap<uint256, CNode*>::iterator mi = mapMix.begin();
							mi != mapMix.end() && nRelayNodes-- > 0; ++mi)
						((*mi).second)->PushAddress(addr);
				}
			}
			// Do not store addresses outside our network
			if (fReachable)
				vAddrOk.push_back(addr);
		}
		g_cAddrman.Add(vAddrOk, pfrom->m_cAddress, 2 * 60 * 60);
		if (vAddr.size() < 1000)
			pfrom->m_bGetAddr = false;
		if (pfrom->m_bOneShot)
			pfrom->m_bDisconnect = true;
	}

	else if (strCommand == "inv") {
		vector<CInv> vInv;
		vRecv >> vInv;
		if (vInv.size() > g_sMaxInvSz) {
			Misbehaving(pfrom->GetId(), 20);
			return ERRORMSG("message inv size() = %u", vInv.size());
		}

		LOCK(g_cs_main);

		for (unsigned int nInv = 0; nInv < vInv.size(); nInv++) {
			const CInv &inv = vInv[nInv];

			boost::this_thread::interruption_point();
			pfrom->AddInventoryKnown(inv);

			bool fAlreadyHave = AlreadyHave(inv);

			int nBlockHeight = 0;
			if (g_mapBlockIndex.count(inv.m_cHash) && inv.m_nType == MSG_BLOCK) {
				nBlockHeight = g_mapBlockIndex[inv.m_cHash]->m_nHeight;
			}
			LogPrint("net", "  got inventory [%d]: %s  %s %d from node:%s\n", nInv, inv.ToString(),
					fAlreadyHave ? "have" : "new", nBlockHeight, pfrom->m_cAddress.ToString());

			if (!fAlreadyHave) {
				if (!SysCfg().IsImporting() && !SysCfg().IsReindex()) {
					if (inv.m_nType == MSG_BLOCK)
						AddBlockToQueue(pfrom->GetId(), inv.m_cHash);
					else
						pfrom->AskFor(inv);  // MSG_TX
				}
			} else if (inv.m_nType == MSG_BLOCK && g_mapOrphanBlocks.count(inv.m_cHash)) {
				ST_OrphanBlock * pOrphanBlock = g_mapOrphanBlocks[inv.m_cHash];
				LogPrint("net", "receive orphan block inv heignt=%d hash=%s lead to getblocks\n", pOrphanBlock->height,
						inv.m_cHash.GetHex());
				PushGetBlocks(pfrom, g_cChainActive.Tip(), GetOrphanRoot(inv.m_cHash));
			}

			// Track requests for our stuff
			g_signals.Inventory(inv.m_cHash);
		}
	}

	else if (strCommand == "getdata") {
		vector<CInv> vInv;
		vRecv >> vInv;
		if (vInv.size() > g_sMaxInvSz) {
			Misbehaving(pfrom->GetId(), 20);
			return ERRORMSG("message getdata size() = %u", vInv.size());
		}

		if ((vInv.size() != 1))
			LogPrint("net", "received getdata (%u invsz)\n", vInv.size());

		if ((vInv.size() > 0) || (vInv.size() == 1))
			LogPrint("net", "received getdata for: %s\n", vInv[0].ToString());

		pfrom->m_vRecvGetData.insert(pfrom->m_vRecvGetData.end(), vInv.begin(), vInv.end());
		ProcessGetData(pfrom);
	}

	else if (strCommand == "getblocks") {
		ST_BlockLocator locator;
		uint256 hashStop;
		vRecv >> locator >> hashStop;

		LOCK(g_cs_main);

		// Find the last block the caller has in the main chain
		CBlockIndex* pindex = g_cChainActive.FindFork(locator);
		CBlockIndex* pContinueIndex = NULL;
		CBlockIndex* pStopIndex = NULL;
		if (g_mapBlockIndex.count(pfrom->m_cHashContinue) > 0) {
			pContinueIndex = g_mapBlockIndex[pfrom->m_cHashContinue];
		}
		if (g_mapBlockIndex.count(hashStop) > 0) {
			pStopIndex = g_mapBlockIndex[hashStop];
		}
		if (NULL != pContinueIndex && (pContinueIndex->m_nHeight > pindex->m_nHeight) /*&& (uint256(0) == hashStop)*/) {
			if (NULL == pStopIndex || pStopIndex->m_nHeight > pContinueIndex->m_nHeight)
				pindex = pContinueIndex;
		}
		// Send the rest of the chain; 向网络上发送请求，(从节点的链与其他节点的链分叉的块)
		if (pindex)
			pindex = g_cChainActive.Next(pindex);
		int nLimit = 500;
		LogPrint("net", "getblocks %d to %s limit %d\n", (pindex ? pindex->m_nHeight : -1), hashStop.ToString(),
				nLimit);
		for (; pindex; pindex = g_cChainActive.Next(pindex)) {

			pfrom->PushInventory(CInv(MSG_BLOCK, pindex->GetBlockHash()));
			if (pindex->GetBlockHash() == hashStop) {
				LogPrint("net", "  getblocks stopping at %d %s\n", pindex->m_nHeight,
						pindex->GetBlockHash().ToString());
				pfrom->m_cHashContinue = hashStop;  //add by frank
				break;
			}
			if (--nLimit <= 0) {
				// When this block is requested, we'll send an inv that'll make them
				// getblocks the next batch of inventory.
				LogPrint("net", "  getblocks stopping at limit %d %s\n", pindex->m_nHeight,
						pindex->GetBlockHash().ToString());
				if (NULL == pContinueIndex || (pindex->m_nHeight > pContinueIndex->m_nHeight))
					pfrom->m_cHashContinue = pindex->GetBlockHash();
				break;
			}
		}
		//add by frank
		if (!pindex)
			pfrom->m_cHashContinue.SetNull();
	}

	else if (strCommand == "getheaders") {
		ST_BlockLocator locator;
		uint256 hashStop;
		vRecv >> locator >> hashStop;

		LOCK(g_cs_main);

		CBlockIndex* pindex = NULL;
		if (locator.IsNull()) {
			// If locator is null, return the hashStop block
			map<uint256, CBlockIndex*>::iterator mi = g_mapBlockIndex.find(hashStop);
			if (mi == g_mapBlockIndex.end())
				return true;
			pindex = (*mi).second;
		} else {
			// Find the last block the caller has in the main chain
			pindex = g_cChainActive.FindFork(locator);
			if (pindex)
				pindex = g_cChainActive.Next(pindex);
		}

		// we must use CBlocks, as CBlockHeaders won't include the 0x00 nTx count at the end
		vector<CBlock> vHeaders;
		int nLimit = 2000;
		LogPrint("net", "getheaders %d to %s\n", (pindex ? pindex->m_nHeight : -1), hashStop.ToString());
		for (; pindex; pindex = g_cChainActive.Next(pindex)) {
			vHeaders.push_back(pindex->GetBlockHeader());
			if (--nLimit <= 0 || pindex->GetBlockHash() == hashStop)
				break;
		}
		pfrom->PushMessage("headers", vHeaders);
	}

	else if (strCommand == "tx") {
		vector<uint256> vWorkQueue;
		vector<uint256> vEraseQueue;
		std::shared_ptr<CBaseTransaction> pBaseTx = CreateNewEmptyTransaction(vRecv[0]);

		if (EM_REWARD_TX == pBaseTx->m_chTxType)
			return ERRORMSG("reward tx can't be transfer in network,Hex:%s", HexStr(vRecv.begin(), vRecv.end()));

		vRecv >> pBaseTx;

		CInv inv(MSG_TX, pBaseTx->GetHash());
		pfrom->AddInventoryKnown(inv);

		LOCK(g_cs_main);
		CValidationState state;
		if (AcceptToMemoryPool(g_cTxMemPool, state, pBaseTx.get(), true)) {
			RelayTransaction(pBaseTx.get(), inv.m_cHash);
			mapAlreadyAskedFor.erase(inv);
			vWorkQueue.push_back(inv.m_cHash);
			vEraseQueue.push_back(inv.m_cHash);

			LogPrint("INFO", "AcceptToMemoryPool: %s %s : accepted %s (poolsz %u)\n", pfrom->m_cAddress.ToString(),
					pfrom->m_strCleanSubVer, pBaseTx->GetHash().ToString(), g_cTxMemPool.m_mapTx.size());

		}
		int nDoS = 0;
		if (state.IsInvalid(nDoS)) {
			LogPrint("INFO", "%s from %s %s was not accepted into the memory pool: %s\n", pBaseTx->GetHash().ToString(),
					pfrom->m_cAddress.ToString(), pfrom->m_strCleanSubVer, state.GetRejectReason());
			pfrom->PushMessage("reject", strCommand, state.GetRejectCode(), state.GetRejectReason(), inv.m_cHash);
//			if (nDoS > 0) {
//				LogPrint("INFO", "Misebehaving, add to tx hash %s mempool error, Misbehavior add %d",  pBaseTx->GetHash().GetHex(), nDoS);
//				Misbehaving(pfrom->GetId(), nDoS);
//			}
		}
	}

	else if (strCommand == "block" && !SysCfg().IsImporting() && !SysCfg().IsReindex()) // Ignore blocks received while importing
			{
		CBlock block;
		vRecv >> block;

		LogPrint("net", "received block %s from %s\n", block.GetHash().ToString(), pfrom->m_cAddress.ToString());
		// block.print();

		CInv inv(MSG_BLOCK, block.GetHash());
		pfrom->AddInventoryKnown(inv);

		LOCK(g_cs_main);
		// Remember who we got this block from.
		g_mapBlockSource[inv.m_cHash] = pfrom->GetId();
		MarkBlockAsReceived(inv.m_cHash, pfrom->GetId());

		CValidationState state;
		ProcessBlock(state, pfrom, &block);

		ST_NodeState *nodestate = State(pfrom->GetId());
		if (nodestate == NULL)
			return false;

		if (0 == nodestate->vBlocksToDownload.size() && pfrom->m_nStartingHeight > g_cChainActive.Tip()->m_nHeight) {
			LogPrint("GetLocator", "continue get block from node:%s\n", pfrom->m_cAddress.ToString());
			PushGetBlocks(pfrom, g_cChainActive.Tip(), inv.m_cHash);
		}
	}

	else if (strCommand == "getaddr") {
		pfrom->m_vcAddrToSend.clear();
		vector<CAddress> vAddr = g_cAddrman.GetAddr();
		for (const auto &addr : vAddr)
			pfrom->PushAddress(addr);
	}

	else if (strCommand == "mempool") {
		LOCK2(g_cs_main, pfrom->m_cs_filter);

		vector<uint256> vtxid;
		g_cTxMemPool.queryHashes(vtxid);
		vector<CInv> vInv;
		for (auto& hash : vtxid) {
			CInv inv(MSG_TX, hash);
			CTransaction tx;
			std::shared_ptr<CBaseTransaction> pBaseTx = g_cTxMemPool.lookup(hash);
			if (pBaseTx.get())
				continue; // another thread removed since queryHashes, maybe...

			if ((pfrom->m_pBloomFilter && pfrom->m_pBloomFilter->contains(hash)) ||     //other type transaction
					(!pfrom->m_pBloomFilter))
				vInv.push_back(inv);

			if (vInv.size() == g_sMaxInvSz) {
				pfrom->PushMessage("inv", vInv);
				vInv.clear();
			}
		}
		if (vInv.size() > 0)
			pfrom->PushMessage("inv", vInv);
	}

	else if (strCommand == "ping") {
//        if (pfrom->nVersion > BIP0031_VERSION)
//        {
		uint64_t nonce = 0;
		vRecv >> nonce;
		// Echo the message back with the nonce. This allows for two useful features:
		//
		// 1) A remote node can quickly check if the connection is operational
		// 2) Remote nodes can measure the latency of the network thread. If this node
		//    is overloaded it won't respond to pings quickly and the remote node can
		//    avoid sending us more work, like chain download requests.
		//
		// The nonce stops the remote getting confused between different pings: without
		// it, if the remote node sends a ping once per second and this node takes 5
		// seconds to respond to each, the 5th ping the remote sends would appear to
		// return very quickly.
		pfrom->PushMessage("pong", nonce);
//        }
	}

	else if (strCommand == "pong") {
		int64_t pingUsecEnd = GetTimeMicros();
		uint64_t nonce = 0;
		size_t nAvail = vRecv.in_avail();
		bool bPingFinished = false;
		string sProblem;

		if (nAvail >= sizeof(nonce)) {
			vRecv >> nonce;

			// Only process pong message if there is an outstanding ping (old ping without nonce should never pong)
			if (pfrom->m_ullPingNonceSent != 0) {
				if (nonce == pfrom->m_ullPingNonceSent) {
					// Matching pong received, this ping is no longer outstanding
					bPingFinished = true;
					int64_t pingUsecTime = pingUsecEnd - pfrom->m_llnPingUsecStart;
					if (pingUsecTime > 0) {
						// Successful ping time measurement, replace previous
						pfrom->m_llPingUsecTime = pingUsecTime;
					} else {
						// This should never happen
						sProblem = "Timing mishap";
					}
				} else {
					// Nonce mismatches are normal when pings are overlapping
					sProblem = "Nonce mismatch";
					if (nonce == 0) {
						// This is most likely a bug in another implementation somewhere, cancel this ping
						bPingFinished = true;
						sProblem = "Nonce zero";
					}
				}
			} else {
				sProblem = "Unsolicited pong without ping";
			}
		} else {
			// This is most likely a bug in another implementation somewhere, cancel this ping
			bPingFinished = true;
			sProblem = "Short payload";
		}

		if (!(sProblem.empty())) {
			LogPrint("net", "pong %s %s: %s, %x expected, %x received, %u bytes\n", pfrom->m_cAddress.ToString(),
					pfrom->m_strCleanSubVer, sProblem, pfrom->m_ullPingNonceSent, nonce, nAvail);
		}
		if (bPingFinished) {
			pfrom->m_ullPingNonceSent = 0;
		}
	}

	else if (strCommand == "alert") {
		CAlert alert;
		vRecv >> alert;

		uint256 alertHash = alert.GetHash();
		if (pfrom->m_setKnown.count(alertHash) == 0) {
			if (alert.ProcessAlert()) {
				// Relay
				pfrom->m_setKnown.insert(alertHash);
				{
					LOCK(g_cs_vNodes);
					for (auto pnode : g_vNodes)
						alert.RelayTo(pnode);
				}
			} else {
				// Small DoS penalty so peers that send us lots of
				// duplicate/expired/invalid-signature/whatever alerts
				// eventually get banned.
				// This isn't a Misbehaving(100) (immediate ban) because the
				// peer might be an older or different implementation with
				// a different signature key, etc.
				Misbehaving(pfrom->GetId(), 10);
			}
		}
	}

	else if (strCommand == "filterload") {
		CBloomFilter filter;
		vRecv >> filter;

		if (!filter.IsWithinSizeConstraints()) {
			LogPrint("INFO", "Misebehaving, filter is not with in size constraints, Misbehavior add 100");
			// There is no excuse for sending a too-large filter
			Misbehaving(pfrom->GetId(), 100);
		} else {
			LOCK(pfrom->m_cs_filter);
			delete pfrom->m_pBloomFilter;
			pfrom->m_pBloomFilter = new CBloomFilter(filter);
			pfrom->m_pBloomFilter->UpdateEmptyFull();
		}
		pfrom->m_bRelayTxes = true;
	}

	else if (strCommand == "filteradd") {
		vector<unsigned char> vData;
		vRecv >> vData;

		// Nodes must NEVER send a data item > 520 bytes (the max size for a script data object,
		// and thus, the maximum size any matched object can have) in a filteradd message
		if (vData.size() > 520)        //MAX_SCRIPT_ELEMENT_SIZE)
				{
			LogPrint("INFO", "Misebehaving, send a data item > 520 bytes, Misbehavior add 100");
			Misbehaving(pfrom->GetId(), 100);
		} else {
			LOCK(pfrom->m_cs_filter);
			if (pfrom->m_pBloomFilter)
				pfrom->m_pBloomFilter->insert(vData);
			else {
				LogPrint("INFO", "Misebehaving, filter error, Misbehavior add 100");
				Misbehaving(pfrom->GetId(), 100);
			}
		}
	}

	else if (strCommand == "filterclear") {
		LOCK(pfrom->m_cs_filter);
		delete pfrom->m_pBloomFilter;
		pfrom->m_pBloomFilter = new CBloomFilter();
		pfrom->m_bRelayTxes = true;
	}

	else if (strCommand == "reject") {
		if (SysCfg().IsDebug()) {
			string strMsg;
			unsigned char ccode;
			string strReason;
			vRecv >> strMsg >> ccode >> strReason;

			ostringstream ss;
			ss << strMsg << " code " << itostr(ccode) << ": " << strReason;

			if (strMsg == "block" || strMsg == "tx") {
				uint256 hash;
				vRecv >> hash;
				ss << ": hash " << hash.ToString();
			}
			// Truncate to reasonable length and sanitize before printing:
			string s = ss.str();
			if (s.size() > 111)
				s.erase(111, string::npos);
			LogPrint("net", "Reject %s\n", SanitizeString(s));
		}
	} else if (strCommand == "checkpoint") {
		LogPrint("CHECKPOINT", "enter checkpoint\n");
		std::vector<int> vIndex;
		std::vector<SyncData::CSyncData> vdata;
		vRecv >> vdata;
		BOOST_FOREACH(SyncData::CSyncData& data, vdata){
		if (data.CheckSignature(SysCfg().GetPublicKey()))
		{
			SyncData::CSyncCheckPoint point;
			point.SetData(data);
			SyncData::CSyncDataDb db;
			if (!db.ExistCheckpoint(point.m_height))
			{
				db.WriteCheckpoint(point.m_height, data);
				Checkpoints::AddCheckpoint(point.m_height, point.m_hashCheckpoint);
				CheckActiveChain(point.m_height, point.m_hashCheckpoint);
				pfrom->m_setcheckPointKnown.insert(point.m_height);
				vIndex.push_back(point.m_height);
			}
		}
	}
		if (vIndex.size() == 1 && vIndex.size() == vdata.size()) {
			LOCK(g_cs_vNodes);
			BOOST_FOREACH(CNode* pnode, g_vNodes){
			if (pnode->m_setcheckPointKnown.count(vIndex[0]) == 0)
			{
				pnode->m_setcheckPointKnown.insert(vIndex[0]);
				pnode->PushMessage("checkpoint", vdata);
			}
		}
	}
}
else if (strCommand == "getcheck")
{
	int height = 0;
	vRecv >> height;
	SyncData::CSyncDataDb db;
	std::vector<SyncData::CSyncData> vdata;
	std::vector<int> vheight;
	Checkpoints::GetCheckpointByHeight(height, vheight);
	for (std::size_t i = 0; i < vheight.size(); ++i)
	{
		SyncData::CSyncData data;
		if (pfrom->m_setcheckPointKnown.count(vheight[i]) == 0
				&& db.ReadCheckpoint(vheight[i], data))
		{
			pfrom->m_setcheckPointKnown.insert(vheight[i]);
			vdata.push_back(data);
		}
	}
	if (!vdata.empty())
	{
		pfrom->PushMessage("checkpoint", vdata);
	}
}
else
{
	// Ignore unknown commands for extensibility
}

// Update the last seen time for this node's address
	if (pfrom->m_bNetworkNode)
		if (strCommand == "version" || strCommand == "addr" || strCommand == "inv" || strCommand == "getdata"
				|| strCommand == "ping")
			AddressCurrentlyConnected(pfrom->m_cAddress);

	return true;
}

// requires LOCK(cs_vRecvMsg)
bool ProcessMessages(CNode* pFromNode) {
	//if (fDebug)
	//    LogPrint("INFO","ProcessMessages(%u messages)\n", pfrom->vRecvMsg.size());

	//
	// Message format
	//  (4) message start
	//  (12) command
	//  (4) size
	//  (4) checksum
	//  (x) data
	//
	bool fOk = true;

	if (!pFromNode->m_vRecvGetData.empty())
		ProcessGetData(pFromNode);

	// this maintains the order of responses
	if (!pFromNode->m_vRecvGetData.empty())
		return fOk;

	deque<CNetMessage>::iterator it = pFromNode->m_vRecvMsg.begin();
	while (!pFromNode->m_bDisconnect && it != pFromNode->m_vRecvMsg.end()) {
		// Don't bother if send buffer is too full to respond anyway
		if (pFromNode->m_unSendSize >= SendBufferSize())
			break;

		// get next message
		CNetMessage& msg = *it;

		//if (fDebug)
		//    LogPrint("INFO","ProcessMessages(message %u msgsz, %u bytes, complete:%s)\n",
		//            msg.hdr.nMessageSize, msg.vRecv.size(),
		//            msg.complete() ? "Y" : "N");

		// end, if an incomplete message is found
		if (!msg.complete())
			break;

		// at this point, any failure means we can delete the current message
		it++;

		// Scan for message start
		if (memcmp(msg.m_cMessageHeader.m_pchMessageStart, SysCfg().MessageStart(), MESSAGE_START_SIZE) != 0) {
			LogPrint("INFO", "\n\nPROCESSMESSAGE: INVALID MESSAGESTART\n\n");
			fOk = false;
			break;
		}

		// Read header
		CMessageHeader& hdr = msg.m_cMessageHeader;
		if (!hdr.IsValid()) {
			LogPrint("INFO", "\n\nPROCESSMESSAGE: ERRORS IN HEADER %s\n\n\n", hdr.GetCommand());
			continue;
		}
		string strCommand = hdr.GetCommand();

		// Message size
		unsigned int nMessageSize = hdr.m_unMessageSize;

		// Checksum
		CDataStream& vRecv = msg.m_cDSRecv;
		uint256 hash = Hash(vRecv.begin(), vRecv.begin() + nMessageSize);
		unsigned int nChecksum = 0;
		memcpy(&nChecksum, &hash, sizeof(nChecksum));
		if (nChecksum != hdr.m_unChecksum) {
			LogPrint("INFO", "ProcessMessages(%s, %u bytes) : CHECKSUM ERROR nChecksum=%08x hdr.nChecksum=%08x\n",
					strCommand, nMessageSize, nChecksum, hdr.m_unChecksum);
			continue;
		}

		// Process message
		bool fRet = false;
		try {
			fRet = ProcessMessage(pFromNode, strCommand, vRecv);
			boost::this_thread::interruption_point();
		} catch (std::ios_base::failure& e) {
			pFromNode->PushMessage("reject", strCommand, REJECT_MALFORMED, string("error parsing message"));
			if (strstr(e.what(), "end of data")) {
				// Allow exceptions from under-length message on vRecv
				LogPrint("INFO",
						"ProcessMessages(%s, %u bytes) : Exception '%s' caught, normally caused by a message being shorter than its stated length\n",
						strCommand, nMessageSize, e.what());
				LogPrint("INFO", "ProcessMessages(%s, %u bytes) : %s\n", strCommand, nMessageSize,
						HexStr(vRecv.begin(), vRecv.end()).c_str());
			} else if (strstr(e.what(), "size too large")) {
				// Allow exceptions from over-long size
				LogPrint("INFO", "ProcessMessages(%s, %u bytes) : Exception '%s' caught\n", strCommand, nMessageSize,
						e.what());
			} else {
				PrintExceptionContinue(&e, "ProcessMessages()");
			}
		} catch (boost::thread_interrupted) {
			throw;
		} catch (std::exception& e) {
			PrintExceptionContinue(&e, "ProcessMessages()");
		} catch (...) {
			PrintExceptionContinue(NULL, "ProcessMessages()");
		}

		if (!fRet)
			LogPrint("INFO", "ProcessMessage(%s, %u bytes) FAILED\n", strCommand, nMessageSize);

		break;
	}

	// In case the connection got shut down, its receive buffer was wiped
	if (!pFromNode->m_bDisconnect)
		pFromNode->m_vRecvMsg.erase(pFromNode->m_vRecvMsg.begin(), it);

	return fOk;
}

bool SendMessages(CNode* pToNode, bool bSendTrickle) {
	{
		// Don't send anything until we get their version message
		if (pToNode->m_nVersion == 0)
			return true;

		//
		// Message: ping
		//
		bool pingSend = false;
		if (pToNode->m_bPingQueued) {
			// RPC ping request by user
			pingSend = true;
		}
		if (pToNode->m_llLastSend && GetTime() - pToNode->m_llLastSend > 30 * 60 && pToNode->m_vSendMsg.empty()) {
			// Ping automatically sent as a keepalive
			pingSend = true;
		}
		if (pingSend) {
			uint64_t nonce = 0;
			while (nonce == 0) {
				RAND_bytes((unsigned char*) &nonce, sizeof(nonce));
			}
			pToNode->m_ullPingNonceSent = nonce;
			pToNode->m_bPingQueued = false;

			// Take timestamp as close as possible before transmitting ping
			pToNode->m_llnPingUsecStart = GetTimeMicros();
			pToNode->PushMessage("ping", nonce);
		}

		TRY_LOCK(g_cs_main, lockMain); // Acquire cs_main for IsInitialBlockDownload() and CNodeState()
		if (!lockMain)
			return true;

		// Address refresh broadcast
		static int64_t nLastRebroadcast;
		if (!IsInitialBlockDownload() && (GetTime() - nLastRebroadcast > 24 * 60 * 60)) {
			{
				LOCK(g_cs_vNodes);
				for (auto pnode : g_vNodes) {
					// Periodically clear setAddrKnown to allow refresh broadcasts
					if (nLastRebroadcast)
						pnode->m_cAddrKnown.clear();

					// Rebroadcast our address
					if (!g_bNoListen) {
						CAddress addr = GetLocalAddress(&pnode->m_cAddress);
						if (addr.IsRoutable())
							pnode->PushAddress(addr);
					}
				}
			}
			nLastRebroadcast = GetTime();
		}

		//
		// Message: addr
		//
		if (bSendTrickle) {
			vector<CAddress> vAddr;
			vAddr.reserve(pToNode->m_vcAddrToSend.size());
			for (const auto& addr : pToNode->m_vcAddrToSend) {
				// returns true if wasn't already contained in the set
				if (pToNode->m_cAddrKnown.insert(addr).second) {
					vAddr.push_back(addr);
					// receiver rejects addr messages larger than 1000
					if (vAddr.size() >= 1000) {
						pToNode->PushMessage("addr", vAddr);
						vAddr.clear();
					}
				}
			}
			pToNode->m_vcAddrToSend.clear();
			if (!vAddr.empty())
				pToNode->PushMessage("addr", vAddr);
		}

		ST_NodeState &state = *State(pToNode->GetId());
		if (state.fShouldBan) {
			if (pToNode->m_cAddress.IsLocal())
				LogPrint("INFO", "Warning: not banning local node %s!\n", pToNode->m_cAddress.ToString());
			else {
				pToNode->m_bDisconnect = true;
				CNode::Ban(pToNode->m_cAddress);
			}
			state.fShouldBan = false;
		}

		for (const auto& reject : state.rejects)
			pToNode->PushMessage("reject", (string) "block", reject.chRejectCode, reject.strRejectReason,
					reject.hashBlock);
		state.rejects.clear();

		// Start block sync
		if (pToNode->m_bStartSync && !SysCfg().IsImporting() && !SysCfg().IsReindex()) {
			pToNode->m_bStartSync = false;
			g_nSyncTipHeight = pToNode->m_nStartingHeight;
			g_cUIInterface.NotifyBlocksChanged(0, g_cChainActive.Tip()->m_nHeight,
					g_cChainActive.Tip()->GetBlockHash());
			LogPrint("net", "start block sync lead to getblocks\n");
			PushGetBlocks(pToNode, g_cChainActive.Tip(), uint256());
		}

		// Resend wallet transactions that haven't gotten in a block yet
		// Except during reindex, importing and IBD, when old wallet
		// transactions become unconfirmed and spams other nodes.
		if (!SysCfg().IsReindex() && !SysCfg().IsImporting() && !IsInitialBlockDownload()) {
			g_signals.Broadcast();
		}

		//
		// Message: inventory
		//
		vector<CInv> vInv;
		vector<CInv> vInvWait;
		{
			LOCK(pToNode->m_cs_inventory);
			vInv.reserve(pToNode->m_vInventoryToSend.size());
			vInvWait.reserve(pToNode->m_vInventoryToSend.size());
			for (const auto& inv : pToNode->m_vInventoryToSend) {
				if (pToNode->m_setInventoryKnown.count(inv))
					continue;

				// trickle out tx inv to protect privacy
				if (inv.m_nType == MSG_TX && !bSendTrickle) {
					// 1/4 of tx invs blast to all immediately
					static uint256 hashSalt;
					if (hashSalt.IsNull())
						hashSalt = GetRandHash();
					uint256 hashRand = ArithToUint256(UintToArith256(inv.m_cHash) ^ UintToArith256(hashSalt));
					hashRand = Hash(BEGIN(hashRand), END(hashRand));
					bool fTrickleWait = ((UintToArith256(hashRand) & 3) != 0);

					if (fTrickleWait) {
						vInvWait.push_back(inv);
						continue;
					}
				}

				// returns true if wasn't already contained in the set
				if (pToNode->m_setInventoryKnown.insert(inv).second) {
					vInv.push_back(inv);
					if (vInv.size() >= 1000) {
						pToNode->PushMessage("inv", vInv);
						vInv.clear();
					}
				}
			}
			pToNode->m_vInventoryToSend = vInvWait;
		}
		if (!vInv.empty())
			pToNode->PushMessage("inv", vInv);

		// Detect stalled peers. Require that blocks are in flight, we haven't
		// received a (requested) block in one minute, and that all blocks are
		// in flight for over two minutes, since we first had a chance to
		// process an incoming block.
		int64_t nNow = GetTimeMicros();
		if (!pToNode->m_bDisconnect && state.nBlocksInFlight
				&& state.nLastBlockReceive < state.nLastBlockProcess - BLOCK_DOWNLOAD_TIMEOUT * 1000000
				&& state.vBlocksInFlight.front().nTime
						< state.nLastBlockProcess - 2 * BLOCK_DOWNLOAD_TIMEOUT * 1000000) {
			LogPrint("INFO", "Peer %s is stalling block download, disconnecting\n", state.name.c_str());
			pToNode->m_bDisconnect = true;
		}

		//
		// Message: getdata (blocks)
		//
		vector<CInv> vGetData;
		int nIndex(0);
		while (!pToNode->m_bDisconnect && state.nBlocksToDownload
				&& state.nBlocksInFlight < MAX_BLOCKS_IN_TRANSIT_PER_PEER) {
			uint256 hash = state.vBlocksToDownload.front();
			vGetData.push_back(CInv(MSG_BLOCK, hash));
			MarkBlockAsInFlight(pToNode->GetId(), hash);
			LogPrint("net", "Requesting block [%d] %s from %s\n", ++nIndex, hash.ToString().c_str(),
					state.name.c_str());
			if (vGetData.size() >= 1000) {
				pToNode->PushMessage("getdata", vGetData);
				vGetData.clear();
				nIndex = 0;
			}
		}

		//
		// Message: getdata (non-blocks)
		//
		while (!pToNode->m_bDisconnect && !pToNode->m_mapAskFor.empty() && (*pToNode->m_mapAskFor.begin()).first <= nNow) {
			const CInv& inv = (*pToNode->m_mapAskFor.begin()).second;
			if (!AlreadyHave(inv)) {

				LogPrint("net", "sending getdata: %s\n", inv.ToString());
				vGetData.push_back(inv);
				if (vGetData.size() >= 1000) {
					pToNode->PushMessage("getdata", vGetData);
					vGetData.clear();
				}
			}
			pToNode->m_mapAskFor.erase(pToNode->m_mapAskFor.begin());
		}
		if (!vGetData.empty())
			pToNode->PushMessage("getdata", vGetData);

	}
	return true;
}

class CMainCleanup {
 public:
	CMainCleanup() {
	}
	~CMainCleanup() {
		// block headers
		map<uint256, CBlockIndex*>::iterator it1 = g_mapBlockIndex.begin();
		for (; it1 != g_mapBlockIndex.end(); it1++) {
			delete (*it1).second;
		}

		g_mapBlockIndex.clear();

		// orphan blocks
		map<uint256, ST_OrphanBlock*>::iterator it2 = g_mapOrphanBlocks.begin();
		for (; it2 != g_mapOrphanBlocks.end(); it2++) {
			delete (*it2).second;
		}

		g_mapOrphanBlocks.clear();
		// orphan transactions
		g_mapOrphanTransactions.clear();
	}
} instance_of_cmaincleanup;

std::shared_ptr<CBaseTransaction> CreateNewEmptyTransaction(unsigned char uchType) {
	switch (uchType) {
	case EM_COMMON_TX:
	case EM_CONTRACT_TX:
		return std::make_shared<CTransaction>();
	case EM_REG_ACCT_TX:
		return std::make_shared<CRegisterAccountTx>();
	case EM_REWARD_TX:
		return std::make_shared<CRewardTransaction>();
	case EM_REG_APP_TX:
		return std::make_shared<CRegisterAppTx>();
	default:
		ERRORMSG("CreateNewEmptyTransaction type error");
		break;
	}
	return NULL;
}

string CBlockUndo::ToString() const {
	vector<CTxUndo>::const_iterator iterUndo = m_vcTxUndo.begin();
	string str("");
	LogPrint("INFO", "list txundo:\n");
	for (; iterUndo != m_vcTxUndo.end(); ++iterUndo) {
		str += iterUndo->ToString();
	}
	return str;
}

bool DisconnectBlockFromTip(CValidationState &cValidationState) {
	return DisconnectTip(cValidationState);
}

bool GetTxOperLog(const uint256 &cTxHash, vector<CAccountLog> &vcAccountLog) {
	if (SysCfg().IsTxIndex()) {
		ST_DiskTxPos postx;
		if (g_pScriptDBTip->ReadTxIndex(cTxHash, postx)) {
			CAutoFile file(OpenBlockFile(postx, true), SER_DISK, g_sClientVersion);
			CBlockHeader header;
			try {
				file >> header;
			} catch (std::exception &e) {
				return ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
			}
			uint256 blockHash = header.GetHash();
			if (g_mapBlockIndex.count(blockHash) > 0) {
				CBlockIndex *pIndex = g_mapBlockIndex[blockHash];
				CBlockUndo blockUndo;
				ST_DiskBlockPos pos = pIndex->GetUndoPos();
				if (pos.IsNull())
					return ERRORMSG("DisconnectBlock() : no undo data available");
				if (!blockUndo.ReadFromDisk(pos, pIndex->m_pPrevBlockIndex->GetBlockHash()))
					return ERRORMSG("DisconnectBlock() : failure reading undo data");

				for (auto &txUndo : blockUndo.m_vcTxUndo) {
					if (txUndo.m_cTxHash == cTxHash) {
						vcAccountLog = txUndo.m_vcAccountLog;
						return true;
					}

				}
			}
		}
	}
	return false;
}

Value ListSetBlockIndexValid() {
	Object result;
	std::set<CBlockIndex*, CBlockIndexWorkComparator>::reverse_iterator it = g_setBlockIndexValid.rbegin();
	for (; it != g_setBlockIndexValid.rend(); ++it) {
		CBlockIndex *pIndex = *it;
		result.push_back(
				Pair(tfm::format("height=%d status=%b", pIndex->m_nHeight, pIndex->m_unStatus).c_str(),
						pIndex->GetBlockHash().GetHex()));
	}
	uint256 hash = uint256S("0x6dccf719d146184b9a26e37d62be193fd51d0d49b2f8aa15f84656d790e1d46c");
	CBlockIndex *blockIndex = g_mapBlockIndex[hash];
	for (; blockIndex != NULL && blockIndex->m_nHeight > 157332; blockIndex = blockIndex->m_pPrevBlockIndex) {
		result.push_back(
				Pair(tfm::format("height=%d status=%b", blockIndex->m_nHeight, blockIndex->m_unStatus).c_str(),
						blockIndex->GetBlockHash().GetHex()));
	}
	return result;
}

bool EraseBlockIndexFromSet(CBlockIndex *pIndex) {
	AssertLockHeld(g_cs_main);
	return g_setBlockIndexValid.erase(pIndex)>0;
}
