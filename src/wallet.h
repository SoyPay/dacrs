// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_WALLET_H
#define BITCOIN_WALLET_H

#include "core.h"
#include "crypter.h"
#include "key.h"
//#include "keystore.h"
#include "main.h"
#include "ui_interface.h"
#include "util.h"
#include "walletdb.h"

#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

#include <memory>

// Settings
extern int64_t nTransactionFee;
extern bool bSpendZeroConfChange;

// -paytxfee default
static const int64_t DEFAULT_TRANSACTION_FEE = 0;
// -paytxfee will warn if called with a higher fee than this amount (in satoshis) per KB
static const int nHighTransactionFeeWarning = 0.01 * COIN;

class CAccountingEntry;
//class CCoinControl;
//class CReserveKey;
//class CScript;
//class CRegID;

/** (client) version numbers for particular wallet features */
enum WalletFeature {
	FEATURE_BASE = 10000, // the earliest version new wallets supports (only useful for getinfo's clientversion output)
};





class CKeyStoreValue {
private:
	CRegID mregId;
	CPubKey mPKey;
	CKey  mCkey;
	CKey  mMinerCkey; //only used for miner
	INT64 nCreationTime;
public:

	string ToString()
	{
		return strprintf("CRegID:%s CPubKey:%s CKey:%s mMinerCkey:%s CreationTime:%d",mregId.ToString(),mPKey.ToString(),mCkey.ToString(),mMinerCkey.ToString(),nCreationTime);
	}
	Object ToJsonObj()const;
	bool UnSersailFromJson(const Object&);
	INT64 getBirthDay()const
	{
		return nCreationTime;
	}
	bool getCKey(CKey& keyOut,bool IsMiner = false) const {
		if(IsMiner == true && mMinerCkey.IsValid()) {
			keyOut = mMinerCkey;
		} else {
			keyOut = mCkey;
		}
		return keyOut.IsValid();
	}

	bool CreateANewKey()
	{
		mCkey.MakeNewKey();
		mPKey = mCkey.GetPubKey();
		nCreationTime = GetTime();
		return true;
	}
	bool GetPubKey(CPubKey &mOutKey,bool IsMiner = false) const
	{
		if(IsMiner == true){
			if(mMinerCkey.IsValid()){
				mOutKey = mMinerCkey.GetPubKey();
				return true;
			}
			return false;
		}

		assert(mCkey.IsValid());
		mOutKey =mPKey;
		assert(mCkey.GetPubKey() == mPKey);
		return  true;
	}
	bool SynchronizSys(CAccountViewCache &view);


	CKeyStoreValue(const CPubKey &pubkey) {
		assert(mCkey.IsValid() == false && pubkey.IsFullyValid()); //the ckey mustbe unvalid
		nCreationTime = GetTime();
		mPKey = pubkey;
	}

	CKeyStoreValue(CKey const &inkey,CKey const &minerKey)
	{
		assert(inkey.IsValid());
		assert(minerKey.IsValid());
		mMinerCkey = minerKey;
		mCkey = inkey ;
		nCreationTime = GetTime();
		mPKey = mCkey.GetPubKey();
	}

	CKeyStoreValue(CKey const &inkey)
	{
		assert(inkey.IsValid());
		mCkey = inkey ;
		nCreationTime = GetTime();
		mPKey = mCkey.GetPubKey();
	}

	CKeyStoreValue()
	{
		nCreationTime = 0 ;
	}
	bool IsCrypted() {
		return mCkey.size() == 0;
	}

	CKeyID GetCKeyID() const {
		return (mPKey.GetKeyID());
	}
	CRegID GetRegID() const {
		return mregId;
	}
	IMPLEMENT_SERIALIZE
	(

			READWRITE(mregId);
			READWRITE(mPKey);
			READWRITE(mCkey);
			READWRITE(mMinerCkey);
			READWRITE(nCreationTime);
	)

};

/** A CWallet is an extension of a keystore, which also maintains a set of transactions and balances,
 * and provides the ability to create new transactions.
 */
