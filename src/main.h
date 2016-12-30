// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_MAIN_H_
#define DACRS_MAIN_H_

#if defined(HAVE_CONFIG_H)
#include "dacrs-config.h"
#endif

#include "bignum.h"
#include "chainparams.h"
#include "core.h"
#include "net.h"
#include "sync.h"
#include "txmempool.h"
#include "uint256.h"
#include "database.h"
#include "arith_uint256.h"

#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

class CBlockIndex;
class CBloomFilter;
class CInv;
class CContractScript;


/** the total blocks of burn fee need */
static const unsigned int DEFAULT_BURN_BLOCK_SIZE = 50;
/** The maximum allowed size for a serialized block, in bytes (network rule) */
static const unsigned int MAX_BLOCK_SIZE = 1000000;
/** Default for -blockmaxsize and -blockminsize, which control the range of sizes the mining code will create **/
static const unsigned int DEFAULT_BLOCK_MAX_SIZE = 750000;
static const unsigned int DEFAULT_BLOCK_MIN_SIZE = 1024*10;
/** Default for -blockprioritysize, maximum space for zero/low-fee transactions **/
static const unsigned int DEFAULT_BLOCK_PRIORITY_SIZE = 50000;
/** The maximum size for transactions we're willing to relay/mine */
static const unsigned int MAX_STANDARD_TX_SIZE = 100000;
/** The maximum allowed number of signature check operations in a block (network rule) */
static const unsigned int MAX_BLOCK_SIGOPS = MAX_BLOCK_SIZE/50;
/** The maximum number of orphan transactions kept in memory */
static const unsigned int MAX_ORPHAN_TRANSACTIONS = MAX_BLOCK_SIZE/100;
/** The maximum number of orphan blocks kept in memory */
static const unsigned int MAX_ORPHAN_BLOCKS = 750;
/** The maximum size of a blk?????.dat file (since 0.8) */
static const unsigned int MAX_BLOCKFILE_SIZE = 0x8000000; // 128 MiB
/** The pre-allocation chunk size for blk?????.dat files (since 0.8) */
static const unsigned int BLOCKFILE_CHUNK_SIZE = 0x1000000; // 16 MiB
/** The pre-allocation chunk size for rev?????.dat files (since 0.8) */
static const unsigned int UNDOFILE_CHUNK_SIZE = 0x100000; // 1 MiB
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = 100;
/** Threshold for nLockTime: below this value it is interpreted as block number, otherwise as UNIX timestamp. */
static const unsigned int LOCKTIME_THRESHOLD = 500000000; // Tue Nov  5 00:53:20 1985 UTC
/** Maximum number of script-checking threads allowed */
static const int MAX_SCRIPTCHECK_THREADS = 16;
/** -par default (number of script-checking threads, 0 = auto) */
static const int DEFAULT_SCRIPTCHECK_THREADS = 0;
/** Number of blocks that can be requested at any given time from a single peer. */
static const int MAX_BLOCKS_IN_TRANSIT_PER_PEER = 128;
/** Timeout in seconds before considering a block download peer unresponsive. */
static const unsigned int BLOCK_DOWNLOAD_TIMEOUT = 60;
static const long MAX_BLOCK_RUN_STEP = 12000000;
static const int64_t POS_REWARD = 10 * COIN;
static const int64_t INIT_FUEL_RATES = 100;   //100 unit / 100 step
static const int64_t MIN_FUEL_RATES = 1;      //1 unit / 100 step

#ifdef USE_UPNP
static const int g_sHaveUPnP = true;
#else
static const int g_sHaveUPnP = false;
#endif

/** "reject" message codes **/
static const unsigned char REJECT_MALFORMED 		= 0x01;
static const unsigned char REJECT_INVALID 			= 0x10;
static const unsigned char REJECT_OBSOLETE 			= 0x11;
static const unsigned char REJECT_DUPLICATE 		= 0x12;
static const unsigned char REJECT_NONSTANDARD 		= 0x40;
static const unsigned char REJECT_DUST 				= 0x41;
static const unsigned char REJECT_INSUFFICIENTFEE 	= 0x42;
static const unsigned char REJECT_CHECKPOINT 		= 0x43;
static const unsigned char UPDATE_ACCOUNT_FAIL 		= 0X50;

//extern CScript COINBASE_FLAGS;
extern CCriticalSection g_cs_main;
extern CTxMemPool g_cTxMemPool;
extern map<uint256, CBlockIndex*> g_mapBlockIndex;
extern uint64_t g_ullLastBlockTx;
extern uint64_t g_ullLastBlockSize;
extern const string g_strMessageMagic;

// Minimum disk space required - used in CheckDiskSpace()
static const uint64_t g_sMinDiskSpace 			= 52428800;
static const int g_sRegAppFuel2FeeForkHeight 	= 200000; 		// 改注册app燃料费为小费高度[Lockie]
static const int g_sBurnRateForkHeight 			= 45000;    	//修改燃烧费率算法
static const int g_sTwelveForwardLimits 		= 28000;   		//修改限制block时间不能超过本地时间12分钟
static const int g_sFixedDifficulty 			= 35001;    	//此高度前的block不检查难度，通过checkpoint保证
static const int g_sNextWorkRequired 			= 85000;     	//修改难度校验算法
static const int g_sFreezeBlackAcctHeight 		= 99854;
static const int g_sLimiteAppHeight 			= 189000;
static const int g_sUpdateTxVersion2Height 		= 196000;  		//主链在此高度后不再接受交易版本为nTxVersion1的交易
static const int g_sUpdateBlockVersionHeight 	= 209000;   	//主链在此高度后，block版本升级
static const int g_sLimite8051AppHeight 		= 160000;    	//在此高度后，不能注册8051脚本
static const int g_sBlockTime4AppAccountHeight 	= 600000;  		//在此高度后，改用block时间取代高度操作脚本应用账户
static const int g_sBlockRemainCoinDayHeight 	= 700000;  		//此高度之后，用新的剩余币龄算法

