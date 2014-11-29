// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txdb.h"
#include "wallet.h"

#include "base58.h"
#include "checkpoints.h"
//#include "coincontrol.h"
#include "net.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/interprocess/sync/file_lock.hpp>


#include <boost/assign/list_of.hpp>
using namespace boost::assign;

using namespace std;
using namespace boost;
#include <boost/algorithm/string/replace.hpp>
#include <openssl/rand.h>
using namespace std;

// Settings
int64_t nTransactionFee = DEFAULT_TRANSACTION_FEE;
bool bSpendZeroConfChange = true;
string CWallet:: defaultFilename("");
//////////////////////////////////////////////////////////////////////////////
//
// mapWallet
//

//struct CompareValueOnly {
//	bool operator()(const pair<int64_t, pair<const CWalletTx*, unsigned int> >& t1,
//			const pair<int64_t, pair<const CWalletTx*, unsigned int> >& t2) const {
//		return t1.first < t2.first;
//	}
//};

//const CAccountTx* CWallet::GetAccountTx(const uint256& hash) const {
//	LOCK(cs_wallet);
//	map<uint256, CAccountTx>::const_iterator it = mapWalletTx.find(hash);
//	if (it == mapWalletTx.end())
//		return NULL;
//	return &(it->second);
//}
//
//bool CWallet::GetTx(const uint256& hash,shared_ptr<CBaseTransaction> &tx) const{
//	LOCK(cs_wallet);
//	for(auto &wtx: mapWalletTx) {
//		const CAccountTx &acctx = wtx.second;
//		for(auto &item: acctx.mapAccountTx) {
//			if (item.first == hash) {
//				tx = item.second;
//				return true;
//			}
//		}
//	}
//	return false;
//}
bool CWallet::AddPubKey(const CPubKey& pk)
{
	AssertLockHeld(cs_wallet);
	CKeyStoreValue tem(pk);
	if(mKeyPool.count(tem.GetCKeyID()) > 0)
		{
		  LogPrint("CWallet","this key is in the CWallet");
		 return false;
		}
	mKeyPool[tem.GetCKeyID()] = tem;
	return FushToDisk();
}

bool CWallet::FushToDisk() const {
	AssertLockHeld(cs_wallet);
	 filesystem::path blocksDir = GetDataDir() / strWalletFile;
//    FILE* fp = fopen(blocksDir.string().c_str(), "wb");

    FILE* fp = fopen(blocksDir.string().c_str(), "wb");
	   if (!fp) throw "Cannot open wallet  file";


	   CAutoFile filein = CAutoFile(fp, SER_DISK, CLIENT_VERSION);

	    if (!filein)
	    	throw "Cannot open wallet  file";
	    filein.clear();
		try {
			filein << *this;
		} catch (...) {
			throw "save wallet failed";
		}

		filein.fclose();
	 return true;
}
bool CWallet::AddKey(const CKey& secret) {
	AssertLockHeld(cs_wallet);


	if (!fFileBacked)
		return true;
	CKeyStoreValue tem(secret);
	if(mKeyPool.count(tem.GetCKeyID()) > 0)
		{
		  LogPrint("CWallet","this key is in the CWallet");
		 return false;
		}

	if (!IsCrypted()) {
		mKeyPool[tem.GetCKeyID()] = tem;
        return FushToDisk();
	}
	else
	{
		assert(0);//to add code
	}
	return true;
}
bool CWallet::AddKey(const CKey& secret,const CKey& minerKey) {
	AssertLockHeld(cs_wallet);

	CKeyStoreValue tem(secret,minerKey);
	if(mKeyPool.count(tem.GetCKeyID()) > 0)
		{
		  LogPrint("CWallet","this key is in the CWallet");
		 return false;
		}

	if (!IsCrypted()) {
		mKeyPool[tem.GetCKeyID()] = tem;
        return FushToDisk();
	}
	else
	{
		assert(0);//to add code
	}
	return true;
}