class CWallet : public CWalletInterface{
private:
	static bool StartUp();

	CMasterKey MasterKey;

	map<CKeyID, CKeyStoreValue> mKeyPool;
	int nWalletVersion;
	CBlockLocator  bestBlock;
	uint256 GetCheckSum()const;

public:
	string strWalletFile;

	map<uint256, CAccountTx> mapInBlockTx;
	map<uint256, std::shared_ptr<CBaseTransaction> > UnConfirmTx;

	map<CKeyID, CKeyStoreValue> GetKeyPool() const
		{
		  AssertLockHeld(cs_wallet);
		  return mKeyPool;
		}

	IMPLEMENT_SERIALIZE
	(
			LOCK(cs_wallet);
			{

				READWRITE(nWalletVersion);
				READWRITE(bestBlock);
				READWRITE(MasterKey);
				READWRITE(mKeyPool);
				READWRITE(mapInBlockTx);
				READWRITE(UnConfirmTx);
				uint256 sun(0);
				if(fWrite){
				 sun = GetCheckSum();
				}
				READWRITE(sun);
				if(fRead) {
					if(sun != GetCheckSum()) {
						throw "wallet file Invalid";
					}
				}
			}
	)
	bool FushToDisk()const;

	int64_t GetRawBalance(int ncurhigh)const;
    bool SynchronizRegId(const CKeyID &keyid,const CAccountViewCache &inview);
    bool Sign(const CUserID &keyID,const uint256 &hash,vector<unsigned char> &signature,bool IsMiner=false)const;
    bool AddKey(const CKey& secret,const CKey& minerKey);
    bool AddKey(const CKeyStoreValue& store);
	bool AddPubKey(const CPubKey& pk);
	bool SynchronizSys(const CAccountViewCache &inview) ;
	static string defaultFilename ;

	static CWallet* getinstance();

	mutable CCriticalSection cs_wallet;
	bool IsCrypted() const
	{
		return false;
	}
	bool GetPubKey(const CKeyID &address, CPubKey& keyOut,bool IsMiner = false);

	bool GetKey(const CKeyID &address, CKey& keyOut, bool IsMiner = false) const ;
	bool GetKey(const CUserID &address, CKey& keyOut,bool IsMiner = false) const ;
	bool GetRegId(const CUserID &address, CRegID& IdOut) const;

	bool GetKeyIds(set<CKeyID>& setKeyID,bool IsMiner = false)const ;


	bool CleanAll(); //just for unit test

    bool count(const CKeyID &address) const
    {
    	AssertLockHeld(cs_wallet);
    	return mKeyPool.count(address) > 0;
    }


	CWallet() {
		SetNull();
	}
	CWallet(string strWalletFileIn) {
		SetNull();

		strWalletFile = strWalletFileIn;

	}
	void SetNull() {
		nWalletVersion = FEATURE_BASE;

	}



	// Adds a key to the store, and saves it to disk.
	bool AddKey(const CKey& key);


	bool LoadMinVersion(int nVersion) {
		AssertLockHeld(cs_wallet);
		nWalletVersion = nVersion;

		return true;
	}


	void SyncTransaction(const uint256 &hash, CBaseTransaction *pTx, const CBlock* pblock);
//	void EraseFromWallet(const uint256 &hash);
	int ScanForWalletTransactions(CBlockIndex* pindexStart, bool fUpdate = false);
//	void ReacceptWalletTransactions();
		void ResendWalletTransactions();

	std::tuple<bool,string>  SendMoney(const CRegID &send,const CUserID &rsv, int64_t nValue);

	bool IsMine(CBaseTransaction*pTx)const;

	void SetBestChain(const CBlockLocator& loc);


	DBErrors LoadWallet(bool fFirstRunRet);
	DBErrors ZapWalletTx();


	void UpdatedTransaction(const uint256 &hashTx);






	// get the current wallet format (the oldest client version guaranteed to understand this wallet)
	int GetVersion() {
		LOCK(cs_wallet);
		return nWalletVersion;
	}

