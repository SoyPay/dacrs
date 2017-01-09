#ifndef DACRS_DATABASE_H_
#define DACRS_DATABASE_H_

#include <map>
#include <vector>
#include "serialize.h"
#include "tx.h"
#include "./vm/appaccount.h"
using namespace std;

class CAccount;
class CKeyID;
class uint256;
class ST_DiskTxPos;
class CVmOperate;

class CAccountView {
 public:
	virtual bool GetAccount(const CKeyID &cKeyId, CAccount &cAccount);
	virtual bool SetAccount(const CKeyID &cKeyId, const CAccount &cAccount);
	virtual bool SetAccount(const vector<unsigned char> &vchAccountId, const CAccount &cAccount);
	virtual bool HaveAccount(const CKeyID &cKeyId);
	virtual uint256 GetBestBlock();
	virtual bool SetBestBlock(const uint256 &cHashBlock);
	virtual bool BatchWrite(const map<CKeyID, CAccount> &mapAccounts,
			const map<vector<unsigned char>, CKeyID> &mapKeyIds, const uint256 &cHashBlock);
	virtual bool BatchWrite(const vector<CAccount> &vcAccounts);
	virtual bool EraseAccount(const CKeyID &cKeyId);
	virtual bool SetKeyId(const vector<unsigned char> &vchAccountId, const CKeyID &cKeyId);
	virtual bool GetKeyId(const vector<unsigned char> &vchAccountId, CKeyID &cKeyId);
	virtual bool EraseKeyId(const vector<unsigned char> &vchAccountId);
	virtual bool GetAccount(const vector<unsigned char> &vchAccountId, CAccount &cAccount);
	virtual bool SaveAccountInfo(const vector<unsigned char> &vchAccountId, const CKeyID &cKeyId, const CAccount &cAccount);
	virtual uint64_t TraverseAccount();
	virtual Object ToJosnObj(char chPrefix);
	virtual ~CAccountView() {
	};
};

class CAccountViewBacked: public CAccountView {
 public:
	CAccountViewBacked(CAccountView &cAccountView);
	bool GetAccount(const CKeyID &cKeyId, CAccount &cAccount);
	bool SetAccount(const CKeyID &cKeyId, const CAccount &cAccount);
	bool SetAccount(const vector<unsigned char> &vchAccountId, const CAccount &cAccount);
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
	bool GetAccount(const vector<unsigned char> &vchAccountId, CAccount &cAccount);
	bool SaveAccountInfo(const vector<unsigned char> &vchAccountId, const CKeyID &cKeyId, const CAccount &cAccount);
	uint64_t TraverseAccount();

 protected:
 	CAccountView * m_pBaseAccountView;
};


class CAccountViewCache: public CAccountViewBacked {
 public:
	CAccountViewCache(CAccountView &cAccountView, bool bDummy = false);
	uint256 GetBestBlock();
	bool SetBestBlock(const uint256 &cHashBlockIn);
	bool BatchWrite(const map<CKeyID, CAccount> &mapAccounts, const map<vector<unsigned char>, CKeyID> &mapKeyIds,
			const uint256 &cHashBlockIn);
	bool BatchWrite(const vector<CAccount> &vcAccounts);
	/**
	 * @brief from use id to reg id
	 * @param userId
	 * @param regId
	 * @return
	 */
	bool GetRegId(const CUserID &cUserId, CRegID &cRegId) const;
	bool GetAccount(const CUserID &cUserId, CAccount &cAccount);
	bool SetAccount(const CUserID &cUserId, const CAccount &cAccount);
	bool GetKeyId(const CUserID &cUserId, CKeyID &cKeyId);
	bool SetKeyId(const CUserID &cUserId, const CKeyID &cKeyId);
	bool EraseAccount(const CUserID &cUserId);
	bool EraseId(const CUserID &cUserId);
	bool HaveAccount(const CUserID &cUserId);
	int64_t GetRawBalance(const CUserID &cUserId) const;
	bool SaveAccountInfo(const CRegID &cRegId, const CKeyID &cKeyId, const CAccount &cAccount);
	uint64_t TraverseAccount();
	bool Flush();
	unsigned int GetCacheSize();
	Object ToJosnObj() const;
	void SetBaseData(CAccountView * pAccountView);

