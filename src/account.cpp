#include "account.h"
#include "util.h"
#include "serialize.h"

bool CAccountView::GetAccount(const CKeyID &keyId, CAccount &account) {return false;}
bool CAccountView::SetAccount(const CKeyID &keyId, const CAccount &account) {return false;}
bool CAccountView::SetAccount(const vector<unsigned char> &accountId, const CAccount &account) {return false;}
bool CAccountView::HaveAccount(const CKeyID &keyId) {return false;}
uint256 CAccountView::GetBestBlock() {return false;}
bool CAccountView::SetBestBlock(const uint256 &hashBlock) {return false;}
bool CAccountView::BatchWrite(const map<CKeyID, CAccount> &mapAccounts, const map<string, CKeyID> &mapKeyIds, const uint256 &hashBlock) {return false;}
bool CAccountView::BatchWrite(const vector<CAccount> &vAccounts) {return false;}
bool CAccountView::EraseAccount(const CKeyID &keyId) {return false;}
bool CAccountView::SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId) {return false;}
bool CAccountView::GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId) {return false;}
bool CAccountView::GetAccount(const vector<unsigned char> &accountId, CAccount &account) {return false;}
bool CAccountView::EraseKeyId(const vector<unsigned char> &accountId){
	return false;
}
bool CAccountView::SaveAccountInfo(const vector<unsigned char> &accountId, const CKeyID &keyId, const CAccount &account) {return false;}


