// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "miner.h"

#include "core.h"
#include "main.h"
#include "tx.h"
#include "net.h"
#ifdef ENABLE_WALLET
#include "wallet.h"
#endif
//////////////////////////////////////////////////////////////////////////////
//
// BitcoinMiner
//

int static FormatHashBlocks(void* pbuffer, unsigned int len) {
	unsigned char* pdata = (unsigned char*) pbuffer;
	unsigned int blocks = 1 + ((len + 8) / 64);
	unsigned char* pend = pdata + 64 * blocks;
	memset(pdata + len, 0, 64 * blocks - len);
	pdata[len] = 0x80;
	unsigned int bits = len * 8;
	pend[-1] = (bits >> 0) & 0xff;
	pend[-2] = (bits >> 8) & 0xff;
	pend[-3] = (bits >> 16) & 0xff;
	pend[-4] = (bits >> 24) & 0xff;
	return blocks;
}

static const unsigned int pSHA256InitState[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f,
		0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };

void SHA256Transform(void* pstate, void* pinput, const void* pinit) {
	SHA256_CTX ctx;
	unsigned char data[64];

	SHA256_Init(&ctx);

	for (int i = 0; i < 16; i++)
		((uint32_t*) data)[i] = ByteReverse(((uint32_t*) pinput)[i]);

	for (int i = 0; i < 8; i++)
		ctx.h[i] = ((uint32_t*) pinit)[i];

	SHA256_Update(&ctx, data, sizeof(data));
	for (int i = 0; i < 8; i++)
		((uint32_t*) pstate)[i] = ctx.h[i];
}

// Some explaining would be appreciated
class COrphan {
public:
	const CTransaction* ptx;
	set<uint256> setDependsOn;
	double dPriority;
	double dFeePerKb;

	COrphan(const CTransaction* ptxIn) {
		ptx = ptxIn;
		dPriority = dFeePerKb = 0;
	}

	void print() const {
		LogPrint("INFO","COrphan(hash=%s, dPriority=%.1f, dFeePerKb=%.1f)\n", ptx->GetHash().ToString(), dPriority,
				dFeePerKb);
		for (const auto& hash : setDependsOn)
			LogPrint("INFO", "   setDependsOn %s\n", hash.ToString());
	}
};

uint64_t nLastBlockTx = 0;
uint64_t nLastBlockSize = 0;

// We want to sort transactions by priority and fee, so:

void GetPriorityTx(vector<TxPriority> &vecPriority, map<uint256, vector<COrphan*> > &mapDependers) {
	vecPriority.reserve(mempool.mapTx.size());
	CBlockIndex* pindexPrev = chainActive.Tip();

	// Priority order to process transactions
	list<COrphan> vOrphan; // list memory doesn't move
	double dPriority = 0;
	for (map<uint256, CTxMemPoolEntry>::iterator mi = mempool.mapTx.begin(); mi != mempool.mapTx.end(); ++mi) {
		int nTxHeight = mi->second.GetHeight();
		CBaseTransaction *pBaseTx = mi->second.GetTx().get();

		if (!pTxCacheTip->IsContainTx(pBaseTx->GetHash())) {
			if (pBaseTx->nTxType == APPEAL_TX) {
				const CAppealTransaction *patx = (const CAppealTransaction *) pBaseTx;
				if (!pTxCacheTip->IsContainTx(patx->preTxHash)) {
					continue;
				}
			} else {

			}
			unsigned int nTxSize = ::GetSerializeSize(pBaseTx->GetNewInstance(), SER_NETWORK, PROTOCOL_VERSION);
			double dFeePerKb = double(pBaseTx->GetFee()) / (double(nTxSize) / 1000.0);
			dPriority = 1000.0 / double(nTxSize);
			vecPriority.push_back(TxPriority(dPriority, dFeePerKb, mi->second.GetTx()));
		}

	}
}

