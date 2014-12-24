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
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "json/json_spirit_value.h"
#include "json/json_spirit_writer_template.h"
#include <boost/algorithm/string/replace.hpp>
#include <openssl/rand.h>
using namespace json_spirit;
using namespace boost::assign;

using namespace std;
using namespace boost;




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
	if (mKeyPool.count(tem.GetCKeyID()) > 0) {
		LogPrint("CWallet", "this key is in the CWallet");
		return false;
	}
	mKeyPool[tem.GetCKeyID()] = tem;
	return db.WriteKeyStoreValue(tem.GetCKeyID(),tem);
}

bool CWallet::FushToDisk() const {

}
bool CWallet::AddKey(const CKey& secret) {
	AssertLockHeld(cs_wallet);

	CKeyStoreValue tem(secret);
	if(mKeyPool.count(tem.GetCKeyID()) > 0)
		{
		  LogPrint("CWallet","this key is in the CWallet");
		 return false;
		}

	if (!IsCrypted()) {
		mKeyPool[tem.GetCKeyID()] = tem;
		return db.WriteKeyStoreValue(tem.GetCKeyID(),tem);
	}
	else
	{
		assert(0);//to add code
	}
	return true;
}
bool CWallet::AddKey(const CKeyStoreValue& storeValue) {
	CPubKey Pk;
	if (!storeValue.GetPubKey(Pk)) {
		return false;
	}
	if (!IsCrypted()) {
		mKeyPool[Pk.GetKeyID()] = storeValue;
		return db.WriteKeyStoreValue(Pk.GetKeyID(),storeValue);
	} else {
		assert(0 && "fix me");
	}
	return false;
}

bool CWallet::AddKey(const CKey& secret,const CKey& minerKey) {
	AssertLockHeld(cs_wallet);

	CKeyStoreValue tem(secret,minerKey);
	if(mKeyPool.count(tem.GetCKeyID()) > 0)	{
		  LogPrint("CWallet","this key is in the CWallet");
		 return false;
		}

	if (!IsCrypted()) {
		mKeyPool[tem.GetCKeyID()] = tem;
		return db.WriteKeyStoreValue(tem.GetCKeyID(),tem);
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
////				db.WriteMasterKey(pMasterKey.first, pMasterKey.second);
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
//		db.WriteOrderPosNext(nOrderPosNext);
//	}
//	return nRet;
//}



void CWallet::SyncTransaction(const uint256 &hash, CBaseTransaction*pTx, const CBlock* pblock) {

	LOCK2(cs_main, cs_wallet);

	assert(pTx != NULL || pblock != NULL);
	bool fIsNeedUpDataRegID = false;
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
//						vector<CAccountOperLog> Log;
//						if (GetTxOperLog(hashtx, Log) == true) {
//							assert(newtx.AddOperLog(hashtx, Log));
//						} else {
//							ERROR("GetTxOperLog  error %s", hashtx.GetHex());
//						}
					}
					newtx.AddTx(hashtx,sptx.get());

				}
				if(UnConfirmTx.count(hashtx)> 0){
					db.EraseUnComFirmedTx(hashtx);
					UnConfirmTx.erase(hashtx);
				}
			}
			if (newtx.GetTxSize() > 0 ) //write to disk
			{
				mapInBlockTx[blockhash] = newtx; //add to map
				newtx.WriteToDisk();
			}
		};
		auto DisConnectBlockProgress = [&]() {
//			CAccountTx Oldtx(this, blockhash);
			int i = 0 ;
			int index = pblock->nHeight;
			for (const auto &sptx : pblock->vptx) {
				if(sptx->IsCoinBase()){
					continue;
				}
				CRegID regid(index, i++);
				if(IsMine(sptx.get())) {
					if (sptx->nTxType == REG_ACCT_TX) {
						for (auto &te : mKeyPool) {
								if(te.second.GetRegID() == regid){
									mKeyPool.erase(te.first);
									break;
								}
							}
					}
					UnConfirmTx[sptx.get()->GetHash()] = sptx.get()->GetNewInstance();
					db.WriteUnComFirmedTx(sptx.get()->GetHash(),UnConfirmTx[sptx.get()->GetHash()]);

				}
//				Oldtx.AddTx(sptx->GetHash(), sptx.get());

			}
//			Oldtx.AcceptToMemoryPool(); //add those tx to mempool
			if (mapInBlockTx.count(blockhash)) {
				db.EraseBlockTx(blockhash);
				mapInBlockTx.erase(blockhash);

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

}

//void CWallet::EraseFromWallet(const uint256 &hash) {
//	if (!fFileBacked)
//		return;
//	{
//		LOCK(cs_wallet);
//		if (mapWalletTx.erase(hash))
//			db.EraseAccountTx(hash);
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
	vector<uint256> erase;
	for (auto &te : UnConfirmTx) {
		std::shared_ptr<CBaseTransaction> pBaseTx = te.second->GetNewInstance();
		auto ret = CommitTransaction(&(*pBaseTx.get()));
		if (!std::get<0>(ret)) {
			erase.push_back(te.first);
		}
	}
	for (auto const & tee : erase) {
		db.EraseUnComFirmedTx(tee);
		UnConfirmTx.erase(tee);
	}
}

