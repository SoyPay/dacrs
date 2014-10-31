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
	virtual bool GetAccount(const CKeyID &keyId, CAccount &secureAccount);
	virtual bool SetAccount(const CKeyID &keyId, const CAccount &secureAccount);
	virtual bool SetAccount(const vector<unsigned char> &accountId, const CAccount &secureAccount);
	virtual bool HaveAccount(const CKeyID &keyId);
	virtual uint256 GetBestBlock();
	virtual bool SetBestBlock(const uint256 &hashBlock);
	virtual bool BatchWrite(const map<CKeyID, CAccount> &mapAccounts, const map<string, CKeyID> &mapKeyIds, const uint256 &hashBlock);
	virtual bool BatchWrite(const vector<CAccount> &vAccounts);
	virtual bool EraseAccount(const CKeyID &keyId);
	virtual bool SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId);
	virtual	bool GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId);
	virtual bool EraseKeyId(const vector<unsigned char> &accountId);
	virtual bool GetAccount(const vector<unsigned char> &accountId, CAccount &secureAccount);
	virtual bool SaveAccountInfo(const vector<unsigned char> &accountId, const CKeyID &keyId, const CAccount &secureAccount);
	virtual ~CAccountView(){};
};

class CAccountViewBacked : public CAccountView
{
protected:
	CAccountView * pBase;
public:
	CAccountViewBacked(CAccountView &accountView);
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


class CAccountViewCache : public CAccountViewBacked
{
public:
    uint256 hashBlock;
    map<CKeyID, CAccount> cacheAccounts;
    map<string, CKeyID> cacheKeyIds;
public:
    CAccountViewCache(CAccountView &base, bool fDummy=false);
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

	bool Flush();
	unsigned int GetCacheSize();

};

class CScriptDBView
{
public:
	virtual bool GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue);
	virtual bool SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue);
	virtual bool BatchWrite(const map<string, vector<unsigned char> > &mapDatas);
	virtual bool EraseKey(const vector<unsigned char> &vKey);
	virtual bool HaveData(const vector<unsigned char> &vKey);
	virtual bool GetScript(const int &nIndex, vector<unsigned char> &vValue);
	virtual bool GetScriptData(const vector<unsigned char> &vScriptId, const int &nIndex,
			vector<unsigned char> &vScriptData, int &nHeight);

	virtual ~CScriptDBView(){};
};

class CScriptDBViewBacked : public CScriptDBView {
protected:
	CScriptDBView * pBase;
public:
	CScriptDBViewBacked(CScriptDBView &dataBaseView);
	bool GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue);
	bool SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue);
	bool BatchWrite(const map<string, vector<unsigned char> > &mapDatas);
	bool EraseKey(const vector<unsigned char> &vKey);
	bool HaveData(const vector<unsigned char> &vKey);
	bool GetScript(const int &nIndex, vector<unsigned char> &vValue);
	bool GetScriptData(const vector<unsigned char> &vScriptId, const int &nIndex, vector<unsigned char> &vScriptData,
			int &nHeight);

};

class CScriptDBViewCache : public CScriptDBViewBacked {
public:
	map<string, vector<unsigned char> > mapDatas;
public:
	CScriptDBViewCache(CScriptDBView &base, bool fDummy = false);
	bool GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue);
	bool SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue);
	bool BatchWrite(const map<string, vector<unsigned char> > &mapDatas);
	bool EraseKey(const vector<unsigned char> &vKey);
	bool HaveData(const vector<unsigned char> &vKey);
	bool GetScript(const vector<unsigned char> &vScriptId, vector<unsigned char> &vValue);
	bool SetScript(const vector<unsigned char> &vScriptId, const vector<unsigned char> &vValue);
	bool GetScript(const int &nIndex, vector<unsigned char> &vValue);
	bool GetScriptData(const vector<unsigned char> &vScriptId, const vector<unsigned char> &vScriptKey,
			vector<unsigned char> &vScriptData, int &nHeight);
	bool GetScriptData(const vector<unsigned char> &vScriptId, const int &nIndex, vector<unsigned char> &vScriptData,
			int &nHeight);
	bool SetScriptData(const vector<unsigned char> &vScriptId, const vector<unsigned char> &vScriptKey,
			const vector<unsigned char> &vScriptData, const int nHeight);

	bool Flush();
	unsigned int GetCacheSize();
};

#endif

