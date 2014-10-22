// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_WALLET_H
#define BITCOIN_WALLET_H

#include "core.h"
#include "crypter.h"
#include "key.h"
#include "keystore.h"
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
class CCoinControl;
class CReserveKey;
class CScript;
//class CRegID;

/** (client) version numbers for particular wallet features */
enum WalletFeature {
	FEATURE_BASE = 10500, // the earliest version new wallets supports (only useful for getinfo's clientversion output)

	FEATURE_WALLETCRYPT = 40000, // wallet encryption
	FEATURE_COMPRPUBKEY = 60000, // compressed public keys

	FEATURE_LATEST = 60000
};

/** A key pool entry */
class CKeyPool {
public:
	int64_t nTime;
	CPubKey vchPubKey;

	CKeyPool() {
		nTime = GetTime();
	}

	CKeyPool(const CPubKey& vchPubKeyIn) {
		nTime = GetTime();
		vchPubKey = vchPubKeyIn;
	}

	IMPLEMENT_SERIALIZE
	(
			if (!(nType & SER_GETHASH))
			READWRITE(nVersion);
			READWRITE(nTime);
			READWRITE(vchPubKey);
	)
};

/** Address book data */
class CAddressBookData {
public:
	string name;
	string purpose;

	CAddressBookData() {
		purpose = "unknown";
	}

	typedef map<string, string> StringMap;
	StringMap destdata;
};

/** A CWallet is an extension of a keystore, which also maintains a set of transactions and balances,
 * and provides the ability to create new transactions.
 */
class CWallet: public CCryptoKeyStore, public CWalletInterface {
private:

	CWalletDB *pwalletdbEncryption;
	// the current wallet version: clients below this version are not able to load the wallet
	int nWalletVersion;

	// the maximum wallet format version: memory-only variable that specifies to what version this wallet may be upgraded
	int nWalletMaxVersion;

	int64_t nNextResend;
	int64_t nLastResend;

public:
	/// Main wallet lock.
	/// This lock protects all the fields added by CWallet
	///   except for:
	///      fFileBacked (immutable after instantiation)
	///      strWalletFile (immutable after instantiation)
	mutable CCriticalSection cs_wallet;

	bool fFileBacked;
	string strWalletFile;

	set<int64_t> setKeyPool;
	map<CKeyID, CKeyMetadata> mapKeyMetadata;
	map<CKeyID, CRegID> mapKeyRegID;
	map<uint256, CRegID> mapScriptRegID;

	typedef map<unsigned int, CMasterKey> MasterKeyMap;
	MasterKeyMap mapMasterKeys;
	unsigned int nMasterKeyMaxID;

	CWallet() {
		SetNull();
	}
	CWallet(string strWalletFileIn) {
		SetNull();

		strWalletFile = strWalletFileIn;
		fFileBacked = true;
	}
	void SetNull() {
		nWalletVersion = FEATURE_BASE;
		nWalletMaxVersion = FEATURE_BASE;
		fFileBacked = false;
		nMasterKeyMaxID = 0;
		pwalletdbEncryption = NULL;
		nOrderPosNext = 0;
		nNextResend = 0;
		nLastResend = 0;
		nTimeFirstKey = 0;
	}

	map<uint256, CAccountTx> mapWalletTx;

	int64_t nOrderPosNext;
	map<uint256, int> mapRequestCount;

	map<CTxDestination, CAddressBookData> mapAddressBook;

	CPubKey vchDefaultKey;

	int64_t nTimeFirstKey;

	const CAccountTx* GetAccountTx(const uint256& hash) const;
	bool GetTx(const uint256& hash,std::shared_ptr<CBaseTransaction> &tx) const;

	// check whether we are allowed to upgrade (or already support) to the named feature
	bool CanSupportFeature(enum WalletFeature wf) {
		AssertLockHeld(cs_wallet);
		return nWalletMaxVersion >= wf;
	}

	// keystore implementation
	// Generate a new key
	CPubKey GenerateNewKey();
	// Adds a key to the store, and saves it to disk.
	bool AddKeyPubKey(const CKey& key, const CPubKey &pubkey);
	// Adds a key to the store, without saving it to disk (used by LoadWallet)
	bool LoadKey(const CKey& key, const CPubKey &pubkey) {
		return CCryptoKeyStore::AddKeyPubKey(key, pubkey);
	}
	// Load metadata (used by LoadWallet)
	bool LoadKeyMetadata(const CPubKey &pubkey, const CKeyMetadata &metadata);

	bool LoadMinVersion(int nVersion) {
		AssertLockHeld(cs_wallet);
		nWalletVersion = nVersion;
		nWalletMaxVersion = max(nWalletMaxVersion, nVersion);
		return true;
	}