//bool CWallet::Unlock(const SecureString& strWalletPassphrase) {
////	CCrypter crypter;
////	CKeyingMaterial vMasterKey;
////
////	{
////		LOCK(cs_wallet);
////		for (const auto& pMasterKey : mapMasterKeys) {
////			if (!crypter.SetKeyFromPassphrase(strWalletPassphrase, pMasterKey.second.vchSalt,
////					pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
////				return false;
////			if (!crypter.Decrypt(pMasterKey.second.vchCryptedKey, vMasterKey))
////				continue; // try another master key
////			if (CCryptoKeyStore::Unlock(vMasterKey))
////				return true;
////		}
////	}
//	return false;
//}
//
//bool CWallet::ChangeWalletPassphrase(const SecureString& strOldWalletPassphrase,
//		const SecureString& strNewWalletPassphrase) {
////	bool fWasLocked = IsLocked();
////
////	{
////		LOCK(cs_wallet);
////		Lock();
////
////		CCrypter crypter;
////		CKeyingMaterial vMasterKey;
////		for (auto& pMasterKey : mapMasterKeys) {
////			if (!crypter.SetKeyFromPassphrase(strOldWalletPassphrase, pMasterKey.second.vchSalt,
////					pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
////				return false;
////			if (!crypter.Decrypt(pMasterKey.second.vchCryptedKey, vMasterKey))
////				return false;
////			if (CCryptoKeyStore::Unlock(vMasterKey)) {
////				int64_t nStartTime = GetTimeMillis();
////				crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt,
////						pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod);
////				pMasterKey.second.nDeriveIterations = pMasterKey.second.nDeriveIterations
////						* (100 / ((double) (GetTimeMillis() - nStartTime)));
////
////				nStartTime = GetTimeMillis();
////				crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt,
////						pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod);
////				pMasterKey.second.nDeriveIterations = (pMasterKey.second.nDeriveIterations
////						+ pMasterKey.second.nDeriveIterations * 100 / ((double) (GetTimeMillis() - nStartTime))) / 2;
////
////				if (pMasterKey.second.nDeriveIterations < 25000)
////					pMasterKey.second.nDeriveIterations = 25000;
////
////				LogPrint("INFO","Wallet passphrase changed to an nDeriveIterations of %i\n",
////						pMasterKey.second.nDeriveIterations);
////
////				if (!crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt,
////						pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
////					return false;
////				if (!crypter.Encrypt(vMasterKey, pMasterKey.second.vchCryptedKey))
////					return false;
////				CWalletDB(strWalletFile).WriteMasterKey(pMasterKey.first, pMasterKey.second);
////				if (fWasLocked)
////					Lock();
////				return true;
////			}
////		}
////	}
//
//	return false;
//}

void CWallet::SetBestChain(const CBlockLocator& loc) {
	AssertLockHeld(cs_wallet);
	bestBlock = loc;
	FushToDisk();
}