CBlockTemplate* CreateNewBlock(const CScript& scriptPubKeyIn) {
//    // Create new block
	auto_ptr<CBlockTemplate> pblocktemplate(new CBlockTemplate());
	if (!pblocktemplate.get())
		return NULL;
//    CBlock *pblock = &pblocktemplate->block; // pointer for convenience
//
//    // Create coinbase tx
//    CTransaction txNew;
//    txNew.vin.resize(1);
//    txNew.vin[0].prevout.SetNull();
//    txNew.vout.resize(1);
//    txNew.vout[0].scriptPubKey = scriptPubKeyIn;
//
//    // Add our coinbase tx as first transaction
//    pblock->vtx.push_back(txNew);
//    pblocktemplate->vTxFees.push_back(-1); // updated at end
//    pblocktemplate->vTxSigOps.push_back(-1); // updated at end
//
//    // Largest block you're willing to create:
//    unsigned int nBlockMaxSize = GetArg("-blockmaxsize", DEFAULT_BLOCK_MAX_SIZE);
//    // Limit to betweeen 1K and MAX_BLOCK_SIZE-1K for sanity:
//    nBlockMaxSize = max((unsigned int)1000, min((unsigned int)(MAX_BLOCK_SIZE-1000), nBlockMaxSize));
//
//    // How much of the block should be dedicated to high-priority transactions,
//    // included regardless of the fees they pay
//    unsigned int nBlockPrioritySize = GetArg("-blockprioritysize", DEFAULT_BLOCK_PRIORITY_SIZE);
//    nBlockPrioritySize = min(nBlockMaxSize, nBlockPrioritySize);
//
//    // Minimum block size you want to create; block will be filled with free transactions
//    // until there are no more or the block reaches this size:
//    unsigned int nBlockMinSize = GetArg("-blockminsize", DEFAULT_BLOCK_MIN_SIZE);
//    nBlockMinSize = min(nBlockMaxSize, nBlockMinSize);
//
//    // Collect memory pool transactions into the block
//    int64_t nFees = 0;
//    {
//        LOCK2(cs_main, mempool.cs);
//        CBlockIndex* pindexPrev = chainActive.Tip();
//        CCoinsViewCache view(*pcoinsTip, true);
//
//
//        bool fPrintPriority = GetBoolArg("-printpriority", false);
//
//        // This vector will be sorted into a priority queue:
//        vector<TxPriority> vecPriority;
//    	map<uint256, vector<COrphan*> > mapDependers;
//        GetPriorityTx(vecPriority, mapDependers);
//
//        // Collect transactions into block
//        uint64_t nBlockSize = 1000;
//        uint64_t nBlockTx = 0;
//        int nBlockSigOps = 100;
//        bool fSortedByFee = true;
//
//        TxPriorityCompare comparer(fSortedByFee);
//        make_heap(vecPriority.begin(), vecPriority.end(), comparer);
//
//        while (!vecPriority.empty())
//        {
//            // Take highest priority transaction off the priority queue:
//            double dPriority = vecPriority.front().get<0>();
//            double dFeePerKb = vecPriority.front().get<1>();
//            const CBaseTransaction *pBaseTx = vecPriority.front().get<2>();
//            //const CTransaction& tx = *(vecPriority.front().get<2>());
//
//            pop_heap(vecPriority.begin(), vecPriority.end(), comparer);
//            vecPriority.pop_back();
//
//            // Size limits
//            unsigned int nTxSize = pBaseTx->GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION);
//            if (nBlockSize + nTxSize >= nBlockMaxSize)
//                continue;
//
//            // Skip free transactions if we're past the minimum block size:
//			if (fSortedByFee && (dFeePerKb < CTransaction::nMinRelayTxFee) && (nBlockSize + nTxSize >= nBlockMinSize))
//				continue;
//
//			// Prioritize by fee once past the priority size or we run out of high-priority
//			// transactions:
//			if (!fSortedByFee &&
//			   ((nBlockSize + nTxSize >= nBlockPrioritySize) || !AllowFree(dPriority)))
//			{
//				fSortedByFee = true;
//				comparer = TxPriorityCompare(fSortedByFee);
//				make_heap(vecPriority.begin(), vecPriority.end(), comparer);
//			}
//
//            if(NORMAL_TX == pBaseTx->nTxType || GENTOSECURE_TX == pBaseTx->nTxType)
//            {
//            	const CTransaction& tx = *((CTransaction *)vecPriority.front().get<2>());
//            	// Legacy limits on sigOps:
//				unsigned int nTxSigOps = GetLegacySigOpCount(tx);
//				if (nBlockSigOps + nTxSigOps >= MAX_BLOCK_SIGOPS)
//					continue;
//
//				if (!view.HaveInputs(tx))
//					continue;
//
//				int64_t nTxFees = view.GetValueIn(tx)-tx.GetValueOut();
//
//				nTxSigOps += GetP2SHSigOpCount(tx, view);
//				if (nBlockSigOps + nTxSigOps >= MAX_BLOCK_SIGOPS)
//					continue;
//
//				CValidationState state;
//				if (!CheckInputs(tx, state, view, true, SCRIPT_VERIFY_P2SH))
//					continue;
//
//				CTxUndo txundo;
//				uint256 hash = tx.GetHash();
//				UpdateCoins(tx, state, view, txundo, pindexPrev->nHeight+1, hash);
//
//				// Added
//				pblock->vtx.push_back(tx);
//				pblocktemplate->vTxFees.push_back(nTxFees);
//				pblocktemplate->vTxSigOps.push_back(nTxSigOps);
//				nBlockSize += nTxSize;
//				++nBlockTx;
//				nBlockSigOps += nTxSigOps;
//				nFees += nTxFees;
//
//				if (fPrintPriority)
//				{
//					LogPrint("INFO","priority %.1f feeperkb %.1f txid %s\n",
//						   dPriority, dFeePerKb, tx.GetHash().ToString());
//				}
//
//				// Add transactions that depend on this one to the priority queue
//				if (mapDependers.count(hash))
//				{
//					BOOST_FOREACH(COrphan* porphan, mapDependers[hash])
//					{
//						if (!porphan->setDependsOn.empty())
//						{
//							porphan->setDependsOn.erase(hash);
//							if (porphan->setDependsOn.empty())
//							{
//								vecPriority.push_back(TxPriority(porphan->dPriority, porphan->dFeePerKb, porphan->ptx));
//								push_heap(vecPriority.begin(), vecPriority.end(), comparer);
//							}
//						}
//					}
//				}
//            }
//            else if(APPEAL_TX == pBaseTx->nTxType) {
//            	pblock->vAppealTx.push_back(*(CAppealTransaction *)pBaseTx);
//            }
//            else if(SECURE_TX == pBaseTx->nTxType) {
//            	pblock->vSecureTx.push_back(*(CSecureTransaction *)pBaseTx);
//            }
//            else if(SECURETOGEN_TX == pBaseTx->nTxType) {
//            	pblock->vSecureToGenTx.push_back(*(CSecureToGeneralTx *)pBaseTx);
//            }
//            else if(FREEZE_TX == pBaseTx->nTxType) {
//            	pblock->vFreezeTx.push_back(*(CFreezeTransaction *)pBaseTx);
//            }
//        }
//
//        nLastBlockTx = nBlockTx;
//        nLastBlockSize = nBlockSize;
//        LogPrint("INFO","CreateNewBlock(): total size %u\n", nBlockSize);
//
//        pblock->vtx[0].vout[0].nValue = GetBlockValue(pindexPrev->nHeight+1, nFees);
//        pblocktemplate->vTxFees[0] = -nFees;
//
//        // Fill in header
//        pblock->hashPrevBlock  = pindexPrev->GetBlockHash();
//        UpdateTime(*pblock, pindexPrev);
//        pblock->nBits          = GetNextWorkRequired(pindexPrev, pblock);
//        pblock->nNonce         = 0;
//        pblock->vtx[0].vin[0].scriptSig = CScript() << OP_0 << OP_0;
//        pblocktemplate->vTxSigOps[0] = GetLegacySigOpCount(pblock->vtx[0]);
//
//        CBlockIndex indexDummy(*pblock);
//        indexDummy.pprev = pindexPrev;
//        indexDummy.nHeight = pindexPrev->nHeight + 1;
//        CCoinsViewCache viewNew(*pcoinsTip, true);
//        CValidationState state;
//        if (!ConnectBlock(*pblock, state, &indexDummy, viewNew, true))
//            throw runtime_error("CreateNewBlock() : ConnectBlock failed");
//    }

	return pblocktemplate.release();
}