CAccountViewBacked::CAccountViewBacked(CAccountView &accountView):pBase(&accountView) {}
bool CAccountViewBacked::GetAccount(const CKeyID &keyId, CAccount &account) {
	return pBase->GetAccount(keyId, account);
}
bool CAccountViewBacked::SetAccount(const CKeyID &keyId, const CAccount &account) {
	return pBase->SetAccount(keyId, account);
}
bool CAccountViewBacked::SetAccount(const vector<unsigned char> &accountId, const CAccount &account) {
	return pBase->SetAccount(accountId, account);
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
bool CAccountViewBacked::BatchWrite(const map<CKeyID , CAccount> &mapAccounts, const map<std::string, CKeyID> &mapKeyIds, const uint256 &hashBlock) {
	return pBase->BatchWrite(mapAccounts, mapKeyIds, hashBlock);
}
bool CAccountViewBacked::BatchWrite(const vector<CAccount> &vAccounts) {
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
bool CAccountViewBacked::GetAccount(const vector<unsigned char> &accountId, CAccount &account) {
	return pBase->GetAccount(accountId, account);
}
bool CAccountViewBacked::SaveAccountInfo(const vector<unsigned char> &accountId, const CKeyID &keyId,
		const CAccount &account) {
	return pBase->SaveAccountInfo(accountId, keyId, account);
}


CAccountViewCache::CAccountViewCache(CAccountView &accountView, bool fDummy):CAccountViewBacked(accountView), hashBlock(0) {}
bool CAccountViewCache::GetAccount(const CKeyID &keyId, CAccount &account) {
	if (cacheAccounts.count(keyId) && cacheAccounts[keyId].keyID != uint160(0)) {
		account = cacheAccounts[keyId];
		return true;
	}
	if (pBase->GetAccount(keyId, account)) {
		cacheAccounts[keyId] = account;
		return true;
	}
	return false;
}
bool CAccountViewCache::SetAccount(const CKeyID &keyId, const CAccount &account) {
	cacheAccounts[keyId] = account;
	cacheAccounts[keyId].accountOperLog.SetNULL();
	return true;
}
bool CAccountViewCache::SetAccount(const vector<unsigned char> &accountId, const CAccount &account) {
	if(cacheKeyIds.count(HexStr(accountId))) {
		cacheAccounts[cacheKeyIds[HexStr(accountId)]] = account;
		cacheAccounts[cacheKeyIds[HexStr(accountId)]].accountOperLog.SetNULL();
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
bool CAccountViewCache::BatchWrite(const map<CKeyID, CAccount> &mapAccounts, const map<std::string, CKeyID> &mapKeyIds, const uint256 &hashBlockIn) {
	for (map<CKeyID, CAccount>::const_iterator it = mapAccounts.begin(); it != mapAccounts.end(); ++it) {
		if (uint160(0) == it->second.keyID) {
			pBase->EraseAccount(it->first);
			cacheAccounts.erase(it->first);
		} else {
			cacheAccounts[it->first] = it->second;
			cacheAccounts[it->first].accountOperLog.SetNULL();
		}
	}

    for	(map<string, CKeyID>::const_iterator itKeyId = mapKeyIds.begin(); itKeyId != mapKeyIds.end(); ++itKeyId)
    	cacheKeyIds[itKeyId->first] = itKeyId->second;
    hashBlock = hashBlockIn;
	return true;
}
bool CAccountViewCache::BatchWrite(const vector<CAccount> &vAccounts) {
	for (vector<CAccount>::const_iterator it = vAccounts.begin(); it != vAccounts.end(); ++it)
		if(it->IsEmptyValue() && !it->IsRegister()) {
			CAccount account = *it;
			account.keyID = uint160(0);
			account.accountOperLog.SetNULL();
			cacheAccounts[it->keyID] = account;
		} else {
			cacheAccounts[it->keyID] = *it;
			cacheAccounts[it->keyID].accountOperLog.SetNULL();
		}
	return true;
}
bool CAccountViewCache::EraseAccount(const CKeyID &keyId) {
	if(cacheAccounts.count(keyId))
		cacheAccounts[keyId].keyID = uint160(0);
	else {
		CAccount account;
		if(pBase->GetAccount(keyId, account)) {
			account.keyID = uint160(0);
			cacheAccounts[keyId] = account;
		}
	}
	return true;
}
bool CAccountViewCache::SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId) {
	cacheKeyIds[HexStr(accountId)] = keyId;
	return true;
}
bool CAccountViewCache::GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId) {
	if(cacheKeyIds.count(HexStr(accountId)) && cacheKeyIds[HexStr(accountId)] != uint160(0))
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
	else {
		CKeyID keyId;
		if(pBase->GetKeyId(accountId, keyId)) {
			cacheKeyIds[HexStr(accountId)] = uint160(0);
		}
	}
	return true;
}
bool CAccountViewCache::GetAccount(const vector<unsigned char> &accountId, CAccount &account) {
	if(cacheKeyIds.count(HexStr(accountId)) && cacheKeyIds[HexStr(accountId)] !=uint160(0)) {
		if(cacheAccounts.count(cacheKeyIds[HexStr(accountId)])){
			if(cacheAccounts[cacheKeyIds[HexStr(accountId)]].keyID != uint160(0)) {  // 判断此帐户是否被删除了
				account = cacheAccounts[cacheKeyIds[HexStr(accountId)]];
				return true;
			}else {
				return false;   //已删除返回false
			}
		}else {
			return pBase->GetAccount(cacheKeyIds[HexStr(accountId)], account);
		}
	}else {
		CKeyID keyId;
		if(pBase->GetKeyId(accountId, keyId)) {
			cacheKeyIds[HexStr(accountId)] = keyId;
			if (cacheAccounts.count(keyId) > 0 ) {
				if (cacheAccounts[cacheKeyIds[HexStr(accountId)]].keyID != uint160(0)) { // 判断此帐户是否被删除了
					account = cacheAccounts[keyId];
					return true;
				} else {
					return false;   //已删除返回false
				}
			}
			bool ret = pBase->GetAccount(keyId, account);
			if(ret) {
				cacheAccounts[keyId] = account;
				return true;
			}
		}
	}
	return false;
}
bool CAccountViewCache::SaveAccountInfo(const CRegID &regid, const CKeyID &keyId, const CAccount &account) {
	cacheKeyIds[HexStr(regid.GetVec6())] = keyId;
	cacheAccounts[keyId] = account;
	cacheAccounts[keyId].accountOperLog.SetNULL();
	return true;
}
bool CAccountViewCache::GetAccount(const CUserID &userId, CAccount &account) {
	bool ret = false;
	if (userId.type() == typeid(CRegID)) {
		ret = GetAccount(boost::get<CRegID>(userId).GetVec6(), account);
		if(ret) assert(boost::get<CRegID>(userId) == account.regID);
	} else if (userId.type() == typeid(CKeyID)) {
		ret = GetAccount(boost::get<CKeyID>(userId), account);
		if(ret) assert(boost::get<CKeyID>(userId) == account.keyID);
	} else if (userId.type() == typeid(CPubKey)) {
		ret = GetAccount(boost::get<CPubKey>(userId).GetKeyID(), account);
		if(ret) assert((boost::get<CPubKey>(userId)).GetKeyID() == account.keyID);
	} else {
		assert(0);
	}
	return ret;
}
bool CAccountViewCache::GetKeyId(const CUserID &userId, CKeyID &keyId) {
	if (userId.type() == typeid(CRegID)) {
		return GetKeyId(boost::get<CRegID>(userId).GetVec6(), keyId);
	} else if (userId.type() == typeid(CPubKey)) {
		keyId = boost::get<CPubKey>(userId).GetKeyID();
		return true;
	} else {
		assert(0);
	}
	return false;
}
bool CAccountViewCache::SetAccount(const CUserID &userId, const CAccount &account) {
	if (userId.type() == typeid(CRegID)) {
		return SetAccount(boost::get<CRegID>(userId).GetVec6(), account);
	} else if (userId.type() == typeid(CKeyID)) {
		return SetAccount(boost::get<CKeyID>(userId), account);
	} else {
		assert(0);
	}
	return false;
}
bool CAccountViewCache::SetKeyId(const CUserID &userId, const CKeyID &keyId) {
	if (userId.type() == typeid(CRegID)) {
		return SetKeyId(boost::get<CRegID>(userId).GetVec6(), keyId);
	} else {
		assert(0);
	}

	return false;

}
bool CAccountViewCache::EraseAccount(const CUserID &userId) {
	if (userId.type() == typeid(CKeyID)) {
		return EraseAccount(boost::get<CKeyID>(userId));
	} else {
		assert(0);
	}
	return false;
}
bool CAccountViewCache::HaveAccount(const CUserID &userId) {
	if (userId.type() == typeid(CKeyID)) {
		return HaveAccount(boost::get<CKeyID>(userId));
	} else {
		assert(0);
	}
	return false;
}
bool CAccountViewCache::EraseId(const CUserID &userId) {
	if (userId.type() == typeid(CRegID)) {
		return EraseKeyId(boost::get<CRegID>(userId).GetVec6());
	} else {
		assert(0);
	}
	return false;
}

bool CAccountViewCache::Flush(){
	 bool fOk = pBase->BatchWrite(cacheAccounts, cacheKeyIds, hashBlock);
	 if (fOk) {
		cacheAccounts.clear();
		cacheKeyIds.clear();
	 }
	 return fOk;
}

bool CAccountViewCache::GetRegId(const CUserID& userId, CRegID& regId) const{

	CAccountViewCache tempView(*this);
	CAccount account;
	if(tempView.GetAccount(userId,account))
	{
		regId =  account.regID;
		return !regId.IsEmpty();
	}
	return false;
}

int64_t CAccountViewCache::GetBalance(const CUserID& userId,int curhigh) const {
	CAccountViewCache tempvew(*this);
	CAccount account;
	if(tempvew.GetAccount(userId,account))
	{
		return  account.GetBalance(curhigh);
	}
	return 0;
}

unsigned int CAccountViewCache::GetCacheSize(){
	return ::GetSerializeSize(cacheAccounts, SER_DISK, CLIENT_VERSION) + ::GetSerializeSize(cacheKeyIds, SER_DISK, CLIENT_VERSION);
}

bool CScriptDBView::GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue) {	return false;}
bool CScriptDBView::SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue) {return false;}
bool CScriptDBView::BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas) {return false;}
bool CScriptDBView::EraseKey(const vector<unsigned char> &vKey) {return false;}
bool CScriptDBView::HaveData(const vector<unsigned char> &vKey) {return false;}
bool CScriptDBView::GetScript(const int &nIndex, vector<unsigned char> &vScriptId, vector<unsigned char> &vValue) {return false;}
bool CScriptDBView::GetScriptData(const vector<unsigned char> &vScriptId, const int &nIndex,
		vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData, int &nHeight) {return false;}

