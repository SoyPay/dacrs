#include "account.h"
#include "util.h"
#include "serialize.h"

bool CAccountView::GetAccount(const CKeyID &keyId, CSecureAccount &secureAccount) {return false;}
bool CAccountView::SetAccount(const CKeyID &keyId, const CSecureAccount &secureAccount) {return false;}
bool CAccountView::SetAccount(const vector<unsigned char> &accountId, const CSecureAccount &secureAccount) {return false;}
bool CAccountView::HaveAccount(const CKeyID &keyId) {return false;}
uint256 CAccountView::GetBestBlock() {return false;}
bool CAccountView::SetBestBlock(const uint256 &hashBlock) {return false;}
bool CAccountView::BatchWrite(const map<CKeyID, CSecureAccount> &mapAccounts, const map<string, CKeyID> &mapKeyIds, const uint256 &hashBlock) {return false;}
bool CAccountView::BatchWrite(const vector<CSecureAccount> &vAccounts) {return false;}
bool CAccountView::EraseAccount(const CKeyID &keyId) {return false;}
bool CAccountView::SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId) {return false;}
bool CAccountView::GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId) {return false;}
bool CAccountView::GetAccount(const vector<unsigned char> &accountId, CSecureAccount &secureAccount) {return false;}
bool CAccountView::EraseKeyId(const vector<unsigned char> &accountId){
	return false;
}
bool CAccountView::SaveAccountInfo(const vector<unsigned char> &accountId, const CKeyID &keyId, const CSecureAccount &secureAccount) {return false;}


CAccountViewBacked::CAccountViewBacked(CAccountView &accountView):pBase(&accountView) {}
bool CAccountViewBacked::GetAccount(const CKeyID &keyId, CSecureAccount &secureAccount) {
	return pBase->GetAccount(keyId, secureAccount);
}
bool CAccountViewBacked::SetAccount(const CKeyID &keyId, const CSecureAccount &secureAccount) {
	return pBase->SetAccount(keyId, secureAccount);
}
bool CAccountViewBacked::SetAccount(const vector<unsigned char> &accountId, const CSecureAccount &secureAccount) {
	return pBase->SetAccount(accountId, secureAccount);
}
bool CAccountViewBacked::HaveAccount(const CKeyID &keyId) {
	return pBase->HaveAccount(keyId);
}
uint256 CAccountViewBacked::GetBestBlock() {
	return pBase->GetBestBlock();
}
bool CAccountViewBacked::SetBestBlock(const uint256 &hashBlock) {
	return pBase->SetBestBlock(hashBlock);
}
bool CAccountViewBacked::BatchWrite(const map<CKeyID , CSecureAccount> &mapAccounts, const map<std::string, CKeyID> &mapKeyIds, const uint256 &hashBlock) {
	return pBase->BatchWrite(mapAccounts, mapKeyIds, hashBlock);
}
bool CAccountViewBacked::BatchWrite(const vector<CSecureAccount> &vAccounts) {
	return pBase->BatchWrite(vAccounts);
}
bool CAccountViewBacked::EraseAccount(const CKeyID &keyId) {
	return pBase->EraseAccount(keyId);
}
bool CAccountViewBacked::SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId) {
	return pBase->SetKeyId(accountId, keyId);
}
bool CAccountViewBacked::GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId) {
	return pBase->GetKeyId(accountId, keyId);
}
bool CAccountViewBacked::EraseKeyId(const vector<unsigned char> &accountId){
	return pBase->EraseKeyId(accountId);
}
bool CAccountViewBacked::GetAccount(const vector<unsigned char> &accountId, CSecureAccount &secureAccount) {
	return pBase->GetAccount(accountId, secureAccount);
}
bool CAccountViewBacked::SaveAccountInfo(const vector<unsigned char> &accountId, const CKeyID &keyId,
		const CSecureAccount &secureAccount) {
	return pBase->SaveAccountInfo(accountId, keyId, secureAccount);
}