class CCoinsDB;
class CBlockTreeDB;
struct ST_DiskBlockPos;
class CTxUndo;
class CValidationState;
class CWalletInterface;
struct ST_NodeStateStats;
class CAccountViewDB;
class CTransactionDB;
class CScriptDB;
struct ST_BlockTemplate;

/** Register a wallet to receive updates from core */
void RegisterWallet(CWalletInterface* pWalletIn);
/** Unregister a wallet from core */
void UnregisterWallet(CWalletInterface* pWalletIn);
/** Unregister all wallets from core */
void UnregisterAllWallets();
/** Push an updated transaction to all registered wallets */
void SyncWithWallets(const uint256 &cHash, CBaseTransaction *pBaseTx, const CBlock* pBlock = NULL);
/** Erase Tx from wallets **/
void EraseTransaction(const uint256 &cHash);
/** Register with a network node to receive its signals */
void RegisterNodeSignals(ST_NodeSignals& tNodeSignals);
/** Unregister a network node */
void UnregisterNodeSignals(ST_NodeSignals& tNodeSignals);
void PushGetBlocks(CNode* pNode, CBlockIndex* pBlockIndexBegin, uint256 cHashEnd);
/** Process an incoming block */
bool ProcessBlock(CValidationState &cValidationState, CNode* pFromNode, CBlock* pBlock, ST_DiskBlockPos *pDiskBlockPos = NULL);
/** Check whether enough disk space is available for an incoming block */
bool CheckDiskSpace(uint64_t ullAdditionalBytes = 0);
/** Open a block file (blk?????.dat) */
FILE* OpenBlockFile(const ST_DiskBlockPos &cDiskBlockPos, bool bReadOnly = false);
/** Open an undo file (rev?????.dat) */
FILE* OpenUndoFile(const ST_DiskBlockPos &cDiskBlockPos, bool bReadOnly = false);
/** Import blocks from an external file */
bool LoadExternalBlockFile(FILE* pFileIn, ST_DiskBlockPos *pDiskBlockPos = NULL);
/** Initialize a new block tree database + block data on disk */
bool InitBlockIndex();
/** Load the block tree and coins database from disk */
bool LoadBlockIndex();
/** Unload database information */
void UnloadBlockIndex();
/** Verify consistency of the block and coin databases */
bool VerifyDB(int nCheckLevel, int nCheckDepth);
/** Print the loaded block tree */
void PrintBlockTree();
/** Process protocol messages received from a given node */
bool ProcessMessages(CNode* pFromNode);
/** Send queued protocol messages to be sent to a give node */
bool SendMessages(CNode* pToNode, bool bSendTrickle);
/** Run an instance of the script checking thread */
void ThreadScriptCheck();
/** Check whether a block hash satisfies the proof-of-work requirement specified by nBits */
bool CheckProofOfWork(uint256 cHash, unsigned int unBits);
/** Calculate the minimum amount of work a received block needs, without knowing its direct parent */
unsigned int ComputeMinWork(unsigned int unBase, int64_t llTime);
/** Get the number of active peers */
int GetNumBlocksOfPeers();
/** Check whether we are doing an initial block download (synchronizing from disk or network) */
bool IsInitialBlockDownload();
/** Format a string that describes several potential problems detected by the core */
string GetWarnings(string strFor);
/** Retrieve a transaction (from memory pool, or from disk, if possible) */
bool GetTransaction(std::shared_ptr<CBaseTransaction> &pBaseTx, const uint256 &cHash, CScriptDBViewCache& cScriptDBCache, bool bSearchMempool=true);
/** Retrieve a transaction high comfirmed in block*/
int GetTxComfirmHigh(const uint256 &cHash, CScriptDBViewCache &cScriptDBCache);
/** Find the best known block, and make it the tip of the block chain */
bool ActivateBestChain(CValidationState &cValidationState);
int64_t GetBlockValue(int nHeight, int64_t llFees);
unsigned int GetNextWorkRequired(const CBlockIndex* pLastBlockIndex, const CBlockHeader *pBlockHeader);
/*calutate difficulty */
double CaculateDifficulty(unsigned int unBits);
/** receive checkpoint check make active chain accord to the checkpoint **/
bool CheckActiveChain(int nHeight,  uint256 cHash);
void UpdateTime(CBlockHeader& cBlockHeader, const CBlockIndex* pPrevBlockIndex);
/** Create a new block index entry for a given block hash */
CBlockIndex * InsertBlockIndex(uint256 cHash);
/** Abort with a message */
bool AbortNode(const string &strMsg);
/** Get statistics from node state */
bool GetNodeStateStats(NodeId nNodeId, ST_NodeStateStats &NodeStateStats);
/** Increase a node's misbehavior score. */
void Misbehaving(NodeId nNodeId, int nHowMuch);
bool CheckSignScript(const uint256 & cSigHash, const std::vector<unsigned char> vchSignature, const CPubKey cPubKey);
/** (try to) add transaction to memory pool **/
bool AcceptToMemoryPool(CTxMemPool& cTxMemPool, CValidationState &cValidationState, CBaseTransaction *pBaseTx,
		  bool bLimitFree, bool bRejectInsaneFee = false);
/** get transaction relate keyid **/
std::shared_ptr<CBaseTransaction> CreateNewEmptyTransaction(unsigned char uchType);

struct ST_NodeStateStats {
    int nMisbehavior;
};

struct ST_DiskBlockPos {
	IMPLEMENT_SERIALIZE(
			READWRITE(VARINT(nFile));
			READWRITE(VARINT(unPos));
	)

	ST_DiskBlockPos() {
		SetNull();
	}
	ST_DiskBlockPos(int nFileIn, unsigned int nPosIn) {
		nFile = nFileIn;
		unPos = nPosIn;
	}
	friend bool operator==(const ST_DiskBlockPos &a, const ST_DiskBlockPos &b) {
		return (a.nFile == b.nFile && a.unPos == b.unPos);
	}
	friend bool operator!=(const ST_DiskBlockPos &a, const ST_DiskBlockPos &b) {
		return !(a == b);
	}
	void SetNull() {
		nFile = -1;
		unPos = 0;
	}
	bool IsNull() const {
		return (nFile == -1);
	}

