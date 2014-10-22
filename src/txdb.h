// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_TXDB_LEVELDB_H
#define BITCOIN_TXDB_LEVELDB_H

#include "leveldbwrapper.h"
#include "main.h"
#include "account.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

class CBigNum;
class uint256;
class CKeyID;
class CTransactionCache;
// -dbcache default (MiB)
static const int64_t nDefaultDbCache = 100;
// max. -dbcache in (MiB)
static const int64_t nMaxDbCache = sizeof(void*) > 4 ? 4096 : 1024;
// min. -dbcache in (MiB)
static const int64_t nMinDbCache = 4;

/** Access to the block database (blocks/index/) */
class CBlockTreeDB : public CLevelDBWrapper
{
public:
    CBlockTreeDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);
private:
    CBlockTreeDB(const CBlockTreeDB&);
    void operator=(const CBlockTreeDB&);
public:
    bool WriteBlockIndex(const CDiskBlockIndex& blockindex);
    bool WriteBestInvalidWork(const CBigNum& bnBestInvalidWork);
    bool ReadBlockFileInfo(int nFile, CBlockFileInfo &fileinfo);
    bool WriteBlockFileInfo(int nFile, const CBlockFileInfo &fileinfo);
    bool ReadLastBlockFile(int &nFile);
    bool WriteLastBlockFile(int nFile);
    bool WriteReindexing(bool fReindex);
    bool ReadReindexing(bool &fReindex);
    bool ReadTxIndex(const uint256 &txid, CDiskTxPos &pos);
    bool WriteTxIndex(const vector<pair<uint256, CDiskTxPos> > &list);
    bool WriteFlag(const string &name, bool fValue);
    bool ReadFlag(const string &name, bool &fValue);
    bool LoadBlockIndexGuts();
};


class CAccountViewDB : public CAccountView
{
private:
	CLevelDBWrapper db;
public:
	CAccountViewDB(size_t nCacheSize, bool fMemory=false, bool fWipe = false);
private:
	CAccountViewDB(const CAccountViewDB&);
	void operator=(const CAccountViewDB&);
public:
	bool GetAccount(const CKeyID &keyId, CSecureAccount &secureAccount);
	bool SetAccount(const CKeyID &keyId, const CSecureAccount &secureAccount);
	bool SetAccount(const vector<unsigned char> &accountId, const CSecureAccount &secureAccount);
	bool HaveAccount(const CKeyID &keyId);
	uint256 GetBestBlock();
	bool SetBestBlock(const uint256 &hashBlock);
	bool BatchWrite(const map<CKeyID, CSecureAccount> &mapAccounts, const map<string, CKeyID> &mapKeyIds, const uint256 &hashBlock);
	bool BatchWrite(const vector<CSecureAccount> &vAccounts);
	bool EraseAccount(const CKeyID &keyId);
	bool SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId);
	bool GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId);
	bool EraseKeyId(const vector<unsigned char> &accountId);
	bool GetAccount(const vector<unsigned char> &accountId, CSecureAccount &secureAccount);
	bool SaveAccountInfo(const vector<unsigned char> &accountId, const CKeyID &keyId, const CSecureAccount &secureAccount);
};

class CTransactionCacheDB: public CLevelDBWrapper {
public:
	CTransactionCacheDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);
private:
	CTransactionCacheDB(const CTransactionCacheDB&);
	void operator=(const CTransactionCacheDB&);
public:
	bool SetRelayTx(const uint256 &prevhash, const vector<uint256> &vHashTx);
	bool GetRelayTx(const uint256 &prevhash, vector<uint256> &vHashTx);
	bool SetTxCache(const uint256 &blockHash, const vector<uint256> &hashTxSet);
	bool GetTxCache(const uint256 &hashblock, vector<uint256> &hashTx);
	bool LoadTransaction(map<uint256, vector<uint256> > &mapTxHashByBlockHash,
			map<uint256, vector<uint256> > &mapTxHashCacheByPrev);
	bool Flush(const map<uint256, vector<uint256> > &mapTxHashByBlockHash,
			const map<uint256, vector<uint256> > &mapTxHashCacheByPrev);
};

class CScriptDB: public CLevelDBWrapper {
public:
	CScriptDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);
private:
	CScriptDB(const CTransactionCacheDB&);
	void operator=(const CTransactionCacheDB&);
public:
	bool SetArbitrator(const vector<unsigned char> &scriptId, const set<string> &setArbitrator);
	bool SetScript(const vector<unsigned char> &scriptId, const vector<unsigned char> &vScript);
	bool GetArbitrator(const vector<unsigned char> &scriptId, set<string> &setArbitrator);
	bool GetScript(const vector<unsigned char> &scriptId, vector<unsigned char> &vScript);
	bool GetContractScript(const vector<unsigned char> &scriptId, CContractScript &contractScript);
	bool EraseScript(const vector<unsigned char> & scriptId);
	bool LoadRegScript(map<string, CContractScript> &mapScriptCache);
	bool Flush(const map<string, CContractScript> &mapScriptCache);
};
#endif // BITCOIN_TXDB_LEVELDB_H