 public:
 	uint256 m_cHashBlock;
 	map<CKeyID, CAccount> m_mapCacheAccounts;
 	map<vector<unsigned char>, CKeyID> m_mapCacheKeyIds; // vector 存的 是accountId

 private:
 	bool GetAccount(const CKeyID &cKeyId, CAccount &cAccount);
 	bool SetAccount(const CKeyID &cKeyId, const CAccount &cAccount);
 	bool SetAccount(const vector<unsigned char> &vchAccountId, const CAccount &cAccount);
 	bool HaveAccount(const CKeyID &cKeyId);
 	bool EraseAccount(const CKeyID &cKeyId);
 	bool SetKeyId(const vector<unsigned char> &vchAccountId, const CKeyID &cKeyId);
 	bool GetKeyId(const vector<unsigned char> &vchAccountId, CKeyID &cKeyId);
 	bool EraseKeyId(const vector<unsigned char> &vchAccountId);
 	bool GetAccount(const vector<unsigned char> &vchAccountId, CAccount &cAccount);
};


class CScriptDBView {
 public:
	virtual bool GetData(const vector<unsigned char> &vchKey, vector<unsigned char> &vchValue);
	virtual bool SetData(const vector<unsigned char> &vchKey, const vector<unsigned char> &vchValue);
	virtual bool BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas);
	virtual bool EraseKey(const vector<unsigned char> &vchKey);
	virtual bool HaveData(const vector<unsigned char> &vchKey);
	virtual bool GetScript(const int &nIndex, vector<unsigned char> &vchScriptId, vector<unsigned char> &vchValue);
	virtual bool GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vchScriptId, const int &nIndex,
			vector<unsigned char> &vchScriptKey, vector<unsigned char> &vchScriptData);
	virtual Object ToJosnObj(string strPrefix);
	virtual bool ReadTxIndex(const uint256 &cTxId, ST_DiskTxPos &cDiskTxPos);
	virtual bool WriteTxIndex(const vector<pair<uint256, ST_DiskTxPos> > &list, vector<CScriptDBOperLog> &vTxIndexOperDB);
	virtual bool WriteTxOutPut(const uint256 &cTxId, const vector<CVmOperate> &vcOutput, CScriptDBOperLog &cScriptDBOperLog);
	virtual bool ReadTxOutPut(const uint256 &cTxId, vector<CVmOperate> &vcOutput);
	virtual bool GetTxHashByAddress(const CKeyID &cKeyId, int nHeight,
			map<vector<unsigned char>, vector<unsigned char> > &mapTxHash);
	virtual bool SetTxHashByAddress(const CKeyID &cKeyId, int nHeight, int nIndex, const string &strTxHash,
			CScriptDBOperLog &cScriptDBOperLog);
	virtual bool GetAllScriptAcc(const CRegID& cRegId, map<vector<unsigned char>, vector<unsigned char> > &mapAcc);
	virtual ~CScriptDBView() {
	};
};