	int nFile;
	unsigned int unPos;
};

struct ST_DiskTxPos: public ST_DiskBlockPos {
	IMPLEMENT_SERIALIZE(
			READWRITE(*(ST_DiskBlockPos*)this);
			READWRITE(VARINT(m_unTxOffset));
	)

	ST_DiskTxPos(const ST_DiskBlockPos &tBlockIn, unsigned int unTxOffsetIn) :
			ST_DiskBlockPos(tBlockIn.nFile, tBlockIn.unPos), m_unTxOffset(unTxOffsetIn) {
	}
	ST_DiskTxPos() {
		SetNull();
	}
	void SetNull() {
		ST_DiskBlockPos::SetNull();
		m_unTxOffset = 0;
	}

	unsigned int m_unTxOffset; // after header
};

enum GetMinFee_mode {
	GMF_RELAY,
	GMF_SEND,
};

int64_t GetMinFee(const CBaseTransaction *pBaseTx, unsigned int unBytes, bool bAllowFree, enum GetMinFee_mode mode);
/** Count ECDSA signature operations the old-fashioned (pre-0.6) way
    @return number of sigops this transaction's outputs will produce when spent
    @see CTransaction::FetchInputs
*/
unsigned int GetLegacySigOpCount(const CTransaction& cTx);

inline bool AllowFree(double dPriority) {
	// Large (in bytes) low-priority (new, small-coin) transactions
	// need a fee.
	// return dPriority > COIN * 144 / 250;
	return true;
}

// Context-independent validity checks
bool CheckTransaction(CBaseTransaction *pBaseTx, CValidationState& cValidationState, CAccountViewCache &cAccountViewCache);
/** Check for standard transaction types
    @return True if all outputs (scriptPubKeys) use only standard transaction forms
*/
bool IsStandardTx(CBaseTransaction *pBaseTx, string& strReason);
bool IsFinalTx(CBaseTransaction *pBaseTx, int nBlockHeight = 0, int64_t llBlockTime = 0);

/** Undo information for a CBlock */
class CBlockUndo {
 public:
	IMPLEMENT_SERIALIZE(
			READWRITE(m_vcTxUndo);
	)

	bool WriteToDisk(ST_DiskBlockPos &tDiskBlockPos, const uint256 &cHashBlock) {
		// Open history file to append
		CAutoFile cFileout = CAutoFile(OpenUndoFile(tDiskBlockPos), SER_DISK, g_sClientVersion);
		if (!cFileout) {
			return ERRORMSG("CBlockUndo::WriteToDisk : OpenUndoFile failed");
		}
		// Write index header
		unsigned int unSize = cFileout.GetSerializeSize(*this);
		cFileout << FLATDATA(SysCfg().MessageStart()) << unSize;
		// Write undo data
		long lFileOutPos = ftell(cFileout);
		if (lFileOutPos < 0) {
			return ERRORMSG("CBlockUndo::WriteToDisk : ftell failed");
		}
		tDiskBlockPos.unPos = (unsigned int) lFileOutPos;
		cFileout << *this;
		// calculate & write checksum
		CHashWriter cHashWriter(SER_GETHASH, g_sProtocolVersion);
		cHashWriter << cHashBlock;
		cHashWriter << *this;
		cFileout << cHashWriter.GetHash();

		// Flush stdio buffers and commit to disk before returning
		fflush(cFileout);
		if (!IsInitialBlockDownload()) {
			FileCommit(cFileout);
		}
		return true;
	}

	bool ReadFromDisk(const ST_DiskBlockPos &tDiskBlockPos, const uint256 &cHashBlock) {
		// Open history file to read
		CAutoFile cFilein = CAutoFile(OpenUndoFile(tDiskBlockPos, true), SER_DISK, g_sClientVersion);
		if (!cFilein) {
			return ERRORMSG("CBlockUndo::ReadFromDisk : OpenBlockFile failed");
		}
		// Read block
		uint256 cHashChecksum;
		try {
			cFilein >> *this;
			cFilein >> cHashChecksum;
		} catch (std::exception &e) {
			return ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
		}

		// Verify checksum
		CHashWriter cHashWriter(SER_GETHASH, g_sProtocolVersion);
		cHashWriter << cHashBlock;
		cHashWriter << *this;
		if (cHashChecksum != cHashWriter.GetHash()) {
			return ERRORMSG("CBlockUndo::ReadFromDisk : Checksum mismatch");
		}
		return true;
	}

	string ToString() const;

	vector<CTxUndo> m_vcTxUndo;
};


/** A transaction with a merkle branch linking it to the block chain. */
class CMerkleTx {
 public:
	CMerkleTx() {
		Init();
	}

	CMerkleTx(std::shared_ptr<CBaseTransaction> pBaseTx) :
			m_ptrTx(pBaseTx) {
		Init();
	}

	void Init() {
		m_cHashBlock = uint256();
		m_nIndex = -1;
		m_bMerkleVerified = false;
	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(m_cHashBlock);
			READWRITE(m_vcMerkleBranch);
			READWRITE(m_nIndex);
			READWRITE(m_ptrTx);
			READWRITE(m_nHeight);
	)

	int SetMerkleBranch(const CBlock* pBlock = NULL);

	// Return depth of transaction in blockchain:
	// -1  : not in blockchain, and not in memory pool (conflicted transaction)
	//  0  : in memory pool, waiting to be included in a block
	// >=1 : this many blocks deep in the main chain
	int GetDepthInMainChain(CBlockIndex* &pBlockIndexRet) const;
	int GetDepthInMainChain() const {
		CBlockIndex *pindexRet;
		return GetDepthInMainChain(pindexRet);
	}

	bool IsInMainChain() const {
		CBlockIndex *pBlockIndexRet;
		return GetDepthInMainChainINTERNAL(pBlockIndexRet) > 0;
	}

	int GetBlocksToMaturity() const;
	bool AcceptToMemoryPool(bool bLimitFree = true);

	uint256 m_cHashBlock;
	vector<uint256> m_vcMerkleBranch;
	int m_nIndex;
	std::shared_ptr<CBaseTransaction> m_ptrTx;
	int m_nHeight;
	// memory only
	mutable bool m_bMerkleVerified;