void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce) {
	// Update nExtraNonce
	static uint256 hashPrevBlock;
	if (hashPrevBlock != pblock->hashPrevBlock) {
		nExtraNonce = 0;
		hashPrevBlock = pblock->hashPrevBlock;
	}
	++nExtraNonce;
	unsigned int nHeight = pindexPrev->nHeight + 1; // Height first in coinbase required for block.version=2
//    pblock->vtx[0].vin[0].scriptSig = (CScript() << nHeight << CBigNum(nExtraNonce)) + COINBASE_FLAGS;
//    assert(pblock->vtx[0].vin[0].scriptSig.size() <= 100);

	pblock->hashMerkleRoot = pblock->BuildMerkleTree();
}

void FormatHashBuffers(CBlock* pblock, char* pmidstate, char* pdata, char* phash1) {
	//
	// Pre-build hash buffers
	//
	struct {
		struct unnamed2 {
			int nVersion;
			uint256 hashPrevBlock;
			uint256 hashMerkleRoot;
			unsigned int nTime;
			unsigned int nBits;
			unsigned int nNonce;
		} block;
		unsigned char pchPadding0[64];
		uint256 hash1;
		unsigned char pchPadding1[64];
	} tmp;
	memset(&tmp, 0, sizeof(tmp));

	tmp.block.nVersion = pblock->nVersion;
	tmp.block.hashPrevBlock = pblock->hashPrevBlock;
	tmp.block.hashMerkleRoot = pblock->hashMerkleRoot;
	tmp.block.nTime = pblock->nTime;
	tmp.block.nBits = pblock->nBits;
	tmp.block.nNonce = pblock->nNonce;

	FormatHashBlocks(&tmp.block, sizeof(tmp.block));
	FormatHashBlocks(&tmp.hash1, sizeof(tmp.hash1));

	// Byte swap all the input buffer
	for (unsigned int i = 0; i < sizeof(tmp) / 4; i++)
		((unsigned int*) &tmp)[i] = ByteReverse(((unsigned int*) &tmp)[i]);

	// Precalc the first half of the first hash, which stays constant
	SHA256Transform(pmidstate, &tmp.block, pSHA256InitState);

	memcpy(pdata, &tmp.block, 128);
	memcpy(phash1, &tmp.hash1, 64);
}

#ifdef ENABLE_WALLET
//////////////////////////////////////////////////////////////////////////////
//
// Internal miner
//
double dHashesPerSec = 0.0;
int64_t nHPSTimerStart = 0;

//
// ScanHash scans nonces looking for a hash with at least some zero bits.
// It operates on big endian data.  Caller does the byte reversing.
// All input buffers are 16-byte aligned.  nNonce is usually preserved
// between calls, but periodically or if nNonce is 0xffff0000 or above,
// the block is rebuilt and nNonce starts over at zero.
//
unsigned int static ScanHash_CryptoPP(char* pmidstate, char* pdata, char* phash1, char* phash,
		unsigned int& nHashesDone) {
	unsigned int& nNonce = *(unsigned int*) (pdata + 12);
	for (;;) {
		// Crypto++ SHA256
		// Hash pdata using pmidstate as the starting state into
		// pre-formatted buffer phash1, then hash phash1 into phash
		nNonce++;
		SHA256Transform(phash1, pdata, pmidstate);
		SHA256Transform(phash, phash1, pSHA256InitState);

		// Return the nonce if the hash has at least some zero bits,
		// caller will check if it has enough to reach the target
		if (((unsigned short*) phash)[14] == 0)
			return nNonce;

		// If nothing found after trying for a while, return -1
		if ((nNonce & 0xffff) == 0) {
			nHashesDone = 0xffff + 1;
			return (unsigned int) -1;
		}
		if ((nNonce & 0xfff) == 0)
			boost::this_thread::interruption_point();
	}
}