CAccountViewCache::CAccountViewCache(CAccountView &accountView, bool fDummy):CAccountViewBacked(accountView), hashBlock(0) {}
bool CAccountViewCache::GetAccount(const CKeyID &keyId, CSecureAccount &secureAccount) {
	if (cacheAccounts.count(keyId)) {
		secureAccount = cacheAccounts[keyId];
		return true;
	}
	if (pBase->GetAccount(keyId, secureAccount)) {
		cacheAccounts[keyId] = secureAccount;
		return true;
	}
	return false;
}
bool CAccountViewCache::SetAccount(const CKeyID &keyId, const CSecureAccount &secureAccount) {
	cacheAccounts[keyId] = secureAccount;
	return true;
}
bool CAccountViewCache::SetAccount(const vector<unsigned char> &accountId, const CSecureAccount &secureAccount) {
	if(cacheKeyIds.count(HexStr(accountId))) {
		cacheAccounts[cacheKeyIds[HexStr(accountId)]] = secureAccount;
		return true;
	}
	return false;
}
bool CAccountViewCache::HaveAccount(const CKeyID &keyId) {
	if(cacheAccounts.count(keyId))
		return true;
	else
		return pBase->HaveAccount(keyId);
}
uint256 CAccountViewCache::GetBestBlock() {
	if(hashBlock == uint256(0))
		return pBase->GetBestBlock();
	return hashBlock;
}
bool CAccountViewCache::SetBestBlock(const uint256 &hashBlockIn) {
	hashBlock = hashBlockIn;
	return true;
}
bool CAccountViewCache::BatchWrite(const map<CKeyID, CSecureAccount> &mapAccounts, const map<std::string, CKeyID> &mapKeyIds, const uint256 &hashBlockIn) {
	for (map<CKeyID, CSecureAccount>::const_iterator it = mapAccounts.begin(); it != mapAccounts.end(); ++it) {
		if (uint160(0) == it->second.keyID) {
			pBase->EraseAccount(it->first);
			cacheAccounts.erase(it->first);
		} else {
			cacheAccounts[it->first] = it->second;
		}
	}

    for	(map<string, CKeyID>::const_iterator itKeyId = mapKeyIds.begin(); itKeyId != mapKeyIds.end(); ++itKeyId)
    	cacheKeyIds[itKeyId->first] = itKeyId->second;
    hashBlock = hashBlockIn;
	return true;
}
bool CAccountViewCache::BatchWrite(const vector<CSecureAccount> &vAccounts) {
	for (vector<CSecureAccount>::const_iterator it = vAccounts.begin(); it != vAccounts.end(); ++it)
		if(it->IsEmptyValue() && !it->IsRegister()) {
			CSecureAccount secureAccount = *it;
			secureAccount.keyID = uint160(0);
			cacheAccounts[it->keyID] = secureAccount;
		} else {
			cacheAccounts[it->keyID] = *it;
		}
	return true;
}
bool CAccountViewCache::EraseAccount(const CKeyID &keyId) {
	if(cacheAccounts.count(keyId))
		cacheAccounts[keyId].keyID = uint160(0);
	return true;
}
bool CAccountViewCache::SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId) {
	cacheKeyIds[HexStr(accountId)] = keyId;
	return true;
}
bool CAccountViewCache::GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId) {
	if(cacheKeyIds.count(HexStr(accountId)))
	{
		keyId = cacheKeyIds[HexStr(accountId)];
		return true;
	}

	if(pBase->GetKeyId(accountId, keyId) ){
		cacheKeyIds[HexStr(accountId)] = keyId;
		return true;
	}

	return false;
}

bool CAccountViewCache::EraseKeyId(const vector<unsigned char> &accountId) {
	if (cacheKeyIds.count(HexStr(accountId)))
		cacheKeyIds[HexStr(accountId)] = uint160(0);
	return true;
}
bool CAccountViewCache::GetAccount(const vector<unsigned char> &accountId, CSecureAccount &secureAccount) {
	if(cacheKeyIds.count(HexStr(accountId))) {
		if(cacheAccounts.count(cacheKeyIds[HexStr(accountId)])){
			secureAccount = cacheAccounts[cacheKeyIds[HexStr(accountId)]];
			return true;
		}else {
			return pBase->GetAccount(cacheKeyIds[HexStr(accountId)], secureAccount);
		}
	}else {
		CKeyID keyId;
		if(pBase->GetKeyId(accountId, keyId)) {
			cacheKeyIds[HexStr(accountId)] = keyId;
			bool ret = pBase->GetAccount(keyId, secureAccount);
			if(ret) {
				cacheAccounts[keyId] = secureAccount;
				return true;
			}
		}
	}
	return false;
}
bool CAccountViewCache::SaveAccountInfo(const vector<unsigned char> &accountId, const CKeyID &keyId, const CSecureAccount &secureAccount) {
	cacheKeyIds[HexStr(accountId)] = keyId;
	cacheAccounts[keyId] = secureAccount;
	return true;
}
bool CAccountViewCache::Flush(){
	 bool fOk = pBase->BatchWrite(cacheAccounts, cacheKeyIds, hashBlock);
	 if (fOk) {
		cacheAccounts.clear();
		cacheKeyIds.clear();
	 }
	 return fOk;
}
unsigned int CAccountViewCache::GetCacheSize(){
	return ::GetSerializeSize(cacheAccounts, SER_DISK, CLIENT_VERSION);
}
