#ifndef _ACCOUNT_H_
#define _ACCOUNT_H_

#include <map>
#include <vector>
#include "serialize.h"
#include "tx.h"

using namespace std;

class CAccount;
class CKeyID;
class uint256;

class CAccountView
{
public:
	virtual bool GetAccount(const CKeyID &keyId, CAccount &account);
	virtual bool SetAccount(const CKeyID &keyId, const CAccount &account);
	virtual bool SetAccount(const vector<unsigned char> &accountId, const CAccount &account);
	virtual bool HaveAccount(const CKeyID &keyId);
	virtual uint256 GetBestBlock();
	virtual bool SetBestBlock(const uint256 &hashBlock);
	virtual bool BatchWrite(const map<CKeyID, CAccount> &mapAccounts, const map<string, CKeyID> &mapKeyIds, const uint256 &hashBlock);
	virtual bool BatchWrite(const vector<CAccount> &vAccounts);
	virtual bool EraseAccount(const CKeyID &keyId);
	virtual bool SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId);
	virtual	bool GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId);
	virtual bool EraseKeyId(const vector<unsigned char> &accountId);
	virtual bool GetAccount(const vector<unsigned char> &accountId, CAccount &account);
	virtual bool SaveAccountInfo(const vector<unsigned char> &accountId, const CKeyID &keyId, const CAccount &account);
	virtual ~CAccountView(){};
};

class CAccountViewBacked : public CAccountView
{
protected:
	CAccountView * pBase;
public:
	CAccountViewBacked(CAccountView &accountView);
	bool GetAccount(const CKeyID &keyId, CAccount &account);
	bool SetAccount(const CKeyID &keyId, const CAccount &account);
	bool SetAccount(const vector<unsigned char> &accountId, const CAccount &account);
	bool HaveAccount(const CKeyID &keyId);
	uint256 GetBestBlock();
	bool SetBestBlock(const uint256 &hashBlock);
	bool BatchWrite(const map<CKeyID, CAccount> &mapAccounts, const map<string, CKeyID> &mapKeyIds, const uint256 &hashBlock);
	bool BatchWrite(const vector<CAccount> &vAccounts);
	bool EraseAccount(const CKeyID &keyId);
	bool SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId);
	bool GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId);
	bool EraseKeyId(const vector<unsigned char> &accountId);
	bool GetAccount(const vector<unsigned char> &accountId, CAccount &account);
	bool SaveAccountInfo(const vector<unsigned char> &accountId, const CKeyID &keyId, const CAccount &account);
};

class CAccountViewCache : public CAccountViewBacked
{
public:
	uint256 hashBlock;
    map<CKeyID, CAccount> cacheAccounts;
    map<string, CKeyID> cacheKeyIds;

private:
	bool GetAccount(const CKeyID &keyId, CAccount &account);
	bool SetAccount(const CKeyID &keyId, const CAccount &account);
	bool SetAccount(const vector<unsigned char> &accountId, const CAccount &account);
	bool HaveAccount(const CKeyID &keyId);
	bool EraseAccount(const CKeyID &keyId);
	bool SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId);
	bool GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId);
	bool EraseKeyId(const vector<unsigned char> &accountId);
	bool GetAccount(const vector<unsigned char> &accountId, CAccount &account);

public:
    CAccountViewCache(CAccountView &base, bool fDummy=false);
	uint256 GetBestBlock();
	bool SetBestBlock(const uint256 &hashBlock);
	bool BatchWrite(const map<CKeyID, CAccount> &mapAccounts, const map<string, CKeyID> &mapKeyIds, const uint256 &hashBlock);
	bool BatchWrite(const vector<CAccount> &vAccounts);
	/**
	 * @brief from use id to reg id
	 * @param userId
	 * @param regId
	 * @return
	 */
	bool GetRegId(const CUserID &userId,CRegID &regId)const;
	bool GetAccount(const CUserID &userId, CAccount &account);
	bool SetAccount(const CUserID &userId, const CAccount &account);
	bool GetKeyId(const CUserID &userId, CKeyID &keyId);
	bool SetKeyId(const CUserID &userId, const CKeyID &keyId);
	bool EraseAccount(const CUserID &userId);
	bool EraseId(const CUserID &userId);
	bool HaveAccount(const CUserID &userId);
	int64_t GetRawBalance(const CUserID &userId,int curhigh)const;

	bool SaveAccountInfo(const CRegID &accountId, const CKeyID &keyId, const CAccount &account);
	bool Flush();
	unsigned int GetCacheSize();

};