CBlockTemplate* CreateNewBlockWithKey(CReserveKey& reservekey) {
	CPubKey pubkey;
	if (!reservekey.GetReservedKey(pubkey))
		return NULL;

	CScript scriptPubKey = CScript() << pubkey << OP_CHECKSIG;
	return CreateNewBlock(scriptPubKey);
}

struct CSecureAccComparator {
	bool operator()(const CSecureAccount &a, const CSecureAccount&b) {
		// First sort by acc over 30days
		if (a.GetSecureAccPos(chainActive.Tip()->nHeight) < b.GetSecureAccPos(chainActive.Tip()->nHeight)) {
			return false;
		}

		return true;
	}
};


struct PosTxInfo {
	int nVersion;
	uint256 hashPrevBlock;
	uint256 hashMerkleRoot;
	uint64_t nValues;
	unsigned int nTime;
	unsigned int nNonce;
	uint256 GetHash()
	{
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		ds<<nVersion;
		ds<<hashPrevBlock;
		ds<<hashMerkleRoot;
		ds<<nValues;
		ds<<nTime;
		ds<<nNonce;
		return Hash(ds.begin(),ds.end());
	}
};


uint256 GetAdjustHash(const uint256 TargetHash, const uint64_t nPos) {

	uint64_t posacc = nPos/COIN;
	posacc = posacc / 100;
	posacc = max(posacc, (uint64_t) 1);
	uint256 adjusthash = TargetHash; //adjust nbits
	uint256 minhash = Params().ProofOfWorkLimit().getuint256();

	while (posacc) {
		adjusthash = adjusthash << 1;
		posacc = posacc >> 1;
		if (adjusthash > minhash) {
			adjusthash = minhash;
			break;
		}
	}

	return adjusthash;
}
extern CWallet* pwalletMain;
bool CreatePosTx(const CBlockIndex *pPrevIndex, CBlock *pBlock,set<CKeyID>&setCreateKey) {
	set<CKeyID> setKeyID;
	CSecureAccount secureAcc;
	set<CSecureAccount, CSecureAccComparator> setSecureAcc;

	{
		LOCK2(cs_main, pwalletMain->cs_wallet);
		pwalletMain->GetKeys(setKeyID); //get addrs
		if (setKeyID.empty()) {
			LogPrint("INFO","CreatePosTx setKeyID empty\n");
			return false;
		}

		CAccountViewCache accView(*pAccountViewTip, true);
		CTransactionCache txCacheTemp(*pTxCacheTip);
		CContractScriptCache contractScriptTemp(*pContractScriptTip);
		{
			for (unsigned int i = 1; i < pBlock->vptx.size(); i++) {
				shared_ptr<CBaseTransaction> pBaseTx = pBlock->vptx[i];
				if (pTxCacheTip->IsContainTx(pBaseTx->GetHash())) {
					LogPrint("INFO","CreatePosTx duplicate tx\n");
					return false;
				}
				CTxUndo txundo;
				CValidationState state;
				if (!pBaseTx->UpdateAccount(i, accView, state, txundo, pPrevIndex->nHeight + 1, txCacheTemp, contractScriptTemp)) {
					LogPrint("INFO","CreatePosTx duplicate tx\n");
					return false;
				}
			}
		}

		for(const auto &keyid:setKeyID) {
			//find CSecureAccount info by keyid
			if(setCreateKey.size()) {
				bool bfind = false;
				for(auto &item: setCreateKey)
				{
					if(item == keyid)
					{
						bfind = true;
						break;
					}
				}
				if(!bfind) continue;
			}
			if (accView.GetAccount(keyid, secureAcc)) {
				//available
				if (secureAcc.IsRegister() && secureAcc.GetSecureAccPos(pPrevIndex->nHeight) > 0) {
					setSecureAcc.insert(secureAcc);
				}
			}
		}
	}

	if (setSecureAcc.empty()) {
		setCreateKey.clear();
		LogPrint("INFO","CreatePosTx setSecureAcc empty\n");
		return false;
	}

	uint64_t maxNonce = CBaseParams::GetArg("-blockmaxnonce", 10000); //cacul times

	uint256 prevblockhash = pPrevIndex->GetBlockHash();
	const uint256 targetHash = CBigNum().SetCompact(pBlock->nBits).getuint256(); //target hash difficult

	for(const auto &item: setSecureAcc) {

		uint64_t posacc = item.GetSecureAccPos(pPrevIndex->nHeight);
		if (posacc == 0) //have no pos
				{
			LogPrint("INFO","CreatePosTx posacc zero\n");
			continue;
		}

		uint256 adjusthash = GetAdjustHash(targetHash, posacc); //adjust nbits

//need compute this block proofofwork
		struct PosTxInfo postxinfo;
		postxinfo.nVersion = pPrevIndex->nVersion;
		postxinfo.hashPrevBlock = prevblockhash;
		postxinfo.hashMerkleRoot = item.BuildMerkleTree(pPrevIndex->nHeight);
		postxinfo.nValues = item.llValues;
		postxinfo.nTime = pBlock->nTime; //max(pPrevIndex->GetMedianTimePast() + 1, GetAdjustedTime());
		for (pBlock->nNonce = 0; pBlock->nNonce < maxNonce; pBlock->nNonce++) {
			postxinfo.nNonce = pBlock->nNonce;
			uint256 curhash = postxinfo.GetHash();

			if (curhash <= adjusthash) {
//				string str("");
//				for (char *pCh = BEGIN(postxinfo.nVersion); pCh != END(postxinfo.nVersion); ++pCh) {
//					str += strprintf("%02X", *(unsigned char*)pCh);
//				}
//				LogPrint("Hash", "nVersion:%s\n", str.c_str());
//				str = "";
//				for (char *pCh = BEGIN(postxinfo.hashPrevBlock); pCh != END(postxinfo.hashPrevBlock); ++pCh) {
//					str += strprintf("%02X", *(unsigned char*)pCh);
//				}
//				LogPrint("Hash", "hashPrevBlock:%s\n", str.c_str());
//				str = "";
//				for (char *pCh = BEGIN(postxinfo.hashMerkleRoot); pCh != END(postxinfo.hashMerkleRoot); ++pCh) {
//					str += strprintf("%02X", *(unsigned char*)pCh);
//				}
//				LogPrint("Hash", "hashMerkleRoot:%s\n", str.c_str());
//				str = "";
//				for (char *pCh = BEGIN(postxinfo.nValues); pCh != END(postxinfo.nValues); ++pCh) {
//					str += strprintf("%02X", *(unsigned char*)pCh);
//				}
//				LogPrint("Hash", "nValues:%s\n", str.c_str());
//				str = "";
//				for (char *pCh = BEGIN(postxinfo.nTime); pCh != END(postxinfo.nTime); ++pCh) {
//					str += strprintf("%02X", *(unsigned char*)pCh);
//				}
//				LogPrint("Hash", "nTime:%s\n", str.c_str());
//				str = "";
//				for (char *pCh = BEGIN(postxinfo.nNonce); pCh != END(postxinfo.nNonce); ++pCh) {
//					str += strprintf("%02X", *(unsigned char*)pCh);
//				}
//				LogPrint("Hash", "nNonce:%s\n", str.c_str());
				printf("curhash smaller then adjusthash\r\n");
				CRegID regid;
				CKey key;
				if (pwalletMain->GetKey(item.keyID, key) && pwalletMain->GetRegID(item.keyID, regid)) {
					CRewardTransaction *prtx = (CRewardTransaction *) pBlock->vptx[0].get();
					prtx->rewardValue += item.GetInterest();
					prtx->account = regid.vRegID;
					prtx->nHeight = pPrevIndex->nHeight+1;
					pBlock->hashMerkleRoot = pBlock->BuildMerkleTree();

					printf("CreatePosTx addr = %s\r\n",CBitcoinAddress(item.keyID).ToString().c_str());
					LogPrint("postx", "find pos tx hash succeed: \n"
									  "   pos hash:%s \n"
									  "adjust hash:%s \r\n", curhash.GetHex(), adjusthash.GetHex());
					LogPrint("postx",
							"nVersion=%d, hashPreBlock=%s, hashMerkleRoot=%s, nValue=%ld, nTime=%ld, nNonce=%ld\n",
							postxinfo.nVersion, postxinfo.hashPrevBlock.GetHex(), postxinfo.hashMerkleRoot.GetHex(),
							postxinfo.nValues, postxinfo.nTime, postxinfo.nNonce);
					if (key.Sign(pBlock->SignatureHash(), pBlock->vSignature)) {
						return true;
					} else {
						LogPrint("postx", "sign fail\r\n");
					}
				} else {
					LogPrint("postx", "GetKey fail or GetRegID fail\r\n");
				}
			}
		}
	}

	return false;
}