//// Call after CreateTransaction unless you want to abort
std::tuple<bool, string> CWallet::CommitTransaction(CBaseTransaction *pTx) {
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
			LogPrint("INFO", "CommitTransaction() : Error: Transaction not valid %s \n",state.GetRejectReason());
			return std::make_tuple (false,state.GetRejectReason());

		}
	}
	uint256 txhash = pTx->GetHash();
	UnConfirmTx[txhash] = pTx->GetNewInstance();
	bool flag =  db.WriteUnComFirmedTx(txhash,UnConfirmTx[txhash]);
	::RelayTransaction(pTx, txhash);
	return std::make_tuple (flag,txhash.ToString());

}

DBErrors CWallet::LoadWallet(bool fFirstRunRet) {
	  fFirstRunRet = false;
	  return db.LoadWallet(this);

}


int64_t CWallet::GetRawBalance(int ncurhigh) const
{
	int64_t ret = 0;
	CAccountViewCache accView(*pAccountViewTip, true);
	{
		LOCK2(cs_main, cs_wallet);
		for(auto &te :mKeyPool)
		{
			ret +=accView.GetRawBalance(te.first,ncurhigh);
		}

	}
	return ret;
}


std::tuple<bool,string>  CWallet::SendMoney(const CRegID &send, const CUserID &rsv, int64_t nValue, int64_t nFee)
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
		if (0 == nFee) {
			tx.llFees = SysCfg().GetTxFee();
		}else
			tx.llFees = nFee;
		tx.nValidHeight = chainActive.Tip()->nHeight;
	}

	CKeyID keID;
	if(!pAccountViewTip->GetKeyId(send,keID)){
		return std::make_tuple (false,"key or keID failed");
	}

	if (!Sign(keID,tx.SignatureHash(), tx.signature)) {
		return std::make_tuple (false,"Sign failed");
	}
	std::tuple<bool,string> ret = CommitTransaction((CBaseTransaction *) &tx);
	if(!std::get<0>(ret))
		return ret;
	return  std::make_tuple (true,tx.GetHash().GetHex());


}



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

	 defaultFilename = SysCfg().GetArg("-wallet", "wallet");
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

Object CAccountTx::ToJosnObj(CKeyID const  &key) const {

	Object obj;
	obj.push_back(Pair("blockHash",  blockHash.ToString()));
	obj.push_back(Pair("blockhigh",  blockhigh));
	Array Tx;
	CAccountViewCache view(*pAccountViewTip, true);
	for (auto const &re : mapAccountTx) {
		if (!key.IsEmpty()) {
			auto find = mapOperLog.find(re.first);
			if (find != mapOperLog.end()) {
				vector<CAccountOperLog>  rep = find->second;
				for (auto &te : rep) {
					if (te.keyID == key) {
						Tx.push_back(re.second.get()->ToString(view));
					}
				}
			}
		} else			//default add all tx to obj
		{
			Tx.push_back(re.second.get()->ToString(view));
		}
	}
	obj.push_back(Pair("Tx", Tx));

	return obj;
}