class CScriptDBView
{
public:
	virtual bool GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue);
	virtual bool SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue);
	virtual bool BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas);
	virtual bool EraseKey(const vector<unsigned char> &vKey);
	virtual bool HaveData(const vector<unsigned char> &vKey);
	virtual bool GetScript(const int &nIndex, vector<unsigned char> &vScriptId, vector<unsigned char> &vValue);
	virtual bool GetScriptData(const vector<unsigned char> &vScriptId, const int &nIndex,
			vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData, int &nHeight);

	virtual ~CScriptDBView(){};
};

class CScriptDBViewBacked : public CScriptDBView {
protected:
	CScriptDBView * pBase;
public:
	CScriptDBViewBacked(CScriptDBView &dataBaseView);
	bool GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue);
	bool SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue);
	bool BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas);
	bool EraseKey(const vector<unsigned char> &vKey);
	bool HaveData(const vector<unsigned char> &vKey);
	bool GetScript(const int &nIndex, vector<unsigned char> &vScriptId, vector<unsigned char> &vValue);
	bool GetScriptData(const vector<unsigned char> &vScriptId, const int &nIndex, vector<unsigned char> &vScriptKey,
			vector<unsigned char> &vScriptData, int &nHeight);

};

class CScriptDBViewCache : public CScriptDBViewBacked {
public:
	map<vector<unsigned char>, vector<unsigned char> > mapDatas;

public:
	CScriptDBViewCache(CScriptDBView &base, bool fDummy = false);
	bool GetScript(const CRegID &scriptId, vector<unsigned char> &vValue);
	bool GetScript(const int nIndex, CRegID &scriptId, vector<unsigned char> &vValue);
	bool SetScript(const CRegID &scriptId, const vector<unsigned char> &vValue);
	bool HaveScript(const CRegID &scriptId);
	bool EraseScript(const CRegID &scriptId);
	bool GetScriptDataCount(const CRegID &scriptId, int &nCount);
	bool EraseScriptData(const CRegID &scriptId, const vector<unsigned char> &vScriptKey, CScriptDBOperLog &operLog);
	bool HaveScriptData(const CRegID &scriptId, const vector<unsigned char > &vScriptKey);
	bool GetScriptData(const CRegID &scriptId, const vector<unsigned char> &vScriptKey,
				vector<unsigned char> &vScriptData, int &nHeight);
	bool GetScriptData(const CRegID &scriptId, const int &nIndex, vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData,
			int &nHeight);
	bool SetScriptData(const CRegID &scriptId, const vector<unsigned char> &vScriptKey,
				const vector<unsigned char> &vScriptData, const int nHeight, CScriptDBOperLog &operLog);
	bool SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue);
	/**
	 * @brief Get all number of scripts in scriptdb
	 * @param nCount
	 * @return true if get succeed, otherwise false
	 */
	bool GetScriptCount(int &nCount);
	bool SetTxRelAccout(const uint256 &txHash, const set<CKeyID> &relAccount);
	bool GetTxRelAccount(const uint256 &txHash, set<CKeyID> &relAccount);

	/**
	 * @brief write all data in the caches to script db
	 * @return
	 */
	bool Flush();
	unsigned int GetCacheSize();