bool VerifyPosTx(const CBlockIndex *pPrevIndex, CAccountViewCache &accView, const CBlock *pBlock, uint64_t &nInterest, CTransactionCache &txCache, CContractScriptCache &scriptCache, bool bJustCheckSign) {

	uint64_t maxNonce = CBaseParams::GetArg("-blockmaxnonce", 10000); //cacul times

	if (pBlock->nNonce > maxNonce) {
		LogPrint("postx", "Nonce is larger than maxNonce\r\n");
		return false;
	}

	if (pBlock->hashMerkleRoot != pBlock->BuildMerkleTree()) {
		LogPrint("postx", "hashMerkleRoot is error\r\n");
		return false;
	}
	CAccountViewCache view(accView);
	CSecureAccount secureAcc;
	{
		CRewardTransaction *prtx = (CRewardTransaction *) pBlock->vptx[0].get();

		{
			for (unsigned int i = 1; i < pBlock->vptx.size(); i++) {
				shared_ptr<CBaseTransaction> pBaseTx = pBlock->vptx[i];
				if (pTxCacheTip->IsContainTx(pBaseTx->GetHash())) {
					LogPrint("INFO","VerifyPosTx duplicate tx\n");
					return false;
				}
				CTxUndo txundo;
				CValidationState state;

				if (!pBaseTx->UpdateAccount(i, view, state, txundo, pPrevIndex->nHeight + 1, txCache, scriptCache)) {
					LogPrint("INFO","transaction UpdateAccount account error\n");
					return false;
				}
			}
		}

		if (view.GetAccount(prtx->account, secureAcc)) {
			//available acc
			if (!secureAcc.publicKey.Verify(pBlock->SignatureHash(), pBlock->vSignature)) {
//				LogPrint("postx", "publickey:%s, keyid:%s\n", secureAcc.publicKey.GetHash().GetHex(), secureAcc.keyID.GetHex());
//				LogPrint("postx", "block verify fail\r\n");
//				LogPrint("postx", "block hash:%s\n", pBlock->GetHash().GetHex());
//				LogPrint("postx", "signature block:%s\n", HexStr(pBlock->vSignature.begin(), pBlock->vSignature.end()));
				return false;
			}

		} else {
			LogPrint("postx", "AccountView have no the accountid\r\n");
			return false;
		}
	}

	if (bJustCheckSign)
		return true;

	nInterest = secureAcc.GetInterest();
	uint256 prevblockhash = pPrevIndex->GetBlockHash();
	const uint256 targetHash = CBigNum().SetCompact(pBlock->nBits).getuint256(); //target hash difficult

	uint64_t posacc = secureAcc.GetSecureAccPos(pPrevIndex->nHeight);
	if (posacc == 0) {
		LogPrint("postx", "Account have no pos\r\n");
		return false;
	}

	uint256 adjusthash = GetAdjustHash(targetHash, posacc); //adjust nbits

//need compute this block proofofwork
	struct PosTxInfo postxinfo;
	postxinfo.nVersion = pPrevIndex->nVersion;
	postxinfo.hashPrevBlock = prevblockhash;
	postxinfo.hashMerkleRoot = secureAcc.BuildMerkleTree(pPrevIndex->nHeight);
	postxinfo.nValues = secureAcc.llValues;
	postxinfo.nTime = pBlock->nTime;
	postxinfo.nNonce = pBlock->nNonce;
	uint256 curhash = postxinfo.GetHash();

//	string str("");
//	LogPrint("Hash", "PosTxInfo size:%d, Hash Input:%s\n", sizeof(PosTxInfo), str.c_str());
//	str = "";
//	for (char *pCh = BEGIN(postxinfo.nVersion); pCh != END(postxinfo.nVersion); ++pCh) {
//		str += strprintf("%02X", *(unsigned char*)pCh);
//	}
//	LogPrint("Hash", "nVersion:%s\n", str.c_str());
//	str = "";
//	for (char *pCh = BEGIN(postxinfo.hashPrevBlock); pCh != END(postxinfo.hashPrevBlock); ++pCh) {
//		str += strprintf("%02X", *(unsigned char*)pCh);
//	}
//	LogPrint("Hash", "hashPrevBlock:%s\n", str.c_str());
//	str = "";
//	for (char *pCh = BEGIN(postxinfo.hashMerkleRoot); pCh != END(postxinfo.hashMerkleRoot); ++pCh) {
//		str += strprintf("%02X", *(unsigned char*)pCh);
//	}
//	LogPrint("Hash", "hashMerkleRoot:%s\n", str.c_str());
//	str = "";
//	for (char *pCh = BEGIN(postxinfo.nValues); pCh != END(postxinfo.nValues); ++pCh) {
//		str += strprintf("%02X", *(unsigned char*)pCh);
//	}
//	LogPrint("Hash", "nValues:%s\n", str.c_str());
//	str = "";
//	for (char *pCh = BEGIN(postxinfo.nTime); pCh != END(postxinfo.nTime); ++pCh) {
//		str += strprintf("%02X", *(unsigned char*)pCh);
//	}
//	LogPrint("Hash", "nTime:%s\n", str.c_str());
//	str = "";
//	for (char *pCh = BEGIN(postxinfo.nNonce); pCh != END(postxinfo.nNonce); ++pCh) {
//		str += strprintf("%02X", *(unsigned char*)pCh);
//	}
//	LogPrint("Hash", "nNonce:%s\n", str.c_str());

	LogPrint("postx", "nVersion=%d, hashPreBlock=%s, hashMerkleRoot=%s, nValue=%ld, nTime=%ld, nNonce=%ld\n",
			postxinfo.nVersion, postxinfo.hashPrevBlock.GetHex(), postxinfo.hashMerkleRoot.GetHex(), postxinfo.nValues,
			postxinfo.nTime, postxinfo.nNonce);
	if (curhash > adjusthash) {
		LogPrint("postx", "Account ProofOfWorkLimit error: \n"
				           "   pos hash:%s \n"
				           "adjust hash:%s\r\n", curhash.GetHex(), adjusthash.GetHex());
		return false;
	}

	return true;
}

