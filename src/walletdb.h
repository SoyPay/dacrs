// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_WALLETDB_H
#define BITCOIN_WALLETDB_H

#include "db.h"
#include "key.h"

#include <list>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

class CAccountInfo;
class CAccountingEntry;
struct CBlockLocator;
class CKeyPool;
class CMasterKey;
//class CScript;
class CWallet;
class uint160;
class uint256;
class CRegID;
class CAccountTx;

/** Error statuses for the wallet database */
enum DBErrors
{
    DB_LOAD_OK,
    DB_CORRUPT,
    DB_NONCRITICAL_ERROR,
    DB_TOO_NEW,
    DB_LOAD_FAIL,
    DB_NEED_REWRITE
};

/*class CKeyMetadata
{
public:
    static const int CURRENT_VERSION=1;
    int nVersion;
    int64_t nCreateTime; // 0 means unknown

    CKeyMetadata()
    {
        SetNull();
    }
    CKeyMetadata(int64_t nCreateTime_)
    {
        nVersion = CKeyMetadata::CURRENT_VERSION;
        nCreateTime = nCreateTime_;
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(nCreateTime);
    )

    void SetNull()
    {
        nVersion = CKeyMetadata::CURRENT_VERSION;
        nCreateTime = 0;
    }
};*/

/** Access to the wallet database (wallet.dat) */
class CWalletDB : public CDB
{
public:
    CWalletDB(string strFilename, const char* pszMode="r+") : CDB(strFilename.c_str(), pszMode)
    {
    }
private:
    CWalletDB(const CWalletDB&);
    void operator=(const CWalletDB&);
public:

    bool WriteCWallet(const CWallet &pwallet);

    bool ReadCWallet(CWallet &pwallet);

    bool WriteName(const string& strAddress, const string& strName);
    bool EraseName(const string& strAddress);

    bool WritePurpose(const string& strAddress, const string& purpose);
    bool ErasePurpose(const string& strAddress);

    bool WriteAccountTx(uint256 hash, const CAccountTx& atx);
    bool EraseAccountTx(uint256 hash);

    bool WriteRegID(CKeyID keyID,const CRegID &regID);
    bool EraseRegID(CKeyID keyID);

    bool WriteScriptRegID(uint256 hash,const CRegID &regID);
    bool EraseScriptRegID(uint256 hash);

//    bool WriteKey(const CPubKey& vchPubKey, const CPrivKey& vchPrivKey, const CKeyMetadata &keyMeta);
//    bool WriteCryptedKey(const CPubKey& vchPubKey, const vector<unsigned char>& vchCryptedSecret, const CKeyMetadata &keyMeta);
    bool WriteMasterKey(unsigned int nID, const CMasterKey& kMasterKey);

//  bool WriteCScript(const uint160& hash, const CScript& redeemScript);

    bool WriteBestBlock(const CBlockLocator& locator);
    bool ReadBestBlock(CBlockLocator& locator);

    bool WriteOrderPosNext(int64_t nOrderPosNext);

    bool WriteDefaultKey(const CPubKey& vchPubKey);

    bool ReadPool(int64_t nPool, CKeyPool& keypool);
    bool WritePool(int64_t nPool, const CKeyPool& keypool);
    bool ErasePool(int64_t nPool);

    bool WriteMinVersion(int nVersion);

    bool ReadAccount(const string& strAccount, CAccountInfo& account);
    bool WriteAccount(const string& strAccount, const CAccountInfo& account);

    /// Write destination data key,value tuple to database
    bool WriteDestData(const string &address, const string &key, const string &value);
    /// Erase destination data tuple from wallet database
    bool EraseDestData(const string &address, const string &key);
private:
    bool WriteAccountingEntry(const uint64_t nAccEntryNum, const CAccountingEntry& acentry);
public:
		bool WriteAccountingEntry(const CAccountingEntry& acentry);
    int64_t GetAccountCreditDebit(const string& strAccount);
    void ListAccountCreditDebit(const string& strAccount, list<CAccountingEntry>& acentries);

    DBErrors ReorderTransactions(CWallet*);
    DBErrors LoadWallet(CWallet& pwallet);
    DBErrors FindWalletTx(CWallet* pwallet, vector<uint256>& vBlockHash);
    DBErrors ZapWalletTx(CWallet* pwallet);
    static bool Recover(CDBEnv& dbenv, string filename, bool fOnlyKeys);
    static bool Recover(CDBEnv& dbenv, string filename);
};

bool BackupWallet(const CWallet& wallet, const string& strDest);

#endif // BITCOIN_WALLETDB_H