//
//bool CWallet::EncryptWallet(const SecureString& strWalletPassphrase) {
////	if (IsCrypted())
////		return false;
////
////	CKeyingMaterial vMasterKey;
////	RandAddSeedPerfmon();
////
////	vMasterKey.resize(WALLET_CRYPTO_KEY_SIZE);
////	RAND_bytes(&vMasterKey[0], WALLET_CRYPTO_KEY_SIZE);
////
////	CMasterKey kMasterKey;
////
////	RandAddSeedPerfmon();
////	kMasterKey.vchSalt.resize(WALLET_CRYPTO_SALT_SIZE);
////	RAND_bytes(&kMasterKey.vchSalt[0], WALLET_CRYPTO_SALT_SIZE);
////
////	CCrypter crypter;
////	int64_t nStartTime = GetTimeMillis();
////	crypter.SetKeyFromPassphrase(strWalletPassphrase, kMasterKey.vchSalt, 25000, kMasterKey.nDerivationMethod);
////	kMasterKey.nDeriveIterations = 2500000 / ((double) (GetTimeMillis() - nStartTime));
////
////	nStartTime = GetTimeMillis();
////	crypter.SetKeyFromPassphrase(strWalletPassphrase, kMasterKey.vchSalt, kMasterKey.nDeriveIterations,
////			kMasterKey.nDerivationMethod);
////	kMasterKey.nDeriveIterations = (kMasterKey.nDeriveIterations
////			+ kMasterKey.nDeriveIterations * 100 / ((double) (GetTimeMillis() - nStartTime))) / 2;
////
////	if (kMasterKey.nDeriveIterations < 25000)
////		kMasterKey.nDeriveIterations = 25000;
////
////	LogPrint("INFO","Encrypting Wallet with an nDeriveIterations of %i\n", kMasterKey.nDeriveIterations);
////
////	if (!crypter.SetKeyFromPassphrase(strWalletPassphrase, kMasterKey.vchSalt, kMasterKey.nDeriveIterations,
////			kMasterKey.nDerivationMethod))
////		return false;
////	if (!crypter.Encrypt(vMasterKey, kMasterKey.vchCryptedKey))
////		return false;
////
////	{
////		LOCK(cs_wallet);
////		mapMasterKeys[++nMasterKeyMaxID] = kMasterKey;
////		if (fFileBacked) {
////			pwalletdbEncryption = new CWalletDB(strWalletFile);
////			if (!pwalletdbEncryption->TxnBegin())
////				return false;
////			pwalletdbEncryption->WriteMasterKey(nMasterKeyMaxID, kMasterKey);
////		}
////
////		if (!EncryptKeys(vMasterKey)) {
////			if (fFileBacked)
////				pwalletdbEncryption->TxnAbort();
////			exit(1); //We now probably have half of our keys encrypted in memory, and half not...die and let the user reload their unencrypted wallet.
////		}
////
////		// Encryption was introduced in version 0.4.0
//////		SetMinVersion(FEATURE_WALLETCRYPT, pwalletdbEncryption, true);
////
////		if (fFileBacked) {
////			if (!pwalletdbEncryption->TxnCommit())
////				exit(1); //We now have keys encrypted in memory, but no on disk...die to avoid confusion and let the user reload their unencrypted wallet.
////
////			delete pwalletdbEncryption;
////			pwalletdbEncryption = NULL;
////		}
////
////		Lock();
////		Unlock(strWalletPassphrase);
//////		NewKeyPool();
////		Lock();
////
////		// Need to completely rewrite the wallet file; if we don't, bdb might keep
////		// bits of the unencrypted private key in slack space in the database file.
////		CDB::Rewrite(strWalletFile);
////
////	}
////	NotifyStatusChanged(this);
//
//	return true;
//}

//int64_t CWallet::IncOrderPosNext(CWalletDB *pwalletdb) {
//	AssertLockHeld(cs_wallet); // nOrderPosNext
//	int64_t nRet = nOrderPosNext++;
//	if (pwalletdb) {
//		pwalletdb->WriteOrderPosNext(nOrderPosNext);
//	} else {
//		CWalletDB(strWalletFile).WriteOrderPosNext(nOrderPosNext);
//	}
//	return nRet;
//}