class CScriptDBViewBacked: public CScriptDBView {
 public:
	CScriptDBViewBacked(CScriptDBView &cDataBaseView);
	bool GetData(const vector<unsigned char> &vchKey, vector<unsigned char> &vchValue);
	bool SetData(const vector<unsigned char> &vchKey, const vector<unsigned char> &vchValue);
	bool BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas);
	bool EraseKey(const vector<unsigned char> &vchKey);
	bool HaveData(const vector<unsigned char> &vchKey);
	bool GetScript(const int &nIndex, vector<unsigned char> &vchScriptId, vector<unsigned char> &vchValue);
	bool GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vchScriptId, const int &nIndex,
			vector<unsigned char> &vchScriptKey, vector<unsigned char> &vchScriptData);
	bool ReadTxIndex(const uint256 &cTxId, ST_DiskTxPos &tDiskTxPos);
	bool WriteTxIndex(const vector<pair<uint256, ST_DiskTxPos> > &list, vector<CScriptDBOperLog> &vTxIndexOperDB);
	bool WriteTxOutPut(const uint256 &cTxId, const vector<CVmOperate> &vOutput, CScriptDBOperLog &cScriptDBOperLog);
	bool ReadTxOutPut(const uint256 &cTxId, vector<CVmOperate> &vOutput);
	bool GetTxHashByAddress(const CKeyID &cKeyId, int nHeight,
			map<vector<unsigned char>, vector<unsigned char> > &mapTxHash);
	bool SetTxHashByAddress(const CKeyID &cKeyId, int nHeight, int nIndex, const string &strTxHash,
			CScriptDBOperLog &cScriptDBOperLog);
	bool GetAllScriptAcc(const CRegID& cRegId, map<vector<unsigned char>, vector<unsigned char> > &mapAcc);

 protected:
 	CScriptDBView * m_pBase;
};


class CScriptDBViewCache: public CScriptDBViewBacked {
 public:
	CScriptDBViewCache(CScriptDBView &cScriptDBView, bool bDummy = false);
	bool GetScript(const CRegID &cRegId, vector<unsigned char> &vchValue);

	bool GetScriptAcc(const CRegID &cRegId, const vector<unsigned char> &vchAccKey, CAppUserAccout& cAppUserAccOut);
	bool SetScriptAcc(const CRegID &cRegId, const CAppUserAccout& cAppUserAccIn, CScriptDBOperLog &cScriptDBOperLog);

	bool GetScript(const int nIndex, CRegID &cRegId, vector<unsigned char> &vchValue);
	bool SetScript(const CRegID &cRegId, const vector<unsigned char> &vchValue);
	bool HaveScript(const CRegID &cRegId);
	bool EraseScript(const CRegID &cRegId);
	bool GetScriptDataCount(const CRegID &cRegId, int &nCount);
	bool EraseScriptData(const CRegID &cRegId, const vector<unsigned char> &vchScriptKey, CScriptDBOperLog &cScriptDBOperLog);
	bool HaveScriptData(const CRegID &cRegId, const vector<unsigned char> &vchScriptKey);
	bool GetScriptData(const int nCurBlockHeight, const CRegID &cRegId, const vector<unsigned char> &vchScriptKey,
			vector<unsigned char> &vchScriptData);
	bool GetScriptData(const int nCurBlockHeight, const CRegID &cRegId, const int &nIndex,
			vector<unsigned char> &vchScriptKey, vector<unsigned char> &vchScriptData);
	bool SetScriptData(const CRegID &cRegID, const vector<unsigned char> &vchScriptKey,
			const vector<unsigned char> &vchScriptData, CScriptDBOperLog &cScriptDBOperLog);

	bool UndoScriptData(const vector<unsigned char> &vchKey, const vector<unsigned char> &vchValue);
	/**
	 * @brief Get all number of scripts in scriptdb
	 * @param nCount
	 * @return true if get succeed, otherwise false
	 */
	bool GetScriptCount(int &nCount);
	bool SetTxRelAccout(const uint256 &cTxHash, const set<CKeyID> &RelAccount);
	bool GetTxRelAccount(const uint256 &cTxHash, set<CKeyID> &RelAccount);
	bool EraseTxRelAccout(const uint256 &cTxHash);
	/**
	 * @brief write all data in the caches to script db
	 * @return
	 */
	bool Flush();
	unsigned int GetCacheSize();
	Object ToJosnObj() const;
	CScriptDBView * GetBaseScriptDB() {
		return m_pBase;
	}
	bool ReadTxIndex(const uint256 &cTxId, ST_DiskTxPos &tDiskTxPos);
	bool WriteTxIndex(const vector<pair<uint256, ST_DiskTxPos> > &list, vector<CScriptDBOperLog> &vTxIndexOperDB);
	void SetBaseData(CScriptDBView *pNewBase);
	string ToString();
	bool WriteTxOutPut(const uint256 &cTxId, const vector<CVmOperate> &vcOutput, CScriptDBOperLog &cScriptDBOperLog);
	bool ReadTxOutPut(const uint256 &cTxId, vector<CVmOperate> &vcOutput);
	bool GetTxHashByAddress(const CKeyID &cKeyId, int nHeight,
			map<vector<unsigned char>, vector<unsigned char> > &mapTxHash);
	bool SetTxHashByAddress(const CKeyID &cKeyId, int nHeight, int nIndex, const string &strTxHash,
			CScriptDBOperLog &cScriptDBOperLog);
	bool GetAllScriptAcc(const CRegID& cRegId, map<vector<unsigned char>, vector<unsigned char> > &mapAcc);