CScriptDBViewBacked::CScriptDBViewBacked(CScriptDBView &dataBaseView) {pBase = &dataBaseView;}
bool CScriptDBViewBacked::GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue) {return pBase->GetData(vKey, vValue);}
bool CScriptDBViewBacked::SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue) {return pBase->SetData(vKey, vValue);}
bool CScriptDBViewBacked::BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas) {return pBase->BatchWrite(mapDatas);}
bool CScriptDBViewBacked::EraseKey(const vector<unsigned char> &vKey) {return pBase->EraseKey(vKey);}
bool CScriptDBViewBacked::HaveData(const vector<unsigned char> &vKey) {return pBase->HaveData(vKey);}
bool CScriptDBViewBacked::GetScript(const int &nIndex, vector<unsigned char> &vScriptId, vector<unsigned char> &vValue) {return pBase->GetScript(nIndex, vScriptId, vValue);}
bool CScriptDBViewBacked::GetScriptData(const vector<unsigned char> &vScriptId, const int &nIndex,
		vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData, int &nHeight) {
	return pBase->GetScriptData(vScriptId, nIndex, vScriptKey, vScriptData, nHeight);
}

CScriptDBViewCache::CScriptDBViewCache(CScriptDBView &base, bool fDummy) : CScriptDBViewBacked(base) {
	mapDatas.clear();
	LogPrint("INFO","new a CScriptDBViewCache mapDatas size:%d\r\n",mapDatas.size());
}

