// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "miner.h"

#include "core.h"
#include "main.h"
#include "tx.h"
#include "net.h"

#include "./wallet/wallet.h"
extern CWallet* g_pwalletMain;
extern void SetMinerStatus(bool bstatue );
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

static const unsigned int g_sSHA256InitState[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f,
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
	COrphan(const CTransaction* ptxIn) {
		m_pTx = ptxIn;
		m_dPriority = m_dFeePerKb = 0;
	}

	void print() const {
		LogPrint("INFO","COrphan(hash=%s, dPriority=%.1f, dFeePerKb=%.1f)\n", m_pTx->GetHash().ToString(), m_dPriority,
				m_dFeePerKb);
		for (const auto& hash : m_setDependsOn)
			LogPrint("INFO", "   setDependsOn %s\n", hash.ToString());
	}

	const CTransaction* m_pTx;
	set<uint256> m_setDependsOn;
	double m_dPriority;
	double m_dFeePerKb;
};

uint64_t g_ullLastBlockTx 	= 0;    // 块中交易的总笔数,不含coinbase
uint64_t g_ullLastBlockSize = 0;  	//被创建的块 尺寸

//base on the last 50 blocks
int GetElementForBurn(CBlockIndex* pBlockIndex) {
	if (NULL == pBlockIndex) {
		return INIT_FUEL_RATES;
	}
	int nBlock = SysCfg().GetArg("-blocksizeforburn", DEFAULT_BURN_BLOCK_SIZE);
	if (nBlock * 2 >= pBlockIndex->m_nHeight - 1) {
		return INIT_FUEL_RATES;
	} else {
		int64_t llTotalFeePerKb(0);
		int64_t llAverageFeePerKb1(0);
		int64_t llAverageFeePerKb2(0);
		int64_t llTotalStep(0);
		int64_t llAverateStep(0);
		CBlockIndex * pTempBlockIndex = pBlockIndex;
		if (pBlockIndex->m_nHeight <= g_sBurnRateForkHeight) {
			if ((pBlockIndex->m_nHeight - 1) % nBlock == 0) {
				for (int ii = 0; ii < nBlock; ii++) {
					llTotalFeePerKb += int64_t(pTempBlockIndex->m_dFeePerKb);
					pTempBlockIndex = pTempBlockIndex->m_pPrevBlockIndex;
				}
				if (pBlockIndex->m_unChainTx - pTempBlockIndex->m_unChainTx < (unsigned int) 10 * nBlock) {
					return pBlockIndex->m_nFuelRate;
				}
				uint64_t ullTxNum = pTempBlockIndex->m_unChainTx;
				llAverageFeePerKb1 = llTotalFeePerKb / nBlock;
				llTotalFeePerKb = 0;
				for (int ii = 0; ii < nBlock; ii++) {
					llTotalFeePerKb += int64_t(pTempBlockIndex->m_dFeePerKb);
					pTempBlockIndex = pTempBlockIndex->m_pPrevBlockIndex;
				}
				llAverageFeePerKb2 = llTotalFeePerKb / nBlock;
				if (ullTxNum - pTempBlockIndex->m_unChainTx < (unsigned int) 10 * nBlock) {
					return pBlockIndex->m_nFuelRate;
				}
				if (0 == llAverageFeePerKb1 || 0 == llAverageFeePerKb2) {
					return pBlockIndex->m_nFuelRate;
				} else {
					int newFuelRate = int(pBlockIndex->m_nFuelRate * (llAverageFeePerKb2 / llAverageFeePerKb1));
					if (newFuelRate < MIN_FUEL_RATES) {
						newFuelRate = MIN_FUEL_RATES;
					}
					LogPrint("fuel",
							"preFuelRate=%d fuelRate=%d, nHeight=%d, nAveragerFeePerKb1=%lf, nAverageFeePerKb2=%lf\n",
							pBlockIndex->m_nFuelRate, newFuelRate, pBlockIndex->m_nHeight, llAverageFeePerKb1,
							llAverageFeePerKb2);
					return newFuelRate;
				}
			} else {
				return pBlockIndex->m_nFuelRate;
			}
		} else {
			for (int ii = 0; ii < nBlock; ii++) {
				llTotalStep += pTempBlockIndex->m_llFuel / pTempBlockIndex->m_nFuelRate * 100;
				pTempBlockIndex = pTempBlockIndex->m_pPrevBlockIndex;
			}
			llAverateStep = llTotalStep / nBlock;
			int nNewFuelRate(0);
			if (pBlockIndex->m_nHeight <= g_sRegAppFuel2FeeForkHeight) {
				if (llAverateStep < MAX_BLOCK_RUN_STEP * 0.75) {
					nNewFuelRate = pBlockIndex->m_nFuelRate * 0.9;
				} else if (llAverateStep > MAX_BLOCK_RUN_STEP * 0.85) {
					nNewFuelRate = pBlockIndex->m_nFuelRate * 1.1;
					LogPrint("HeighStep", "Height: %d, AverateStep: %ld\n", pBlockIndex->m_nHeight, llAverateStep);
				} else {
					nNewFuelRate = pBlockIndex->m_nFuelRate;
				}
			} else { // 燃料费率不能上调问题，改浮点运算为整数运算
				if (llAverateStep < MAX_BLOCK_RUN_STEP * 3 / 4 /* 0.75 */) {
					nNewFuelRate = pBlockIndex->m_nFuelRate * 9 / 10; /* 0.9 */
				} else if (llAverateStep > MAX_BLOCK_RUN_STEP * 17 / 20 /* 0.85 */) {
					nNewFuelRate = pBlockIndex->m_nFuelRate * 11 / 10; /* 1.1 */
					if (nNewFuelRate <= pBlockIndex->m_nFuelRate) {
						nNewFuelRate = pBlockIndex->m_nFuelRate + 1; //如果跟之前块相同或更小则强制等于之前块加1
					}
					LogPrint("HeighStep", "Height: %d, AverateStep: %ld\n", pBlockIndex->m_nHeight, llAverateStep);
				} else {
					nNewFuelRate = pBlockIndex->m_nFuelRate;
				}
			}
			if (nNewFuelRate < MIN_FUEL_RATES) {
				nNewFuelRate = MIN_FUEL_RATES;
			}

			LogPrint("fuel", "preFuelRate=%d fuelRate=%d, nHeight=%d\n", pBlockIndex->m_nFuelRate, nNewFuelRate,
					pBlockIndex->m_nHeight);

			return nNewFuelRate;
		}
	}
}