uint256 CWallet::GetCheckSum() const {
	CHashWriter ss(SER_GETHASH, CLIENT_VERSION);
	ss << nWalletVersion << bestBlock << MasterKey << mKeyPool << mapInBlockTx;
	return ss.GetHash();
}

bool CWallet::GetRegId(const CUserID& address, CRegID& IdOut) const  {
	AssertLockHeld(cs_wallet);
	if (address.type() == typeid(CRegID)) {
		IdOut = boost::get<CRegID>(address);
		return true;
	} else if (address.type() == typeid(CKeyID)) {
		CKeyID te = boost::get<CKeyID>(address);
		if (count(te)) {
			auto tep = mKeyPool.find(te);
			if (tep != mKeyPool.end()) {
				IdOut = tep->second.GetRegID();
				return true;
			}
		}

	} else {
		assert(0 && "to fixme");
	}

	return false;
}

bool CWallet::GetKey(const CUserID& address, CKey& keyOut,bool IsMiner) const{
	AssertLockHeld(cs_wallet);
	CAccountViewCache dumy(*pAccountViewTip,true);
	CKeyID keyid;
	if (dumy.GetKeyId(address, keyid)) {
		if (mKeyPool.count(keyid)) {
			auto tep = mKeyPool.find(keyid);
			if (tep != mKeyPool.end())
				return tep->second.getCKey(keyOut, IsMiner);
		}
	}
	return false;
}

bool CWallet::GetKey(const CKeyID& address, CKey& keyOut, bool IsMiner) const {
	AssertLockHeld(cs_wallet);
	if (mKeyPool.count(address)) {
		auto tep = mKeyPool.find(address);
		if(tep != mKeyPool.end())
		return tep->second.getCKey(keyOut,IsMiner);
	}
	return false;
}

bool CWallet::GetPubKey(const CKeyID& address, CPubKey& keyOut, bool IsMiner) {
	AssertLockHeld(cs_wallet);
	if (mKeyPool.count(address)) {
		return mKeyPool[address].GetPubKey(keyOut,IsMiner);
	}
	return false;

}

bool CWallet::SynchronizRegId(const CKeyID& keyid, const CAccountViewCache& inview) {
	CAccountViewCache view(inview);
	if(count(keyid)> 0)
	{
		if(mKeyPool[keyid].SynchronizSys(view))
			{
			return db.WriteKeyStoreValue(keyid,mKeyPool[keyid]);
			}
	}
	return false;
}

bool CWallet::IsMine(CBaseTransaction* pTx) const{

	set<CKeyID> vaddr;
	CAccountViewCache view(*pAccountViewTip, true);
	if (!pTx->GetAddress(vaddr, view)) {
		return false;
	}
	for (auto &keyid : vaddr) {
		if (count(keyid) > 0) {
			return true;
		}
	}
	return false;
}

bool CWallet::SynchronizSys(const CAccountViewCache& inview) {
	CAccountViewCache view(const_cast<CAccountViewCache &>(inview), true);
	for (auto &te : mKeyPool) {
		te.second.SynchronizSys(view);
	}
	return true;
}

bool CWallet::GetKeyIds(set<CKeyID>& setKeyID,bool IsMiner) const {
	AssertLockHeld(cs_wallet);
	setKeyID.clear();
	for (auto const & tem : mKeyPool) {
		if (IsMiner == false) {
			setKeyID.insert(tem.first);
		} else if (!tem.second.GetRegID().IsEmpty()) {			//only the reged key is useful fo miner
			setKeyID.insert(tem.first);
		}
	}
	return setKeyID.size() > 0;
}