 public:
	map<vector<unsigned char>, vector<unsigned char> > m_mapDatas;
	/*取脚本 时 第一个vector 是scriptKey = "def" + "scriptid";
	 取应用账户时第一个vector是scriptKey = "acct" + "scriptid"+"_" + "accUserId";
	 取脚本总条数时第一个vector是scriptKey ="snum",
	 取脚本数据总条数时第一个vector是scriptKey ="sdnum";
	 取脚本数据时第一个vector是scriptKey ="data" + "vScriptId" + "_" + "vScriptKey"
	 取交易关联账户时第一个vector是scriptKey ="tx" + "txHash"
	 * */

 private:
	bool GetData(const vector<unsigned char> &vchKey, vector<unsigned char> &vchValue);
	bool SetData(const vector<unsigned char> &vchKey, const vector<unsigned char> &vchValue);
	bool BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas);
	bool EraseKey(const vector<unsigned char> &vchKey);
	bool HaveData(const vector<unsigned char> &vchKey);

	/**
	 * @brief Get script content from scriptdb by scriptid
	 * @param vScriptId
	 * @param vValue
	 * @return true if get script succeed,otherwise false
	 */
	bool GetScript(const vector<unsigned char> &vchScriptId, vector<unsigned char> &vchValue);
	/**
	 * @brief Get Script content from scriptdb by index
	 * @param nIndex the value must be non-negative
	 * @param vScriptId
	 * @param vValue
	 * @return true if get script succeed, otherwise false
	 */
	bool GetScript(const int nIndex, vector<unsigned char> &vchScriptId, vector<unsigned char> &vchValue);
	/**
	 * @brief Save script content to scriptdb
	 * @param vScriptId
	 * @param vValue
	 * @return true if save succeed, otherwise false
	 */
	bool SetScript(const vector<unsigned char> &vchScriptId, const vector<unsigned char> &vchValue);
	/**
	 * @brief Detect if scriptdb contains the script by scriptid
	 * @param vScriptId
	 * @return true if contains script, otherwise false
	 */
	bool HaveScript(const vector<unsigned char> &vchScriptId);
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
	bool EraseScript(const vector<unsigned char> &vchScriptId);
	/**
	 * @brief Get all numbers of script data in script db
	 * @param vScriptId
	 * @param nCount
	 * @return true if get succeed, otherwise false
	 */
	bool GetScriptDataCount(const vector<unsigned char> &vchScriptId, int &nCount);
	/**
	 * @brief Save count of the script's data into script db
	 * @param vScriptId
	 * @param nCount
	 * @return true if save succeed, otherwise false
	 */
	bool SetScriptDataCount(const vector<unsigned char> &vchScriptId, int nCount);
	/**
	 * @brief Delete the item of the scirpt's data by scriptId and scriptKey
	 * @param vScriptId
	 * @param vScriptKey must be 8 bytes
	 * @return true if delete succeed, otherwise false
	 */
	bool EraseScriptData(const vector<unsigned char> &vchScriptId, const vector<unsigned char> &vchScriptKey,
			CScriptDBOperLog &cScriptDBOperLog);
	bool EraseScriptData(const vector<unsigned char> &vchKey);
	/**
	 * @brief Detect if scriptdb contains the item of script's data by scriptid and scriptkey
	 * @param vScriptId
	 * @param vScriptKey must be 8 bytes
	 * @return true if contains the item, otherwise false
	 */
	bool HaveScriptData(const vector<unsigned char> &vchScriptId, const vector<unsigned char> &vchScriptKey);
	/**
	 * @brief Get script data and valide height by scriptid and scriptkey
	 * @param vScriptId
	 * @param vScriptKey must be 8 bytes
	 * @param vScriptData
	 * @param nHeight valide height of script data
	 * @return true if get succeed, otherwise false
	 */
	bool GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vchScriptId,
			const vector<unsigned char> &vchScriptKey, vector<unsigned char> &vchScriptData);
	/**
	 * @brief Get script data and valide height by scriptid and nIndex
	 * @param vScriptId
	 * @param nIndex get first script data will be 0, otherwise be 1
	 * @param vScriptKey must be 8 bytes, get first script data will be empty, otherwise get next scirpt data will be previous script key
	 * @param vScriptData
	 * @param nHeight valide height of script data
	 * @return true if get succeed, otherwise false
	 */
	bool GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vchScriptId, const int &nIndex,
			vector<unsigned char> &vchScriptKey, vector<unsigned char> &vchScriptData);
	/**
	 * @brief Save script data and valide height into script db
	 * @param vScriptId
	 * @param vScriptKey must be 8 bytes
	 * @param vScriptData
	 * @param nHeight valide height of script data
	 * @return true if save succeed, otherwise false
	 */
	bool SetScriptData(const vector<unsigned char> &vchScriptId, const vector<unsigned char> &vchScriptKey,
			const vector<unsigned char> &vchScriptData, CScriptDBOperLog &cScriptDBOperLog);
};