	// Adds an encrypted key to the store, and saves it to disk.
	bool AddCryptedKey(const CPubKey &vchPubKey, const vector<unsigned char> &vchCryptedSecret);
	// Adds an encrypted key to the store, without saving it to disk (used by LoadWallet)
	bool LoadCryptedKey(const CPubKey &vchPubKey, const vector<unsigned char> &vchCryptedSecret);
	bool AddCScript(const CScript& redeemScript);
	bool LoadCScript(const CScript& redeemScript) {
		return CCryptoKeyStore::AddCScript(redeemScript);
	}

	/// Adds a destination data tuple to the store, and saves it to disk
	bool AddDestData(const CTxDestination &dest, const string &key, const string &value);
	/// Erases a destination data tuple in the store and on disk
	bool EraseDestData(const CTxDestination &dest, const string &key);
	/// Adds a destination data tuple to the store, without saving it to disk
	bool LoadDestData(const CTxDestination &dest, const string &key, const string &value);
	/// Look up a destination data tuple in the store, return true if found false otherwise
	bool GetDestData(const CTxDestination &dest, const string &key, string *value) const;

	bool Unlock(const SecureString& strWalletPassphrase);
	bool ChangeWalletPassphrase(const SecureString& strOldWalletPassphrase, const SecureString& strNewWalletPassphrase);
	bool EncryptWallet(const SecureString& strWalletPassphrase);

	void GetKeyBirthTimes(map<CKeyID, int64_t> &mapKeyBirth) const;

	/** Increment the next transaction order id
	 @return next transaction order id
	 */
	int64_t IncOrderPosNext(CWalletDB *pwalletdb = NULL);

	bool AddToWallet(const CAccountTx& accTx);
	//  void SyncTransaction(const CBaseTransaction *pTx, const CBlock* pblock, const bool bConnect = true);
	void SyncTransaction(const uint256 &hash, const CBaseTransaction *pTx, const CBlock* pblock);
	void EraseFromWallet(const uint256 &hash);
	int ScanForWalletTransactions(CBlockIndex* pindexStart, bool fUpdate = false);
	void ReacceptWalletTransactions();
	void ResendWalletTransactions();

	bool NewKeyPool();
	bool TopUpKeyPool(unsigned int kpSize = 0);
	int64_t AddReserveKey(const CKeyPool& keypool);
	void ReserveKeyFromKeyPool(int64_t& nIndex, CKeyPool& keypool);
	void KeepKey(int64_t nIndex);
	void ReturnKey(int64_t nIndex);
	bool GetKeyFromPool(CPubKey &key);
	int64_t GetOldestKeyPoolTime();
	void GetAllReserveKeys(set<CKeyID>& setAddress) const;

	set<set<CTxDestination> > GetAddressGroupings();
	map<CTxDestination, int64_t> GetAddressBalances();

	set<CTxDestination> GetAccountAddresses(string strAccount) const;

	bool IsMine(const CBaseTransaction*pTx) {

		vector<CKeyID> vaddr;

		if (!pTx->GetAddress(vaddr, *pAccountViewTip)) {
			return false;
		}
		for (auto &keyid : vaddr) {
			if (HaveKey(keyid)) {
				return true;
			}
		}
		return false;
	}

	bool GetRegID(const CKeyID &keyID, CRegID &regID) {
		if (mapKeyRegID.count(keyID)) {
			regID = mapKeyRegID[keyID];
			return true;
		}
		return false;
	}

	void SetBestChain(const CBlockLocator& loc);

	void DelInvalidRegID();

	DBErrors LoadWallet(bool& fFirstRunRet);
	DBErrors ZapWalletTx();

	bool SetAddressBook(const CTxDestination& address, const string& strName, const string& purpose);

	bool DelAddressBook(const CTxDestination& address);

	void UpdatedTransaction(const uint256 &hashTx);

	void Inventory(const uint256 &hash) {
		{
			LOCK(cs_wallet);
			map<uint256, int>::iterator mi = mapRequestCount.find(hash);
			if (mi != mapRequestCount.end())
				(*mi).second++;
		}
	}

	unsigned int GetKeyPoolSize() {
		AssertLockHeld(cs_wallet); // setKeyPool
		return setKeyPool.size();
	}

	bool SetDefaultKey(const CPubKey &vchPubKey);

	// signify that a particular wallet feature is now used. this may change nWalletVersion and nWalletMaxVersion if those are lower
	bool SetMinVersion(enum WalletFeature, CWalletDB* pwalletdbIn = NULL, bool fExplicit = false);

	// change which version we're allowed to upgrade to (note that this does not immediately imply upgrading to that format)
	bool SetMaxVersion(int nVersion);

	// get the current wallet format (the oldest client version guaranteed to understand this wallet)
	int GetVersion() {
		LOCK(cs_wallet);
		return nWalletVersion;
	}

	bool CommitTransaction(CBaseTransaction *pTx);

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

/** A key allocated from the key pool. */
class CReserveKey {
protected:
	CWallet* pwallet;
	int64_t nIndex;
	CPubKey vchPubKey;
public:
	CReserveKey(CWallet* pwalletIn) {
		nIndex = -1;
		pwallet = pwalletIn;
	}

