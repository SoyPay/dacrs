// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The DACRS developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef DACRS_WALLET_WALLET_H
#define DACRS_WALLET_WALLET_H

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

enum emWalletFeature {
    EM_FEATURE_BASE = 0, 					// initialize version
    EM_FEATURE_WALLETCRYPT = 10000, 		// wallet encryption
};

// -paytxfee will warn if called with a higher fee than this amount (in satoshis) per KB
static const int nHighTransactionFeeWarning = 0.01 * COIN;

class CAccountingEntry;

/** A CWallet is an extension of a keystore, which also maintains a set of transactions and balances,
 * and provides the ability to create new transactions.
 */
class CWallet : public CCryptoKeyStore, public CWalletInterface {
 public:
	IMPLEMENT_SERIALIZE
	(
		LOCK(m_cs_wallet);
		{
			READWRITE(m_nWalletVersion);
			READWRITE(m_cBestBlock);
			READWRITE(m_mapMasterKeys);
			READWRITE(m_mapInBlockTx);
			READWRITE(m_mapUnConfirmTx);
			uint256 sun;
			if (fWrite) {
				sun = GetCheckSum();
			}
			READWRITE(sun);
			if (fRead) {
				if (sun != GetCheckSum()) {
					throw "wallet file Invalid";
				}
			}
		}
	)
	virtual ~CWallet() {};
	int64_t GetRawBalance(bool bIsConfirmed=true) const;

    bool Sign(const CKeyID &keyID, const uint256 &hash, vector<unsigned char> &vchSignature, bool bIsMiner=false) const;
    //! Adds an encrypted key to the store, and saves it to disk.
    bool AddCryptedKey(const CPubKey &vchPubKey, const std::vector<unsigned char> &vchCryptedSecret);
    bool LoadCryptedKey(const CPubKey &vchPubKey, const std::vector<unsigned char> &vchCryptedSecret);

    //! Adds a key to the store, without saving it to disk (used by LoadWallet)
    bool LoadKeyCombi(const CKeyID & keyId, const CKeyCombi& keycombi) { return CBasicKeyStore::AddKeyCombi(keyId, keycombi); }
    // Adds a key to the store, and saves it to disk.
    bool AddKey(const CKey& secret,const CKey& minerKey);
    bool AddKey(const CKeyID &keyId, const CKeyCombi& store);
    bool AddKey(const CKey& key);

	bool CleanAll(); //just for unit test
    bool IsReadyForCoolMiner(const CAccountViewCache& view) const;
    bool ClearAllCkeyForCoolMiner();

	CWallet(string strWalletFileIn);
	void SetNull();

	bool LoadMinVersion(int nVersion);

	void SyncTransaction(const uint256 &hash, CBaseTransaction *pTx, const CBlock* pblock);
	void EraseFromWallet(const uint256 &hash);
	int ScanForWalletTransactions(CBlockIndex* pindexStart, bool bUpdate = false);
	//	void ReacceptWalletTransactions();
	void ResendWalletTransactions();

	bool IsMine(CBaseTransaction*pTx) const;

	void SetBestChain(const ST_BlockLocator& loc);

	emDBErrors LoadWallet(bool bFirstRunRet);

	void UpdatedTransaction(const uint256 &hashTx);

	bool EncryptWallet(const SecureString& strWalletPassphrase);

	bool Unlock(const SecureString& strWalletPassphrase);

	bool ChangeWalletPassphrase(const SecureString& strOldWalletPassphrase,	const SecureString& strNewWalletPassphrase);

	// get the current wallet format (the oldest client version guaranteed to understand this wallet)
	int GetVersion() ;

	bool SetMinVersion(enum emWalletFeature emVersion, CWalletDB* pwalletdbIn);

	static CWallet* getinstance();

	std::tuple<bool,string>  CommitTransaction(CBaseTransaction *pTx);

	//	std::tuple<bool,string>  SendMoney(const CRegID &send,const CUserID &rsv, int64_t nValue, int64_t nFee=0);

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

public:
	CPubKey m_vchDefaultKey;

	bool m_bFileBacked;         	//初始化钱包文件名，为true
	string m_strWalletFile;     	//钱包文件名

	map<uint256, CAccountTx> m_mapInBlockTx;
	map<uint256, std::shared_ptr<CBaseTransaction>> m_mapUnConfirmTx;
	mutable CCriticalSection m_cs_wallet;
	//map<CKeyID, CKeyCombi> GetKeyPool() const;

	typedef std::map<unsigned int, CMasterKey> MapMasterKeyMap;
	MapMasterKeyMap m_mapMasterKeys;
	unsigned int m_unMasterKeyMaxID;

	static string m_stadefaultFilename;    //默认钱包文件名  wallet.dat

 private:
	CWallet();

	CWalletDB *m_pWalletdbEncryption;

	static bool StartUp(string &strWalletFile);
	uint256 GetCheckSum() const;

	//	CMasterKey MasterKey;
	int m_nWalletVersion;
	ST_BlockLocator  m_cBestBlock;

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
	if (nOrderPos == -1) {
		return;
	}
	mapValue["n"] = i64tostr(nOrderPos);
}

/** Private key that includes an expiration date in case it never gets used. */
class CWalletKey {
 public:
	CPrivKey m_vchPrivKey;
	int64_t m_llTimeCreated;
	int64_t m_llTimeExpires;
	string m_strComment;
	//// todo: add something to note what created it (user, getnewaddress, change)
	////   maybe should have a map<string, string> property map

	CWalletKey(int64_t nExpires = 0) {
		m_llTimeCreated = (nExpires ? GetTime() : 0);
		m_llTimeExpires = nExpires;
	}