bool CScriptDBViewCache::GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue) {
	if (mapDatas.count(vKey) > 0 && !mapDatas[vKey].empty()) {
		vValue = mapDatas[vKey];
		return true;
	}
	if (!pBase->GetData(vKey, vValue))
		return false;
	mapDatas[vKey] = vValue;
//	if(vKey == vTempTest) {
//		LogPrint("INFO", "set mapDatas key:%s, value:%s\n", HexStr(vKey), HexStr(vValue));
//	}
	return true;
}
bool CScriptDBViewCache::SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue) {
	mapDatas[vKey] = vValue;
//	if (vKey == vTempTest) {
//		LogPrint("INFO", "set mapDatas key:%s, value:%s\n", HexStr(vKey), HexStr(vValue));
//	}
	return true;
}
bool CScriptDBViewCache::BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapData) {
	for (auto &items : mapData) {
		mapDatas[items.first] = items.second;
//		if (items.first == vTempTest) {
//			LogPrint("INFO", "set mapDatas key:%s, value:%s\n", HexStr(items.first), HexStr(mapDatas[items.first]));
//		}
	}
	return true;
}
bool CScriptDBViewCache::EraseKey(const vector<unsigned char> &vKey) {
	if (mapDatas.count(vKey) > 0) {
		mapDatas[vKey].clear();
	} else {
		vector<unsigned char> vValue;
		if (pBase->GetData(vKey, vValue)) {
			vValue.clear();
			mapDatas[vKey] = vValue;
//			if (vKey == vTempTest) {
//				LogPrint("INFO", "set mapDatas key:%s, value:%s\n", HexStr(vKey), HexStr(mapDatas[vKey]));
//			}
		}
		else {
			return false;
		}
	}
	return true;
}
bool CScriptDBViewCache::HaveData(const vector<unsigned char> &vKey) {
	if (mapDatas.count(vKey) > 0) {
		if(!mapDatas[vKey].empty())
			return true;
		else
			return false;
	}
	return pBase->HaveData(vKey);
}
bool CScriptDBViewCache::GetScript(const int nIndex, vector<unsigned char> &vScriptId, vector<unsigned char> &vValue) {
	return pBase->GetScript(nIndex, vScriptId, vValue);
}
bool CScriptDBViewCache::SetScript(const vector<unsigned char> &vScriptId, const vector<unsigned char> &vValue) {
	vector<unsigned char> scriptKey = {'d','e','f'};
	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());

	if (!HaveScript(vScriptId)) {
		int nCount(0);
		GetScriptCount(nCount);
		++nCount;
		if (!SetScriptCount(nCount))
			return false;
	}
	return SetData(scriptKey, vValue);
}
bool CScriptDBViewCache::Flush() {
	bool ok = pBase->BatchWrite(mapDatas);
	if(ok) {
		mapDatas.clear();
	}
	LogPrint("SetScriptData","Flush ret: %d %p\r\n",ok,this);

	return ok;
}
unsigned int CScriptDBViewCache::GetCacheSize() {
	return ::GetSerializeSize(mapDatas, SER_DISK, CLIENT_VERSION);
}