	std::tuple<bool,string>  CommitTransaction(CBaseTransaction *pTx);

	/** Address book entry changed.
	 * @note called with lock cs_wallet held.
	 */
	boost::signals2::signal<
			void(CWallet *wallet, const CTxDestination &address, const string &label, bool isMine,
					const string &purpose, ChangeType status)> NotifyAddressBookChanged;

	/** Wallet transaction added, removed or updated.
	 * @note called with lock cs_wallet held.
	 */
	boost::signals2::signal<void(CWallet *wallet, const uint256 &hashTx, ChangeType status)> NotifyTransactionChanged;

	/** Show progress e.g. for rescan */
	boost::signals2::signal<void(const string &title, int nProgress)> ShowProgress;
};



typedef map<string, string> mapValue_t;

static void ReadOrderPos(int64_t& nOrderPos, mapValue_t& mapValue) {
	if (!mapValue.count("n")) {
		nOrderPos = -1; // TODO: calculate elsewhere
		return;
	}
	nOrderPos = atoi64(mapValue["n"].c_str());
}

static void WriteOrderPos(const int64_t& nOrderPos, mapValue_t& mapValue) {
	if (nOrderPos == -1)
		return;
	mapValue["n"] = i64tostr(nOrderPos);
}

/** Private key that includes an expiration date in case it never gets used. */
class CWalletKey {
public:
	CPrivKey vchPrivKey;
	int64_t nTimeCreated;
	int64_t nTimeExpires;
	string strComment;
	//// todo: add something to note what created it (user, getnewaddress, change)
	////   maybe should have a map<string, string> property map

	CWalletKey(int64_t nExpires = 0) {
		nTimeCreated = (nExpires ? GetTime() : 0);
		nTimeExpires = nExpires;
	}

	IMPLEMENT_SERIALIZE
	(
			if (!(nType & SER_GETHASH))
			READWRITE(nVersion);
			READWRITE(vchPrivKey);
			READWRITE(nTimeCreated);
			READWRITE(nTimeExpires);
			READWRITE(strComment);
	)
};

/** Account information.
 * Stored in wallet with key "acc"+string account name.
 */
class CAccountInfo {
public:
	CPubKey vchPubKey;

	CAccountInfo() {
		SetNull();
	}

	void SetNull() {
		vchPubKey = CPubKey();
	}

	IMPLEMENT_SERIALIZE
	(
			if (!(nType & SER_GETHASH))
			READWRITE(nVersion);
			READWRITE(vchPubKey);
	)
};

/** Internal transfers.
 * Database key is acentry<account><counter>.
 */
class CAccountingEntry {
public:
	string strAccount;
	int64_t nCreditDebit;
	int64_t nTime;
	string strOtherAccount;
	string strComment;
	mapValue_t mapValue;
	int64_t nOrderPos;  // position in ordered transaction list
	uint64_t nEntryNo;

	CAccountingEntry() {
		SetNull();
	}

	void SetNull() {
		nCreditDebit = 0;
		nTime = 0;
		strAccount.clear();
		strOtherAccount.clear();
		strComment.clear();
		nOrderPos = -1;
	}

	IMPLEMENT_SERIALIZE
	(
			CAccountingEntry& me = *const_cast<CAccountingEntry*>(this);
			if (!(nType & SER_GETHASH))
			READWRITE(nVersion);
			// Note: strAccount is serialized as part of the key, not here.
			READWRITE(nCreditDebit);
			READWRITE(nTime);
			READWRITE(strOtherAccount);

			if (!fRead)
			{
				WriteOrderPos(nOrderPos, me.mapValue);

				if (!(mapValue.empty() && _ssExtra.empty()))
				{
					CDataStream ss(nType, nVersion);
					ss.insert(ss.begin(), '\0');
					ss << mapValue;
					ss.insert(ss.end(), _ssExtra.begin(), _ssExtra.end());
					me.strComment.append(ss.str());
				}
			}

			READWRITE(strComment);

			size_t nSepPos = strComment.find("\0", 0, 1);
			if (fRead)
			{
				me.mapValue.clear();
				if (string::npos != nSepPos)
				{
					CDataStream ss(vector<char>(strComment.begin() + nSepPos + 1, strComment.end()), nType, nVersion);
					ss >> me.mapValue;
					me._ssExtra = vector<char>(ss.begin(), ss.end());
				}
				ReadOrderPos(me.nOrderPos, me.mapValue);
			}
			if (string::npos != nSepPos)
			me.strComment.erase(nSepPos);

			me.mapValue.erase("n");
	)

private:
	vector<char> _ssExtra;
};