 private:
	int GetDepthInMainChainINTERNAL(CBlockIndex* &pBlockIndexRet) const;
};

/** Data structure that represents a partial merkle tree.
 *
 * It respresents a subset of the txid's of a known block, in a way that
 * allows recovery of the list of txid's and the merkle root, in an
 * authenticated way.
 *
 * The encoding works as follows: we traverse the tree in depth-first order,
 * storing a bit for each traversed node, signifying whether the node is the
 * parent of at least one matched leaf txid (or a matched txid itself). In
 * case we are at the leaf level, or this bit is 0, its merkle node hash is
 * stored, and its children are not explorer further. Otherwise, no hash is
 * stored, but we recurse into both (or the only) child branch. During
 * decoding, the same depth-first traversal is performed, consuming bits and
 * hashes as they written during encoding.
 *
 * The serialization is fixed and provides a hard guarantee about the
 * encoded size:
 *
 *   SIZE <= 10 + ceil(32.25*N)
 *
 * Where N represents the number of leaf nodes of the partial tree. N itself
 * is bounded by:
 *
 *   N <= total_transactions
 *   N <= 1 + matched_transactions*tree_height
 *
 * The serialization format:
 *  - uint32     total_transactions (4 bytes)
 *  - varint     number of hashes   (1-3 bytes)
 *  - uint256[]  hashes in depth-first order (<= 32*N bytes)
 *  - varint     number of bytes of flag bits (1-3 bytes)
 *  - byte[]     flag bits, packed per 8 in a byte, least significant bit first (<= 2*N-1 bits)
 * The size constraints follow from this.
 */
class CPartialMerkleTree {
 public:
    // serialization implementation
    IMPLEMENT_SERIALIZE(
        READWRITE(m_unTransactions);
        READWRITE(m_vcHash);
        vector<unsigned char> vcBytes;
        if (fRead) {
            READWRITE(vcBytes);
            CPartialMerkleTree &cUS = *(const_cast<CPartialMerkleTree*>(this));
            cUS.m_vbBits.resize(vcBytes.size() * 8);
            for (unsigned int p = 0; p < cUS.m_vbBits.size(); p++) {
            	cUS.m_vbBits[p] = (vcBytes[p / 8] & (1 << (p % 8))) != 0;
            }
            cUS.m_bBad = false;
        } else {
            vcBytes.resize((m_vbBits.size()+7)/8);
            for (unsigned int p = 0; p < m_vbBits.size(); p++) {
            	vcBytes[p / 8] |= m_vbBits[p] << (p % 8);
            }
            READWRITE(vcBytes);
        }
    )

    // Construct a partial merkle tree from a list of transaction id's, and a mask that selects a subset of them
    CPartialMerkleTree(const vector<uint256> &vcTxid, const vector<bool> &vbMatch);
    CPartialMerkleTree();
    // extract the matching txid's represented by this partial merkle tree.
    // returns the merkle root, or 0 in case of failure
    uint256 ExtractMatches(vector<uint256> &vcMatch);

 protected:
	// calculate the hash of a node in the merkle tree (at leaf level: the txid's themself)
	uint256 CalcHash(int nHeight, unsigned int nPos, const vector<uint256> &vcTxid);
	// recursive function that traverses tree nodes, storing the data as bits and hashes
	void TraverseAndBuild(int nHeight, unsigned int nPos, const vector<uint256> &vcTxid, const vector<bool> &vbMatch);
	// recursive function that traverses tree nodes, consuming the bits and hashes produced by TraverseAndBuild.
	// it returns the hash of the respective node.
	uint256 TraverseAndExtract(int nHeight, unsigned int nPos, unsigned int &unBitsUsed, unsigned int &unHashUsed,
			vector<uint256> &vcMatch);

	// helper function to efficiently calculate the number of nodes at given height in the merkle tree
	unsigned int CalcTreeWidth(int nHeight) {
		return (m_unTransactions + (1 << nHeight) - 1) >> nHeight;
	}
    // the total number of transactions in the block
    unsigned int m_unTransactions;
    // node-is-parent-of-matched-txid bits
    vector<bool> m_vbBits;
    // txids and internal hashes
    vector<uint256> m_vcHash;
    // flag set when encountering invalid data
    bool m_bBad;
};

/** Functions for disk access for blocks */
bool WriteBlockToDisk(CBlock& cBlock, ST_DiskBlockPos& tDiskBlockPos);
bool ReadBlockFromDisk(CBlock& cBlock, const ST_DiskBlockPos& tDiskBlockPos);
bool ReadBlockFromDisk(CBlock& cBlock, const CBlockIndex* pBlockIndex);

/** Functions for validating blocks and updating the block tree */

/** Undo the effects of this block (with given index) on the UTXO set represented by coins.
 *  In case pfClean is provided, operation will try to be tolerant about errors, and *pfClean
 *  will be true if no problems were found. Otherwise, the return value will be false in case
 *  of problems. Note that in any case, coins may be modified. */
bool DisconnectBlock(CBlock& cBlock, CValidationState& cValidationState, CAccountViewCache &cAccountViewCache,
		CBlockIndex* pBlockIndex, CTransactionDBCache &cTxCache, CScriptDBViewCache &cScriptCache,
		bool* pbClean = NULL);

// Apply the effects of this block (with given index) on the UTXO set represented by coins
bool ConnectBlock(CBlock& cBlock, CValidationState& cValidationState, CAccountViewCache &cAccountViewCache,
		CBlockIndex* pBlockIndex, CTransactionDBCache &cTxCache, CScriptDBViewCache &cScriptCache, bool bJustCheck = false);
// Add this block to the block index, and if necessary, switch the active block chain to this
bool AddToBlockIndex(CBlock& cBlock, CValidationState& cValidationState, const ST_DiskBlockPos& tDiskBlockPos);
// Context-independent validity checks
bool CheckBlock(const CBlock& cBlock, CValidationState& cValidationState, CAccountViewCache &cAccountViewCache,
		CScriptDBViewCache &cScriptDBCache, bool bCheckTx = true, bool bCheckMerkleRoot = true);