bool CScriptDBViewCache::GetScript(const vector<unsigned char> &vScriptId, vector<unsigned char> &vValue) {
	vector<unsigned char> scriptKey = { 'd', 'e', 'f' };

	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());
	return GetData(scriptKey, vValue);
}
bool CScriptDBViewCache::GetScript(const CRegID &scriptId, vector<unsigned char> &vValue) {
	return GetScript(scriptId.GetVec6(), vValue);
}

bool CScriptDBViewCache::GetScriptData(const vector<unsigned char> &vScriptId, const vector<unsigned char> &vScriptKey,
		vector<unsigned char> &vScriptData, int &nHeight) {
//	assert(vScriptKey.size() == 8);
	vector<unsigned char> vKey = { 'd', 'a', 't', 'a' };

	vKey.insert(vKey.end(), vScriptId.begin(), vScriptId.end());
	vKey.push_back('_');
	vKey.insert(vKey.end(), vScriptKey.begin(), vScriptKey.end());
	vector<unsigned char> vValue;
	if (!GetData(vKey, vValue))
		return false;
	if(vValue.empty())
		return false;
	CDataStream ds(vValue, SER_DISK, CLIENT_VERSION);
	ds >> nHeight;
	ds >> vScriptData;
	return true;
}
bool CScriptDBViewCache::GetScriptData(const vector<unsigned char> &vScriptId, const int &nIndex,
		vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData, int &nHeight) {
	if(0 == nIndex) {
		vector<unsigned char> vKey = { 'd', 'a', 't', 'a' };
		vKey.insert(vKey.end(), vScriptId.begin(), vScriptId.end());
		vKey.push_back('_');
		vector<unsigned char> vDataKey;
		vector<unsigned char> vDataValue;
		vDataKey.clear();
		vDataValue.clear();
		for (auto &item : mapDatas) {
			vector<unsigned char> vTemp(item.first.begin(),item.first.begin()+vScriptId.size()+5);
			if(vKey == vTemp) {
				if(item.second.empty()) {
					continue;
				}
				vDataKey = item.first;
				vDataValue = item.second;
				break;
			}
		}
		if(!pBase->GetScriptData(vScriptId, nIndex, vScriptKey, vScriptData, nHeight)) {
			if (vDataKey.empty())
				return false;
			else {
				vScriptKey.clear();
				vScriptData.clear();
				vScriptKey.insert(vScriptKey.end(), vDataKey.begin()+11, vDataKey.end());
				if (mapDatas[vDataKey].empty()) {
					return false;
				}
				CDataStream ds(mapDatas[vDataKey], SER_DISK, CLIENT_VERSION);
				ds >> nHeight;
				ds >> vScriptData;
				return true;
			}
		}
		else {
			vector<unsigned char> dataKeyTemp(vKey.begin(), vKey.end());
			dataKeyTemp.insert(dataKeyTemp.end(), vScriptKey.begin(), vScriptKey.end());
			if(vDataKey.empty()) {  //缓存中没有符合条件的key，直接返回从数据库中查询结果
				return true;
			}
			if(dataKeyTemp < vDataKey )  {  //若数据库中查询的key小于缓存的key,且此key在缓存中没有，则直接返回数据库中查询的结果
				if(mapDatas.count(dataKeyTemp) == 0)
					return true;
				else {
					mapDatas[dataKeyTemp].clear();  //在缓存中dataKeyTemp已经被删除过了，重新将此key对应的value清除
					return GetScriptData(vScriptId, 1, vScriptKey, vScriptData, nHeight); //重新从数据库中获取下一条数据
				}
			}
			else if(dataKeyTemp == vDataKey && vDataValue.empty()){ //若数据库中查询的key与缓存查询的key相等，直接返回查询的结果
				return true;
			}
			else{
				vScriptKey.clear();
				vScriptData.clear();
				vScriptKey.insert(vScriptKey.end(), vDataKey.begin()+11, vDataKey.end());
				CDataStream ds(mapDatas[vDataKey], SER_DISK, CLIENT_VERSION);
				ds >> nHeight;
				ds >> vScriptData;
				return true;
			}
		}
	}
	else if (1 == nIndex) {
		vector<unsigned char> vKey = { 'd', 'a', 't', 'a' };
		vKey.insert(vKey.end(), vScriptId.begin(), vScriptId.end());
		vKey.push_back('_');
		vKey.insert(vKey.end(), vScriptKey.begin(), vScriptKey.end());
		map<vector<unsigned char>, vector<unsigned char> >::iterator iterFindKey = mapDatas.find(vKey);
		vector<unsigned char> vDataKey;
		vDataKey.clear();
		while (iterFindKey != mapDatas.end() && ++iterFindKey != mapDatas.end()) {
			vector<unsigned char> vKeyTemp(vKey.begin(), vKey.begin() + vScriptId.size() + 5);
			vector<unsigned char> vTemp(iterFindKey->first.begin(), iterFindKey->first.begin() + vScriptId.size() + 5);
			if (vKeyTemp == vTemp) {
				if(iterFindKey->second.empty())
					continue;
				else {
					vDataKey = iterFindKey->first;
					break;
				}

			}
		}
		if (!pBase->GetScriptData(vScriptId, nIndex, vScriptKey, vScriptData, nHeight)) {
			if (vDataKey.empty())
				return false;
			else {
				vScriptKey.clear();
				vScriptData.clear();
				vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
				if (mapDatas[vDataKey].empty()) {
					return false;
				}
				CDataStream ds(mapDatas[vDataKey], SER_DISK, CLIENT_VERSION);
				ds >> nHeight;
				ds >> vScriptData;
				return true;
			}
		} else {
			vector<unsigned char> dataKeyTemp(vKey.begin(), vKey.end());
			dataKeyTemp.insert(dataKeyTemp.end(), vScriptKey.begin(), vScriptKey.end());
			if (vDataKey.empty())
					return true;
			if (dataKeyTemp < vDataKey) {
				return true;
			} else {
				vScriptKey.clear();
				vScriptData.clear();
				vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
				CDataStream ds(mapDatas[vDataKey], SER_DISK, CLIENT_VERSION);
				ds >> nHeight;
				ds >> vScriptData;
				return true;
			}
		}

	}
	else {
		assert(0);
	}
	return pBase->GetScriptData(vScriptId, nIndex, vScriptKey, vScriptData, nHeight);
}
bool CScriptDBViewCache::SetScriptData(const vector<unsigned char> &vScriptId, const vector<unsigned char> &vScriptKey,
		const vector<unsigned char> &vScriptData, const int nHeight, CScriptDBOperLog &operLog) {
	vector<unsigned char> vKey = { 'd', 'a', 't', 'a' };
	vKey.insert(vKey.end(), vScriptId.begin(), vScriptId.end());
	vKey.push_back('_');
	vKey.insert(vKey.end(), vScriptKey.begin(), vScriptKey.end());
  //  LogPrint("vm","add data:%s",HexStr(vScriptKey).c_str());
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << nHeight;
	ds << vScriptData;
	vector<unsigned char> vValue(ds.begin(), ds.end());
	if (!HaveScriptData(vScriptId, vScriptKey)) {
		int nCount(0);
		GetScriptDataCount(vScriptId, nCount);
		++nCount;
		if (!SetScriptDataCount(vScriptId, nCount))
			return false;
	}
	vector<unsigned char> oldValue;
	oldValue.clear();
	GetData(vKey, oldValue);
	operLog = CScriptDBOperLog(vKey, oldValue);
	return SetData(vKey, vValue);
}

