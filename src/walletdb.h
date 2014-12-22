// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_WALLETDB_H
#define BITCOIN_WALLETDB_H


#include "key.h"

#include <list>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>
#include "stdio.h"
#include "leveldbwrapper.h"

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
class CKeyStoreValue;

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
class CWalletDB
{

public:
    CWalletDB(const string& strFilename);


private:
	CLevelDBWrapper db;
	unsigned int nWalletDBUpdated;
    CWalletDB(const CWalletDB&);
    void operator=(const CWalletDB&);
public:
    bool WriteKeyStoreValue(const CKeyID &keyId, const CKeyStoreValue& KeyStoreValue);
    bool EraseKeyStoreValue(const CKeyID &keyId);
    bool WriteBlockTx(const uint256 &hash, const CAccountTx& atx);
    bool EraseBlockTx(const uint256& hash);
    bool WriteUnComFirmedTx(const uint256 &hash, const std::shared_ptr<CBaseTransaction> &tx);
    bool EraseUnComFirmedTx(const uint256& hash);
    bool WriteMasterKey(const CMasterKey& kMasterKey);
    bool EraseMasterKey();
    DBErrors LoadWallet(CWallet* pwallet);

};

bool BackupWallet(const CWallet& wallet, const string& strDest);

#endif // BITCOIN_WALLETDB_H