bool CheckBlockProofWorkWithCoinDay(const CBlock& cBlock, CValidationState& cValidationState);
// Store block on disk
// if dbp is provided, the file is known to already reside on disk
bool AcceptBlock(CBlock& cBlock, CValidationState& cValidationState, ST_DiskBlockPos* pDiskBlockPos = NULL);
//disconnect block for test
bool DisconnectBlockFromTip(CValidationState &cValidationState);
//get tx operate account log
bool GetTxOperLog(const uint256 &cTxHash, vector<CAccountLog> &vcAccountLog);
//get setBlockIndexValid
Value ListSetBlockIndexValid();

class CBlockFileInfo {
 public:
	IMPLEMENT_SERIALIZE(
			READWRITE(VARINT(m_unBlocks));
			READWRITE(VARINT(m_unSize));
			READWRITE(VARINT(m_unUndoSize));
			READWRITE(VARINT(m_unHeightFirst));
			READWRITE(VARINT(m_unHeightLast));
			READWRITE(VARINT(m_ullTimeFirst));
			READWRITE(VARINT(m_ullTimeLast));
	)

	void SetNull() {
		m_unBlocks = 0;
		m_unSize = 0;
		m_unUndoSize = 0;
		m_unHeightFirst = 0;
		m_unHeightLast = 0;
		m_ullTimeFirst = 0;
		m_ullTimeLast = 0;
	}

	CBlockFileInfo() {
		SetNull();
	}

	string ToString() const {
		return strprintf("CBlockFileInfo(blocks=%u, size=%u, heights=%u...%u, time=%s...%s)", m_unBlocks, m_unSize, m_unHeightFirst, m_unHeightLast, DateTimeStrFormat("%Y-%m-%d", m_ullTimeFirst).c_str(), DateTimeStrFormat("%Y-%m-%d", m_ullTimeLast).c_str());
	}

	// update statistics (does not update nSize)
	void AddBlock(unsigned int nHeightIn, uint64_t nTimeIn) {
		if (m_unBlocks==0 || m_unHeightFirst > nHeightIn) {
			m_unHeightFirst = nHeightIn;
		}
		if (m_unBlocks==0 || m_ullTimeFirst > nTimeIn) {
			m_ullTimeFirst = nTimeIn;
		}
		m_unBlocks++;
		if (nHeightIn > m_unHeightLast) {
			m_unHeightLast = nHeightIn;
		}
		if (nTimeIn > m_ullTimeLast) {
			m_ullTimeLast = nTimeIn;
		}
	}

	unsigned int m_unBlocks;     	// number of blocks stored in file
	unsigned int m_unSize;			// number of used bytes of block file
	unsigned int m_unUndoSize;		// number of used bytes in the undo file
	unsigned int m_unHeightFirst;	// lowest height of block in file
	unsigned int m_unHeightLast;	// highest height of block in file
	uint64_t m_ullTimeFirst;		// earliest time of block in file
	uint64_t m_ullTimeLast;			// latest time of block in file
};

enum BlockStatus {
    BLOCK_VALID_UNKNOWN      =    0,
    BLOCK_VALID_HEADER       =    1, // parsed, version ok, hash satisfies claimed PoW, 1 <= vtx count <= max, timestamp not in future
    BLOCK_VALID_TREE         =    2, // parent found, difficulty matches, timestamp >= median previous, checkpoint
    BLOCK_VALID_TRANSACTIONS =    3, // only first tx is coinbase, 2 <= coinbase input script length <= 100, transactions valid, no duplicate txids, sigops, size, merkle root
    BLOCK_VALID_CHAIN        =    4, // outputs do not overspend inputs, no double spends, coinbase output ok, immature coinbase spends, BIP30
    BLOCK_VALID_SCRIPTS      =    5, // scripts/signatures ok                      0000 0101
    BLOCK_VALID_MASK         =    7, //                                            0000 0111

    BLOCK_HAVE_DATA          =    8, // full block available in blk*.dat           0000 1000
    BLOCK_HAVE_UNDO          =   16, // undo data available in rev*.dat            0001 0000
    BLOCK_HAVE_MASK          =   24, //                                            0001 1000

    BLOCK_FAILED_VALID       =   32, // stage after last reached validness failed  0010 0000
    BLOCK_FAILED_CHILD       =   64, // descends from failed block                 0100 0000
    BLOCK_FAILED_MASK        =   96  //                                            0110 0000
};

/** The block chain is a tree shaped structure starting with the
 * genesis block at the root, with each block potentially having multiple
 * candidates to be the next block. A blockindex may have multiple pprev pointing
 * to it, but at most one of them can be part of the currently active branch.
 */
class CBlockIndex {
 public:
	CBlockIndex() {
		m_pHashBlock 		= NULL;
		m_pPrevBlockIndex 	= NULL;
		m_nHeight 			= 0;
		m_nFile 			= 0;
		m_nDataPos 			= 0;
		m_nUndoPos 			= 0;
		m_cChainWork 		= 0;
		m_unTx 				= 0;
		m_unChainTx 		= 0;
		m_unStatus 			= 0;
		m_uSequenceId 		= 0;
		m_dFeePerKb 		= 0.0;
		m_ullBlockfee 		= 0; //add the block's fee

		m_nVersion 			= 0;
		m_cHashMerkleRoot 	= uint256();
		m_cHashPos 			= uint256();
		m_unTime 			= 0;
		m_unBits 			= 0;
		m_unNonce 			= 0;
		m_llFuel 			= 0;
		m_nFuelRate 		= INIT_FUEL_RATES;
		m_vchSignature.clear();
	}