class CTransactionDBView {
 public:
	virtual uint256 IsContainTx(const uint256 & cTxHash);
	virtual bool IsContainBlock(const CBlock &cBlock);
	virtual bool AddBlockToCache(const CBlock &cBlock);
	virtual bool DeleteBlockFromCache(const CBlock &cBlock);
	virtual bool LoadTransaction(map<uint256, vector<uint256> > &mapTxHashByBlockHash);
	virtual bool BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHashIn);
	virtual ~CTransactionDBView() {
	};
};

class CTransactionDBViewBacked: public CTransactionDBView {
 public:
	CTransactionDBViewBacked(CTransactionDBView &cTransactionView);
	bool BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHashIn);
	uint256 IsContainTx(const uint256 & cTxHash);
	bool IsContainBlock(const CBlock &cBlock);
	bool AddBlockToCache(const CBlock &cBlock);
	bool DeleteBlockFromCache(const CBlock &cBlock);

 protected:
 	CTransactionDBView * m_pBase;
 	bool LoadTransaction(map<uint256, vector<uint256> > &mapTxHashByBlockHash);
};

class CTransactionDBCache: public CTransactionDBViewBacked {
 public:
	CTransactionDBCache(CTransactionDBView &cTxCacheDBView, bool bDummy);
	bool IsContainBlock(const CBlock &cBlock);
	bool AddBlockToCache(const CBlock &cBlock);
	bool DeleteBlockFromCache(const CBlock &cBlock);
	uint256 IsContainTx(const uint256 & cTxHash);
	map<uint256, vector<uint256> > GetTxHashCache(void);
	bool BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHashIn);
	void AddTxHashCache(const uint256 & blockHash, const vector<uint256> &vTxHash);
	bool Flush();
	bool LoadTransaction();
	void Clear();
	Object ToJosnObj() const;
	int GetSize();
	void SetBaseData(CTransactionDBView *pNewBase);
	const map<uint256, vector<uint256> > &GetCacheMap();
	void SetCacheMap(const map<uint256, vector<uint256> > &mapCache);

 private:
 	CTransactionDBCache(CTransactionDBCache &cTransactionView);
 	map<uint256, vector<uint256> > mapTxHashByBlockHash;  // key:block hash  value:tx hash
 	bool IsInMap(const map<uint256, vector<uint256> >&mMap, const uint256&hash) const;
};

#endif