void CWallet::SyncTransaction(const uint256 &hash, CBaseTransaction*pTx, const CBlock* pblock) {

	LOCK2(cs_main, cs_wallet);

	assert(pTx != NULL || pblock != NULL);
	bool fIsNeedUpDataRegID = false;
	bool bupdate = false;
	if(hash == 0 && pTx == NULL) //this is block Sync
	{
		uint256 blockhash = pblock->GetHash();
		auto GenesisBlockProgress = [&]() {
			unsigned short i = 0;
			for (const auto &sptx : pblock->vptx) {
				//confirm the tx GenesisBlock
				CRewardTransaction* prtx = (CRewardTransaction*) sptx.get();
				CPubKey pubkey = boost::get<CPubKey>(prtx->account);
				CKeyID keyid = pubkey.GetKeyID();
				CRegID regid(0, i);
				CAccount account;
				if (IsMine(sptx.get())) {
					AddPubKey(pubkey);
					fIsNeedUpDataRegID = true;
				}
				i++;
			}
		};

		auto ConnectBlockProgress = [&]() {
			CAccountTx newtx(this, blockhash,pblock->nHeight);
			for (const auto &sptx : pblock->vptx) {
				uint256 hashtx = sptx->GetHash();
				//confirm the tx is mine
				if (IsMine(sptx.get())) {
					if (sptx->nTxType == REG_ACCT_TX) {
						fIsNeedUpDataRegID = true;
					} else if (sptx->nTxType == CONTRACT_TX) {
						vector<CAccountOperLog> Log;
						if (GetTxOperLog(hashtx, Log) == true) {
							assert(newtx.AddOperLog(hashtx, Log));
						} else {
							ERROR("GetTxOperLog  error %s", hashtx.GetHex());
						}
					}
					newtx.AddTx(hashtx,sptx.get());
					bupdate = true;
				}
				if(UnConfirmTx.count(hashtx)> 0){
					UnConfirmTx.erase(hashtx);
					bupdate = true;
				}
			}
			if (newtx.GetTxSize() > 0 ) //write to disk
			{
				mapInBlockTx[blockhash] = newtx; //add to map
			}
		};
		auto DisConnectBlockProgress = [&]() {
			CAccountTx Oldtx(this, blockhash);
			for (const auto &sptx : pblock->vptx) {
				if (sptx->nTxType == REG_ACCT_TX) {
					if (IsMine(sptx.get())) {
						fIsNeedUpDataRegID = true;
					}
				}
				Oldtx.AddTx(sptx->GetHash(), sptx.get());
			}

			Oldtx.AcceptToMemoryPool(); //add those tx to mempool
				if (mapInBlockTx.count(blockhash)) {
					mapInBlockTx.erase(blockhash);
					bupdate = true;
				}
		};
		auto IsConnect = [&]() // test is connect or disconct
			{
				return mapBlockIndex.count(blockhash) && chainActive.Contains(mapBlockIndex[blockhash]);
			};
		// GenesisBlock progress
		if (SysCfg().HashGenesisBlock() == blockhash) {
			GenesisBlockProgress();
		} else if (IsConnect()) {
			//connect block
			ConnectBlockProgress();
		} else {
			//disconnect block
			DisConnectBlockProgress();
		}
	}
	else if (pTx != NULL)
	{
		LogPrint("todo","acept in mempool tx %s\r\n",pTx->GetHash().ToString());
    }

	if (fIsNeedUpDataRegID == true) {
		SynchronizSys(*pAccountViewTip);
	}

	if (bupdate == true || fIsNeedUpDataRegID == true)
		FushToDisk();

}

//void CWallet::EraseFromWallet(const uint256 &hash) {
//	if (!fFileBacked)
//		return;
//	{
//		LOCK(cs_wallet);
//		if (mapWalletTx.erase(hash))
//			CWalletDB(strWalletFile).EraseAccountTx(hash);
//	}
//	return;
//}
// Scan the block chain (starting in pindexStart) for transactions
// from or to us. If fUpdate is true, found transactions that already
// exist in the wallet will be updated.
int CWallet::ScanForWalletTransactions(CBlockIndex* pindexStart, bool fUpdate) {

	LogPrint("todo","todo list by ranger.shi");
//	int ret = 0;
//	int64_t nNow = GetTime();
//
//	CBlockIndex* pindex = pindexStart;
//	{
//		LOCK2(cs_main, cs_wallet);
//
//		// no need to read and scan block, if block was created before
//		// our wallet birthday (as adjusted for block time variability)
//		while (pindex && nTimeFirstKey && (pindex->nTime < (nTimeFirstKey - 7200)))
//			pindex = chainActive.Next(pindex);
//
//		ShowProgress(_("Rescanning..."), 0); // show rescan progress in GUI as dialog or on splashscreen, if -rescan on startup
//		double dProgressStart = Checkpoints::GuessVerificationProgress(pindex, false);
//		double dProgressTip = Checkpoints::GuessVerificationProgress(chainActive.Tip(), false);
//		while (pindex) {
//			if (pindex->nHeight % 100 == 0 && dProgressTip - dProgressStart > 0.0)
//				ShowProgress(_("Rescanning..."),
//						max(1,
//								min(99,
//										(int) ((Checkpoints::GuessVerificationProgress(pindex, false) - dProgressStart)
//												/ (dProgressTip - dProgressStart) * 100))));
//
//			CBlock block;
//			ReadBlockFromDisk(block, pindex);
//
//			SyncWithWallets(0, NULL, &block);
//
//			pindex = chainActive.Next(pindex);
//			if (GetTime() >= nNow + 60) {
//				nNow = GetTime();
//				LogPrint("INFO","Still rescanning. At block %d. Progress=%f\n", pindex->nHeight,
//						Checkpoints::GuessVerificationProgress(pindex));
//			}
//		}
//		ShowProgress(_("Rescanning..."), 100); // hide progress dialog in GUI
//	}
	return true;
}