// We want to sort transactions by priority and fee, so:

void GetPriorityTx(vector<TxPriority> &vPriority, int nFuelRate) {
	vPriority.reserve(g_cTxMemPool.m_mapTx.size());
	// Priority order to process transactions
	list<COrphan> vOrphan; // list memory doesn't move
	double dPriority = 0;
	for (map<uint256, CTxMemPoolEntry>::iterator mi = g_cTxMemPool.m_mapTx.begin(); mi != g_cTxMemPool.m_mapTx.end(); ++mi) {
		CBaseTransaction *pBaseTx = mi->second.GetTx().get();
		if (uint256() == std::move(g_pTxCacheTip->IsContainTx(std::move(pBaseTx->GetHash())))) {
			unsigned int unTxSize = ::GetSerializeSize(*pBaseTx, SER_NETWORK, g_sProtocolVersion);
			double dFeePerKb = double(pBaseTx->GetFee() - pBaseTx->GetFuel(nFuelRate))/ (double(unTxSize) / 1000.0);
			dPriority = 1000.0 / double(unTxSize);
			vPriority.push_back(TxPriority(dPriority, dFeePerKb, mi->second.GetTx()));
		}
	}
}

void IncrementExtraNonce(CBlock* pBlock, CBlockIndex* pBlockIndexPrev, unsigned int& unExtraNonce) {
	// Update nExtraNonce
	static uint256 cHashPrevBlock;
	if (cHashPrevBlock != pBlock->GetHashPrevBlock()) {
		unExtraNonce = 0;
		cHashPrevBlock = pBlock->GetHashPrevBlock();
	}
	++unExtraNonce;
//	unsigned int nHeight = pindexPrev->nHeight + 1; // Height first in coinbase required for block.version=2
//    pblock->vtx[0].vin[0].scriptSig = (CScript() << nHeight << CBigNum(nExtraNonce)) + COINBASE_FLAGS;
//    assert(pblock->vtx[0].vin[0].scriptSig.size() <= 100);

	pBlock->GetHashMerkleRoot() = pBlock->BuildMerkleTree();
}

struct ST_AccountComparator {
	bool operator()(const CAccount &cAccountA, const CAccount&cAccountB) {
		// First sort by acc over 30days
		CAccount &a1 = const_cast<CAccount &>(cAccountA);
		CAccount &b1 = const_cast<CAccount &>(cAccountB);
		if (a1.GetAccountPos(g_cChainActive.Tip()->m_nHeight+1) < b1.GetAccountPos(g_cChainActive.Tip()->m_nHeight+1)) {
			return false;
		}

		return true;
	}
};

struct ST_PosTxInfo {
	int 			nVersion;
	uint256 		cHashPrevBlock;
	uint256 		cHashMerkleRoot;
	uint64_t 		ullValues;
	unsigned int 	unTime;
	unsigned int 	unNonce;
    unsigned int 	unHeight;
    int64_t    		llFuel;
    int 			nFuelRate;
	uint256 GetHash()
	{
		CDataStream ds(SER_DISK, g_sClientVersion);
		ds<<nVersion;
		ds<<cHashPrevBlock;
		ds<<cHashMerkleRoot;
		ds<<ullValues;
		ds<<unTime;
		ds<<unNonce;
		ds<<unHeight;
		ds<<llFuel;
		ds<<nFuelRate;
		return Hash(ds.begin(),ds.end());
	}
	string ToString() {
		return strprintf("nVersion=%d, cHashPrevBlock=%s, cHashMerkleRoot=%s, ullValues=%ld, unTime=%ld, unNonce=%ld, unHeight=%d, llFuel=%ld nFuelRate=%d\n",
		nVersion, cHashPrevBlock.GetHex(), cHashMerkleRoot.GetHex(), ullValues, unTime, unNonce, unHeight, llFuel, nFuelRate);
	}
};