	CBlockIndex(CBlock& block) {
		m_pHashBlock 		= NULL;
		m_pPrevBlockIndex 	= NULL;
		m_nHeight 			= 0;
		m_nFile 			= 0;
		m_nDataPos 			= 0;
		m_nUndoPos 			= 0;
		m_cChainWork 		= 0;
		m_unTx 				= 0;
		m_unChainTx 		= 0;
		m_unStatus 			= 0;
		m_uSequenceId 		= 0;
		m_ullBlockfee 		= block.GetFee(); //add the block's fee

		int64_t llTxSize(0);
		for (auto & pTx : block.vptx) {
			llTxSize += pTx->GetSerializeSize(SER_DISK, g_sProtocolVersion);
		}

		m_dFeePerKb 		= double((m_ullBlockfee - block.GetFuel())) / (double(llTxSize / 1000.0));
		m_nVersion 			= block.GetVersion();
		m_cHashMerkleRoot 	= block.GetHashMerkleRoot();
		m_cHashPos 			= block.GetHashPos();
		m_unTime 			= block.GetTime();
		m_unBits 			= block.GetBits();
		m_unNonce 			= block.GetNonce();
		m_llFuel 			= block.GetFuel();
		m_nFuelRate 		= block.GetFuelRate();
		m_vchSignature 	= block.GetSignature();
	}

	ST_DiskBlockPos GetBlockPos() const {
		ST_DiskBlockPos ret;
		if (m_unStatus & BLOCK_HAVE_DATA) {
			ret.nFile = m_nFile;
			ret.unPos = m_nDataPos;
		}
		return ret;
	}

	ST_DiskBlockPos GetUndoPos() const {
		ST_DiskBlockPos ret;
		if (m_unStatus & BLOCK_HAVE_UNDO) {
			ret.nFile = m_nFile;
			ret.unPos = m_nUndoPos;
		}
		return ret;
	}

	CBlockHeader GetBlockHeader() const {
		CBlockHeader cBlock;
		cBlock.SetVersion(m_nVersion);
		if (m_pPrevBlockIndex) {
			cBlock.SetHashPrevBlock(m_pPrevBlockIndex->GetBlockHash());
		}
		cBlock.SetHashMerkleRoot(m_cHashMerkleRoot);
		cBlock.SetHashPos(m_cHashPos);
		cBlock.SetTime(m_unTime);
		cBlock.SetBits(m_unBits);
		cBlock.SetNonce(m_unNonce);
		cBlock.SetHeight(m_nHeight);
		cBlock.SetSignature(m_vchSignature);
		return cBlock;
	}

	int64_t GetBlockFee() const {
		return m_ullBlockfee;
	}

	uint256 GetBlockHash() const {
		return *m_pHashBlock;
	}

	int64_t GetBlockTime() const {
		return (int64_t) m_unTime;
	}

	CBigNum GetBlockWork() const {
		CBigNum bnTarget;
		bnTarget.SetCompact(m_unBits);
		if (bnTarget <= 0)
			return 0;
		return (CBigNum(1) << 256) / (bnTarget + 1);
	}

	bool CheckIndex() const {
		return CheckProofOfWork(GetBlockHash(), m_unBits);
	}

	enum {
		nMedianTimeSpan = 11
	};

	int64_t GetMedianTimePast() const {
		int64_t llMedian[nMedianTimeSpan];
		int64_t* pBegin = &llMedian[nMedianTimeSpan];
		int64_t* pEnd = &llMedian[nMedianTimeSpan];

		const CBlockIndex* pIndex = this;
		for (int i = 0; i < nMedianTimeSpan && pIndex; i++, pIndex = pIndex->m_pPrevBlockIndex) {
			*(--pBegin) = pIndex->GetBlockTime();
		}
		sort(pBegin, pEnd);
		return pBegin[(pEnd - pBegin) / 2];
	}

	int64_t GetMedianTime() const;

	/**
	 * Returns true if there are nRequired or more blocks of minVersion or above
	 * in the last nToCheck blocks, starting at pstart and going backwards.
	 */
	static bool IsSuperMajority(int nMinVersion, const CBlockIndex* pStartBlockIndex, unsigned int unRequired,
			unsigned int unToCheck);

	string ToString() const {
		return strprintf("CBlockIndex(pprev=%p, nHeight=%d, merkle=%s, hashBlock=%s, blockfee=%d, chainWork=%s, feePerKb=%lf)",
		m_pPrevBlockIndex, m_nHeight, m_cHashMerkleRoot.ToString().c_str(), GetBlockHash().ToString().c_str(), m_ullBlockfee, m_cChainWork.ToString().c_str(), m_dFeePerKb);
	}

	void print() const 	{
		LogPrint("INFO","%s\n", ToString().c_str());
	}

	// pointer to the hash of the block, if any. memory is owned by this CBlockIndex
	const uint256* m_pHashBlock;

	// pointer to the index of the predecessor of this block
	CBlockIndex* m_pPrevBlockIndex;

	// height of the entry in the chain. The genesis block has height 0
	int m_nHeight;

	// Which # file this block is stored in (blk?????.dat)
	int m_nFile;

	// Byte offset within blk?????.dat where this block's data is stored
	unsigned int m_nDataPos;

	// Byte offset within rev?????.dat where this block's undo data is stored
	unsigned int m_nUndoPos;

	// (memory only) Total amount of work (expected number of hashes) in the chain up to and including this block
	arith_uint256 m_cChainWork;

	// Number of transactions in this block.
	// Note: in a potential headers-first mode, this number cannot be relied upon
	unsigned int m_unTx;
	// (memory only) Number of transactions in the chain up to and including this block
	unsigned int m_unChainTx;// change to 64-bit type when necessary; won't happen before 2030
	// Verification status of this block. See enum BlockStatus
	unsigned int m_unStatus;
	//the block's fee
	uint64_t m_ullBlockfee;
	// block header
	int m_nVersion;
	uint256 m_cHashMerkleRoot;
	uint256 m_cHashPos;
	unsigned int m_unTime;
	unsigned int m_unBits;
	unsigned int m_unNonce;
	int64_t m_llFuel;
	int m_nFuelRate;
	vector<unsigned char> m_vchSignature;
	double m_dFeePerKb;
	// (memory only) Sequencial id assigned to distinguish order in which blocks are received.
	uint32_t m_uSequenceId;
};


/** Used to marshal pointers into hashes for db storage. */
class CDiskBlockIndex: public CBlockIndex {
 public:
	CDiskBlockIndex() : m_cHashPrev(uint256()) {
	}