bool CScriptDBViewCache::HaveScript(const vector<unsigned char> &vScriptId) {
	vector<unsigned char> scriptKey = { 'd', 'e', 'f' };
	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());
	return HaveData(scriptKey);
}
bool CScriptDBViewCache::GetScriptCount(int &nCount) {
	vector<unsigned char> scriptKey = { 's', 'n', 'u','m'};
	vector<unsigned char> vValue;
	if (!GetData(scriptKey, vValue))
		return false;
	CDataStream ds(vValue, SER_DISK, CLIENT_VERSION);
	ds >> nCount;
	return true;
}
bool CScriptDBViewCache::SetScriptCount(const int nCount) {
	vector<unsigned char> scriptKey = { 's', 'n', 'u','m'};
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << nCount;
	vector<unsigned char> vValue(ds.begin(), ds.end());
	if(!SetData(scriptKey, vValue))
		return false;
	return true;
}
bool CScriptDBViewCache::EraseScript(const vector<unsigned char> &vScriptId) {
	vector<unsigned char> scriptKey = { 'd', 'e', 'f' };
	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());
	if (HaveScript(vScriptId)) {
		int nCount(0);
		if (!GetScriptCount(nCount))
			return false;
		--nCount;
		if (!SetScriptCount(nCount))
			return false;
	}
	return EraseKey(scriptKey);
}
bool CScriptDBViewCache::GetScriptDataCount(const vector<unsigned char> &vScriptId, int &nCount) {
	vector<unsigned char> scriptKey = { 's', 'd', 'n', 'u','m'};
	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());
	vector<unsigned char> vValue;
	if(!GetData(scriptKey, vValue))
		return false;
	CDataStream ds(vValue, SER_DISK, CLIENT_VERSION);
	ds >> nCount;
	return true;
}
bool CScriptDBViewCache::SetScriptDataCount(const vector<unsigned char> &vScriptId, int nCount) {
	vector<unsigned char> scriptKey = { 's', 'd', 'n', 'u','m'};

	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << nCount;
	vector<unsigned char> vValue(ds.begin(), ds.end());
	if(!SetData(scriptKey, vValue))
		return false;
	return true;
}