bool CWallet::CleanAll() {

	for_each(UnConfirmTx.begin(), UnConfirmTx.end(),
			[&](std::map<uint256, std::shared_ptr<CBaseTransaction> >::reference a) {
				db.EraseUnComFirmedTx(a.first);
			});
	UnConfirmTx.clear();

	for_each(mapInBlockTx.begin(), mapInBlockTx.end(), [&](std::map<uint256, CAccountTx >::reference a) {
		db.EraseUnComFirmedTx(a.first);
	});
	mapInBlockTx.clear();

	bestBlock.SetNull();

	for_each(mKeyPool.begin(), mKeyPool.end(), [&](std::map<CKeyID, CKeyStoreValue >::reference a) {
		db.EraseKeyStoreValue(a.first);
	});
	mKeyPool.clear();
	db.EraseMasterKey();
	MasterKey.SetNull();
	return true;
}

bool CWallet::Sign(const CUserID& Userid, const uint256& hash, vector<unsigned char> &signature,bool IsMiner)const {
	CKey key;
	if(GetKey(Userid, key,IsMiner)){
//		if(IsMiner == true)
//		{
//			cout <<"Sign miner key PubKey"<< key.GetPubKey().ToString()<< endl;
//			cout <<"Sign miner hash"<< hash.ToString()<< endl;
//			cout <<"Sign user Id" << boost::get<CKeyID>(Userid).ToString() << endl;
//		}
	return(key.Sign(hash, signature));
	}
	return false;

}

Object CKeyStoreValue::ToJsonObj()const {
	Object reply;
	reply.push_back(Pair("mregId",mregId.ToString()));
	reply.push_back(Pair("mPKey",mPKey.ToString()));
	reply.push_back(Pair("mCkey",mCkey.ToString()));
	reply.push_back(Pair("mMinerCkey",mMinerCkey.ToString()));
	reply.push_back(Pair("nCreationTime",nCreationTime));
    return std::move(reply);
}
bool CKeyStoreValue::UnSersailFromJson(const Object& obj){
	try {
		Object reply;
		mregId = (find_value(obj, "mregId").get_str());
		mPKey= ::ParseHex(find_value(obj, "mPKey").get_str());
		auto const &tem1 = ::ParseHex(find_value(obj, "mCkey").get_str());
		mCkey.Set(tem1.begin(),tem1.end(),true);
		auto const &tem2=::ParseHex(find_value(obj, "mMinerCkey").get_str());
		mMinerCkey.Set(tem2.begin(),tem2.end(),true);
		nCreationTime =find_value(obj, "nCreationTime").get_int64();
		assert(mCkey.GetPubKey() == mPKey);
	} catch (...) {
		ERROR("UnSersailFromJson Failed !");
		return false;
	}

    return true;
}

bool CKeyStoreValue::SelfCheck()const {
  if(mCkey.IsValid())
  {
	  if(mCkey.GetPubKey() != mPKey)
	  {
		  return false;
	  }
  }
  if(!mPKey.IsValid())
  {
	  return false;
  }
  return true;
}

bool CKeyStoreValue::SynchronizSys(CAccountViewCache& view){
	 CAccount account;
	if(!view.GetAccount(CUserID(mPKey.GetKeyID()),account))
	{
		mregId.clean();
	}
	else if(account.PublicKey.IsValid())//是注册的账户
	{
		mregId = account.regID;
		if(account.PublicKey != mPKey)
			{
				ERROR("shit %s acc %s mPKey:%s\r\n","not fix the bug",account.ToString(),this->ToString());
				assert(0);
			}
		if(account.MinerPKey.IsValid())
			assert(account.MinerPKey == mMinerCkey.GetPubKey());
	}
	else//有可能是没有注册的账户
	{
		mregId.clean();
	}

	LogPrint("wallet","%s \r\n",this->ToString());
	return true;
}