	explicit CDiskBlockIndex(CBlockIndex* pindex) : CBlockIndex(*pindex) {
		m_cHashPrev = (m_pPrevBlockIndex ? m_pPrevBlockIndex->GetBlockHash() : uint256());
	}

	IMPLEMENT_SERIALIZE
	(
		if (!(nType & SER_GETHASH))
		READWRITE(VARINT(nVersion));

		READWRITE(m_ullBlockfee);
		READWRITE(VARINT(m_nHeight));
		READWRITE(VARINT(m_unStatus));
		READWRITE(VARINT(m_unTx));
		if (m_unStatus & (BLOCK_HAVE_DATA | BLOCK_HAVE_UNDO))
		READWRITE(VARINT(m_nFile));
		if (m_unStatus & BLOCK_HAVE_DATA)
		READWRITE(VARINT(m_nDataPos));
		if (m_unStatus & BLOCK_HAVE_UNDO)
		READWRITE(VARINT(m_nUndoPos));

		// block header
		READWRITE(this->m_nVersion);
		READWRITE(m_cHashPrev);
		READWRITE(m_cHashMerkleRoot);
		READWRITE(m_cHashPos);
		READWRITE(m_unTime);
		READWRITE(m_unBits);
		READWRITE(m_unNonce);
		READWRITE(m_llFuel);
		READWRITE(m_nFuelRate);
		READWRITE(m_vchSignature);
		READWRITE(m_dFeePerKb);
	)

	uint256 GetBlockHash() const {
		CBlockHeader cBlock;
		cBlock.SetVersion(m_nVersion);
		cBlock.SetHashPrevBlock(m_cHashPrev);
		cBlock.SetHashMerkleRoot(m_cHashMerkleRoot);
		cBlock.SetHashPos(m_cHashPos);
		cBlock.SetTime(m_unTime);
		cBlock.SetBits(m_unBits);
		cBlock.SetNonce(m_unNonce);
		cBlock.SetHeight(m_nHeight);
		cBlock.SetFuel(m_llFuel);
		cBlock.SetFuelRate(m_nFuelRate);
		cBlock.SetSignature(m_vchSignature);

		return cBlock.GetHash();
	}

	string ToString() const {
		string str = "CDiskBlockIndex(";
		str += CBlockIndex::ToString();
		str += strprintf("\n                hashBlock=%s, m_cHashPrev=%s)",
		GetBlockHash().ToString().c_str(),
		m_cHashPrev.ToString().c_str());
		return str;
	}

	void print() const {
		LogPrint("INFO", "%s\n", ToString().c_str());
	}

	uint256 m_cHashPrev;
};

/** Capture information about block/transaction validation */
class CValidationState {
 public:
	CValidationState() : m_emMode(EM_MODE_VALID), m_nDoS(0), m_bCorruptionPossible(false) {
	}
    bool DoS(int nLevel, bool bRet = false,
             unsigned char chRejectCodeIn = 0, string strRejectReasonIn = "",
             bool bCorruptionIn = false) {
        m_chRejectCode 			= chRejectCodeIn;
        m_strRejectReason 		= strRejectReasonIn;
        m_bCorruptionPossible 	= bCorruptionIn;
        if (m_emMode == EM_MODE_ERROR) {
        	return bRet;
        }
        m_nDoS += nLevel;
        m_emMode = EM_MODE_INVALID;
        return bRet;
    }

    bool Invalid(bool bRet = false, unsigned char _chRejectCode=0, string _strRejectReason="") {
        return DoS(0, bRet, _chRejectCode, _strRejectReason);
    }

    bool Error(string strRejectReasonIn = "") {
        if (m_emMode == EM_MODE_VALID) {
        	m_strRejectReason = strRejectReasonIn;
        }
        m_emMode = EM_MODE_ERROR;
        return false;
    }

    bool Abort(const string &msg) {
        AbortNode(msg);
        return Error(msg);
    }

    bool IsValid() const {
        return m_emMode == EM_MODE_VALID;
    }

    bool IsInvalid() const {
        return m_emMode == EM_MODE_INVALID;
    }

    bool IsError() const {
        return m_emMode == EM_MODE_ERROR;
    }

    bool IsInvalid(int &nDoSOut) const {
        if (IsInvalid()) {
            nDoSOut = m_nDoS;
            return true;
        }
        return false;
    }

    bool CorruptionPossible() const {
        return m_bCorruptionPossible;
    }

    unsigned char GetRejectCode() const {
		return m_chRejectCode;
	}
	string GetRejectReason() const {
		return m_strRejectReason;
	}

 private:
     enum emModeState {
         EM_MODE_VALID,   // everything ok
         EM_MODE_INVALID, // network rule violation (DoS value may be set)
         EM_MODE_ERROR,   // run-time error
     } m_emMode;

     int m_nDoS;
     string m_strRejectReason;
     unsigned char m_chRejectCode;
     bool m_bCorruptionPossible;
};

/** An in-memory indexed chain of blocks. */
class CChain {
 public:
    /** Returns the index entry for the genesis block of this chain, or NULL if none. */
    CBlockIndex *Genesis() const {
        return m_vcChain.size() > 0 ? m_vcChain[0] : NULL;
    }

    /** Returns the index entry for the tip of this chain, or NULL if none. */
    CBlockIndex *Tip() const {
        return m_vcChain.size() > 0 ? m_vcChain[m_vcChain.size() - 1] : NULL;
    }

    /** Returns the index entry at a particular height in this chain, or NULL if no such height exists. */
    CBlockIndex *operator[](int nHeight) const {
        if (nHeight < 0 || nHeight >= (int)m_vcChain.size()) {
        	 return NULL;
        }
        return m_vcChain[nHeight];
    }

    /** Compare two chains efficiently. */
    friend bool operator==(const CChain &a, const CChain &b) {
        return a.m_vcChain.size() == b.m_vcChain.size() &&
               a.m_vcChain[a.m_vcChain.size() - 1] == b.m_vcChain[b.m_vcChain.size() - 1];
    }

