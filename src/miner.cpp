// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "miner.h"

#include "core.h"
#include "main.h"
#include "tx.h"
#include "net.h"

#include "wallet.h"
extern CWallet* pwalletMain;
//////////////////////////////////////////////////////////////////////////////
//
// DacrsMiner
//

//int static FormatHashBlocks(void* pbuffer, unsigned int len) {
//	unsigned char* pdata = (unsigned char*) pbuffer;
//	unsigned int blocks = 1 + ((len + 8) / 64);
//	unsigned char* pend = pdata + 64 * blocks;
//	memset(pdata + len, 0, 64 * blocks - len);
//	pdata[len] = 0x80;
//	unsigned int bits = len * 8;
//	pend[-1] = (bits >> 0) & 0xff;
//	pend[-2] = (bits >> 8) & 0xff;
//	pend[-3] = (bits >> 16) & 0xff;
//	pend[-4] = (bits >> 24) & 0xff;
//	return blocks;
//}

static const unsigned int pSHA256InitState[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f,
		0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };

//void SHA256Transform(void* pstate, void* pinput, const void* pinit) {
//	SHA256_CTX ctx;
//	unsigned char data[64];
//
//	SHA256_Init(&ctx);
//
//	for (int i = 0; i < 16; i++)
//		((uint32_t*) data)[i] = ByteReverse(((uint32_t*) pinput)[i]);
//
//	for (int i = 0; i < 8; i++)
//		ctx.h[i] = ((uint32_t*) pinit)[i];
//
//	SHA256_Update(&ctx, data, sizeof(data));
//	for (int i = 0; i < 8; i++)
//		((uint32_t*) pstate)[i] = ctx.h[i];
//}

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

//base on the last 500 blocks
uint64_t GetElementForBurn(CBlockIndex* pindex)
{
	uint64_t sumfee;
	int nBlock = SysCfg().GetArg("-blocksizeforburn", DEFAULT_BURN_BLOCK_SIZE);
//	CBlockIndex* pindex = chainActive.Tip();
	if (nBlock > pindex->nHeight) {
		return 100000;
	} else {
		for (int ii = 0; ii < nBlock; ii++) {
			sumfee += pindex->GetBlockFee();
			pindex = pindex->pprev;
		}
		sumfee  = sumfee / nBlock;
		if(sumfee < 100000) //如果平均值小于最小值 这取最小值
			sumfee = 100000;
	}
	return sumfee;
}

// We want to sort transactions by priority and fee, so:

void GetPriorityTx(vector<TxPriority> &vecPriority) {
	vecPriority.reserve(mempool.mapTx.size());
//	CBlockIndex* pindexPrev = chainActive.Tip();

	// Priority order to process transactions
	list<COrphan> vOrphan; // list memory doesn't move
	double dPriority = 0;
	for (map<uint256, CTxMemPoolEntry>::iterator mi = mempool.mapTx.begin(); mi != mempool.mapTx.end(); ++mi) {
//		int nTxHeight = mi->second.GetHeight();//get Chain height when the tx entering the mempool
		CBaseTransaction *pBaseTx = mi->second.GetTx().get();

		if (uint256(0) == std::move(pTxCacheTip->IsContainTx(std::move(pBaseTx->GetHash())))) {
			unsigned int nTxSize = ::GetSerializeSize(*pBaseTx, SER_NETWORK, PROTOCOL_VERSION);
#if 0
			{
				uint64_t element = GetElementForBurn();
				uint64_t burnfee = xx(element, pBaseTx);
				if(pBaseTx->GetFee() < burnfee)
				{
					LogPrint("INFO","the pBaseTx->GetFee() < burnfee\n");
					assert(0);
				}
				double dFeePerKb = double(pBaseTx->GetFee() - ) / (double(nTxSize) / 1000.0);
			}
#else
			double dFeePerKb = double(pBaseTx->GetFee()) / (double(nTxSize) / 1000.0);
#endif
			dPriority = 1000.0 / double(nTxSize);
			vecPriority.push_back(TxPriority(dPriority, dFeePerKb, mi->second.GetTx()));
		}

	}
}

//CBlockTemplate* CreateNewBlock(const CScript& scriptPubKeyIn) {
////    // Create new block
//	auto_ptr<CBlockTemplate> pblocktemplate(new CBlockTemplate());
//	if (!pblocktemplate.get())
//		return NULL;
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
//            if(COMMON_TX == pBaseTx->nTxType || GENTOSECURE_TX == pBaseTx->nTxType)
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