private:
	bool GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue);

	bool BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas);
	bool EraseKey(const vector<unsigned char> &vKey);
	bool HaveData(const vector<unsigned char> &vKey);

    /**
     * @brief Get script content from scriptdb by scriptid
     * @param vScriptId
     * @param vValue
     * @return true if get script succeed,otherwise false
     */
	bool GetScript(const vector<unsigned char> &vScriptId, vector<unsigned char> &vValue);
	/**
	 * @brief Get Script content from scriptdb by index
	 * @param nIndex the value must be non-negative
	 * @param vScriptId
	 * @param vValue
	 * @return true if get script succeed, otherwise false
	 */
	bool GetScript(const int nIndex, vector<unsigned char> &vScriptId, vector<unsigned char> &vValue);
	/**
	 * @brief Save script content to scriptdb
	 * @param vScriptId
	 * @param vValue
	 * @return true if save succeed, otherwise false
	 */
	bool SetScript(const vector<unsigned char> &vScriptId, const vector<unsigned char> &vValue);
	/**
	 * @brief Detect if scriptdb contains the script by scriptid
	 * @param vScriptId
	 * @return true if contains script, otherwise false
	 */
	bool HaveScript(const vector<unsigned char> &vScriptId);
	/**
	 * @brief Save all number of scripts in scriptdb
	 * @param nCount
	 * @return true if save count succeed, otherwise false
	 */
	bool SetScriptCount(const int nCount);
	/**
	 * @brief Delete script from script db by scriptId
	 * @param vScriptId
	 * @return true if delete succeed, otherwise false
	 */
	bool EraseScript(const vector<unsigned char> &vScriptId);
	/**
	 * @brief Get all numbers of script data in script db
	 * @param vScriptId
	 * @param nCount
	 * @return true if get succeed, otherwise false
	 */
	bool GetScriptDataCount(const vector<unsigned char> &vScriptId, int &nCount);
	/**
	 * @brief Save count of the script's data into script db
	 * @param vScriptId
	 * @param nCount
	 * @return true if save succeed, otherwise false
	 */
	bool SetScriptDataCount(const vector<unsigned char> &vScriptId, int nCount);
	/**
	 * @brief Delete the item of the scirpt's data by scriptId and scriptKey
	 * @param vScriptId
	 * @param vScriptKey must be 8 bytes
	 * @return true if delete succeed, otherwise false
	 */
	bool EraseScriptData(const vector<unsigned char> &vScriptId, const vector<unsigned char> &vScriptKey, CScriptDBOperLog &operLog);
	/**
	 * @brief Detect if scriptdb contains the item of script's data by scriptid and scriptkey
	 * @param vScriptId
	 * @param vScriptKey must be 8 bytes
	 * @return true if contains the item, otherwise false
	 */
	bool HaveScriptData(const vector<unsigned char> &vScriptId, const vector<unsigned char > &vScriptKey);
	/**
	 * @brief Get script data and valide height by scriptid and scriptkey
	 * @param vScriptId
	 * @param vScriptKey must be 8 bytes
	 * @param vScriptData
	 * @param nHeight valide height of script data
	 * @return true if get succeed, otherwise false
	 */
	bool GetScriptData(const vector<unsigned char> &vScriptId, const vector<unsigned char> &vScriptKey,
			vector<unsigned char> &vScriptData, int &nHeight);
	/**
	 * @brief Get script data and valide height by scriptid and nIndex
	 * @param vScriptId
	 * @param nIndex get first script data will be 0, otherwise be 1
	 * @param vScriptKey must be 8 bytes, get first script data will be empty, otherwise get next scirpt data will be previous script key
	 * @param vScriptData
	 * @param nHeight valide height of script data
	 * @return true if get succeed, otherwise false
	 */
	bool GetScriptData(const vector<unsigned char> &vScriptId, const int &nIndex, vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData,
			int &nHeight);
	/**
	 * @brief Save script data and valide height into script db
	 * @param vScriptId
	 * @param vScriptKey must be 8 bytes
	 * @param vScriptData
	 * @param nHeight valide height of script data
	 * @return true if save succeed, otherwise false
	 */
	bool SetScriptData(const vector<unsigned char> &vScriptId, const vector<unsigned char> &vScriptKey,
			const vector<unsigned char> &vScriptData, const int nHeight, CScriptDBOperLog &operLog);

};

class CTransactionDBView {
public:
	virtual bool IsContainTx(const uint256 & txHash);
	virtual bool IsContainBlock(const CBlock &block);
	virtual bool AddBlockToCache(const CBlock &block);
	virtual bool DeleteBlockFromCache(const CBlock &block);
	virtual bool LoadTransaction(map<uint256, vector<uint256> > &mapTxHashByBlockHash);
	virtual bool BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHashIn);
	virtual ~CTransactionDBView(){};
};

class CTransactionDBViewBacked : public CTransactionDBView {
protected:
	CTransactionDBView * pBase;
	bool LoadTransaction(map<uint256, vector<uint256> > &mapTxHashByBlockHash);
public:
	CTransactionDBViewBacked(CTransactionDBView &transactionView);
	bool BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHashIn);
	bool IsContainTx(const uint256 & txHash);
	bool IsContainBlock(const CBlock &block);
	bool AddBlockToCache(const CBlock &block);
	bool DeleteBlockFromCache(const CBlock &block);
};

class CTransactionDBCache : public CTransactionDBViewBacked{
private:
	CTransactionDBCache(CTransactionDBCache &transactionView);
	map<uint256, vector<uint256> > mapTxHashByBlockHash;  // key:block hash  value:tx hash
public:
	CTransactionDBCache(CTransactionDBView &pTxCacheDB, bool fDummy);
	bool IsContainBlock(const CBlock &block);
	bool AddBlockToCache(const CBlock &block);
	bool DeleteBlockFromCache(const CBlock &block);
	bool IsContainTx(const uint256 & txHash);
	map<uint256, vector<uint256> > GetTxHashCache(void);
	bool BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHashIn);
	void AddTxHashCache(const uint256 & blockHash, const vector<uint256> &vTxHash);
	bool Flush();
	bool LoadTransaction();
	void Clear();
};

#endif