CBlockTemplate* CreateNewBlock() {
//    // Create new block
	auto_ptr<CBlockTemplate> pblocktemplate(new CBlockTemplate());
	if (!pblocktemplate.get())
		return NULL;
	CBlock *pblock = &pblocktemplate->block; // pointer for convenience

	// Create coinbase tx
	CRewardTransaction rtx;

	// Add our coinbase tx as first transaction
	pblock->vptx.push_back(make_shared<CRewardTransaction>(rtx));
	pblocktemplate->vTxFees.push_back(-1); // updated at end
	pblocktemplate->vTxSigOps.push_back(-1); // updated at end

	// Largest block you're willing to create:
	unsigned int nBlockMaxSize = GetArg("-blockmaxsize", DEFAULT_BLOCK_MAX_SIZE);
	// Limit to betweeen 1K and MAX_BLOCK_SIZE-1K for sanity:
	nBlockMaxSize = max((unsigned int) 1000, min((unsigned int) (MAX_BLOCK_SIZE - 1000), nBlockMaxSize));

	// How much of the block should be dedicated to high-priority transactions,
	// included regardless of the fees they pay
	unsigned int nBlockPrioritySize = GetArg("-blockprioritysize", DEFAULT_BLOCK_PRIORITY_SIZE);
	nBlockPrioritySize = min(nBlockMaxSize, nBlockPrioritySize);

	// Minimum block size you want to create; block will be filled with free transactions
	// until there are no more or the block reaches this size:
	unsigned int nBlockMinSize = GetArg("-blockminsize", DEFAULT_BLOCK_MIN_SIZE);
	nBlockMinSize = min(nBlockMaxSize, nBlockMinSize);

	// Collect memory pool transactions into the block
	int64_t nFees = 0;
	{
		LOCK2(cs_main, mempool.cs);
		CBlockIndex* pIndexPrev = chainActive.Tip();
		CAccountViewCache accview(*pAccountViewTip, true);

		bool fPrintPriority = GetBoolArg("-printpriority", false);

		// This vector will be sorted into a priority queue:
		vector<TxPriority> vecPriority;
		map<uint256, vector<COrphan*> > mapDependers;
		GetPriorityTx(vecPriority, mapDependers);

		// Collect transactions into block
		uint64_t nBlockSize = ::GetSerializeSize(*pblock, SER_NETWORK, PROTOCOL_VERSION);
		uint64_t nBlockTx = 0;
		int nBlockSigOps = 100;
		bool fSortedByFee = true;

		TxPriorityCompare comparer(fSortedByFee);
		make_heap(vecPriority.begin(), vecPriority.end(), comparer);

		while (!vecPriority.empty()) {
			// Take highest priority transaction off the priority queue:
			double dPriority = vecPriority.front().get<0>();
			double dFeePerKb = vecPriority.front().get<1>();
			shared_ptr<CBaseTransaction> stx = vecPriority.front().get<2>();
			CBaseTransaction *pBaseTx = stx.get();
			//const CTransaction& tx = *(vecPriority.front().get<2>());

			pop_heap(vecPriority.begin(), vecPriority.end(), comparer);
			vecPriority.pop_back();

			// Size limits
			unsigned int nTxSize = ::GetSerializeSize(pBaseTx->GetNewInstance(), SER_NETWORK, PROTOCOL_VERSION);
			if (nBlockSize + nTxSize >= nBlockMaxSize)
				continue;

			// Skip free transactions if we're past the minimum block size:
			if (fSortedByFee && (dFeePerKb < CTransaction::nMinRelayTxFee) && (nBlockSize + nTxSize >= nBlockMinSize))
				continue;

			// Prioritize by fee once past the priority size or we run out of high-priority
			// transactions:
			if (!fSortedByFee && ((nBlockSize + nTxSize >= nBlockPrioritySize) || !AllowFree(dPriority))) {
				fSortedByFee = true;
				comparer = TxPriorityCompare(fSortedByFee);
				make_heap(vecPriority.begin(), vecPriority.end(), comparer);
			}

			CTxUndo txundo;
			CValidationState state;
			CAccountViewCache accviewtemp(accview);
			CTransactionCache txCacheTemp(*pTxCacheTip);
			CContractScriptCache contractScriptTemp(*pContractScriptTip);
			if (!pBaseTx->UpdateAccount(nBlockTx + 1, accviewtemp, state, txundo, pIndexPrev->nHeight + 1, txCacheTemp, contractScriptTemp)) {
				continue;
			}
			accview = accviewtemp;
			nBlockTx++;
			pblock->vptx.push_back(stx);
			nFees += pBaseTx->GetFee();

		}

		nLastBlockTx = nBlockTx;
		nLastBlockSize = nBlockSize;
		LogPrint("INFO","CreateNewBlock(): total size %u\n", nBlockSize);

		((CRewardTransaction*) pblock->vptx[0].get())->rewardValue = nFees;

		// Fill in header
		pblock->hashPrevBlock = pIndexPrev->GetBlockHash();
		UpdateTime(*pblock, pIndexPrev);
		pblock->nBits = GetNextWorkRequired(pIndexPrev, pblock);
		pblock->nNonce = 0;
		pblock->nHeight = pIndexPrev->nHeight + 1;

	}

	return pblocktemplate.release();
}