//	return pblocktemplate.release();
//}

void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce) {
	// Update nExtraNonce
	static uint256 hashPrevBlock;
	if (hashPrevBlock != pblock->hashPrevBlock) {
		nExtraNonce = 0;
		hashPrevBlock = pblock->hashPrevBlock;
	}
	++nExtraNonce;
//	unsigned int nHeight = pindexPrev->nHeight + 1; // Height first in coinbase required for block.version=2
//    pblock->vtx[0].vin[0].scriptSig = (CScript() << nHeight << CBigNum(nExtraNonce)) + COINBASE_FLAGS;
//    assert(pblock->vtx[0].vin[0].scriptSig.size() <= 100);

	pblock->hashMerkleRoot = pblock->BuildMerkleTree();
}


struct CAccountComparator {
	bool operator()(const CAccount &a, const CAccount&b) {
		// First sort by acc over 30days
		if (a.GetAccountPos(chainActive.Tip()->nHeight) < b.GetAccountPos(chainActive.Tip()->nHeight)) {
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
	uint256 minhash = SysCfg().ProofOfWorkLimit().getuint256();

	while (posacc) {
		adjusthash = adjusthash << 1;
		posacc = posacc >> 1;
		if (adjusthash > minhash) {
			adjusthash = minhash;
			break;
		}
	}

	return std::move(adjusthash);
}
bool CreatePosTx(const CBlockIndex *pPrevIndex, CBlock *pBlock, set<CKeyID>&setCreateKey, CAccountViewCache &view,
		CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	set<CKeyID> setKeyID;

	set<CAccount, CAccountComparator> setAcctInfo;

	LogPrint("INFO","CreatePosTx block time:%d\n",  pBlock->nTime);

	{
		LOCK2(cs_main, pwalletMain->cs_wallet);
		if (!pwalletMain->GetKeyIds(setKeyID, true)) {
			return ERRORMSG("CreatePosTx setKeyID empty");
		}

		for(const auto &keyid:setKeyID) {
			//find CAccount info by keyid
			if(setCreateKey.size()) {
				bool bfind = false;
				for(auto &item: setCreateKey){
					if(item == keyid){
						bfind = true;
						break;
					}
				}
				if (!bfind)
					continue;
			}
			CUserID userId = keyid;
			CAccount acctInfo;
			if (view.GetAccount(userId, acctInfo)) {
				//available
				if (acctInfo.IsRegister() && acctInfo.GetAccountPos(pPrevIndex->nHeight) > 0) {
					setAcctInfo.insert(std::move(acctInfo));
				}
			}
		}
	}

	if (setAcctInfo.empty()) {
		setCreateKey.clear();
		return ERRORMSG("CreatePosTx setSecureAcc empty");
	}

	uint64_t maxNonce = SysCfg().GetBlockMaxNonce(); //cacul times

	uint256 prevblockhash = pPrevIndex->GetBlockHash();
	const uint256 targetHash = CBigNum().SetCompact(pBlock->nBits).getuint256(); //target hash difficult

	for (const auto &item : setAcctInfo) {
		uint64_t posacc = item.GetAccountPos(pPrevIndex->nHeight);
		if (0 == posacc) {  //have no pos
			LogPrint("ERROR", "CreatePosTx posacc zero\n");
			continue;
		}

		uint256 adjusthash = GetAdjustHash(targetHash, posacc); //adjust nbits

//need compute this block proofofwork
		struct PosTxInfo postxinfo;
		postxinfo.nVersion = pBlock->nVersion;
		postxinfo.hashPrevBlock = prevblockhash;
		postxinfo.hashMerkleRoot = item.BuildMerkleTree(pPrevIndex->nHeight);
		postxinfo.nValues = item.llValues;
		postxinfo.nTime = pBlock->nTime; //max(pPrevIndex->GetMedianTimePast() + 1, GetAdjustedTime());
		for (pBlock->nNonce = 0; pBlock->nNonce < maxNonce; ++pBlock->nNonce) {
			postxinfo.nNonce = pBlock->nNonce;
			uint256 curhash = postxinfo.GetHash();

			if (curhash <= adjusthash) {
				CRegID regid;

				if (pAccountViewTip->GetRegId(item.keyID, regid)) {
					CRewardTransaction *prtx = (CRewardTransaction *) pBlock->vptx[0].get();
					prtx->rewardValue += item.GetInterest();
					prtx->account = regid;
					prtx->nHeight = pPrevIndex->nHeight+1;
					pBlock->hashMerkleRoot = pBlock->BuildMerkleTree();
					LogPrint("MINER", "Miner hight:%d time:%s addr = %s \r\n", prtx->nHeight,
							DateTimeStrFormat("%Y-%m-%d %H:%M:%S", GetTime()), item.keyID.ToAddress());
					LogPrint("INFO", "find pos tx hash succeed: \n"
									  "   pos hash:%s \n"
									  "adjust hash:%s \r\n", curhash.GetHex(), adjusthash.GetHex());
					LogPrint("INFO",
							"nVersion=%d, hashPreBlock=%s, hashMerkleRoot=%s, nValue=%ld, nTime=%ld, nNonce=%ld\n",
							postxinfo.nVersion, postxinfo.hashPrevBlock.GetHex(), postxinfo.hashMerkleRoot.GetHex(),
							postxinfo.nValues, postxinfo.nTime, postxinfo.nNonce);
//					LogPrint("INFO", "Miner account info:%s\n", item.ToString());
					if (pwalletMain->Sign(item.keyID, pBlock->SignatureHash(), pBlock->vSignature,
							item.MinerPKey.IsValid())) {
						LogPrint("INFO", "Create new block,hash:%s\n", pBlock->GetHash().GetHex());
						return true;
					} else {
						LogPrint("ERROR", "sign fail\r\n");
					}
				} else {
					LogPrint("ERROR", "GetKey fail or GetVec6 fail\r\n");
				}

			}
		}
	}

	return false;
}

bool VerifyPosTx(CAccountViewCache &accView, const CBlock *pBlock, uint64_t &nInterest, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache, bool bNeedRunTx) {
	LogPrint("INFO", "VerifyPoxTx begin\n");
	uint64_t maxNonce = SysCfg().GetBlockMaxNonce(); //cacul times

	if (pBlock->nNonce > maxNonce) {
		return ERRORMSG("Nonce is larger than maxNonce");
	}

	if (pBlock->hashMerkleRoot != pBlock->BuildMerkleTree()) {
		return ERRORMSG("hashMerkleRoot is error");
	}
	CAccountViewCache view(accView, true);
	CScriptDBViewCache scriptDBView(scriptCache, true);
	CAccount account;
	CRewardTransaction *prtx = (CRewardTransaction *) pBlock->vptx[0].get();
	if (view.GetAccount(prtx->account, account)) {
		if(!CheckSignScript(pBlock->SignatureHash(), pBlock->vSignature, account.PublicKey)) {
			if (!CheckSignScript(pBlock->SignatureHash(), pBlock->vSignature, account.MinerPKey)) {
				return ERRORMSG("Verify miner publickey signature error");
			}
		}
	} else {
		return ERRORMSG("AccountView have no the accountid");
	}

	if (bNeedRunTx) {
		uint64_t nTotalRunStep(0);
		for (unsigned int i = 1; i < pBlock->vptx.size(); i++) {
			shared_ptr<CBaseTransaction> pBaseTx = pBlock->vptx[i];
			if (uint256(0) != txCache.IsContainTx(pBaseTx->GetHash())) {
				return ERRORMSG("VerifyPosTx duplicate tx hash:%s", pBaseTx->GetHash().GetHex());
			}

			CTxUndo txundo;
			CValidationState state;

			if (!pBaseTx->UpdateAccount(i, view, state, txundo, pBlock->nHeight, txCache, scriptDBView)) {
				return ERRORMSG("transaction UpdateAccount account error");
			}
			nTotalRunStep += pBaseTx->nRunStep;
			if(nTotalRunStep > MAX_BLOCK_RUN_STEP) {
				return ERRORMSG("block total run steps exceed max run step");
			}
		}

		if (view.GetAccount(prtx->account, account)) {
			//available acc
//     		cout << "check block hash:" << pBlock->SignatureHash().GetHex() << endl;
//			cout << "check signature:" << HexStr(pBlock->vSignature) << endl;
//			cout <<"account miner"<< account.ToString()<< endl;

			if (!account.PublicKey.Verify(pBlock->SignatureHash(), pBlock->vSignature)) {
				if (!account.MinerPKey.Verify(pBlock->SignatureHash(), pBlock->vSignature)) {
//					LogPrint("postx", "publickey:%s, keyid:%s\n", secureAcc.PublicKey.GetHash().GetHex(),
	//						secureAcc.keyID.GetHex());
//					LogPrint("postx", "block verify fail\r\n");
//					LogPrint("postx", "block hash:%s\n", pBlock->GetHash().GetHex());
//					LogPrint("postx", "signature block:%s\n",
//							HexStr(pBlock->vSignature.begin(), pBlock->vSignature.end()));
				LogPrint("ERROR", "Verify signature error");
					return false;
				}
			}

		} else {
			LogPrint("ERROR", "AccountView have no the accountid\r\n");
			return false;
		}
	}

	nInterest = account.GetInterest();
	uint256 prevblockhash = pBlock->hashPrevBlock;
	const uint256 targetHash = CBigNum().SetCompact(pBlock->nBits).getuint256(); //target hash difficult

	uint64_t posacc = account.GetAccountPos(pBlock->nHeight-1);
	if (posacc == 0) {
		return ERRORMSG("Account have no pos");
	}

	uint256 adjusthash = GetAdjustHash(targetHash, posacc); //adjust nbits

    //need compute this block proofofwork
	struct PosTxInfo postxinfo;
	postxinfo.nVersion = pBlock->nVersion;
	postxinfo.hashPrevBlock = prevblockhash;
	postxinfo.hashMerkleRoot = account.BuildMerkleTree(pBlock->nHeight-1);
	postxinfo.nValues = account.llValues;
	postxinfo.nTime = pBlock->nTime;
	postxinfo.nNonce = pBlock->nNonce;
	uint256 curhash = postxinfo.GetHash();

	LogPrint("INFO", "postxinfo.nVersion=%d, postxinfo.hashPreBlock=%s, postxinfo.hashMerkleRoot=%s, postxinfo.nValue=%ld, postxinfo.nTime=%ld, postxinfo.nNonce=%ld, postxinfo.blockHash=%s\n",
			postxinfo.nVersion, postxinfo.hashPrevBlock.GetHex(), postxinfo.hashMerkleRoot.GetHex(), postxinfo.nValues,
			postxinfo.nTime, postxinfo.nNonce, pBlock->GetHash().GetHex());
	if (curhash > adjusthash) {
		return ERRORMSG("Account ProofOfWorkLimit error: \n"
		           "   pos hash:%s \n"
		           "adjust hash:%s\r\n", curhash.GetHex(), adjusthash.GetHex());
	}

	return true;
}
CBlockTemplate* CreateNewBlock(CAccountViewCache &view, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache){

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
		unsigned int nBlockMaxSize = SysCfg().GetArg("-blockmaxsize", DEFAULT_BLOCK_MAX_SIZE);
		// Limit to betweeen 1K and MAX_BLOCK_SIZE-1K for sanity:
		nBlockMaxSize = max((unsigned int) 1000, min((unsigned int) (MAX_BLOCK_SIZE - 1000), nBlockMaxSize));

		// How much of the block should be dedicated to high-priority transactions,
		// included regardless of the fees they pay
		unsigned int nBlockPrioritySize = SysCfg().GetArg("-blockprioritysize", DEFAULT_BLOCK_PRIORITY_SIZE);
		nBlockPrioritySize = min(nBlockMaxSize, nBlockPrioritySize);

		// Minimum block size you want to create; block will be filled with free transactions
		// until there are no more or the block reaches this size:
		unsigned int nBlockMinSize = SysCfg().GetArg("-blockminsize", DEFAULT_BLOCK_MIN_SIZE);
		nBlockMinSize = min(nBlockMaxSize, nBlockMinSize);

		// Collect memory pool transactions into the block
		int64_t nFees = 0;
		{
			LOCK2(cs_main, mempool.cs);
			CBlockIndex* pIndexPrev = chainActive.Tip();
	//		CAccountViewCache accview(*pAccountViewTip, true);

//			bool fPrintPriority = SysCfg().GetBoolArg("-printpriority", false);

			// This vector will be sorted into a priority queue:
			vector<TxPriority> vecPriority;

			GetPriorityTx(vecPriority);

			// Collect transactions into block
			uint64_t nBlockSize = ::GetSerializeSize(*pblock, SER_NETWORK, PROTOCOL_VERSION);
			uint64_t nBlockTx = 0;
//			int nBlockSigOps = 100;
			bool fSortedByFee = true;
			uint64_t nTotalRunStep = 0;

			TxPriorityCompare comparer(fSortedByFee);
			make_heap(vecPriority.begin(), vecPriority.end(), comparer);
//			CAccountViewCache accviewtemp(*pAccountViewTip, true);
//			CTransactionDBCache txCacheTemp(*pTxCacheTip, true);
//			CScriptDBViewCache contractScriptTemp(*pScriptDBTip, true);

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
				unsigned int nTxSize = ::GetSerializeSize(*pBaseTx, SER_NETWORK, PROTOCOL_VERSION);
				if (nBlockSize + nTxSize >= nBlockMaxSize)
					continue;
				// Run step limits
				if(nTotalRunStep + pBaseTx->nRunStep >= MAX_BLOCK_RUN_STEP)
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
				if(uint256(0) != std::move(txCache.IsContainTx(std::move(pBaseTx->GetHash())))) {
					LogPrint("INFO","CreatePosTx duplicate tx\n");
					continue;
				}

				CTxUndo txundo;
				CValidationState state;
				if(pBaseTx->IsCoinBase()){
					assert(0); //never come here
				}
				CAccountViewCache viewTemp(view, true);
				CScriptDBViewCache scriptCacheTemp(scriptCache, true);
				if (!pBaseTx->UpdateAccount(nBlockTx + 1, viewTemp, state, txundo, pIndexPrev->nHeight + 1,
						txCache, scriptCacheTemp)) {
					continue;
				}
				assert(viewTemp.Flush());
				assert(scriptCacheTemp.Flush());
				nBlockTx++;
				pblock->vptx.push_back(stx);
				nFees += pBaseTx->GetFee();
				nBlockSize += stx->GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION);
				nTotalRunStep += pBaseTx->nRunStep;
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
			return ERRORMSG("DacrsMiner : generated block is stale");

		// Remove key from key pool
	//	reservekey.KeepKey();

		// Track how many getdata requests this block gets
//		{
//			LOCK(wallet.cs_wallet);
//			wallet.mapRequestCount[pblock->GetHash()] = 0;
//		}

		// Process this block the same as if we had received it from another node
		CValidationState state;
		if (!ProcessBlock(state, NULL, pblock))
			return ERRORMSG("DacrsMiner : ProcessBlock, block not accepted");
	}

	return true;
}
bool static  MiningBlock(CBlock *pblock,CWallet *pwallet,CBlockIndex* pindexPrev,unsigned int nTransactionsUpdatedLast,CAccountViewCache &view, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache){

	int64_t nStart = GetTime();

	unsigned int lasttime = 0xFFFFFFFF;
	while (true) {

		// Check for stop or if block needs to be rebuilt
		boost::this_thread::interruption_point();
		if (vNodes.empty() && SysCfg().NetworkID() != CBaseParams::REGTEST)
			return false;


		if (pindexPrev != chainActive.Tip())
			return false;

		//获取时间 同时等待下次时间到
		auto GetNextTimeAndSleep = [&]() {
			while(max(pindexPrev->GetMedianTimePast() + 1, GetAdjustedTime()) == lasttime)
			{
				::MilliSleep(800);
			}
			return (lasttime = max(pindexPrev->GetMedianTimePast() + 1, GetAdjustedTime()));
		};

		pblock->nTime = GetNextTimeAndSleep();	// max(pindexPrev->GetMedianTimePast() + 1, GetAdjustedTime());


		if (pindexPrev != chainActive.Tip())
			return false;


		set<CKeyID> setCreateKey;
		setCreateKey.clear();
		int64_t lasttime = GetTimeMillis();
		bool increatedfalg =CreatePosTx(pindexPrev, pblock, setCreateKey,view,txCache,scriptCache);
		LogPrint("MINER","CreatePosTx used time :%d ms\n",   GetTimeMillis() - lasttime);
		if (increatedfalg == true) {
			SetThreadPriority(THREAD_PRIORITY_NORMAL);
			{
			int64_t lasttime1 = GetTimeMillis();
			CheckWork(pblock, *pwallet);
		    LogPrint("MINER","CheckWork used time :%d ms\n",   GetTimeMillis() - lasttime1);
			}
			SetThreadPriority(THREAD_PRIORITY_LOWEST);

			return true;
		}

		if (mempool.GetTransactionsUpdated() != nTransactionsUpdatedLast && GetTime() - nStart > 60)
				return false;
	}
	return false;
}
void static DacrsMiner(CWallet *pwallet) {
	LogPrint("INFO","Miner started\n");

	SetThreadPriority(THREAD_PRIORITY_LOWEST);
	RenameThread("Dacrs-miner");

	{
		LOCK2(cs_main, pwalletMain->cs_wallet);
		set<CKeyID> dummy;
		if (!pwalletMain->GetKeyIds(dummy, true)) {
			LogPrint("INFO","DacrsMiner  terminated\n");
		    ERRORMSG("ERROR:%s ", "no key for minering\n");
			SysCfg().SoftSetArgCover("-ismining", "0");
		    throw ;
		}

	}


	try {
		while (true) {
			if (SysCfg().NetworkID() != CBaseParams::REGTEST) {
				// Busy-wait for the network to come online so we don't waste time mining
				// on an obsolete chain. In regtest mode we expect to fly solo.
				while (vNodes.empty())
					MilliSleep(1000);
			}

			//
			// Create new block
			//
			unsigned int LastTrsa = mempool.GetTransactionsUpdated();
			CBlockIndex* pindexPrev = chainActive.Tip();

			CAccountViewCache accview(*pAccountViewTip, true);
			CTransactionDBCache txCache(*pTxCacheTip, true);
			CScriptDBViewCache ScriptDbTemp(*pScriptDBTip, true);
			int64_t lasttime1 = GetTimeMillis();
			shared_ptr<CBlockTemplate> pblocktemplate(CreateNewBlock(accview,txCache,ScriptDbTemp));
			LogPrint("MINER","CreateNewBlock blocksize%d used time :%d ms\n",pblocktemplate.get()->block.vptx.size(),GetTimeMillis() - lasttime1);
			if (!pblocktemplate.get())
				throw;
			CBlock *pblock = &pblocktemplate.get()->block;
			MiningBlock(pblock,pwallet,pindexPrev,LastTrsa,accview,txCache,ScriptDbTemp);
			
			if (SysCfg().NetworkID() != CBaseParams::MAIN)
				if(SysCfg().GetBoolArg("-iscutmine", false)== false)
				{
					SysCfg().SoftSetArgCover("-ismining", "0");
				  throw boost::thread_interrupted();
				}	
		}
	} catch (...) {
		LogPrint("INFO","DacrsMiner  terminated\n");
		SysCfg().SoftSetArgCover("-ismining", "0");
		throw;
	}
}

uint256 CreateBlockWithAppointedAddr(CKeyID const &keyID)
{
	if (SysCfg().NetworkID() == CBaseParams::REGTEST)
	{
//		unsigned int nTransactionsUpdatedLast = mempool.GetTransactionsUpdated();
		mempool.GetTransactionsUpdated();
		CBlockIndex* pindexPrev = chainActive.Tip();

		CAccountViewCache accview(*pAccountViewTip, true);
		CTransactionDBCache txCache(*pTxCacheTip, true);
		CScriptDBViewCache ScriptDbTemp(*pScriptDBTip, true);
		shared_ptr<CBlockTemplate> pblocktemplate(CreateNewBlock(accview,txCache,ScriptDbTemp));
		if (!pblocktemplate.get())
			return false;
		CBlock *pblock = &pblocktemplate.get()->block;

//		int nBlockSize = pblock->GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION);
		pblock->GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION);

//		int64_t nStart = GetTime();
		while (true) {

			pblock->nTime = max(pindexPrev->GetMedianTimePast() + 1, GetAdjustedTime());
			set<CKeyID> setCreateKey;
			setCreateKey.clear();
			setCreateKey.insert(keyID);
			if (CreatePosTx(pindexPrev, pblock,setCreateKey,accview,txCache,ScriptDbTemp)) {
				CheckWork(pblock, *pwalletMain);
//				int nBlockSize = pblock->GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION);
			}
			if(setCreateKey.empty())
			{
				LogPrint("postx", "%s is not exist in the wallet\r\n",keyID.ToAddress());
				break;
			}
			::MilliSleep(1);
			if (pindexPrev != chainActive.Tip())
			{
				return chainActive.Tip()->GetBlockHash() ;
			}
		}
	}
	return uint256(0);
}

void GenerateSoys(bool fGenerate, CWallet* pwallet, int nThreads) {
	static boost::thread_group* minerThreads = NULL;

	if (minerThreads != NULL) {
		minerThreads->interrupt_all();
		SysCfg().SoftSetArgCover("-ismining", "0");
		delete minerThreads;
		minerThreads = NULL;
	}

	if (!fGenerate)
		return;
	//in pos system one thread is enough  marked by ranger.shi
	minerThreads = new boost::thread_group();
	minerThreads->create_thread(boost::bind(&DacrsMiner, pwallet));

	SysCfg().SoftSetArgCover("-ismining", "1");
//	minerThreads->join_all();
}