bool CScriptDBViewCache::EraseScriptData(const vector<unsigned char> &vScriptId, const vector<unsigned char> &vScriptKey, CScriptDBOperLog &operLog) {
	vector<unsigned char> scriptKey = { 'd', 'a', 't', 'a'};

	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());
	scriptKey.push_back('_');
	scriptKey.insert(scriptKey.end(), vScriptKey.begin(), vScriptKey.end());

	if (HaveScriptData(vScriptId, vScriptKey)) {
		int nCount(0);
		GetScriptDataCount(vScriptId, nCount);
		--nCount;
		if (!SetScriptDataCount(vScriptId, nCount))
			return false;
	}

	vector<unsigned char> vValue;
	if(!GetData(scriptKey, vValue))
		return false;
	operLog = CScriptDBOperLog(scriptKey, vValue);

	if(!EraseKey(scriptKey))
		return false;

	return true;
}
bool CScriptDBViewCache::HaveScriptData(const vector<unsigned char> &vScriptId, const vector<unsigned char > &vScriptKey) {
	vector<unsigned char> scriptKey = { 'd', 'a', 't', 'a'};
	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());
	scriptKey.push_back('_');
	scriptKey.insert(scriptKey.end(), vScriptKey.begin(), vScriptKey.end());
	return HaveData(scriptKey);
}