    /** Efficiently check whether a block is present in this chain. */
    bool Contains(const CBlockIndex *pIndex) const {
        return (*this)[pIndex->m_nHeight] == pIndex;
    }

    /** Find the successor of a block in this chain, or NULL if the given index is not found or is the tip. */
    CBlockIndex *Next(const CBlockIndex *pIndex) const {
        if (Contains(pIndex)) {
        	return (*this)[pIndex->m_nHeight + 1];
        } else {
        	return NULL;
        }
    }

    /** Return the maximal height in the chain. Is equal to chain.Tip() ? chain.Tip()->nHeight : -1. */
    int Height() const {
        return m_vcChain.size() - 1;
    }

    /** Set/initialize a chain with a given tip. Returns the forking point. */
    CBlockIndex *SetTip(CBlockIndex *pIndex);

    /** Return a CBlockLocator that refers to a block in this chain (by default the tip). */
    ST_BlockLocator GetLocator(const CBlockIndex *pIndex = NULL) const;

    /** Find the last common block between this chain and a locator. */
    CBlockIndex *FindFork(const ST_BlockLocator &tLocator) const;

 private:
     vector<CBlockIndex*> m_vcChain;
};

/** The currently-connected chain of blocks. */
extern CChain g_cChainActive;

/** The currently best known chain of headers (some of which may be invalid). */
extern CChain g_cChainMostWork;

/** Global variable that points to the active block tree (protected by cs_main) */
extern CBlockTreeDB *g_pblocktree;

/** account db cache*/
extern CAccountViewCache *g_pAccountViewTip;

/** account db */
extern CAccountViewDB *g_pAccountViewDB;

/** transaction db cache*/
extern CTransactionDB *g_pTxCacheDB;

/** srcipt db */
extern CScriptDB *g_pScriptDB;

/** tx db cache */
extern CTransactionDBCache *g_pTxCacheTip;

/** contract script db cache */
extern CScriptDBViewCache *g_pScriptDBTip;

/** nSyncTipHight  */
extern int g_nSyncTipHeight;

extern std::tuple<bool, boost::thread*> RunDacrs(int argc, char* argv[]);
extern bool WriteBlockLog(bool bFalg, string strSuffix);
//extern set<uint256> setTxHashCache;
//extern map<uint256, set<uint256> > mapTxHashCacheByPrev;

//extern map<string, CContractScript> mapScript;

struct ST_BlockTemplate {
	CBlock cBlock;
	vector<int64_t> vTxFees;
	vector<int64_t> vTxSigOps;
};

bool EraseBlockIndexFromSet(CBlockIndex *pIndex);

/** Used to relay blocks as header + vector<merkle branch>
 * to filtered nodes.
 */
class CMerkleBlock {
 public:
	// Public only for unit testing and relay testing
	// (not relayed)
	vector<pair<unsigned int, uint256> > m_vMatchedTxn;

	// Create from a CBlock, filtering transactions according to filter
	// Note that this will call IsRelevantAndUpdate on the filter for each transaction,
	// thus the filter will likely be modified.
	CMerkleBlock(const CBlock& cBlock, CBloomFilter& cBloomFilter);

	IMPLEMENT_SERIALIZE
	(
			READWRITE(m_cBlockHeader);
			READWRITE(m_cTxNum);
	)

 public:
 	// Public only for unit testing
 	CBlockHeader m_cBlockHeader;
 	CPartialMerkleTree m_cTxNum;
};


class CWalletInterface {
 protected:
    virtual void SyncTransaction(const uint256 &cHash, CBaseTransaction *pBaseTx, const CBlock *pBlock) =0;
    virtual void EraseFromWallet(const uint256 &cHash) =0;
    virtual void SetBestChain(const ST_BlockLocator &tLocator) =0;
    virtual void UpdatedTransaction(const uint256 &cHash) =0;
    virtual void ResendWalletTransactions() =0;
    friend void ::RegisterWallet(CWalletInterface*);
    friend void ::UnregisterWallet(CWalletInterface*);
    friend void ::UnregisterAllWallets();
};

class CSignatureCache {
 public:
	bool Get(const uint256 &cHash, const std::vector<unsigned char>& vchSig, const CPubKey& cPubKey) {
		boost::shared_lock<boost::shared_mutex> lock(m_cs_SigCache);

		m_SigDataType k(cHash, vchSig, cPubKey);
		std::set<m_SigDataType>::iterator mi = m_setValid.find(k);
		if (mi != m_setValid.end()) {
			return true;
		}
		return false;
	}

	void Set(const uint256 &cHash, const std::vector<unsigned char>& vchSig, const CPubKey& cPubKey) {
		// DoS prevention: limit cache size to less than 10MB
		// (~200 bytes per cache entry times 50,000 entries)
		// Since there are a maximum of 20,000 signature operations per block
		// 50,000 is a reasonable default.
		int64_t llMaxCacheSize = SysCfg().GetArg("-maxsigcachesize", 50000);
		if (llMaxCacheSize <= 0) {
			return;
		}

		boost::unique_lock<boost::shared_mutex> lock(m_cs_SigCache);

		while (static_cast<int64_t>(m_setValid.size()) > llMaxCacheSize) {
			// Evict a random entry. Random because that helps
			// foil would-be DoS attackers who might try to pre-generate
			// and re-use a set of valid signatures just-slightly-greater
			// than our cache size.
			uint256 cRandomHash = GetRandHash();
			std::vector<unsigned char> unUsed;
			std::set<m_SigDataType>::iterator it = m_setValid.lower_bound(m_SigDataType(cRandomHash, unUsed, unUsed));
			if (it == m_setValid.end()) {
				it = m_setValid.begin();
			}
			m_setValid.erase(*it);
		}

		m_SigDataType k(cHash, vchSig, cPubKey);
		m_setValid.insert(k);
	}

 private:
	// sigdata_type is (signature hash, signature, public key):
	typedef std::tuple<uint256, std::vector<unsigned char>, CPubKey> m_SigDataType;
	std::set<m_SigDataType> m_setValid;
	boost::shared_mutex m_cs_SigCache;
};

extern CSignatureCache g_cSignatureCache;

#endif