	~CReserveKey() {
		ReturnKey();
	}

	void ReturnKey();
	bool GetReservedKey(CPubKey &pubkey);
	void KeepKey();
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
class CAccount {
public:
	CPubKey vchPubKey;

	CAccount() {
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
	map<const uint256, std::shared_ptr<CBaseTransaction> > mapAccountTx;
public:
	CAccountTx(CWallet* pwallet = NULL, uint256 hash = uint256(0)) {
		pWallet = pwallet;
		blockHash = hash;
		mapAccountTx.clear();
	}

	~CAccountTx() {

	}

	void BindWallet(CWallet* pwallet) {
		if (pWallet == NULL) {
			assert(pwallet != NULL);
			pWallet = pwallet;
		}
	}

	bool AddTx(const uint256 &hash, const CBaseTransaction*pTx) {
		switch (pTx->nTxType) {
		case NORMAL_TX:
			mapAccountTx[hash] = make_shared<CTransaction>(pTx);
			break;
		case REG_ACCT_TX:
			mapAccountTx[hash] = make_shared<CRegisterAccountTx>(pTx);
			break;
		case APPEAL_TX:
			mapAccountTx[hash] = make_shared<CAppealTransaction>(pTx);
			break;
		case SECURE_TX:
			mapAccountTx[hash] = make_shared<CSecureTransaction>(pTx);
			break;
		case FREEZE_TX:
			mapAccountTx[hash] = make_shared<CFreezeTransaction>(pTx);
			break;
		case REWARD_TX:
			mapAccountTx[hash] = make_shared<CRewardTransaction>(pTx);
			break;
		case REG_SCRIPT_TX:
			mapAccountTx[hash] = make_shared<CRegistScriptTx>(pTx);
		default:
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
			}
			else
			{
				vhash.push_back(item.first);
			}
		}

		for (auto hash : vhash){
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

	unsigned int GetSerializeSize(int nType, int nVersion) const {
		return 0;
	}

	template<typename Stream>
	void Serialize(Stream& s, int nType, int nVersion) const {
		CSerActionSerialize ser_action;
		unsigned int nSerSize = 0;

		READWRITE(blockHash);

		for (const auto& item : mapAccountTx) {
			READWRITE(item.second->nTxType);
			READWRITE(item.first);
			switch (item.second->nTxType) {
			case NORMAL_TX:
				READWRITE(*((CTransaction* )item.second.get()));
				break;
			case REG_ACCT_TX:
				READWRITE(*((CRegisterAccountTx* )item.second.get()));
				break;
			case APPEAL_TX:
				READWRITE(*((CAppealTransaction* )item.second.get()));
				break;
			case SECURE_TX:
				READWRITE(*((CSecureTransaction* )item.second.get()));
				break;
			case FREEZE_TX:
				READWRITE(*((CFreezeTransaction* )item.second.get()));
				break;
			case REWARD_TX:
				READWRITE(*((CRewardTransaction* )item.second.get()));
				break;
			case REG_SCRIPT_TX:
				READWRITE(*((CRegistScriptTx* )item.second.get()));
				break;
			default:
				break;
			}
		}
		unsigned char txtype = NULL_TX;
		READWRITE(txtype);

	}

	template<typename Stream>
	void Unserialize(Stream& s, int nType, int nVersion) {

		CSerActionUnserialize ser_action;
		unsigned int nSerSize = 0;

		READWRITE(blockHash);

		unsigned char txtype = 0;
		uint256 hash;
		READWRITE(txtype);
		while (txtype != NULL_TX) {
			READWRITE(hash);

			switch (txtype) {
			case NORMAL_TX:

			{
				CTransaction tx;
				READWRITE(tx);
				AddTx(hash, (CBaseTransaction*) &tx);
			}

				break;
			case REG_ACCT_TX:

			{
				CRegisterAccountTx tx;
				READWRITE(tx);
				AddTx(hash, (CBaseTransaction*) &tx);
			}
				break;
			case APPEAL_TX: {
				CAppealTransaction tx;
				READWRITE(tx);
				AddTx(hash, (CBaseTransaction*) &tx);
			}
				break;
			case SECURE_TX:

			{
				CSecureTransaction tx;
				READWRITE(tx);
				AddTx(hash, (CBaseTransaction*) &tx);
			}
				break;
			case FREEZE_TX:

			{
				CFreezeTransaction tx;
				READWRITE(tx);
				AddTx(hash, (CBaseTransaction*) &tx);
			}
				break;
			case REWARD_TX:

			{
				CRewardTransaction tx;
				READWRITE(tx);
				AddTx(hash, (CBaseTransaction*) &tx);
			}
				break;

			case REG_SCRIPT_TX:

			{
				CRegistScriptTx tx;
				READWRITE(tx);
				AddTx(hash, (CBaseTransaction*) &tx);
			}
				break;
			default:
				break;
			}

			READWRITE(txtype);
		}
	}

};

#endif