bool CheckWork(CBlock* pblock, CWallet& wallet) {
//	uint256 hash = pblock->GetHash();
//	uint256 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint256();
//
//	if (hash > hashTarget)
//		return false;

	//// debug print
//	LogPrint("INFO","proof-of-work found  \n  hash: %s  \ntarget: %s\n", hash.GetHex(), hashTarget.GetHex());
	pblock->print(*pAccountViewTip);
	// LogPrint("INFO","generated %s\n", FormatMoney(pblock->vtx[0].vout[0].nValue));

	// Found a solution
	{
		LOCK(cs_main);
		if (pblock->hashPrevBlock != chainActive.Tip()->GetBlockHash())
			return ERROR("BitcoinMiner : generated block is stale");

		// Remove key from key pool
	//	reservekey.KeepKey();

		// Track how many getdata requests this block gets
		{
			LOCK(wallet.cs_wallet);
			wallet.mapRequestCount[pblock->GetHash()] = 0;
		}

		// Process this block the same as if we had received it from another node
		CValidationState state;
		if (!ProcessBlock(state, NULL, pblock))
			return ERROR("BitcoinMiner : ProcessBlock, block not accepted");
	}

	return true;
}

void static BitcoinMiner(CWallet *pwallet) {
	LogPrint("INFO","BitcoinMiner started\n");
	SetThreadPriority(THREAD_PRIORITY_LOWEST);
	RenameThread("bitcoin-miner");

	// Each thread has its own key and counter
	unsigned int nExtraNonce = 0;

	try {
		while (true) {
			if (Params().NetworkID() != CBaseParams::REGTEST) {
				// Busy-wait for the network to come online so we don't waste time mining
				// on an obsolete chain. In regtest mode we expect to fly solo.
				while (vNodes.empty())
					MilliSleep(1000);
			}

			//
			// Create new block
			//
			unsigned int nTransactionsUpdatedLast = mempool.GetTransactionsUpdated();
			CBlockIndex* pindexPrev = chainActive.Tip();

			auto_ptr<CBlockTemplate> pblocktemplate(CreateNewBlock());
			if (!pblocktemplate.get())
				return;
			CBlock *pblock = &pblocktemplate.get()->block;

			int64_t nStart = GetTime();
			while (true) {

				pblock->nTime = max(pindexPrev->GetMedianTimePast() + 1, GetAdjustedTime());
				set<CKeyID> setCreateKey;
				setCreateKey.clear();
				if (CreatePosTx(pindexPrev, pblock,setCreateKey)) {

					SetThreadPriority(THREAD_PRIORITY_NORMAL);
					CheckWork(pblock, *pwallet);
					SetThreadPriority(THREAD_PRIORITY_LOWEST);

					if (Params().NetworkID() == CBaseParams::REGTEST)
						throw boost::thread_interrupted();
					::MilliSleep(800);
					break;
				}
				::MilliSleep(800);

				// Check for stop or if block needs to be rebuilt
				boost::this_thread::interruption_point();
				if (vNodes.empty() && Params().NetworkID() != CBaseParams::REGTEST)
					break;
				if (mempool.GetTransactionsUpdated() != nTransactionsUpdatedLast && GetTime() - nStart > 60)
					break;
				if (pindexPrev != chainActive.Tip())
					break;
			}
		}
	} catch (boost::thread_interrupted) {
		LogPrint("INFO","BitcoinMiner terminated\n");
		throw;
	}
}

