#ifndef _ACCOUNT_H_
#define _ACCOUNT_H_

#include <map>
#include <vector>
#include "serialize.h"
#include "tx.h"

using namespace std;

class CSecureAccount;
class CKeyID;
class uint256;

class CAccountView
{
public:
	virtual bool GetAccount(const CKeyID &keyId, CSecureAccount &secureAccount);
	virtual bool SetAccount(const CKeyID &keyId, const CSecureAccount &secureAccount);
	virtual bool SetAccount(const vector<unsigned char> &accountId, const CSecureAccount &secureAccount);
	virtual bool HaveAccount(const CKeyID &keyId);
	virtual uint256 GetBestBlock();
	virtual bool SetBestBlock(const uint256 &hashBlock);
	virtual bool BatchWrite(const map<CKeyID, CSecureAccount> &mapAccounts, const map<string, CKeyID> &mapKeyIds, const uint256 &hashBlock);
	virtual bool BatchWrite(const vector<CSecureAccount> &vAccounts);
	virtual bool EraseAccount(const CKeyID &keyId);
	virtual bool SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId);
	virtual	bool GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId);
	virtual bool EraseKeyId(const vector<unsigned char> &accountId);
	virtual bool GetAccount(const vector<unsigned char> &accountId, CSecureAccount &secureAccount);
	virtual bool SaveAccountInfo(const vector<unsigned char> &accountId, const CKeyID &keyId, const CSecureAccount &secureAccount);
	virtual ~CAccountView(){};
};

class CAccountViewBacked : public CAccountView
{
protected:
	CAccountView * pBase;
public:
	CAccountViewBacked(CAccountView &accountView);
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


class CAccountViewCache : public CAccountViewBacked
{
public:
    uint256 hashBlock;
    map<CKeyID, CSecureAccount> cacheAccounts;
    map<string, CKeyID> cacheKeyIds;
public:
    CAccountViewCache(CAccountView &base, bool fDummy=false);
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

	bool Flush();
	unsigned int GetCacheSize();

};

#endif