//void CWallet::ReacceptWalletTransactions() {
//	LOCK2(cs_main, cs_wallet);
//
//	mapWalletTx[uint256(0)].AcceptToMemoryPool();
//}

void CWallet::ResendWalletTransactions() {
	// Do this infrequently and randomly to avoid giving away
	// that these are our transactions.
//	if (GetTime() < nNextResend)
//		return;
//	bool fFirst = (nNextResend == 0);
//	uint64_t rand = max(GetRand(30 * 60), (uint64_t) 120);
//	nNextResend = GetTime() + rand;
//	if (fFirst)
//		return;
//
//	// Only do it if there's been a new block since last time
//	if (Params().GetBestRecvTime() < nLastResend)
//		return;
//	nLastResend = GetTime();
//
//	// Rebroadcast any of our txes that aren't in a block yet
//	LogPrint("INFO","ResendWalletTransactions()\n");
//	{
//		LOCK(cs_wallet);
//		// Sort them in chronological order
////		multimap<unsigned int, CWalletTx*> mapSorted;
////		BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item, mapWallet) {
////			CWalletTx& wtx = item.second;
////			// Don't rebroadcast until it's had plenty of time that
////			// it should have gotten in already by now.
////			if (nTimeBestReceived - (int64_t) wtx.nTimeReceived > 5 * 60)
////				mapSorted.insert(make_pair(wtx.nTimeReceived, &wtx));
////		}
//		mapWalletTx[uint256(0)].RelayWalletTransaction();
//	}
}

//// Call after CreateTransaction unless you want to abort
bool CWallet::CommitTransaction(CBaseTransaction *pTx) {
	LOCK2(cs_main, cs_wallet);
	LogPrint("INFO", "CommitTransaction:\n%s", pTx->ToString(*pAccountViewTip));
//		{
//			// This is only to keep the database open to defeat the auto-flush for the
//			// duration of this scope.  This is the only place where this optimization
//			// maybe makes sense; please don't do it anywhere else.
//			CWalletDB* pwalletdb = fFileBacked ? new CWalletDB(strWalletFile, "r") : NULL;
//
//			// Add tx to wallet, because if it has change it's also ours,
//			// otherwise just for transaction history.
//			AddToWallet(wtxNew);
//
//			// Notify that old coins are spent
//			set<CWalletTx*> setCoins;
//			BOOST_FOREACH(const CTxIn& txin, wtxNew.vin) {
//				CWalletTx &coin = mapWallet[txin.prevout.hash];
//				coin.BindWallet(this);
//				NotifyTransactionChanged(this, coin.GetHash(), CT_UPDATED);
//			}
//
//			if (fFileBacked)
//				delete pwalletdb;
//		}

	// Broadcast
	{
		CValidationState state;
		if (!::AcceptToMemoryPool(mempool, state, pTx, true, NULL)) {
			// This must not fail. The transaction has already been signed and recorded.
			LogPrint("INFO", "CommitTransaction() : Error: Transaction not valid\n");
			return false;
		}
	}
	uint256 txhash = pTx->GetHash();
	UnConfirmTx[txhash] = pTx->GetNewInstance();
	::RelayTransaction(pTx, txhash);
	return FushToDisk();
}