bool CreateBlockWithAppointedAddr(CKeyID &keyID)
{
	if (Params().NetworkID() == CBaseParams::REGTEST)
	{
		unsigned int nTransactionsUpdatedLast = mempool.GetTransactionsUpdated();
		CBlockIndex* pindexPrev = chainActive.Tip();

		auto_ptr<CBlockTemplate> pblocktemplate(CreateNewBlock());
		if (!pblocktemplate.get())
			return false;
		CBlock *pblock = &pblocktemplate.get()->block;

		int64_t nStart = GetTime();
		while (true) {

			pblock->nTime = max(pindexPrev->GetMedianTimePast() + 1, GetAdjustedTime());
			set<CKeyID> setCreateKey;
			setCreateKey.clear();
			setCreateKey.insert(keyID);
			if (CreatePosTx(pindexPrev, pblock,setCreateKey)) {
				CheckWork(pblock, *pwalletMain);
			}
			if(setCreateKey.empty())
			{
				LogPrint("postx", "%s is not exist in the wallet\r\n",CBitcoinAddress(keyID).ToString().c_str());
				break;
			}
			::MilliSleep(1);
			if (pindexPrev != chainActive.Tip())
			{
				return true;
			}
		}
	}
	return false;
}

void GenerateBitcoins(bool fGenerate, CWallet* pwallet, int nThreads) {
	static boost::thread_group* minerThreads = NULL;

	if (nThreads < 0) {
		if (Params().NetworkID() == CBaseParams::REGTEST)
			nThreads = 1;
		else
			nThreads = boost::thread::hardware_concurrency();
	}

	if (minerThreads != NULL) {
		minerThreads->interrupt_all();
		delete minerThreads;
		minerThreads = NULL;
	}

	if (nThreads == 0 || !fGenerate)
		return;

	minerThreads = new boost::thread_group();
	for (int i = 0; i < nThreads; i++)
		minerThreads->create_thread(boost::bind(&BitcoinMiner, pwallet));
}

#endif