	IMPLEMENT_SERIALIZE
	(
		if (!(nType & SER_GETHASH))
		READWRITE(nVersion);
		READWRITE(m_vchPrivKey);
		READWRITE(m_llTimeCreated);
		READWRITE(m_llTimeExpires);
		READWRITE(m_strComment);
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
	string 		m_strAccount;
	int64_t 	m_llCreditDebit;
	int64_t 	m_llTime;
	string 		m_strOtherAccount;
	string 		m_strComment;
	mapValue_t 	m_mapValue;
	int64_t 	m_llOrderPos;  // position in ordered transaction list
	uint64_t 	m_ullEntryNo;

	CAccountingEntry() {
		SetNull();
	}

	void SetNull() {
		m_llCreditDebit = 0;
		m_llTime = 0;
		m_strAccount.clear();
		m_strOtherAccount.clear();
		m_strComment.clear();
		m_llOrderPos = -1;
	}

	IMPLEMENT_SERIALIZE
	(
		CAccountingEntry& me = *const_cast<CAccountingEntry*>(this);
		if (!(nType & SER_GETHASH)) {
			READWRITE(nVersion);
		}

		// Note: strAccount is serialized as part of the key, not here.
		READWRITE(m_llCreditDebit);
		READWRITE(m_llTime);
		READWRITE(m_strOtherAccount);

		if (!fRead) {
			WriteOrderPos(m_llOrderPos, me.m_mapValue);

			if (!(m_mapValue.empty() && m_vchExtra.empty())) {
				CDataStream ss(nType, nVersion);
				ss.insert(ss.begin(), '\0');
				ss << m_mapValue;
				ss.insert(ss.end(), m_vchExtra.begin(), m_vchExtra.end());
				me.m_strComment.append(ss.str());
			}
		}

		READWRITE(m_strComment);

		size_t nSepPos = m_strComment.find("\0", 0, 1);
		if (fRead) {
			me.m_mapValue.clear();
			if (string::npos != nSepPos) {
				CDataStream ss(vector<char>(m_strComment.begin() + nSepPos + 1, m_strComment.end()), nType, nVersion);
				ss >> me.m_mapValue;
				me.m_vchExtra = vector<char>(ss.begin(), ss.end());
			}
			ReadOrderPos(me.m_llOrderPos, me.m_mapValue);
		}
		if (string::npos != nSepPos) {
			me.m_strComment.erase(nSepPos);
		}

		me.m_mapValue.erase("n");
	)

 private:
	vector<char> m_vchExtra;
};

class CAccountTx {
 public:
	uint256 m_cBlockHash;
	int m_nBlockhigh;
//	set<uint256> Txhash;

	map<uint256, std::shared_ptr<CBaseTransaction>> m_mapAccountTx;
 public:
	CAccountTx(CWallet* pwallet = NULL, uint256 hash = uint256(),int high = 0) {
		m_pWallet = pwallet;
		m_cBlockHash = hash;
		m_mapAccountTx.clear();
		m_nBlockhigh = high;
	}

	~CAccountTx() {

	}

	void BindWallet(CWallet* pwallet) {
		if (m_pWallet == NULL) {
			assert(pwallet != NULL);
			m_pWallet = pwallet;
		}
	}

	bool AddTx(const uint256 &hash, const CBaseTransaction*pTx) {
		switch (pTx->m_chTxType) {
		case EM_COMMON_TX:
		case EM_CONTRACT_TX:
			m_mapAccountTx[hash] = std::make_shared<CTransaction>(pTx);
			break;
		case REG_ACCT_TX:
			m_mapAccountTx[hash] = std::make_shared<CRegisterAccountTx>(pTx);
			break;
		case REWARD_TX:
			m_mapAccountTx[hash] = std::make_shared<CRewardTransaction>(pTx);
			break;
		case REG_APP_TX:
			m_mapAccountTx[hash] = std::make_shared<CRegisterAppTx>(pTx);
			break;
		default:
//			assert(0);
			return false;
			break;
		}
		return true;
	}
	bool HaveTx(const uint256 &hash) {
		if (m_mapAccountTx.end() != m_mapAccountTx.find(hash)) {
			return true;
		}
		return false;
	}
	bool DelTx(const uint256 &hash) {
		return m_mapAccountTx.erase(hash);
	}

	size_t GetTxSize() {
		return m_mapAccountTx.size();
	}

	bool AcceptToMemoryPool() {
		vector<uint256> vhash;
		for (auto& item : m_mapAccountTx) {
//			const uint256& txid = item.first;
			CValidationState state;
			if (item.second->m_chTxType != REWARD_TX) {
				if (!::AcceptToMemoryPool(g_cTxMemPool, state, const_cast<CBaseTransaction*>(item.second.get()), false,
					false)) {
					vhash.push_back(item.first);
				}
			} else {
				vhash.push_back(item.first);
			}
		}

		for (auto hash : vhash) {
			m_mapAccountTx.erase(hash);
		}
		return true;
	}

	void RelayWalletTransaction() {
		for (auto& item : m_mapAccountTx) {
			if (item.second->m_chTxType != REWARD_TX) {
				::RelayTransaction(const_cast<CBaseTransaction*>(item.second.get()), item.first);
			}
		}
	}

	bool WriteToDisk() {
		return CWalletDB(m_pWallet->m_strWalletFile).WriteBlockTx(m_cBlockHash, *this);
	}

	Object ToJosnObj(CKeyID const &key = CKeyID()) const;

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_cBlockHash);
		READWRITE(m_nBlockhigh);
		READWRITE(m_mapAccountTx);
	)

 private:
	CWallet* m_pWallet;
};

#endif