uint256 GetAdjustHash(const uint256 TargetHash, const uint64_t nPos, const int nCurHeight) {
	uint64_t ullPosAcc = nPos/COIN;
	ullPosAcc /= SysCfg().GetIntervalPos();
	ullPosAcc = max(ullPosAcc, (uint64_t) 1);
	arith_uint256 cAdjustHash = UintToArith256(TargetHash); //adjust nbits
	arith_uint256 cMinHash = SysCfg().ProofOfWorkLimit();

	while (ullPosAcc) {
		cAdjustHash = cAdjustHash << 1;
		ullPosAcc = ullPosAcc >> 1;
		if (cAdjustHash > cMinHash) {
			cAdjustHash = cMinHash;
			break;
		}
	}

	return std::move(ArithToUint256(cAdjustHash));
}

bool CreatePosTx(const CBlockIndex *pPrevIndex, CBlock *pBlock, set<CKeyID>&setCreateKey, CAccountViewCache &cAccountViewCache,
		CTransactionDBCache &cTxCache, CScriptDBViewCache &cScriptCache) {
	set<CKeyID> setKeyID;
	setKeyID.clear();

	set<CAccount, ST_AccountComparator> setAcctInfo;

	{
		LOCK2(g_cs_main, g_pwalletMain->m_cs_wallet);

		if((unsigned int)(g_cChainActive.Tip()->m_nHeight + 1) !=  pBlock->GetHeight()) {
			return false;
		}

		g_pwalletMain->GetKeys(setKeyID, true);                         // first:get keyID from pwalletMain
		if (setKeyID.empty()) {
			return ERRORMSG("CreatePosTx setKeyID empty");
		}

		LogPrint("INFO","CreatePosTx block time:%d\n",  pBlock->GetTime());
		for(const auto &keyid:setKeyID) {                             //second:get account by keyID
			//find CAccount info by keyid
			if(setCreateKey.size()) {
				bool bfind = false;
				for(auto &item: setCreateKey){
					if(item == keyid){
						bfind = true;
						break;
					}
				}
				if (!bfind) {
					continue;
				}
			}
			CUserID userId = keyid;
			CAccount cAccountInfo;
			if (cAccountViewCache.GetAccount(userId, cAccountInfo)) {     // check  acctInfo is or not allowed to mining ,
				//available
//				LogPrint("miner", "account info:regid=%s keyid=%s ncoinday=%lld isMiner=%d\n", acctInfo.regID.ToString(),
//						acctInfo.keyID.ToString(), acctInfo.GetAccountPos(pBlock->nHeight), acctInfo.IsMiner(pBlock->nHeight));
				if (cAccountInfo.IsRegister() && cAccountInfo.GetAccountPos(pBlock->GetHeight()) > 0 && cAccountInfo.IsMiner(pBlock->GetHeight())) {
					setAcctInfo.insert(std::move(cAccountInfo));
//					LogPrint("miner", "miner account info:%s\n", acctInfo.ToString());
				}
			}
		}
	}

	if (setAcctInfo.empty()) {
		setCreateKey.clear();
		LogPrint("INFO", "CreatePosTx setSecureAcc empty");
		return false;
	}

	uint64_t ullMaxNonce 		= SysCfg().GetBlockMaxNonce(); //cacul times
	uint256 cPrevBlockHash 		= pPrevIndex->GetBlockHash();
	const uint256 targetHash 	= CBigNum().SetCompact(pBlock->GetBits()).getuint256(); //target hash difficult
	set<CAccount, ST_AccountComparator>::iterator iterAcct = setAcctInfo.begin();
	for (;iterAcct!=setAcctInfo.end();++iterAcct) { //third: 根据不同的账户 ,去计算挖矿
		CAccount  &cAccountItem = const_cast<CAccount&>(*iterAcct);
		uint64_t ullPosAcc = cAccountItem.GetAccountPos(pBlock->GetHeight());
		if (0 == ullPosAcc) {  //have no pos
			LogPrint("ERROR", "CreatePosTx posacc zero\n");
			continue;
		}
		LogPrint("miner", "miner account:%s\n", cAccountItem.ToString());
//		LogPrint("INFO", "target hash:%s\n", targetHash.ToString());
//		LogPrint("INFO", "posacc:%d\n", posacc);
		uint256 adjusthash = GetAdjustHash(targetHash, ullPosAcc, pBlock->GetHeight()-1); //adjust nbits
//		LogPrint("INFO", "adjusthash:%s\n", adjusthash.ToString());

		//need compute this block proofofwork
		struct ST_PosTxInfo tPosTxinfo;
		tPosTxinfo.nVersion 			= pBlock->GetVersion();
		tPosTxinfo.cHashPrevBlock 	= cPrevBlockHash;
		tPosTxinfo.cHashMerkleRoot 	= cAccountItem.GetHash();
		tPosTxinfo.ullValues 		= cAccountItem.m_ullValues;
		tPosTxinfo.unHeight 			= pBlock->GetHeight();
		tPosTxinfo.llFuel 			= pBlock->GetFuel();
		tPosTxinfo.nFuelRate 		= pBlock->GetFuelRate();
		tPosTxinfo.unTime 			= pBlock->GetTime(); //max(pPrevIndex->GetMedianTimePast() + 1, GetAdjustedTime());
		unsigned int nNonce 		= 0;
		for (; nNonce < ullMaxNonce; ++nNonce) {        //循环的 更改随机数，计算curhash看是否满足
			tPosTxinfo.unNonce = nNonce;
			pBlock->SetNonce(nNonce);
			uint256 curhash = tPosTxinfo.GetHash();

			if (UintToArith256(curhash) <= UintToArith256(adjusthash)) {
				CRegID cRegId;
				if (g_pAccountViewTip->GetRegId(cAccountItem.m_cKeyID, cRegId)) {
					CRewardTransaction *prtx = (CRewardTransaction *) pBlock->vptx[0].get();
					prtx->m_cAccount = cRegId;                                   //存矿工的 账户ID
					prtx->m_nHeight = pPrevIndex->m_nHeight+1;

					pBlock->SetHashMerkleRoot(pBlock->BuildMerkleTree());
					pBlock->SetHashPos(curhash);
					LogPrint("INFO", "find pos tx hash succeed: \n"
									  "   pos hash:%s \n"
									  "adjust hash:%s \r\n", curhash.GetHex(), adjusthash.GetHex());
					vector<unsigned char> vSign;
					if (g_pwalletMain->Sign(cAccountItem.m_cKeyID, pBlock->SignatureHash(), vSign,
							cAccountItem.m_cMinerPKey.IsValid())) {
						LogPrint("INFO", "Create new block hash:%s\n", pBlock->GetHash().GetHex());
						LogPrint("miner", "Miner account info:%s\n", cAccountItem.ToString());
						LogPrint("miner", "CreatePosTx block hash:%s, postxinfo:%s\n",pBlock->GetHash().GetHex(), tPosTxinfo.ToString().c_str());
						pBlock->SetSignature(vSign);
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

bool VerifyPosTx(CAccountViewCache &cAccountViewCache, const CBlock *pBlock, CTransactionDBCache &cTxDBCache,
		CScriptDBViewCache &cScriptDBViewCache, bool bNeedRunTx) {
//	LogPrint("INFO", "VerifyPoxTx begin\n");
	uint64_t ullMaxNonce = SysCfg().GetBlockMaxNonce(); //cacul times

	if (pBlock->GetNonce() > ullMaxNonce) {
		return ERRORMSG("Nonce is larger than maxNonce");
	}
	if (pBlock->GetHashMerkleRoot() != pBlock->BuildMerkleTree()) {
		return ERRORMSG("hashMerkleRoot is error");
	}

	CAccountViewCache cAccountView(cAccountViewCache, true);
	CScriptDBViewCache cScriptDBView(cScriptDBViewCache, true);
	CAccount cAccount;
	CRewardTransaction *prtx = (CRewardTransaction *) pBlock->vptx[0].get();
	if (cAccountView.GetAccount(prtx->m_cAccount, cAccount)) {
		/*
		if(pBlock->GetHeight() > nFreezeBlackAcctHeight && account.IsBlackAccount()) {
			return ERRORMSG("Black Account mining\n");
		}
		*/
		if(!CheckSignScript(pBlock->SignatureHash(), pBlock->GetSignature(), cAccount.m_cPublicKey)) {
			if (!CheckSignScript(pBlock->SignatureHash(), pBlock->GetSignature(), cAccount.m_cMinerPKey)) {
				return ERRORMSG("Verify miner publickey signature error");
			}
		}
	} else {
		return ERRORMSG("AccountView have no the accountid");
	}

	//校验reward_tx 版本是否正确
	if (pBlock->GetHeight() > g_sUpdateTxVersion2Height) {
		if (prtx->m_nVersion != g_sTxVersion2) {
			return ERRORMSG("CTransaction CheckTransction,tx version is not equal current version, (tx version %d: vs current %d)",
					prtx->m_nVersion, g_sTxVersion2);
		}
	}

	if (bNeedRunTx) {
		int64_t llTotalFuel(0);
		uint64_t ullTotalRunStep(0);
		for (unsigned int i = 1; i < pBlock->vptx.size(); i++) {
			shared_ptr<CBaseTransaction> pBaseTx = pBlock->vptx[i];
			if (uint256() != cTxDBCache.IsContainTx(pBaseTx->GetHash())) {
				return ERRORMSG("VerifyPosTx duplicate tx hash:%s", pBaseTx->GetHash().GetHex());
			}
			CTxUndo cTxUndo;
			CValidationState cValidState;
			if (EM_CONTRACT_TX == pBaseTx->m_chTxType) {
				LogPrint("vm", "tx hash=%s VerifyPosTx run contract\n", pBaseTx->GetHash().GetHex());
			}
			pBaseTx->m_nFuelRate = pBlock->GetFuelRate();
			if (!pBaseTx->ExecuteTx(i, cAccountView, cValidState, cTxUndo, pBlock->GetHeight(), cTxDBCache, cScriptDBView)) {
				return ERRORMSG("transaction UpdateAccount account error");
			}
			ullTotalRunStep += pBaseTx->m_ullRunStep;
			if(ullTotalRunStep > MAX_BLOCK_RUN_STEP) {
				return ERRORMSG("block total run steps exceed max run step");
			}

			llTotalFuel += pBaseTx->GetFuel(pBlock->GetFuelRate());
			LogPrint("fuel", "VerifyPosTx total fuel:%d, tx fuel:%d runStep:%d fuelRate:%d txhash:%s \n",llTotalFuel, pBaseTx->GetFuel(pBlock->GetFuelRate()), pBaseTx->m_ullRunStep, pBlock->GetFuelRate(), pBaseTx->GetHash().GetHex());
		}

		if(llTotalFuel != pBlock->GetFuel()) {
			return ERRORMSG("fuel value at block header calculate error");
		}

		if (cAccountView.GetAccount(prtx->m_cAccount, cAccount)) {
			if(cAccount.GetAccountPos(pBlock->GetHeight()) <= 0 || !cAccount.IsMiner(pBlock->GetHeight()))
				return ERRORMSG("coindays of account dismatch, can't be miner, account info:%s", cAccount.ToString());
		} else {
			LogPrint("ERROR", "AccountView have no the accountid\r\n");
			return false;
		}
	}

	uint256 cPrevBlockHash 		= pBlock->GetHashPrevBlock();
	const uint256 cTargetHash 	= CBigNum().SetCompact(pBlock->GetBits()).getuint256(); //target hash difficult

	uint64_t ullPosAcc = cAccount.GetAccountPos(pBlock->GetHeight());
/*
	arith_uint256 bnNew;
	bnNew.SetCompact(pBlock->nBits);
	string  timeFormat = DateTimeStrFormat("%Y-%m-%d %H:%M:%S", pBlock->nTime);
	LogPrint("coinday","height:%.10d, account:%-10s, coinday:%.10ld, difficulty=%.8lf,  time=%-20s, interval=%d\n", pBlock->nHeight, account.regID.ToString(), posacc/COIN/SysCfg().GetIntervalPos(), CaculateDifficulty(bnNew.GetCompact()), timeFormat,  pBlock->nTime-chainActive[pBlock->nHeight-1]->nTime);
*/
	if (ullPosAcc == 0) {
		return ERRORMSG("Account have no pos");
	}

	uint256 adjusthash = GetAdjustHash(cTargetHash, ullPosAcc, pBlock->GetHeight()-1); //adjust nbits

    //need compute this block proofofwork
	struct ST_PosTxInfo tPosTxInfo;
	tPosTxInfo.nVersion 		= pBlock->GetVersion();
	tPosTxInfo.cHashPrevBlock 	= cPrevBlockHash;
	tPosTxInfo.cHashMerkleRoot 	= cAccount.GetHash();
	tPosTxInfo.ullValues 		= cAccount.m_ullValues;
	tPosTxInfo.unTime 			= pBlock->GetTime();
	tPosTxInfo.unNonce 			= pBlock->GetNonce();
	tPosTxInfo.unHeight 		= pBlock->GetHeight();
	tPosTxInfo.llFuel 			= pBlock->GetFuel();
	tPosTxInfo.nFuelRate 		= pBlock->GetFuelRate();
	uint256 cCurHash = tPosTxInfo.GetHash();

	LogPrint("miner", "Miner account info:%s\n", cAccount.ToString());
	LogPrint("miner", "VerifyPosTx block hash:%s, postxinfo:%s\n", pBlock->GetHash().GetHex(), tPosTxInfo.ToString().c_str());
	if(pBlock->GetHashPos() != cCurHash) {
		return ERRORMSG("PosHash Error: \n"
					" computer PoS hash:%s \n"
					" block PoS hash:%s\n", cCurHash.GetHex(),  pBlock->GetHashPos().GetHex());
	}
	if (UintToArith256(cCurHash) > UintToArith256(adjusthash)) {
		return ERRORMSG("Account ProofOfWorkLimit error: \n"
		           "   pos hash:%s \n"
		           "adjust hash:%s\r\n", cCurHash.GetHex(), adjusthash.GetHex());
	}

	return true;
}

ST_BlockTemplate* CreateNewBlock(CAccountViewCache &cAccViewCache, CTransactionDBCache &cTxCache,
		CScriptDBViewCache &cScriptCache) {
	// Create new block
	auto_ptr<ST_BlockTemplate> pblocktemplate(new ST_BlockTemplate());
	if (!pblocktemplate.get()) {
		return NULL;
	}

	CBlock *pBlock = &pblocktemplate->cBlock; // pointer for convenience
	// Create coinbase tx
	CRewardTransaction cRewardTx;

	// Add our coinbase tx as first transaction
	pBlock->vptx.push_back(std::make_shared<CRewardTransaction>(cRewardTx));
	pblocktemplate->vTxFees.push_back(-1); // updated at end
	pblocktemplate->vTxSigOps.push_back(-1); // updated at end

	// Largest block you're willing to create:
	unsigned int unBlockMaxSize = SysCfg().GetArg("-blockmaxsize", DEFAULT_BLOCK_MAX_SIZE);
	// Limit to betweeen 1K and MAX_BLOCK_SIZE-1K for sanity:
	unBlockMaxSize = max((unsigned int) 1000, min((unsigned int) (MAX_BLOCK_SIZE - 1000), unBlockMaxSize));

	// How much of the block should be dedicated to high-priority transactions,
	// included regardless of the fees they pay
	unsigned int unBlockPrioritySize = SysCfg().GetArg("-blockprioritysize", DEFAULT_BLOCK_PRIORITY_SIZE);
	unBlockPrioritySize = min(unBlockMaxSize, unBlockPrioritySize);

	// Minimum block size you want to create; block will be filled with free transactions
	// until there are no more or the block reaches this size:
	unsigned int unBlockMinSize = SysCfg().GetArg("-blockminsize", DEFAULT_BLOCK_MIN_SIZE);
	unBlockMinSize = min(unBlockMaxSize, unBlockMinSize);

	// Collect memory pool transactions into the block
	int64_t llFees = 0;
	{
		LOCK2(g_cs_main, g_cTxMemPool.m_cs);
		CBlockIndex* pIndexPrev = g_cChainActive.Tip();
		pBlock->SetFuelRate(GetElementForBurn(pIndexPrev));

		// This vector will be sorted into a priority queue:
		vector<TxPriority> vecPriority;
		GetPriorityTx(vecPriority, pBlock->GetFuelRate());

		// Collect transactions into block
		uint64_t ullBlockSize = ::GetSerializeSize(*pBlock, SER_NETWORK, g_sProtocolVersion);
		uint64_t ullBlockTx(0);
		bool bSortedByFee(true);
		uint64_t ullTotalRunStep(0);
		int64_t llTotalFuel(0);
		TxPriorityCompare cTxPriorityComparer(bSortedByFee);
		make_heap(vecPriority.begin(), vecPriority.end(), cTxPriorityComparer);

		while (!vecPriority.empty()) {
			// Take highest priority transaction off the priority queue:
			double dPriority = vecPriority.front().get<0>();
			double dFeePerKb = vecPriority.front().get<1>();
			shared_ptr<CBaseTransaction> stx = vecPriority.front().get<2>();
			CBaseTransaction *pBaseTx = stx.get();
			//const CTransaction& tx = *(vecPriority.front().get<2>());

			pop_heap(vecPriority.begin(), vecPriority.end(), cTxPriorityComparer);
			vecPriority.pop_back();

			// Size limits
			unsigned int nTxSize = ::GetSerializeSize(*pBaseTx, SER_NETWORK, g_sProtocolVersion);
			if (ullBlockSize + nTxSize >= unBlockMaxSize) {
				continue;
			}
			// Skip free transactions if we're past the minimum block size:
			if (bSortedByFee && (dFeePerKb < CTransaction::m_sMinRelayTxFee) && (ullBlockSize + nTxSize >= unBlockMinSize)) {
				continue;
			}
			// Prioritize by fee once past the priority size or we run out of high-priority
			// transactions:
			if (!bSortedByFee && ((ullBlockSize + nTxSize >= unBlockPrioritySize) || !AllowFree(dPriority))) {
				bSortedByFee = true;
				cTxPriorityComparer = TxPriorityCompare(bSortedByFee);
				make_heap(vecPriority.begin(), vecPriority.end(), cTxPriorityComparer);
			}
			if (uint256() != std::move(cTxCache.IsContainTx(std::move(pBaseTx->GetHash())))) {
				LogPrint("INFO", "CreatePosTx duplicate tx\n");
				continue;
			}
			CTxUndo cTxUndo;
			CValidationState cState;
			if (pBaseTx->IsCoinBase()) {
				ERRORMSG("TX type is coin base tx error......");
			}
			if (EM_CONTRACT_TX == pBaseTx->m_chTxType) {
				LogPrint("vm", "tx hash=%s CreateNewBlock run contract\n", pBaseTx->GetHash().GetHex());
			}
			CAccountViewCache cViewTemp(cAccViewCache, true);
			CScriptDBViewCache scriptCacheTemp(cScriptCache, true);
			pBaseTx->m_nFuelRate = pBlock->GetFuelRate();
			if (!pBaseTx->ExecuteTx(ullBlockTx + 1, cViewTemp, cState, cTxUndo, pIndexPrev->m_nHeight + 1, cTxCache,
					scriptCacheTemp)) {
				continue;
			}
			// Run step limits
			if (ullTotalRunStep + pBaseTx->m_ullRunStep >= MAX_BLOCK_RUN_STEP) {
				continue;
			}

			assert(cViewTemp.Flush());
			assert(scriptCacheTemp.Flush());
			llFees += pBaseTx->GetFee();
			ullBlockSize += stx->GetSerializeSize(SER_NETWORK, g_sProtocolVersion);
			ullTotalRunStep += pBaseTx->m_ullRunStep;
			llTotalFuel += pBaseTx->GetFuel(pBlock->GetFuelRate());
			ullBlockTx++;
			pBlock->vptx.push_back(stx);
			LogPrint("fuel", "miner total fuel:%d, tx fuel:%d runStep:%d fuelRate:%d txhash:%s\n", llTotalFuel,
					pBaseTx->GetFuel(pBlock->GetFuelRate()), pBaseTx->m_ullRunStep, pBlock->GetFuelRate(),
					pBaseTx->GetHash().GetHex());
		}

		g_ullLastBlockTx = ullBlockTx;
		g_ullLastBlockSize = ullBlockSize;
		LogPrint("INFO", "CreateNewBlock(): total size %u\n", ullBlockSize);

		assert(llFees - llTotalFuel >= 0);
		((CRewardTransaction*) pBlock->vptx[0].get())->m_ullRewardValue = llFees - llTotalFuel + POS_REWARD;

		// Fill in header
		pBlock->SetHashPrevBlock(pIndexPrev->GetBlockHash());
		UpdateTime(*pBlock, pIndexPrev);
		pBlock->SetBits(GetNextWorkRequired(pIndexPrev, pBlock));
		pBlock->SetNonce(0);
		pBlock->SetHeight(pIndexPrev->m_nHeight + 1);
		pBlock->SetFuel(llTotalFuel);
	}

	return pblocktemplate.release();
}

bool CheckWork(CBlock* pBlock, CWallet& cWallet) {
	pBlock->print(*g_pAccountViewTip);
	// Found a solution
	{
		LOCK(g_cs_main);
		if (pBlock->GetHashPrevBlock() != g_cChainActive.Tip()->GetBlockHash()) {
			return ERRORMSG("DacrsMiner : generated block is stale");
		}

		// Process this block the same as if we had received it from another node
		CValidationState cValidationState;
		if (!ProcessBlock(cValidationState, NULL, pBlock)) {
			return ERRORMSG("DacrsMiner : ProcessBlock, block not accepted");
		}
	}

	return true;
}

bool static MiningBlock(CBlock *pBlock, CWallet *pWallet, CBlockIndex* pBlockIndexPrev,
		unsigned int unTransactionsUpdatedLast, CAccountViewCache &cAccountViewCache, CTransactionDBCache &cTxDBCache,
		CScriptDBViewCache &scriptCache) {
	int64_t llStart = GetTime();
	unsigned int unLastTime = 0xFFFFFFFF;
	while (true) {
		// Check for stop or if block needs to be rebuilt
		boost::this_thread::interruption_point();
		if (g_vNodes.empty() && SysCfg().NetworkID() != CBaseParams::EM_REGTEST) {
			return false;
		}
		if (pBlockIndexPrev != g_cChainActive.Tip()) {
			return false;
		}
		//获取时间 同时等待下次时间到
		auto GetNextTimeAndSleep = [&]() {
			while(max(pBlockIndexPrev->GetMedianTimePast() + 1, GetAdjustedTime()) == unLastTime) {
				::MilliSleep(800);
			}
			return (unLastTime = max(pBlockIndexPrev->GetMedianTimePast() + 1, GetAdjustedTime()));
		};

		GetNextTimeAndSleep();	// max(pindexPrev->GetMedianTimePast() + 1, GetAdjustedTime());
		UpdateTime(*pBlock, pBlockIndexPrev);

		if (pBlockIndexPrev != g_cChainActive.Tip()) {
			return false;
		}

		set<CKeyID> setCreateKey;
		setCreateKey.clear();
		int64_t llLastTime 	= GetTimeMillis();
		bool bIncreatedfalg = CreatePosTx(pBlockIndexPrev, pBlock, setCreateKey,cAccountViewCache,cTxDBCache,scriptCache);
		LogPrint("MINER","CreatePosTx used time :%d ms\n",   GetTimeMillis() - llLastTime);
		if (bIncreatedfalg == true) {
			SetThreadPriority(THREAD_PRIORITY_NORMAL);
			{
				int64_t llLastTime1 = GetTimeMillis();
				CheckWork(pBlock, *pWallet);
				LogPrint("MINER","CheckWork used time :%d ms\n",   GetTimeMillis() - llLastTime1);
			}
			SetThreadPriority(THREAD_PRIORITY_LOWEST);
			return true;
		}
		if (g_cTxMemPool.GetTransactionsUpdated() != unTransactionsUpdatedLast || GetTime() - llStart > 60) {
			return false;
		}
	}
	return false;
}

void static DacrsMiner(CWallet *pWallet,int nTargetConter) {
	LogPrint("INFO","Miner started\n");

	SetThreadPriority(THREAD_PRIORITY_LOWEST);
	RenameThread("Dacrs-miner");

	auto CheckIsHaveMinerKey = [&]() {
		    LOCK2(g_cs_main, g_pwalletMain->m_cs_wallet);
			set<CKeyID> setMineKey;
			setMineKey.clear();
			g_pwalletMain->GetKeys(setMineKey, true);
			return !setMineKey.empty();
		};

	if (!CheckIsHaveMinerKey()) {
			LogPrint("INFO", "DacrsMiner  terminated\n");
			ERRORMSG("ERROR:%s ", "no key for minering\n");
            return ;
		}

	auto getcurhigh = [&]() {
		LOCK(g_cs_main);
		return g_cChainActive.Height();
	};

	nTargetConter = nTargetConter+getcurhigh();
	try {
	       SetMinerStatus(true);
		while (true) {
			if (SysCfg().NetworkID() != CBaseParams::EM_REGTEST) {
				// Busy-wait for the network to come online so we don't waste time mining
				// on an obsolete chain. In regtest mode we expect to fly solo.
				while (g_vNodes.empty()
						|| (g_cChainActive.Tip() && g_cChainActive.Tip()->m_nHeight > 1
								&& GetAdjustedTime() - g_cChainActive.Tip()->m_unTime > 60 * 60))
					MilliSleep(1000);
			}
			// Create new block
			unsigned int unLastTrsa 		= g_cTxMemPool.GetTransactionsUpdated();
			CBlockIndex* pBlockIndexPrev 	= g_cChainActive.Tip();
			CAccountViewCache cAccountViewCache(*g_pAccountViewTip, true);
			CTransactionDBCache cTxDBCache(*g_pTxCacheTip, true);
			CScriptDBViewCache cTempScriptDBViewCache(*g_pScriptDBTip, true);
			int64_t llLastTime1 = GetTimeMillis();
			shared_ptr<ST_BlockTemplate> pblocktemplate(CreateNewBlock(cAccountViewCache, cTxDBCache, cTempScriptDBViewCache));
			if (!pblocktemplate.get()){
				throw runtime_error("Create new block fail.");
			}
			LogPrint("MINER", "CreateNewBlock tx count:%d used time :%d ms\n", pblocktemplate.get()->cBlock.vptx.size(),
					GetTimeMillis() - llLastTime1);
			CBlock *pBlock = &pblocktemplate.get()->cBlock;
			MiningBlock(pBlock, pWallet, pBlockIndexPrev, unLastTrsa, cAccountViewCache, cTxDBCache, cTempScriptDBViewCache);
			
			if (SysCfg().NetworkID() != CBaseParams::EM_MAIN)
				if (nTargetConter <= getcurhigh()) {
					throw boost::thread_interrupted();
				}	
		}
	} catch (...) {
		LogPrint("INFO","DacrsMiner  terminated\n");
    	SetMinerStatus(false);
		throw;
	}
}

uint256 CreateBlockWithAppointedAddr(CKeyID const &cKeyID) {
	if (SysCfg().NetworkID() == CBaseParams::EM_REGTEST) {
		g_cTxMemPool.GetTransactionsUpdated();
		CBlockIndex* pPrevBlockIndex = g_cChainActive.Tip();
		CAccountViewCache cAccountViewCache(*g_pAccountViewTip, true);
		CTransactionDBCache cTxDBCache(*g_pTxCacheTip, true);
		CScriptDBViewCache cTempScriptDBViewCache(*g_pScriptDBTip, true);
		shared_ptr<ST_BlockTemplate> pblocktemplate(CreateNewBlock(cAccountViewCache, cTxDBCache, cTempScriptDBViewCache));
		if (!pblocktemplate.get()) {
			return uint256();
		}

		CBlock *pBlock = &pblocktemplate.get()->cBlock;
		pBlock->GetSerializeSize(SER_NETWORK, g_sProtocolVersion);
		while (true) {
			pBlock->SetTime(max(pPrevBlockIndex->GetMedianTimePast() + 1, GetAdjustedTime()));
			set<CKeyID> setCreateKey;
			setCreateKey.clear();
			setCreateKey.insert(cKeyID);
			if (CreatePosTx(pPrevBlockIndex, pBlock, setCreateKey, cAccountViewCache, cTxDBCache, cTempScriptDBViewCache)) {
				CheckWork(pBlock, *g_pwalletMain);
			}
			if (setCreateKey.empty()) {
				LogPrint("postx", "%s is not exist in the wallet\r\n", cKeyID.ToAddress());
				break;
			}
			::MilliSleep(1);
			if (pPrevBlockIndex != g_cChainActive.Tip()) {
				return g_cChainActive.Tip()->GetBlockHash();
			}
		}
	}
	return uint256();
}

void GenerateDacrsBlock(bool bGenerate, CWallet* pWallet, int targetHigh) {
	static boost::thread_group* minerThreads = NULL;
	if (minerThreads != NULL) {
		minerThreads->interrupt_all();
		delete minerThreads;
		minerThreads = NULL;
	}

	if (targetHigh <= 0 && bGenerate == true) {
		ERRORMSG("targetHigh, fGenerate value error");
		return;
	}
	if (!bGenerate) {
		return;
	}

	//in pos system one thread is enough  marked by ranger.shi
	minerThreads = new boost::thread_group();
	minerThreads->create_thread(boost::bind(&DacrsMiner, pWallet, targetHigh));
}