DBErrors CWallet::LoadWallet(bool fFirstRunRet) {
	if (!fFileBacked)
		return DB_LOAD_OK;

//	Object reply;
//	reply.push_back(Pair("created by Soypay", CLIENT_BUILD + CLIENT_DATE));
//	reply.push_back(Pair("Created Time ", EncodeDumpTime(GetTime())));
//	reply.push_back(Pair("Best block index hight ", chainActive.Height()));
//	reply.push_back(Pair("Best block hash ", chainActive.Tip()->GetBlockHash().ToString()));
//
//	LOCK2(cs_main, pwalletMain->cs_wallet);
//	map<CKeyID, CKeyStoreValue> tepmKeyPool = pwalletMain->GetKeyPool();
//	int index = 0;
//	for (auto &te : tepmKeyPool) {
//		reply.push_back(Pair(strprintf("index%d",index++), te.second.ToString()));
//	}

//	LOCK2(cs_main, pwalletMain->cs_wallet);
	   filesystem::path blocksDir = GetDataDir() / strWalletFile;
//	   FILE* fp = fopen(blocksDir.string().c_str(), "wb+");
	   FILE* fp = fopen(blocksDir.string().c_str(), "rb");
	 
	   if (!fp) throw "Cannot open wallet dump file";



    CAutoFile filein = CAutoFile(fp, SER_DISK, CLIENT_VERSION);
    if (!filein)
    	throw "Cannot open wallet dump file";


	try {
		  filein >>  *this;
	} catch (...) {
		cout<< "sesail failed !"<< endl;
	}


	filein.fclose();
//
////	fFirstRunRet = false;
//	{
////		DBErrors nLoadWalletRet =
//				CWalletDB(strWalletFile, "cr+").LoadWallet(*this);
////		if (nLoadWalletRet == DB_NEED_REWRITE) {
////			if (CDB::Rewrite(strWalletFile, "\x04pool")) {
////				LOCK(cs_wallet);
//////				setKeyPool.clear();
////				// Note: can't top-up keypool here, because wallet is locked.
////				// User will be prompted to unlock wallet the next operation
////				// the requires a new key.
////			}
////		}
//
////		if (nLoadWalletRet != DB_LOAD_OK)
////			return nLoadWalletRet;
//	}
////	fFirstRunRet = !vchDefaultKey.IsValid();
////
////	uiInterface.LoadWallet(this);
////
////	DelInvalidRegID();
	return DB_LOAD_OK;
}

DBErrors CWallet::ZapWalletTx() {
	if (!fFileBacked)
		return DB_LOAD_OK;
	DBErrors nZapWalletTxRet = CWalletDB(strWalletFile, "cr+").ZapWalletTx(this);
	if (nZapWalletTxRet == DB_NEED_REWRITE) {
		if (CDB::Rewrite(strWalletFile, "\x04pool")) {
			LOCK(cs_wallet);
//			setKeyPool.clear();
			// Note: can't top-up keypool here, because wallet is locked.
			// User will be prompted to unlock wallet the next operation
			// the requires a new key.
		}
	}

	if (nZapWalletTxRet != DB_LOAD_OK)
		return nZapWalletTxRet;

	return DB_LOAD_OK;
}


/***********************************creat tx*********************************************/


int64_t CWallet::GetBalance(int ncurhigh) const
{
	int64_t ret = 0;
	CAccountViewCache accView(*pAccountViewTip, true);
	{
		LOCK2(cs_main, cs_wallet);
		for(auto &te :mKeyPool)
		{
			ret +=accView.GetBalance(te.first,ncurhigh);
		}

	}
	return ret;
}


std::string CWallet::SendMoney(CRegID &send, CRegID &rsv, int64_t nValue)
{
//	if (IsLocked())
//	{
//		return _("Error: Wallet locked, unable to create transaction!");
//	}

	CTransaction tx;
	{
		LOCK2(cs_main, cs_wallet);
		tx.srcUserId = send;
		tx.desUserId = rsv;
		tx.llValues = nValue;
		tx.llFees = nTransactionFee;
		tx.nValidHeight = chainActive.Tip()->nHeight;
	}

	CKey key;
	CKeyID keID;
	if(!pAccountViewTip->GetKeyId(send,keID) ||
			!GetKey(keID, key))
	{
		return _("key or keID failed");
	}

	if (!key.Sign(tx.SignatureHash(), tx.signature)) {
		return _("Sign failed");
	}

	if (!CommitTransaction((CBaseTransaction *) &tx)) {
		return _("CommitTransaction failed");
	}
	return tx.GetHash().GetHex();

}

/****************************************************************************************/