bool CScriptDBViewCache::GetScript(const int nIndex, CRegID &scriptId, vector<unsigned char> &vValue) {
	return GetScript(nIndex, scriptId.GetVec6(), vValue);
}
bool CScriptDBViewCache::SetScript(const CRegID &scriptId, const vector<unsigned char> &vValue) {
	return SetScript(scriptId.GetVec6(), vValue);
}
bool CScriptDBViewCache::HaveScript(const CRegID &scriptId) {
	return HaveScript(scriptId.GetVec6());
}
bool CScriptDBViewCache::EraseScript(const CRegID &scriptId) {
	return EraseScript(scriptId.GetVec6());
}
bool CScriptDBViewCache::GetScriptDataCount(const CRegID &scriptId, int &nCount) {
	return GetScriptDataCount(scriptId.GetVec6(), nCount);
}
bool CScriptDBViewCache::EraseScriptData(const CRegID &scriptId, const vector<unsigned char> &vScriptKey, CScriptDBOperLog &operLog) {
	bool  temp = EraseScriptData(scriptId.GetVec6(), vScriptKey, operLog);
	LogPrint("SetScriptData","EraseScriptData sriptid ID:%s key:%s ret:%d %p \r\n",scriptId.ToString(),HexStr(vScriptKey),temp,this);
	return temp;
}
bool CScriptDBViewCache::HaveScriptData(const CRegID &scriptId, const vector<unsigned char > &vScriptKey) {
	return HaveScriptData(scriptId.GetVec6(), vScriptKey);
}
bool CScriptDBViewCache::GetScriptData(const CRegID &scriptId, const vector<unsigned char> &vScriptKey,
			vector<unsigned char> &vScriptData, int &nHeight) {
	return GetScriptData(scriptId.GetVec6() , vScriptKey, vScriptData, nHeight);
}
bool CScriptDBViewCache::GetScriptData(const CRegID &scriptId, const int &nIndex, vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData,
		int &nHeight) {
	return GetScriptData(scriptId.GetVec6(), nIndex, vScriptKey, vScriptData, nHeight);
}
bool CScriptDBViewCache::SetScriptData(const CRegID &scriptId, const vector<unsigned char> &vScriptKey,
			const vector<unsigned char> &vScriptData, const int nHeight, CScriptDBOperLog &operLog) {
	bool  temp =SetScriptData(scriptId.GetVec6(), vScriptKey, vScriptData, nHeight, operLog);
	LogPrint("SetScriptData","SetScriptData sriptid ID:%s key:%s value:%s ret: %d %p \r\n",scriptId.ToString(),HexStr(vScriptKey),HexStr(vScriptData),temp,this);
	return temp;
}
bool CScriptDBViewCache::SetTxRelAccout(const uint256 &txHash, const set<CKeyID> &relAccount) {
	vector<unsigned char> vKey = {'t','x'};
	vector<unsigned char> vValue;
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << txHash;
	vKey.insert(vKey.end(), ds.begin(), ds.end());
	ds.clear();
	ds << relAccount;
	vValue.assign(ds.begin(), ds.end());
	return SetData(vKey, vValue);
}
bool CScriptDBViewCache::GetTxRelAccount(const uint256 &txHash, set<CKeyID> &relAccount) {
	vector<unsigned char> vKey = {'t','x'};
	vector<unsigned char> vValue;
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << txHash;
	vKey.insert(vKey.end(), ds.begin(), ds.end());
	if(!GetData(vKey, vValue))
		return false;
	ds.clear();
	vector<char> temp;
	temp.assign(vValue.begin(), vValue.end());
	ds.insert(ds.end(), temp.begin(), temp.end());
	ds >> relAccount;
	return true;
}
