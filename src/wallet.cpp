// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txdb.h"
#include "wallet.h"

#include "base58.h"
#include "checkpoints.h"
#include "coincontrol.h"
#include "net.h"

#include <boost/algorithm/string/replace.hpp>
#include <openssl/rand.h>
using namespace std;

// Settings
int64_t nTransactionFee = DEFAULT_TRANSACTION_FEE;
bool bSpendZeroConfChange = true;

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

const CAccountTx* CWallet::GetAccountTx(const uint256& hash) const {
	LOCK(cs_wallet);
	map<uint256, CAccountTx>::const_iterator it = mapWalletTx.find(hash);
	if (it == mapWalletTx.end())
		return NULL;
	return &(it->second);
}

bool CWallet::GetTx(const uint256& hash,shared_ptr<CBaseTransaction> &tx) const{
	LOCK(cs_wallet);
	for(auto &wtx: mapWalletTx) {
		const CAccountTx &acctx = wtx.second;
		for(auto &item: acctx.mapAccountTx) {
			if (item.first == hash) {
				tx = item.second;
				return true;
			}
		}
	}
	return false;
}
CPubKey CWallet::GenerateNewKey() {
	AssertLockHeld(cs_wallet); // mapKeyMetadata
	bool fCompressed = CanSupportFeature(FEATURE_COMPRPUBKEY); // default to compressed public keys if we want 0.6.0 wallets

	RandAddSeedPerfmon();
	CKey secret;
	secret.MakeNewKey(fCompressed);

	// Compressed public keys were introduced in version 0.6.0
	if (fCompressed)
		SetMinVersion(FEATURE_COMPRPUBKEY);

	CPubKey pubkey = secret.GetPubKey();

	// Create new metadata
	int64_t nCreationTime = GetTime();
	mapKeyMetadata[pubkey.GetID()] = CKeyMetadata(nCreationTime);
	if (!nTimeFirstKey || nCreationTime < nTimeFirstKey)
		nTimeFirstKey = nCreationTime;

	if (!AddKeyPubKey(secret, pubkey))
		throw runtime_error("CWallet::GenerateNewKey() : AddKey failed");
	return pubkey;
}

bool CWallet::AddKeyPubKey(const CKey& secret, const CPubKey &pubkey) {
	AssertLockHeld(cs_wallet); // mapKeyMetadata
	if (!CCryptoKeyStore::AddKeyPubKey(secret, pubkey))
		return false;
	if (!fFileBacked)
		return true;
	if (!IsCrypted()) {
		return CWalletDB(strWalletFile).WriteKey(pubkey, secret.GetPrivKey(), mapKeyMetadata[pubkey.GetID()]);
	}
	return true;
}

bool CWallet::AddCryptedKey(const CPubKey &vchPubKey, const vector<unsigned char> &vchCryptedSecret) {
	if (!CCryptoKeyStore::AddCryptedKey(vchPubKey, vchCryptedSecret))
		return false;
	if (!fFileBacked)
		return true;
	{
		LOCK(cs_wallet);
		if (pwalletdbEncryption)
			return pwalletdbEncryption->WriteCryptedKey(vchPubKey, vchCryptedSecret, mapKeyMetadata[vchPubKey.GetID()]);
		else
			return CWalletDB(strWalletFile).WriteCryptedKey(vchPubKey, vchCryptedSecret,
					mapKeyMetadata[vchPubKey.GetID()]);
	}
	return false;
}

bool CWallet::LoadKeyMetadata(const CPubKey &pubkey, const CKeyMetadata &meta) {
	AssertLockHeld(cs_wallet); // mapKeyMetadata
	if (meta.nCreateTime && (!nTimeFirstKey || meta.nCreateTime < nTimeFirstKey))
		nTimeFirstKey = meta.nCreateTime;

	mapKeyMetadata[pubkey.GetID()] = meta;
	return true;
}

bool CWallet::LoadCryptedKey(const CPubKey &vchPubKey, const vector<unsigned char> &vchCryptedSecret) {
	return CCryptoKeyStore::AddCryptedKey(vchPubKey, vchCryptedSecret);
}

bool CWallet::AddCScript(const CScript& redeemScript) {
	if (!CCryptoKeyStore::AddCScript(redeemScript))
		return false;
	if (!fFileBacked)
		return true;
	return CWalletDB(strWalletFile).WriteCScript(Hash160(redeemScript), redeemScript);
}