void CWallet::UpdatedTransaction(const uint256 &hashTx) {
	{
		LOCK(cs_wallet);
		// Only notify UI if this transaction is in this wallet
//		map<uint256, CWalletTx>::const_iterator mi = mapWallet.find(hashTx);
//		if (mi != mapWallet.end())
		NotifyTransactionChanged(this, hashTx, CT_UPDATED);
	}
}

bool CWallet::StartUp() {
//	[](int i) { return i+4; };
	 auto InitError = [] (const string &str)
	{
        uiInterface.ThreadSafeMessageBox(str, "", CClientUIInterface::MSG_WARNING | CClientUIInterface::NOSHOWGUI);
	    return true;
	};

	 defaultFilename = SysCfg().GetArg("-wallet", "wallet.dat");
	  bool fDisableWallet = SysCfg().GetBoolArg("-disablewallet", false);
    string strDataDir = GetDataDir().string();

	    // Wallet file must be a plain filename without a directory
	    if (defaultFilename != boost::filesystem::basename(defaultFilename) + boost::filesystem::extension(defaultFilename))
	        return InitError(strprintf(("Wallet %s resides outside data directory %s"), defaultFilename, strDataDir));
//
////	 if (!fDisableWallet) {
////	        LogPrint("INFO","Using wallet %s\n", defaultFilename);
////	        uiInterface.InitMessage(("Verifying wallet..."));
////
//	        if (!bitdb.Open(GetDataDir()))
//	        {
//	            // try moving the database env out of the way
//	            boost::filesystem::path pathDatabase = GetDataDir() / "database";
//	            boost::filesystem::path pathDatabaseBak = GetDataDir() / strprintf("database.%d.bak", GetTime());
//	            try {
//	                boost::filesystem::rename(pathDatabase, pathDatabaseBak);
//	                LogPrint("INFO","Moved old %s to %s. Retrying.\n", pathDatabase.string(), pathDatabaseBak.string());
//	            } catch(boost::filesystem::filesystem_error &error) {
//	                 // failure is ok (well, not really, but it's not worse than what we started with)
//	            }
//
//	            // try again
//	            if (!bitdb.Open(GetDataDir())) {
//	                // if it still fails, it probably means we can't even create the database env
//	                string msg = strprintf(("Error initializing wallet database environment %s!"), strDataDir);
//	                return InitError(msg);
//	            }
//	        }
////
//	        if (GetBoolArg("-salvagewallet", false))
//	        {
//	            // Recover readable keypairs:
//	            if (!CWalletDB::Recover(bitdb, defaultFilename, true))
//	                return false;
//	        }
//
//	        if (filesystem::exists(GetDataDir() / defaultFilename))
//	        {
//	            CDBEnv::VerifyResult r = bitdb.Verify(defaultFilename, CWalletDB::Recover);
//	            if (r == CDBEnv::RECOVER_OK)
//	            {
//	                string msg = strprintf(_("Warning: wallet.dat corrupt, data salvaged!"
//	                                         " Original wallet.dat saved as wallet.{timestamp}.bak in %s; if"
//	                                         " your balance or transactions are incorrect you should"
//	                                         " restore from a backup."), strDataDir);
//	                InitError(msg);
//	            }
//	            if (r == CDBEnv::RECOVER_FAIL)
//	                return InitError(_("wallet.dat corrupt, salvage failed"));
//	        }
////	    } // (!fDisableWallet)
  return true;
	}


CWallet* CWallet::getinstance() {

	if(StartUp())
	{
		return new CWallet(defaultFilename);
	}
	assert(0);
	return NULL;

}

Object CAccountTx::ToJosnObj() const {

	Object obj;
	obj.push_back(Pair("blockHash",  blockHash.ToString()));
	obj.push_back(Pair("blockhigh",  blockhigh));
	Array Tx;
	CAccountViewCache view(*pAccountViewTip);
	for(auto const &re:mapAccountTx)
	{
	  Tx.push_back(re.second.get()->ToString(view));
	}
	obj.push_back(Pair("Tx",  Tx));

	return obj;
}

uint256 CWallet::GetCheckSum() const {
	{
			CHashWriter ss(SER_GETHASH, CLIENT_VERSION);
			ss << nWalletVersion << bestBlock << MasterKey << mKeyPool << mapInBlockTx;
			return ss.GetHash();
		}
}
