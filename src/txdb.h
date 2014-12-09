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
class CTransactionDBCache;
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
    bool EraseBlockIndex(const uint256 &blockHash);
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
	CAccountViewDB(const string& name,size_t nCacheSize, bool fMemory, bool fWipe);
private:
	CAccountViewDB(const CAccountViewDB&);
	void operator=(const CAccountViewDB&);
public:
	bool GetAccount(const CKeyID &keyId, CAccount &secureAccount);
	bool SetAccount(const CKeyID &keyId, const CAccount &secureAccount);
	bool SetAccount(const vector<unsigned char> &accountId, const CAccount &secureAccount);
	bool HaveAccount(const CKeyID &keyId);
	uint256 GetBestBlock();
	bool SetBestBlock(const uint256 &hashBlock);
	bool BatchWrite(const map<CKeyID, CAccount> &mapAccounts, const map<string, CKeyID> &mapKeyIds, const uint256 &hashBlock);
	bool BatchWrite(const vector<CAccount> &vAccounts);
	bool EraseAccount(const CKeyID &keyId);
	bool SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId);
	bool GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId);
	bool EraseKeyId(const vector<unsigned char> &accountId);
	bool GetAccount(const vector<unsigned char> &accountId, CAccount &secureAccount);
	bool SaveAccountInfo(const vector<unsigned char> &accountId, const CKeyID &keyId, const CAccount &secureAccount);
};

class CTransactionDB: public CTransactionDBView{
private:
	CLevelDBWrapper db;
public:
	CTransactionDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);
private:
	CTransactionDB(const CTransactionDB&);
	void operator=(const CTransactionDB&);
public:
	bool SetTxCache(const uint256 &blockHash, const vector<uint256> &hashTxSet);
	bool GetTxCache(const uint256 &hashblock, vector<uint256> &hashTx);
	bool LoadTransaction(map<uint256, vector<uint256> > &mapTxHashByBlockHash);
	bool BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHash);
};

class CScriptDB: public CScriptDBView
{
private:
	CLevelDBWrapper db;
public:
	CScriptDB(const string&name,size_t nCacheSize,  bool fMemory=false, bool fWipe = false);
	CScriptDB(size_t nCacheSize, bool fMemory=false, bool fWipe = false);
private:
	CScriptDB(const CScriptDB&);
	void operator=(const CScriptDB&);
public:
	bool GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue);
	bool SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue);
	bool BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas);
	bool EraseKey(const vector<unsigned char> &vKey);
	bool HaveData(const vector<unsigned char> &vKey);
	bool GetScript(const int &nIndex, vector<unsigned char> &vScriptId, vector<unsigned char> &vValue);
	bool GetScriptData(const vector<unsigned char> &vScriptId, const int &nIndex, vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData,
			int &nHeight);

};
#endif // BITCOIN_TXDB_LEVELDB_H