bool CWallet::Unlock(const SecureString& strWalletPassphrase) {
	CCrypter crypter;
	CKeyingMaterial vMasterKey;

	{
		LOCK(cs_wallet);
		for (const auto& pMasterKey : mapMasterKeys) {
			if (!crypter.SetKeyFromPassphrase(strWalletPassphrase, pMasterKey.second.vchSalt,
					pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
				return false;
			if (!crypter.Decrypt(pMasterKey.second.vchCryptedKey, vMasterKey))
				continue; // try another master key
			if (CCryptoKeyStore::Unlock(vMasterKey))
				return true;
		}
	}
	return false;
}

bool CWallet::ChangeWalletPassphrase(const SecureString& strOldWalletPassphrase,
		const SecureString& strNewWalletPassphrase) {
	bool fWasLocked = IsLocked();

	{
		LOCK(cs_wallet);
		Lock();

		CCrypter crypter;
		CKeyingMaterial vMasterKey;
		for (auto& pMasterKey : mapMasterKeys) {
			if (!crypter.SetKeyFromPassphrase(strOldWalletPassphrase, pMasterKey.second.vchSalt,
					pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
				return false;
			if (!crypter.Decrypt(pMasterKey.second.vchCryptedKey, vMasterKey))
				return false;
			if (CCryptoKeyStore::Unlock(vMasterKey)) {
				int64_t nStartTime = GetTimeMillis();
				crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt,
						pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod);
				pMasterKey.second.nDeriveIterations = pMasterKey.second.nDeriveIterations
						* (100 / ((double) (GetTimeMillis() - nStartTime)));

				nStartTime = GetTimeMillis();
				crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt,
						pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod);
				pMasterKey.second.nDeriveIterations = (pMasterKey.second.nDeriveIterations
						+ pMasterKey.second.nDeriveIterations * 100 / ((double) (GetTimeMillis() - nStartTime))) / 2;

				if (pMasterKey.second.nDeriveIterations < 25000)
					pMasterKey.second.nDeriveIterations = 25000;

				LogPrint("INFO","Wallet passphrase changed to an nDeriveIterations of %i\n",
						pMasterKey.second.nDeriveIterations);

				if (!crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt,
						pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
					return false;
				if (!crypter.Encrypt(vMasterKey, pMasterKey.second.vchCryptedKey))
					return false;
				CWalletDB(strWalletFile).WriteMasterKey(pMasterKey.first, pMasterKey.second);
				if (fWasLocked)
					Lock();
				return true;
			}
		}
	}

	return false;
}

void CWallet::SetBestChain(const CBlockLocator& loc) {
	CWalletDB walletdb(strWalletFile);
	walletdb.WriteBestBlock(loc);
}

bool CWallet::SetMinVersion(enum WalletFeature nVersion, CWalletDB* pwalletdbIn, bool fExplicit) {
	LOCK(cs_wallet); // nWalletVersion
	if (nWalletVersion >= nVersion)
		return true;

	// when doing an explicit upgrade, if we pass the max version permitted, upgrade all the way
	if (fExplicit && nVersion > nWalletMaxVersion)
		nVersion = FEATURE_LATEST;

	nWalletVersion = nVersion;

	if (nVersion > nWalletMaxVersion)
		nWalletMaxVersion = nVersion;

	if (fFileBacked) {
		CWalletDB* pwalletdb = pwalletdbIn ? pwalletdbIn : new CWalletDB(strWalletFile);
		if (nWalletVersion > 40000)
			pwalletdb->WriteMinVersion(nWalletVersion);
		if (!pwalletdbIn)
			delete pwalletdb;
	}

	return true;
}

bool CWallet::SetMaxVersion(int nVersion) {
	LOCK(cs_wallet); // nWalletVersion, nWalletMaxVersion
	// cannot downgrade below current version
	if (nWalletVersion > nVersion)
		return false;

	nWalletMaxVersion = nVersion;

	return true;
}

bool CWallet::EncryptWallet(const SecureString& strWalletPassphrase) {
	if (IsCrypted())
		return false;

	CKeyingMaterial vMasterKey;
	RandAddSeedPerfmon();

	vMasterKey.resize(WALLET_CRYPTO_KEY_SIZE);
	RAND_bytes(&vMasterKey[0], WALLET_CRYPTO_KEY_SIZE);

	CMasterKey kMasterKey;

	RandAddSeedPerfmon();
	kMasterKey.vchSalt.resize(WALLET_CRYPTO_SALT_SIZE);
	RAND_bytes(&kMasterKey.vchSalt[0], WALLET_CRYPTO_SALT_SIZE);

	CCrypter crypter;
	int64_t nStartTime = GetTimeMillis();
	crypter.SetKeyFromPassphrase(strWalletPassphrase, kMasterKey.vchSalt, 25000, kMasterKey.nDerivationMethod);
	kMasterKey.nDeriveIterations = 2500000 / ((double) (GetTimeMillis() - nStartTime));

	nStartTime = GetTimeMillis();
	crypter.SetKeyFromPassphrase(strWalletPassphrase, kMasterKey.vchSalt, kMasterKey.nDeriveIterations,
			kMasterKey.nDerivationMethod);
	kMasterKey.nDeriveIterations = (kMasterKey.nDeriveIterations
			+ kMasterKey.nDeriveIterations * 100 / ((double) (GetTimeMillis() - nStartTime))) / 2;

	if (kMasterKey.nDeriveIterations < 25000)
		kMasterKey.nDeriveIterations = 25000;

	LogPrint("INFO","Encrypting Wallet with an nDeriveIterations of %i\n", kMasterKey.nDeriveIterations);

	if (!crypter.SetKeyFromPassphrase(strWalletPassphrase, kMasterKey.vchSalt, kMasterKey.nDeriveIterations,
			kMasterKey.nDerivationMethod))
		return false;
	if (!crypter.Encrypt(vMasterKey, kMasterKey.vchCryptedKey))
		return false;

	{
		LOCK(cs_wallet);
		mapMasterKeys[++nMasterKeyMaxID] = kMasterKey;
		if (fFileBacked) {
			pwalletdbEncryption = new CWalletDB(strWalletFile);
			if (!pwalletdbEncryption->TxnBegin())
				return false;
			pwalletdbEncryption->WriteMasterKey(nMasterKeyMaxID, kMasterKey);
		}

		if (!EncryptKeys(vMasterKey)) {
			if (fFileBacked)
				pwalletdbEncryption->TxnAbort();
			exit(1); //We now probably have half of our keys encrypted in memory, and half not...die and let the user reload their unencrypted wallet.
		}

		// Encryption was introduced in version 0.4.0
		SetMinVersion(FEATURE_WALLETCRYPT, pwalletdbEncryption, true);

		if (fFileBacked) {
			if (!pwalletdbEncryption->TxnCommit())
				exit(1); //We now have keys encrypted in memory, but no on disk...die to avoid confusion and let the user reload their unencrypted wallet.

			delete pwalletdbEncryption;
			pwalletdbEncryption = NULL;
		}

		Lock();
		Unlock(strWalletPassphrase);
		NewKeyPool();
		Lock();

		// Need to completely rewrite the wallet file; if we don't, bdb might keep
		// bits of the unencrypted private key in slack space in the database file.
		CDB::Rewrite(strWalletFile);

	}
	NotifyStatusChanged(this);

	return true;
}

int64_t CWallet::IncOrderPosNext(CWalletDB *pwalletdb) {
	AssertLockHeld(cs_wallet); // nOrderPosNext
	int64_t nRet = nOrderPosNext++;
	if (pwalletdb) {
		pwalletdb->WriteOrderPosNext(nOrderPosNext);
	} else {
		CWalletDB(strWalletFile).WriteOrderPosNext(nOrderPosNext);
	}
	return nRet;
}

bool CWallet::AddToWallet(const CAccountTx& accTx) {

	LOCK(cs_wallet);
	mapWalletTx[accTx.blockHash] = accTx;
	return true;
}

void CWallet::SyncTransaction(const uint256 &hash, const CBaseTransaction*pTx, const CBlock* pblock) {

	LOCK2(cs_main, cs_wallet);

	assert(pTx != NULL || pblock != NULL);

	CAccountTx &undotx = mapWalletTx[uint256(0)];
	undotx.BindWallet(this);

	bool bupdate = false; //unconfirmed tx update

	if (pblock != NULL) {

		uint256 blockhash = pblock->GetHash();

		if (Params().HashGenesisBlock() == blockhash) {
			unsigned short i = 0;
			for (const auto &sptx : pblock->vptx) {
				//confirm the tx is mine
				if (IsMine(sptx.get())) {
					CRewardTransaction* prtx = (CRewardTransaction*) sptx.get();
					CPubKey pubkey(prtx->account);
					CKeyID keyid = pubkey.GetID();
					CRegID regid(0, i);
					mapKeyRegID[keyid] = regid;
					{
						CWalletDB(strWalletFile).WriteRegID(keyid, regid);
					}
				}
				i++;
			}
		} else {
			if (mapBlockIndex.count(blockhash) && chainActive.Contains(mapBlockIndex[blockhash])) //connect block
					{
				bool bwrite = false;
				CAccountTx newtx(this, blockhash);
				unsigned short i = 0;
				for (const auto &sptx : pblock->vptx) {
					//confirm the tx is mine
					if (IsMine(sptx.get())) {

						uint256 hashtx = sptx->GetHash();

						if (sptx->nTxType == REG_ACCT_TX) {
							CKeyID keyid = ((CRegisterAccountTx*) sptx.get())->pubKey.GetID();
							CRegID regid(pblock->nHeight, i);
							mapKeyRegID[keyid] = regid;
							{
								CWalletDB(strWalletFile).WriteRegID(keyid, regid);
							}
						} else if (sptx->nTxType == REG_SCRIPT_TX) {
							CRegID regid(pblock->nHeight, i);
							mapScriptRegID.insert(make_pair(hashtx, regid));
							{
								CWalletDB(strWalletFile).WriteScriptRegID(hashtx, regid);
							}
						}

						newtx.AddTx(hashtx, sptx.get());
						//delete the same tx in unconfirmed tx
						if (undotx.DelTx(hashtx)) {
							bupdate = true;
						}

						bwrite = true;
					}
					i++;
				}

				if (bwrite) //write to disk
				{
					mapWalletTx[blockhash] = newtx; //add to map
					newtx.WriteToDisk();
				}
			} else //disconnect block
			{
				map<uint256, CAccountTx>::const_iterator it = mapWalletTx.find(blockhash);
				if (it != mapWalletTx.end()) {
					//move tx in block to unconfirmed tx,and delete it.
					const CAccountTx &newtx = it->second;
					for (const auto& item : newtx.mapAccountTx) {
						if (item.second->nTxType != REWARD_TX) {
							undotx.AddTx(item.first, item.second.get());
							bupdate = true;

							if (item.second->nTxType == REG_ACCT_TX) {
								CKeyID keyid = ((CRegisterAccountTx*) item.second.get())->pubKey.GetID();
								if (mapKeyRegID.erase(keyid)) {
									CWalletDB(strWalletFile).EraseRegID(keyid);
								}
							} else if (item.second->nTxType == REG_SCRIPT_TX) {
								if (mapScriptRegID.erase(item.first)) {
									CWalletDB(strWalletFile).EraseScriptRegID(item.first);
								}
							}
						}

					}
					EraseFromWallet(blockhash);
				}
			}
		}

	} else if (pTx != NULL) { //add unconfirmed tx

		if (IsMine(pTx) && !undotx.HaveTx(hash) && pTx->nTxType != REWARD_TX) {
			//if find the same tx,update it.
			undotx.AddTx(hash, pTx);
			bupdate = true;
		}
	}

	if (bupdate) {
		undotx.WriteToDisk();
	}
}

void CWallet::EraseFromWallet(const uint256 &hash) {
	if (!fFileBacked)
		return;
	{
		LOCK(cs_wallet);
		if (mapWalletTx.erase(hash))
			CWalletDB(strWalletFile).EraseAccountTx(hash);
	}
	return;
}
// Scan the block chain (starting in pindexStart) for transactions
// from or to us. If fUpdate is true, found transactions that already
// exist in the wallet will be updated.
int CWallet::ScanForWalletTransactions(CBlockIndex* pindexStart, bool fUpdate) {
	int ret = 0;
	int64_t nNow = GetTime();

	CBlockIndex* pindex = pindexStart;
	{
		LOCK2(cs_main, cs_wallet);

		// no need to read and scan block, if block was created before
		// our wallet birthday (as adjusted for block time variability)
		while (pindex && nTimeFirstKey && (pindex->nTime < (nTimeFirstKey - 7200)))
			pindex = chainActive.Next(pindex);

		ShowProgress(_("Rescanning..."), 0); // show rescan progress in GUI as dialog or on splashscreen, if -rescan on startup
		double dProgressStart = Checkpoints::GuessVerificationProgress(pindex, false);
		double dProgressTip = Checkpoints::GuessVerificationProgress(chainActive.Tip(), false);
		while (pindex) {
			if (pindex->nHeight % 100 == 0 && dProgressTip - dProgressStart > 0.0)
				ShowProgress(_("Rescanning..."),
						max(1,
								min(99,
										(int) ((Checkpoints::GuessVerificationProgress(pindex, false) - dProgressStart)
												/ (dProgressTip - dProgressStart) * 100))));

			CBlock block;
			ReadBlockFromDisk(block, pindex);

			SyncWithWallets(0, NULL, &block);

			pindex = chainActive.Next(pindex);
			if (GetTime() >= nNow + 60) {
				nNow = GetTime();
				LogPrint("INFO","Still rescanning. At block %d. Progress=%f\n", pindex->nHeight,
						Checkpoints::GuessVerificationProgress(pindex));
			}
		}
		ShowProgress(_("Rescanning..."), 100); // hide progress dialog in GUI
	}
	return ret;
}

void CWallet::ReacceptWalletTransactions() {
	LOCK2(cs_main, cs_wallet);

	mapWalletTx[uint256(0)].AcceptToMemoryPool();
}

void CWallet::ResendWalletTransactions() {
	// Do this infrequently and randomly to avoid giving away
	// that these are our transactions.
	if (GetTime() < nNextResend)
		return;
	bool fFirst = (nNextResend == 0);
	uint64_t rand = max(GetRand(30 * 60), (uint64_t) 120);
	nNextResend = GetTime() + rand;
	if (fFirst)
		return;

	// Only do it if there's been a new block since last time
	if (Params().GetBestRecvTime() < nLastResend)
		return;
	nLastResend = GetTime();

	// Rebroadcast any of our txes that aren't in a block yet
	LogPrint("INFO","ResendWalletTransactions()\n");
	{
		LOCK(cs_wallet);
		// Sort them in chronological order
//		multimap<unsigned int, CWalletTx*> mapSorted;
//		BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item, mapWallet) {
//			CWalletTx& wtx = item.second;
//			// Don't rebroadcast until it's had plenty of time that
//			// it should have gotten in already by now.
//			if (nTimeBestReceived - (int64_t) wtx.nTimeReceived > 5 * 60)
//				mapSorted.insert(make_pair(wtx.nTimeReceived, &wtx));
//		}
		mapWalletTx[uint256(0)].RelayWalletTransaction();
	}
}

//// Call after CreateTransaction unless you want to abort
bool CWallet::CommitTransaction(CBaseTransaction *pTx) {
	{
		LOCK2(cs_main, cs_wallet);
		LogPrint("INFO","CommitTransaction:\n%s", pTx->ToString(*pAccountViewTip));
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

		uint256 txhash = pTx->GetHash();

		// Track how many getdata requests our transaction gets
		mapRequestCount[txhash] = 0;

		// Broadcast
		{
			CValidationState state;
			if (!::AcceptToMemoryPool(mempool, state, pTx, true, NULL, true)) {
				// This must not fail. The transaction has already been signed and recorded.
				LogPrint("INFO","CommitTransaction() : Error: Transaction not valid\n");
				return false;
			}
		}
		::RelayTransaction(pTx, txhash);
	}
	return true;
}

void CWallet::DelInvalidRegID() {

	{
		vector<uint256> vhash;
		LOCK2(cs_main, cs_wallet);
		for (auto&wtx : mapWalletTx) {

			if (wtx.first != uint256(0)
					&& (!mapBlockIndex.count(wtx.first) || !chainActive.Contains(mapBlockIndex[wtx.first]))) {
				CAccountTx &acctx = wtx.second;
				for (auto&item : acctx.mapAccountTx) {
					if (item.second->nTxType == REG_ACCT_TX) {

						if (IsMine(item.second.get())) {
							CKeyID keyid = ((CRegisterAccountTx*) item.second.get())->pubKey.GetID();
							if (mapKeyRegID.erase(keyid)) {
								CWalletDB(strWalletFile).EraseRegID(keyid);
							}
						}
					} else if (item.second->nTxType == REG_SCRIPT_TX) {

						if (IsMine(item.second.get())) {
							if (mapScriptRegID.erase(item.first)) {
								CWalletDB(strWalletFile).EraseScriptRegID(item.first);
							}
						}
					}
				}
				vhash.push_back(wtx.first);
			}
		}

		for (auto hash : vhash) {
			EraseFromWallet(hash);
		}
	}

}
DBErrors CWallet::LoadWallet(bool& fFirstRunRet) {
	if (!fFileBacked)
		return DB_LOAD_OK;
	fFirstRunRet = false;
	{
		DBErrors nLoadWalletRet = CWalletDB(strWalletFile, "cr+").LoadWallet(this);
		if (nLoadWalletRet == DB_NEED_REWRITE) {
			if (CDB::Rewrite(strWalletFile, "\x04pool")) {
				LOCK(cs_wallet);
				setKeyPool.clear();
				// Note: can't top-up keypool here, because wallet is locked.
				// User will be prompted to unlock wallet the next operation
				// the requires a new key.
			}
		}

		if (nLoadWalletRet != DB_LOAD_OK)
			return nLoadWalletRet;
	}
	fFirstRunRet = !vchDefaultKey.IsValid();

	uiInterface.LoadWallet(this);

	DelInvalidRegID();
	return DB_LOAD_OK;
}

DBErrors CWallet::ZapWalletTx() {
	if (!fFileBacked)
		return DB_LOAD_OK;
	DBErrors nZapWalletTxRet = CWalletDB(strWalletFile, "cr+").ZapWalletTx(this);
	if (nZapWalletTxRet == DB_NEED_REWRITE) {
		if (CDB::Rewrite(strWalletFile, "\x04pool")) {
			LOCK(cs_wallet);
			setKeyPool.clear();
			// Note: can't top-up keypool here, because wallet is locked.
			// User will be prompted to unlock wallet the next operation
			// the requires a new key.
		}
	}

	if (nZapWalletTxRet != DB_LOAD_OK)
		return nZapWalletTxRet;

	return DB_LOAD_OK;
}

bool CWallet::SetAddressBook(const CTxDestination& address, const string& strName, const string& strPurpose) {
	bool fUpdated = false;
	{
		LOCK(cs_wallet); // mapAddressBook
		map<CTxDestination, CAddressBookData>::iterator mi = mapAddressBook.find(address);
		fUpdated = mi != mapAddressBook.end();
		mapAddressBook[address].name = strName;
		if (!strPurpose.empty()) /* update purpose only if requested */
			mapAddressBook[address].purpose = strPurpose;
	}
	NotifyAddressBookChanged(this, address, strName, ::IsMine(*this, address), strPurpose,
			(fUpdated ? CT_UPDATED : CT_NEW));
	if (!fFileBacked)
		return false;
	if (!strPurpose.empty() && !CWalletDB(strWalletFile).WritePurpose(CBitcoinAddress(address).ToString(), strPurpose))
		return false;
	return CWalletDB(strWalletFile).WriteName(CBitcoinAddress(address).ToString(), strName);
}

bool CWallet::DelAddressBook(const CTxDestination& address) {
	{
		LOCK(cs_wallet); // mapAddressBook

		if (fFileBacked) {
			// Delete destdata tuples associated with address
			string strAddress = CBitcoinAddress(address).ToString();
			for (const auto &item : mapAddressBook[address].destdata) {
				CWalletDB(strWalletFile).EraseDestData(strAddress, item.first);
			}
		}
		mapAddressBook.erase(address);
	}

	NotifyAddressBookChanged(this, address, "", ::IsMine(*this, address), "", CT_DELETED);

	if (!fFileBacked)
		return false;
	CWalletDB(strWalletFile).ErasePurpose(CBitcoinAddress(address).ToString());
	return CWalletDB(strWalletFile).EraseName(CBitcoinAddress(address).ToString());
}

bool CWallet::SetDefaultKey(const CPubKey &vchPubKey) {
	if (fFileBacked) {
		if (!CWalletDB(strWalletFile).WriteDefaultKey(vchPubKey))
			return false;
	}
	vchDefaultKey = vchPubKey;
	return true;
}

//
// Mark old keypool keys as used,
// and generate all new keys
//
bool CWallet::NewKeyPool() {
	{
		LOCK(cs_wallet);
		CWalletDB walletdb(strWalletFile);
		for (auto nIndex : setKeyPool)
			walletdb.ErasePool(nIndex);
		setKeyPool.clear();

		if (IsLocked())
			return false;

		int64_t nKeys = max(GetArg("-keypool", 100), (int64_t) 0);
		for (int i = 0; i < nKeys; i++) {
			int64_t nIndex = i + 1;
			walletdb.WritePool(nIndex, CKeyPool(GenerateNewKey()));
			setKeyPool.insert(nIndex);
		}
		LogPrint("INFO","CWallet::NewKeyPool wrote %d new keys\n", nKeys);
	}
	return true;
}

bool CWallet::TopUpKeyPool(unsigned int kpSize) {
	return false;
	{
		LOCK(cs_wallet);

		if (IsLocked())
			return false;

		CWalletDB walletdb(strWalletFile);

		// Top up key pool
		unsigned int nTargetSize;
		if (kpSize > 0)
			nTargetSize = kpSize;
		else
			nTargetSize = max(GetArg("-keypool", 100), (int64_t) 0);

		while (setKeyPool.size() < (nTargetSize + 1)) {
			int64_t nEnd = 1;
			if (!setKeyPool.empty())
				nEnd = *(--setKeyPool.end()) + 1;
			if (!walletdb.WritePool(nEnd, CKeyPool(GenerateNewKey())))
				throw runtime_error("TopUpKeyPool() : writing generated key failed");
			setKeyPool.insert(nEnd);
			LogPrint("INFO","keypool added key %d, size=%u\n", nEnd, setKeyPool.size());
		}
	}
	return true;
}

void CWallet::ReserveKeyFromKeyPool(int64_t& nIndex, CKeyPool& keypool) {
	nIndex = -1;
	keypool.vchPubKey = CPubKey();
	{
		LOCK(cs_wallet);

		if (!IsLocked())
			TopUpKeyPool();

		// Get the oldest key
		if (setKeyPool.empty())
			return;

		CWalletDB walletdb(strWalletFile);

		nIndex = *(setKeyPool.begin());
		setKeyPool.erase(setKeyPool.begin());
		if (!walletdb.ReadPool(nIndex, keypool))
			throw runtime_error("ReserveKeyFromKeyPool() : read failed");
		if (!HaveKey(keypool.vchPubKey.GetID()))
			throw runtime_error("ReserveKeyFromKeyPool() : unknown key in key pool");
		assert(keypool.vchPubKey.IsValid());
		LogPrint("INFO","keypool reserve %d\n", nIndex);
	}
}

int64_t CWallet::AddReserveKey(const CKeyPool& keypool) {
	{
		LOCK2(cs_main, cs_wallet);
		CWalletDB walletdb(strWalletFile);

		int64_t nIndex = 1 + *(--setKeyPool.end());
		if (!walletdb.WritePool(nIndex, keypool))
			throw runtime_error("AddReserveKey() : writing added key failed");
		setKeyPool.insert(nIndex);
		return nIndex;
	}
	return -1;
}

void CWallet::KeepKey(int64_t nIndex) {
	// Remove from key pool
	if (fFileBacked) {
		CWalletDB walletdb(strWalletFile);
		walletdb.ErasePool(nIndex);
	}
	LogPrint("INFO","keypool keep %d\n", nIndex);
}

void CWallet::ReturnKey(int64_t nIndex) {
	// Return to key pool
	{
		LOCK(cs_wallet);
		setKeyPool.insert(nIndex);
	}
	LogPrint("INFO","keypool return %d\n", nIndex);
}

bool CWallet::GetKeyFromPool(CPubKey& result) {
	int64_t nIndex = 0;
	CKeyPool keypool;
	{
		LOCK(cs_wallet);
		ReserveKeyFromKeyPool(nIndex, keypool);
		if (nIndex == -1) {
			if (IsLocked())
				return false;
			result = GenerateNewKey();
			return true;
		}
		KeepKey(nIndex);
		result = keypool.vchPubKey;
	}
	return true;
}

int64_t CWallet::GetOldestKeyPoolTime() {
	int64_t nIndex = 0;
	CKeyPool keypool;
	ReserveKeyFromKeyPool(nIndex, keypool);
	if (nIndex == -1)
		return GetTime();
	ReturnKey(nIndex);
	return keypool.nTime;
}

map<CTxDestination, int64_t> CWallet::GetAddressBalances() {
	map<CTxDestination, int64_t> balances;

	{
		LOCK(cs_wallet);
//		BOOST_FOREACH(PAIRTYPE(uint256, CWalletTx) walletEntry, mapWallet) {
//			CWalletTx *pcoin = &walletEntry.second;
//
//			if (!IsFinalTx(*pcoin) || !pcoin->IsTrusted())
//				continue;
//
//			if (pcoin->IsCoinBase() && pcoin->GetBlocksToMaturity() > 0)
//				continue;
//
//			int nDepth = pcoin->GetDepthInMainChain();
//			if (nDepth < (pcoin->IsFromMe() ? 0 : 1))
//				continue;
//
//			for (unsigned int i = 0; i < pcoin->vout.size(); i++) {
//				CTxDestination addr;
//				if (!IsMine(pcoin->vout[i]))
//					continue;
//				if (!ExtractDestination(pcoin->vout[i].scriptPubKey, addr))
//					continue;
//
//				int64_t n = IsSpent(walletEntry.first, i) ? 0 : pcoin->vout[i].nValue;
//
//				if (!balances.count(addr))
//					balances[addr] = 0;
//				balances[addr] += n;
//			}
//		}
	}

	return balances;
}

set<set<CTxDestination> > CWallet::GetAddressGroupings() {
	AssertLockHeld(cs_wallet); // mapWallet
	set<set<CTxDestination> > groupings;
	set<CTxDestination> grouping;

//	BOOST_FOREACH(PAIRTYPE(uint256, CWalletTx) walletEntry, mapWallet) {
//		CWalletTx *pcoin = &walletEntry.second;
//
//		if (pcoin->vin.size() > 0) {
//			bool any_mine = false;
//			// group all input addresses with each other
//			BOOST_FOREACH(CTxIn txin, pcoin->vin) {
//				CTxDestination address;
//				if (!IsMine(txin)) /* If this input isn't mine, ignore it */
//					continue;
//				if (!ExtractDestination(mapWallet[txin.prevout.hash].vout[txin.prevout.n].scriptPubKey, address))
//					continue;
//				grouping.insert(address);
//				any_mine = true;
//			}
//
//			// group change with input addresses
//			if (any_mine) {
//				BOOST_FOREACH(CTxOut txout, pcoin->vout)
//					if (IsChange(txout)) {
//						CTxDestination txoutAddr;
//						if (!ExtractDestination(txout.scriptPubKey, txoutAddr))
//							continue;
//						grouping.insert(txoutAddr);
//					}
//			}
//			if (grouping.size() > 0) {
//				groupings.insert(grouping);
//				grouping.clear();
//			}
//		}
//
//		// group lone addrs by themselves
//		for (unsigned int i = 0; i < pcoin->vout.size(); i++)
//			if (IsMine(pcoin->vout[i])) {
//				CTxDestination address;
//				if (!ExtractDestination(pcoin->vout[i].scriptPubKey, address))
//					continue;
//				grouping.insert(address);
//				groupings.insert(grouping);
//				grouping.clear();
//			}
//	}

	set<set<CTxDestination>*> uniqueGroupings; // a set of pointers to groups of addresses
	map<CTxDestination, set<CTxDestination>*> setmap;  // map addresses to the unique group containing it
	for (auto grouping : groupings) {
		// make a set of all the groups hit by this new group
		set<set<CTxDestination>*> hits;
		map<CTxDestination, set<CTxDestination>*>::iterator it;
		for (const auto& address : grouping)
			if ((it = setmap.find(address)) != setmap.end())
				hits.insert((*it).second);

		// merge all hit groups into a new single group and delete old groups
		set<CTxDestination>* merged = new set<CTxDestination>(grouping);
		for (auto hit : hits) {
			merged->insert(hit->begin(), hit->end());
			uniqueGroupings.erase(hit);
			delete hit;
		}
		uniqueGroupings.insert(merged);

		// update setmap
		for (auto element : *merged)
			setmap[element] = merged;
	}

	set<set<CTxDestination> > ret;
	for (auto uniqueGrouping : uniqueGroupings) {
		ret.insert(*uniqueGrouping);
		delete uniqueGrouping;
	}

	return ret;
}

set<CTxDestination> CWallet::GetAccountAddresses(string strAccount) const {
	AssertLockHeld(cs_wallet); // mapWallet
	set<CTxDestination> result;
	for (const auto& item : mapAddressBook) {
		const CTxDestination& address = item.first;
		const string& strName = item.second.name;
		if (strName == strAccount)
			result.insert(address);
	}
	return result;
}

bool CReserveKey::GetReservedKey(CPubKey& pubkey) {
	if (nIndex == -1) {
		CKeyPool keypool;
		pwallet->ReserveKeyFromKeyPool(nIndex, keypool);
		if (nIndex != -1)
			vchPubKey = keypool.vchPubKey;
		else {
			if (pwallet->vchDefaultKey.IsValid()) {
				LogPrint("INFO",
						"CReserveKey::GetReservedKey(): Warning: Using default key instead of a new key, top up your keypool!");
				vchPubKey = pwallet->vchDefaultKey;
			} else
				return false;
		}
	}
	assert(vchPubKey.IsValid());
	pubkey = vchPubKey;
	return true;
}

void CReserveKey::KeepKey() {
	if (nIndex != -1)
		pwallet->KeepKey(nIndex);
	nIndex = -1;
	vchPubKey = CPubKey();
}

void CReserveKey::ReturnKey() {
	if (nIndex != -1)
		pwallet->ReturnKey(nIndex);
	nIndex = -1;
	vchPubKey = CPubKey();
}

void CWallet::GetAllReserveKeys(set<CKeyID>& setAddress) const {
	setAddress.clear();

	CWalletDB walletdb(strWalletFile);

	LOCK2(cs_main, cs_wallet);
	for (const auto& id : setKeyPool) {
		CKeyPool keypool;
		if (!walletdb.ReadPool(id, keypool))
			throw runtime_error("GetAllReserveKeyHashes() : read failed");
		assert(keypool.vchPubKey.IsValid());
		CKeyID keyID = keypool.vchPubKey.GetID();
		if (!HaveKey(keyID))
			throw runtime_error("GetAllReserveKeyHashes() : unknown key in key pool");
		setAddress.insert(keyID);
	}
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

void CWallet::GetKeyBirthTimes(map<CKeyID, int64_t> &mapKeyBirth) const {
	AssertLockHeld(cs_wallet); // mapKeyMetadata
	mapKeyBirth.clear();

	// get birth times for keys with metadata
	for (map<CKeyID, CKeyMetadata>::const_iterator it = mapKeyMetadata.begin(); it != mapKeyMetadata.end(); it++)
		if (it->second.nCreateTime)
			mapKeyBirth[it->first] = it->second.nCreateTime;

	// map in which we'll infer heights of other keys
	CBlockIndex *pindexMax = chainActive[max(0, chainActive.Height() - 144)]; // the tip can be reorganised; use a 144-block safety margin
	map<CKeyID, CBlockIndex*> mapKeyFirstBlock;
	set<CKeyID> setKeys;
	GetKeys(setKeys);
	for (const auto &keyid : setKeys) {
		if (mapKeyBirth.count(keyid) == 0)
			mapKeyFirstBlock[keyid] = pindexMax;
	}
	setKeys.clear();

	// if there are no such keys, we're done
	if (mapKeyFirstBlock.empty())
		return;

	// find first block that affects those keys, if there are any left
	vector<CKeyID> vAffected;
//	for (map<uint256, CWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); it++) {
//		// iterate over all wallet transactions...
//		const CWalletTx &wtx = (*it).second;
//		map<uint256, CBlockIndex*>::const_iterator blit = mapBlockIndex.find(wtx.hashBlock);
//		if (blit != mapBlockIndex.end() && chainActive.Contains(blit->second)) {
//			// ... which are already in a block
//			int nHeight = blit->second->nHeight;
//			BOOST_FOREACH(const CTxOut &txout, wtx.vout) {
//				// iterate over all their outputs
//				::ExtractAffectedKeys(*this, txout.scriptPubKey, vAffected);
//				BOOST_FOREACH(const CKeyID &keyid, vAffected) {
//					// ... and all their affected keys
//					map<CKeyID, CBlockIndex*>::iterator rit = mapKeyFirstBlock.find(keyid);
//					if (rit != mapKeyFirstBlock.end() && nHeight < rit->second->nHeight)
//						rit->second = blit->second;
//				}
//				vAffected.clear();
//			}
//		}
//	}

	// Extract block timestamps for those keys
	for (map<CKeyID, CBlockIndex*>::const_iterator it = mapKeyFirstBlock.begin(); it != mapKeyFirstBlock.end();
			it++)
		mapKeyBirth[it->first] = it->second->nTime - 7200; // block times can be 2h off
}

bool CWallet::AddDestData(const CTxDestination &dest, const string &key, const string &value) {
	if (boost::get<CNoDestination>(&dest))
		return false;

	mapAddressBook[dest].destdata.insert(make_pair(key, value));
	if (!fFileBacked)
		return true;
	return CWalletDB(strWalletFile).WriteDestData(CBitcoinAddress(dest).ToString(), key, value);
}

bool CWallet::EraseDestData(const CTxDestination &dest, const string &key) {
	if (!mapAddressBook[dest].destdata.erase(key))
		return false;
	if (!fFileBacked)
		return true;
	return CWalletDB(strWalletFile).EraseDestData(CBitcoinAddress(dest).ToString(), key);
}

bool CWallet::LoadDestData(const CTxDestination &dest, const string &key, const string &value) {
	mapAddressBook[dest].destdata.insert(make_pair(key, value));
	return true;
}

bool CWallet::GetDestData(const CTxDestination &dest, const string &key, string *value) const {
	map<CTxDestination, CAddressBookData>::const_iterator i = mapAddressBook.find(dest);
	if (i != mapAddressBook.end()) {
		CAddressBookData::StringMap::const_iterator j = i->second.destdata.find(key);
		if (j != i->second.destdata.end()) {
			if (value)
				*value = j->second;
			return true;
		}
	}
	return false;
}

