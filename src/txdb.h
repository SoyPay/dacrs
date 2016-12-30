// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The DACRS developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_TXDB_H_
#define DACRS_TXDB_H_

#include "leveldbwrapper.h"
#include "main.h"
#include "database.h"
#include "arith_uint256.h"
#include <map>
#include <string>
#include <utility>
#include <vector>

class CBigNum;
class uint256;
class CKeyID;
class CTransactionDBCache;
// -dbcache default (MiB)
static const int64_t g_sDefaultDbCache = 100;
// max. -dbcache in (MiB)
static const int64_t g_sMaxDbCache = sizeof(void*) > 4 ? 4096 : 1024;
// min. -dbcache in (MiB)
static const int64_t g_sMinDbCache = 4;

/** Access to the block database (blocks/index/) */
class CBlockTreeDB: public CLevelDBWrapper {
 public:
	CBlockTreeDB(size_t unCacheSize, bool bMemory = false, bool bWipe = false);

 public:
	bool WriteBlockIndex(const CDiskBlockIndex& cBlockindex);
	bool EraseBlockIndex(const uint256 &cBlockHash);
	bool WriteBestInvalidWork(const uint256& cBestInvalidWork);
	bool ReadBlockFileInfo(int nFile, CBlockFileInfo &cFileinfo);
	bool WriteBlockFileInfo(int nFile, const CBlockFileInfo &cFileinfo);
	bool ReadLastBlockFile(int &nFile);
	bool WriteLastBlockFile(int nFile);
	bool WriteReindexing(bool bReindex);
	bool ReadReindexing(bool &bReindex);
	bool WriteFlag(const string &strName, bool bValue);
	bool ReadFlag(const string &strName, bool &bValue);
	bool LoadBlockIndexGuts();

 private:
 	CBlockTreeDB(const CBlockTreeDB&);
 	void operator=(const CBlockTreeDB&);
};

class CAccountViewDB: public CAccountView {
 public:
	CAccountViewDB(size_t unCacheSize, bool bMemory = false, bool bWipe = false);
	CAccountViewDB(const string& strName, size_t unCacheSize, bool bMemory, bool bWipe);

 public:
	bool GetAccount(const CKeyID &cKeyId, CAccount &cSecureAccount);
	bool SetAccount(const CKeyID &cKeyId, const CAccount &cSecureAccount);
	bool SetAccount(const vector<unsigned char> &vchAccountId, const CAccount &cSecureAccount);
	bool HaveAccount(const CKeyID &cKeyId);
	uint256 GetBestBlock();
	bool SetBestBlock(const uint256 &cHashBlock);
	bool BatchWrite(const map<CKeyID, CAccount> &mapAccounts, const map<vector<unsigned char>, CKeyID> &mapKeyIds,
			const uint256 &cHashBlock);
	bool BatchWrite(const vector<CAccount> &vcAccounts);
	bool EraseAccount(const CKeyID &cKeyId);
	bool SetKeyId(const vector<unsigned char> &vchAccountId, const CKeyID &cKeyId);
	bool GetKeyId(const vector<unsigned char> &vchAccountId, CKeyID &cKeyId);
	bool EraseKeyId(const vector<unsigned char> &vchAccountId);
	bool GetAccount(const vector<unsigned char> &vchAccountId, CAccount &cSecureAccount);
	bool SaveAccountInfo(const vector<unsigned char> &vchAccountId, const CKeyID &cKeyId, const CAccount &cSecureAccount);
	uint64_t TraverseAccount();
	int64_t GetDbCount() {
		return m_cLevelDBWrapper.GetDbCount();
	}
	Object ToJosnObj(char chPrefix);

 private:
	CAccountViewDB(const CAccountViewDB&);
	void operator=(const CAccountViewDB&);

 private:
	CLevelDBWrapper m_cLevelDBWrapper;
};

class CTransactionDB: public CTransactionDBView {
 public:
	CTransactionDB(size_t unCacheSize, bool bMemory = false, bool bWipe = false);

 public:
	bool SetTxCache(const uint256 &cBlockHash, const vector<uint256> &vcHashTxSet);
	bool GetTxCache(const uint256 &cHashblock, vector<uint256> &vcHashTx);
	bool LoadTransaction(map<uint256, vector<uint256> > &mapTxHashByBlockHash);
	bool BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHash);
	int64_t GetDbCount() {
		return m_LevelDBWrapper.GetDbCount();
	}

 private:
	CTransactionDB(const CTransactionDB&);
	void operator=(const CTransactionDB&);

 private:
	CLevelDBWrapper m_LevelDBWrapper;
};

class CScriptDB: public CScriptDBView {
 public:
	CScriptDB(const string& strName, size_t unCacheSize, bool bMemory = false, bool bWipe = false);
	CScriptDB(size_t unCacheSize, bool bMemory = false, bool bWipe = false);

 public:
	bool GetData(const vector<unsigned char> &vchKey, vector<unsigned char> &vchValue);
	bool SetData(const vector<unsigned char> &vchKey, const vector<unsigned char> &vchValue);
	bool BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas);
	bool EraseKey(const vector<unsigned char> &vchKey);
	bool HaveData(const vector<unsigned char> &vchKey);
	bool GetScript(const int &nIndex, vector<unsigned char> &vchScriptId, vector<unsigned char> &vchValue);
	bool GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vchScriptId, const int &nIndex,
			vector<unsigned char> &vchScriptKey, vector<unsigned char> &vchScriptData);
	int64_t GetDbCount() {
		return m_LevelDBWrapper.GetDbCount();
	}
	bool GetTxHashByAddress(const CKeyID &cKeyId, int nHeight,
			map<vector<unsigned char>, vector<unsigned char> > &mapTxHash);
	Object ToJosnObj(string strPrefix);

 private:
	CScriptDB(const CScriptDB&);
	void operator=(const CScriptDB&);

 private:
	CLevelDBWrapper m_LevelDBWrapper;
};

#endif // DACRS_TXDB_H_