class CAccountTx {
private:
	CWallet* pWallet;

public:
	uint256 blockHash;
	int blockhigh;
//	set<uint256> Txhash;

	map<uint256, vector<CAccountOperLog> > mapOperLog;

	map<uint256, std::shared_ptr<CBaseTransaction> > mapAccountTx;
public:
	CAccountTx(CWallet* pwallet = NULL, uint256 hash = uint256(0),int high = 0) {
		pWallet = pwallet;
		blockHash = hash;
		mapAccountTx.clear();
		blockhigh = high;
	}

	~CAccountTx() {

	}

	void BindWallet(CWallet* pwallet) {
		if (pWallet == NULL) {
			assert(pwallet != NULL);
			pWallet = pwallet;
		}
	}
	bool AddOperLog(const uint256 &hash, const vector<CAccountOperLog> &log)
	{
		assert(mapOperLog.count(hash) == 0 );
		mapOperLog[hash] = log;
		return true;
	}
	bool AddTx(const uint256 &hash, const CBaseTransaction*pTx) {
		switch (pTx->nTxType) {
		case NORMAL_TX:
			mapAccountTx[hash] = make_shared<CTransaction>(pTx);
			break;
		case REG_ACCT_TX:
			mapAccountTx[hash] = make_shared<CRegisterAccountTx>(pTx);
			break;
		case CONTRACT_TX:
			mapAccountTx[hash] = make_shared<CContractTransaction>(pTx);
			break;
		case FREEZE_TX:
			mapAccountTx[hash] = make_shared<CFreezeTransaction>(pTx);
			break;
		case REWARD_TX:
			mapAccountTx[hash] = make_shared<CRewardTransaction>(pTx);
			break;
		case REG_SCRIPT_TX:
			mapAccountTx[hash] = make_shared<CRegistScriptTx>(pTx);
			break;
		default:
			assert(0);
			return false;
			break;
		}
		return true;
	}
	bool HaveTx(const uint256 &hash) {
		if (mapAccountTx.end() != mapAccountTx.find(hash)) {
			return true;
		}
		return false;
	}
	bool DelTx(const uint256 &hash) {
		return mapAccountTx.erase(hash);
	}

	size_t GetTxSize() {
		return mapAccountTx.size();
	}

	bool AcceptToMemoryPool() {

		vector<uint256> vhash;
		for (auto& item : mapAccountTx) {
			const uint256& txid = item.first;
			CValidationState state;
			if (item.second->nTxType != REWARD_TX) {
				if (!::AcceptToMemoryPool(mempool, state, const_cast<CBaseTransaction*>(item.second.get()), false, NULL,
						false)) {
					vhash.push_back(item.first);
				}
			} else {
				vhash.push_back(item.first);
			}
		}

		for (auto hash : vhash) {
			mapAccountTx.erase(hash);
		}
		return true;
	}

	void RelayWalletTransaction() {
		for (auto& item : mapAccountTx) {
			if (item.second->nTxType != REWARD_TX) {
				::RelayTransaction(const_cast<CBaseTransaction*>(item.second.get()), item.first);
			}
		}
	}

	bool WriteToDisk() {
		return CWalletDB(pWallet->strWalletFile).WriteAccountTx(blockHash, *this);
	}

	Object ToJosnObj(CKeyID const &key = CKeyID()) const;


	IMPLEMENT_SERIALIZE
	(
			READWRITE(blockHash);
			READWRITE(blockhigh);
			READWRITE(mapAccountTx);
	)

};

#endif
