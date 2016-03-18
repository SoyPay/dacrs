#include "database.h"
#include "util.h"
#include "serialize.h"
#include "core.h"
#include "main.h"
#include "chainparams.h"
#include "vm/vmrunevn.h"
#include <algorithm>

bool CAccountView::GetAccount(const CKeyID &keyId, CAccount &account) {return false;}
bool CAccountView::SetAccount(const CKeyID &keyId, const CAccount &account) {return false;}
bool CAccountView::SetAccount(const vector<unsigned char> &accountId, const CAccount &account) {return false;}
bool CAccountView::HaveAccount(const CKeyID &keyId) {return false;}
uint256 CAccountView::GetBestBlock() {return uint256();}
bool CAccountView::SetBestBlock(const uint256 &hashBlock) {return false;}
bool CAccountView::BatchWrite(const map<CKeyID, CAccount> &mapAccounts, const map<vector<unsigned char>, CKeyID> &mapKeyIds, const uint256 &hashBlock) {return false;}
bool CAccountView::BatchWrite(const vector<CAccount> &vAccounts) {return false;}
bool CAccountView::EraseAccount(const CKeyID &keyId) {return false;}
bool CAccountView::SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId) {return false;}
bool CAccountView::GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId) {return false;}
bool CAccountView::GetAccount(const vector<unsigned char> &accountId, CAccount &account) {return false;}
bool CAccountView::EraseKeyId(const vector<unsigned char> &accountId){
	return false;
}
bool CAccountView::SaveAccountInfo(const vector<unsigned char> &accountId, const CKeyID &keyId, const CAccount &account) {return false;}
Object CAccountView::ToJosnObj(char Prefix){
	Object obj;
	return obj;
}
uint64_t CAccountView::TraverseAccount(){return 0;}

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
bool CAccountViewBacked::BatchWrite(const map<CKeyID , CAccount> &mapAccounts, const map<std::vector<unsigned char>, CKeyID> &mapKeyIds, const uint256 &hashBlock) {
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
uint64_t CAccountViewBacked::TraverseAccount(){return pBase->TraverseAccount();}

CAccountViewCache::CAccountViewCache(CAccountView &accountView, bool fDummy):CAccountViewBacked(accountView), hashBlock(uint256()) {}
bool CAccountViewCache::GetAccount(const CKeyID &keyId, CAccount &account) {
	if (cacheAccounts.count(keyId)) {
		if (cacheAccounts[keyId].keyID != uint160()) {
			account = cacheAccounts[keyId];
			return true;
		} else
			return false;
	}
	if (pBase->GetAccount(keyId, account)) {
		cacheAccounts[keyId] = account;
		return true;
	}
	return false;
}
bool CAccountViewCache::SetAccount(const CKeyID &keyId, const CAccount &account) {
	cacheAccounts[keyId] = account;
	return true;
}
bool CAccountViewCache::SetAccount(const vector<unsigned char> &accountId, const CAccount &account) {
	if(accountId.empty()) {
		return false;
	}
	if(cacheKeyIds.count(accountId)) {
		cacheAccounts[cacheKeyIds[accountId]] = account;
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
	if(hashBlock == uint256())
		return pBase->GetBestBlock();
	return hashBlock;
}
bool CAccountViewCache::SetBestBlock(const uint256 &hashBlockIn) {
	hashBlock = hashBlockIn;
	return true;
}
bool CAccountViewCache::BatchWrite(const map<CKeyID, CAccount> &mapAccounts, const map<vector<unsigned char>, CKeyID> &mapKeyIds, const uint256 &hashBlockIn) {
	for (map<CKeyID, CAccount>::const_iterator it = mapAccounts.begin(); it != mapAccounts.end(); ++it) {
		if (uint160() == it->second.keyID) {
			pBase->EraseAccount(it->first);
			cacheAccounts.erase(it->first);
		} else {
			cacheAccounts[it->first] = it->second;
		}
	}

    for	(map<vector<unsigned char>, CKeyID>::const_iterator itKeyId = mapKeyIds.begin(); itKeyId != mapKeyIds.end(); ++itKeyId)
    	cacheKeyIds[itKeyId->first] = itKeyId->second;
    hashBlock = hashBlockIn;
	return true;
}
bool CAccountViewCache::BatchWrite(const vector<CAccount> &vAccounts) {
	for (vector<CAccount>::const_iterator it = vAccounts.begin(); it != vAccounts.end(); ++it) {
		if (it->IsEmptyValue() && !it->IsRegister()) {
			cacheAccounts[it->keyID] = *it;
			cacheAccounts[it->keyID].keyID = uint160();
		} else {
			cacheAccounts[it->keyID] = *it;
		}
	}
	return true;
}
bool CAccountViewCache::EraseAccount(const CKeyID &keyId) {
	if(cacheAccounts.count(keyId))
		cacheAccounts[keyId].keyID = uint160();
	else {
		CAccount account;
		if(pBase->GetAccount(keyId, account)) {
			account.keyID = uint160();
			cacheAccounts[keyId] = account;
		}
	}
	return true;
}
bool CAccountViewCache::SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId) {
	if(accountId.empty())
		return false;
	cacheKeyIds[accountId] = keyId;
	return true;
}
bool CAccountViewCache::GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId) {
	if(accountId.empty())
		return false;
	if(cacheKeyIds.count(accountId)){
		keyId = cacheKeyIds[accountId];
		if(keyId != uint160())
		{
			return true;
		}
		else {
			return false;
		}
	}
	if(pBase->GetKeyId(accountId, keyId) ){
		cacheKeyIds[accountId] = keyId;
		return true;
	}
	return false;
}

bool CAccountViewCache::EraseKeyId(const vector<unsigned char> &accountId) {
	if(accountId.empty())
		return false;
	if (cacheKeyIds.count(accountId))
		cacheKeyIds[accountId] = uint160();
	else {
		CKeyID keyId;
		if(pBase->GetKeyId(accountId, keyId)) {
			cacheKeyIds[accountId] = uint160();
		}
	}
	return true;
}
bool CAccountViewCache::GetAccount(const vector<unsigned char> &accountId, CAccount &account) {
	if(accountId.empty()) {
		return false;
	}
	if(cacheKeyIds.count(accountId)) {
		CKeyID keyId(cacheKeyIds[accountId]);
		if(keyId != uint160()) {
			if(cacheAccounts.count(keyId)){
				account = cacheAccounts[keyId];
				if(account.keyID != uint160()) {  // �жϴ��ʻ��Ƿ�ɾ����
					return true;
				}else {
					return false;   //��ɾ������false
				}
			}else {
				return pBase->GetAccount(keyId, account); //����map��û�У����ϼ���ȡ
			}
		}
		else {
			return false;  //accountId��ɾ��˵���˻���ϢҲ��ɾ��
		}
	}else {
		CKeyID keyId;
		if(pBase->GetKeyId(accountId, keyId)) {
			cacheKeyIds[accountId] = keyId;
			if (cacheAccounts.count(keyId) > 0 ) {
				account = cacheAccounts[keyId];
				if (account.keyID != uint160()) { // �жϴ��ʻ��Ƿ�ɾ����
					return true;
				} else {
					return false;   //��ɾ������false
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
	cacheKeyIds[regid.GetVec6()] = keyId;
	cacheAccounts[keyId] = account;
	return true;
}

uint64_t CAccountViewCache::TraverseAccount() {
	return pBase->TraverseAccount();
}

bool CAccountViewCache::GetAccount(const CUserID &userId, CAccount &account) {
	bool ret = false;
	if (userId.type() == typeid(CRegID)) {
		ret = GetAccount(boost::get<CRegID>(userId).GetVec6(), account);
//		if(ret) assert(boost::get<CRegID>(userId) == account.regID);
	} else if (userId.type() == typeid(CKeyID)) {
		ret = GetAccount(boost::get<CKeyID>(userId), account);
//		if(ret) assert(boost::get<CKeyID>(userId) == account.keyID);
	} else if (userId.type() == typeid(CPubKey)) {
		ret = GetAccount(boost::get<CPubKey>(userId).GetKeyID(), account);
//		if(ret) assert((boost::get<CPubKey>(userId)).GetKeyID() == account.keyID);
	} else if (userId.type() == typeid(CNullID)){
		return ERRORMSG("GetAccount input userid can't be CNullID type");
	}
	return ret;
}
bool CAccountViewCache::GetKeyId(const CUserID &userId, CKeyID &keyId) {
	if (userId.type() == typeid(CRegID)) {
		return GetKeyId(boost::get<CRegID>(userId).GetVec6(), keyId);
	} else if (userId.type() == typeid(CPubKey)) {
		keyId = boost::get<CPubKey>(userId).GetKeyID();
		return true;
	} else if (userId.type() == typeid(CKeyID)) {
		keyId = boost::get<CKeyID>(userId);
		return true;
	} else if (userId.type() == typeid(CNullID)) {
		return ERRORMSG("GetKeyId input userid can't be CNullID type");
	}
	return ERRORMSG("GetKeyId input userid is unknow type");
}
bool CAccountViewCache::SetAccount(const CUserID &userId, const CAccount &account) {
	if (userId.type() == typeid(CRegID)) {
		return SetAccount(boost::get<CRegID>(userId).GetVec6(), account);
	} else if (userId.type() == typeid(CKeyID)) {
		return SetAccount(boost::get<CKeyID>(userId), account);
	} else if (userId.type() == typeid(CPubKey)) {
		return SetAccount(boost::get<CPubKey>(userId).GetKeyID(), account);
	} else if (userId.type() == typeid(CNullID)) {
		return ERRORMSG("SetAccount input userid can't be CNullID type");
	}
	return ERRORMSG("SetAccount input userid is unknow type");
}
bool CAccountViewCache::SetKeyId(const CUserID &userId, const CKeyID &keyId) {
	if (userId.type() == typeid(CRegID)) {
		return SetKeyId(boost::get<CRegID>(userId).GetVec6(), keyId);
	} else {
//		assert(0);
	}

	return false;

}
bool CAccountViewCache::EraseAccount(const CUserID &userId) {
	if (userId.type() == typeid(CKeyID)) {
		return EraseAccount(boost::get<CKeyID>(userId));
	} else if(userId.type() == typeid(CPubKey)) {
		return EraseAccount(boost::get<CPubKey>(userId).GetKeyID());
	}
	else {
		return ERRORMSG("EraseAccount account type error!");
//		assert(0);
	}
	return false;
}
bool CAccountViewCache::HaveAccount(const CUserID &userId) {
	if (userId.type() == typeid(CKeyID)) {
		return HaveAccount(boost::get<CKeyID>(userId));
	} else {
//		assert(0);
	}
	return false;
}
bool CAccountViewCache::EraseId(const CUserID &userId) {
	if (userId.type() == typeid(CRegID)) {
		return EraseKeyId(boost::get<CRegID>(userId).GetVec6());
	} else {
//		assert(0);
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
	if(userId.type() == typeid(CRegID)) {
		regId = boost::get<CRegID>(userId);
		return true;
	}
	if(tempView.GetAccount(userId,account))
	{
		regId =  account.regID;
		return !regId.IsEmpty();
	}
	return false;
}

int64_t CAccountViewCache::GetRawBalance(const CUserID& userId) const {
	CAccountViewCache tempvew(*this);
	CAccount account;
	if(tempvew.GetAccount(userId,account))
	{
		return  account.GetRawBalance();
	}
	return 0;
}

unsigned int CAccountViewCache::GetCacheSize(){
	return ::GetSerializeSize(cacheAccounts, SER_DISK, CLIENT_VERSION) + ::GetSerializeSize(cacheKeyIds, SER_DISK, CLIENT_VERSION);
}

Object CAccountViewCache::ToJosnObj() const {
	Object obj;
	Array arrayObj;
	obj.push_back(Pair("hashBlock", hashBlock.ToString()));
	arrayObj.push_back(pBase->ToJosnObj('a'));
	arrayObj.push_back(pBase->ToJosnObj('k'));
	obj.push_back(Pair("cacheView", arrayObj));
//	Array arrayObj;
//	for (auto& item : cacheAccounts) {
//		Object obj;
//		obj.push_back(Pair("keyID", item.first.ToString()));
//		obj.push_back(Pair("account", item.second.ToString()));
//		arrayObj.push_back(obj);
//	}
//	obj.push_back(Pair("cacheAccounts", arrayObj));
//
//	for (auto& item : cacheKeyIds) {
//		Object obj;
//		obj.push_back(Pair("accountID", HexStr(item.first)));
//		obj.push_back(Pair("keyID", item.second.ToString()));
//		arrayObj.push_back(obj);
//	}
//
//	obj.push_back(Pair("cacheKeyIds", arrayObj));
	return obj;
}

void CAccountViewCache::SetBaseData(CAccountView * pNewBase){
	pBase = pNewBase;
}

bool CScriptDBView::GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue) {	return false;}
bool CScriptDBView::SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue) {return false;}
bool CScriptDBView::BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas) {return false;}
bool CScriptDBView::EraseKey(const vector<unsigned char> &vKey) {return false;}
bool CScriptDBView::HaveData(const vector<unsigned char> &vKey) {return false;}
bool CScriptDBView::GetScript(const int &nIndex, vector<unsigned char> &vScriptId, vector<unsigned char> &vValue) {return false;}
bool CScriptDBView::GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vScriptId, const int &nIndex,
		vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData) {
	return false;
}
bool CScriptDBView::ReadTxIndex(const uint256 &txid, CDiskTxPos &pos){return false;}
bool CScriptDBView::WriteTxIndex(const vector<pair<uint256, CDiskTxPos> > &list, vector<CScriptDBOperLog> &vTxIndexOperDB) {return false;}
bool CScriptDBView::WriteTxOutPut(const uint256 &txid, const vector<CVmOperate> &vOutput, CScriptDBOperLog &operLog) {return false;}
bool CScriptDBView::ReadTxOutPut(const uint256 &txid, vector<CVmOperate> &vOutput) {return false;}
bool CScriptDBView::GetTxHashByAddress(const CKeyID &keyId, int nHeight, map<vector<unsigned char>, vector<unsigned char> > &vTxHash) { return false;}
bool CScriptDBView::SetTxHashByAddress(const CKeyID &keyId, int nHeight, int nIndex, const string & strTxHash, CScriptDBOperLog &operLog){return false;}

Object CScriptDBView:: ToJosnObj(string Prefix){
	Object obj;
	return obj;
}
CScriptDBViewBacked::CScriptDBViewBacked(CScriptDBView &dataBaseView) {pBase = &dataBaseView;}
bool CScriptDBViewBacked::GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue) {return pBase->GetData(vKey, vValue);}
bool CScriptDBViewBacked::SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue) {return pBase->SetData(vKey, vValue);}
bool CScriptDBViewBacked::BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas) {return pBase->BatchWrite(mapDatas);}
bool CScriptDBViewBacked::EraseKey(const vector<unsigned char> &vKey) {return pBase->EraseKey(vKey);}
bool CScriptDBViewBacked::HaveData(const vector<unsigned char> &vKey) {return pBase->HaveData(vKey);}
bool CScriptDBViewBacked::GetScript(const int &nIndex, vector<unsigned char> &vScriptId, vector<unsigned char> &vValue) {return pBase->GetScript(nIndex, vScriptId, vValue);}
bool CScriptDBViewBacked::GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vScriptId,
		const int &nIndex, vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData) {
	return pBase->GetScriptData(nCurBlockHeight, vScriptId, nIndex, vScriptKey, vScriptData);
}
bool CScriptDBViewBacked::ReadTxIndex(const uint256 &txid, CDiskTxPos &pos){return pBase->ReadTxIndex(txid, pos);}
bool CScriptDBViewBacked::WriteTxIndex(const vector<pair<uint256, CDiskTxPos> > &list, vector<CScriptDBOperLog> &vTxIndexOperDB){return pBase->WriteTxIndex(list, vTxIndexOperDB);}
bool CScriptDBViewBacked::WriteTxOutPut(const uint256 &txid, const vector<CVmOperate> &vOutput, CScriptDBOperLog &operLog) {return pBase->WriteTxOutPut(txid, vOutput, operLog);}
bool CScriptDBViewBacked::ReadTxOutPut(const uint256 &txid, vector<CVmOperate> &vOutput) {return pBase->ReadTxOutPut(txid, vOutput);}
bool CScriptDBViewBacked::GetTxHashByAddress(const CKeyID &keyId, int nHeight, map<vector<unsigned char>, vector<unsigned char> > &vTxHash) { return pBase->GetTxHashByAddress(keyId, nHeight, vTxHash);}
bool CScriptDBViewBacked::SetTxHashByAddress(const CKeyID &keyId, int nHeight, int nIndex, const string & strTxHash, CScriptDBOperLog &operLog){return pBase->SetTxHashByAddress(keyId, nHeight, nIndex, strTxHash, operLog);}


CScriptDBViewCache::CScriptDBViewCache(CScriptDBView &base, bool fDummy) : CScriptDBViewBacked(base) {
	mapDatas.clear();
}
bool CScriptDBViewCache::GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue) {
	if (mapDatas.count(vKey) > 0) {
		if (!mapDatas[vKey].empty()) {
			vValue = mapDatas[vKey];
			return true;
		} else {
			return false;
		}
	}
	if (!pBase->GetData(vKey, vValue)) {
		return false;
	}
	mapDatas[vKey] = vValue;
	return true;
}
bool CScriptDBViewCache::SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue) {
	mapDatas[vKey] = vValue;
	return true;
}
bool CScriptDBViewCache::UndoScriptData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue) {
	vector<unsigned char> vPrefix(vKey.begin(), vKey.begin() + 4);
	vector<unsigned char> vScriptDataPrefix = { 'd', 'a', 't', 'a' };
	if (vPrefix == vScriptDataPrefix) {
		assert(vKey.size() > 10);
		if (vKey.size() < 10) {
			return ERRORMSG("UndoScriptData(): vKey=%s error!\n", HexStr(vKey));
		}
		vector<unsigned char> vScriptCountKey = { 's', 'd', 'n', 'u', 'm' };
		vector<unsigned char> vScriptId(vKey.begin() + 4, vKey.begin() + 10);
		vector<unsigned char> vOldValue;
		if (mapDatas.count(vKey)) {
			vOldValue = mapDatas[vKey];
		} else {
			GetData(vKey, vOldValue);
		}
		vScriptCountKey.insert(vScriptCountKey.end(), vScriptId.begin(), vScriptId.end());
		CDataStream ds(SER_DISK, CLIENT_VERSION);

		int nCount(0);
		if (vValue.empty()) {   //key����Ӧ��ֵ�ɷǿ�����Ϊ�գ�������1
			if (!vOldValue.empty()) {
				if (!GetScriptDataCount(vScriptId, nCount))
					return false;
				--nCount;
				if (!SetScriptDataCount(vScriptId, nCount))
					return false;
			}
		} else {    //key����Ӧ��ֵ�ɿ�����Ϊ�ǿգ�������1
			if (vOldValue.empty()) {
				GetScriptDataCount(vScriptId, nCount);
				++nCount;
				if (!SetScriptDataCount(vScriptId, nCount))
					return false;
			}
		}
	}
	mapDatas[vKey] = vValue;
	return true;
}
bool CScriptDBViewCache::BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapData) {
	for (auto &items : mapData) {
		mapDatas[items.first] = items.second;
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
	if (0 == nIndex) {
		vector<unsigned char> scriptKey = {'d','e','f'};
		vector<unsigned char> vDataKey;
		vector<unsigned char> vDataValue;
		vDataKey.clear();
		vDataValue.clear();
		for (auto &item : mapDatas) {   //���������������ݣ��ҳ��Ϸ�����С��keyֵ
			vector<unsigned char> vTemp(item.first.begin(), item.first.begin() + 3);
			if (scriptKey == vTemp) {
				if (item.second.empty()) {
					continue;
				}
				vDataKey = item.first;
				vDataValue = item.second;
				break;
			}
		}
		if (!pBase->GetScript(nIndex, vScriptId, vValue)) { //�ϼ�û�л�ȡ����������keyֵ
			if (vDataKey.empty())
				return false;
			else {//���ر�������Ĳ�ѯ���
				vScriptId.clear();
				vValue.clear();
				vScriptId.assign(vDataKey.begin()+3, vDataKey.end());
				vValue = vDataValue;
				return true;
			}
		} else { //�ϼ���ȡ������������keyֵ
			if (vDataKey.empty()) {  //������û�з���������key��ֱ�ӷ����ϼ��Ĳ�ѯ���
				return true;
			}
			vector<unsigned char> dataKeyTemp = {'d', 'e', 'f'};
			dataKeyTemp.insert(dataKeyTemp.end(), vScriptId.begin(), vScriptId.end()); //�ϼ��õ���keyֵ
			if (dataKeyTemp < vDataKey) {  //���ϼ���ѯ��keyС�ڱ��������key,�Ҵ�key�ڻ�����û�У���ֱ�ӷ������ݿ��в�ѯ�Ľ��
				if (mapDatas.count(dataKeyTemp) == 0)
					return true;
				else {
					mapDatas[dataKeyTemp].clear();  //�ڻ�����dataKeyTemp�Ѿ���ɾ�����ˣ����½���key��Ӧ��value���
					return GetScript(nIndex, vScriptId, vValue); //���´����ݿ��л�ȡ��һ������
				}
			} else {  //���ϼ���ѯ��key���ڵ��ڱ��������key,���ر���������
				vScriptId.clear();
				vValue.clear();
				vScriptId.assign(vDataKey.begin()+3, vDataKey.end());
				vValue = vDataValue;
				return true;
			}
		}
	} else if (1 == nIndex) {
		vector<unsigned char> vKey = { 'd','e','f' };
		vKey.insert(vKey.end(), vScriptId.begin(), vScriptId.end());
		vector<unsigned char> vPreKey(vKey);
		map<vector<unsigned char>, vector<unsigned char> >::iterator iterFindKey = mapDatas.upper_bound(vPreKey);
		vector<unsigned char> vDataKey;
		vector<unsigned char> vDataValue;
 		vDataKey.clear();
 		vDataValue.clear();
		vector<unsigned char> vKeyTemp={'d','e','f'};
		while (iterFindKey != mapDatas.end()) {
			vector<unsigned char> vTemp(iterFindKey->first.begin(), iterFindKey->first.begin() + 3);
			if (vKeyTemp == vTemp) {
				if (iterFindKey->second.empty()){
					++iterFindKey;
					continue;
				}
				else {
					vDataKey = iterFindKey->first;
					vDataValue = iterFindKey->second;
					break;
				}
			}else {
				++iterFindKey;
			}
		}
		if (!pBase->GetScript(nIndex, vScriptId, vValue)) { //��BASE��ȡָ����ֵ֮�����һ��ֵ
			if (vDataKey.empty())
				return false;
			else {
				vScriptId.clear();
				vValue.clear();
				vScriptId.assign(vDataKey.begin()+3, vDataKey.end());
				vValue = vDataValue;
				return true;
			}
		} else {
			if (vDataKey.empty())    //������û�з���������key��ֱ�ӷ����ϼ��Ĳ�ѯ���
				return true;
			vector<unsigned char> dataKeyTemp = {'d', 'e', 'f'};
			dataKeyTemp.insert(dataKeyTemp.end(), vScriptId.begin(), vScriptId.end()); //�ϼ��õ���keyֵ
			if (dataKeyTemp < vDataKey) {
				if (mapDatas.count(dataKeyTemp) == 0)
						return true;
				else {
					mapDatas[dataKeyTemp].clear();  //�ڻ�����dataKeyTemp�Ѿ���ɾ�����ˣ����½���key��Ӧ��value���
					return GetScript(nIndex, vScriptId, vValue); //���´����ݿ��л�ȡ��һ������
				}
			} else { //���ϼ���ѯ��key���ڵ��ڱ��������key,���ر���������
				vScriptId.clear();
				vValue.clear();
				vScriptId.assign(vDataKey.begin()+3, vDataKey.end());
				vValue = vDataValue;
				return true;
			}
		}

	} else {
//		assert(0);
	}
	return true;
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
	return ok;
}
unsigned int CScriptDBViewCache::GetCacheSize() {
	return ::GetSerializeSize(mapDatas, SER_DISK, CLIENT_VERSION);
}

bool CScriptDBViewCache::WriteTxOutPut(const uint256 &txid, const vector<CVmOperate> &vOutput, CScriptDBOperLog &operLog) {
	vector<unsigned char> vKey = {'o','u','t','p','u','t'};
	CDataStream ds1(SER_DISK, CLIENT_VERSION);
	ds1 << txid;
	vKey.insert(vKey.end(), ds1.begin(), ds1.end());

	vector<unsigned char> vValue;
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << vOutput;
	vValue.assign(ds.begin(), ds.end());

	vector<unsigned char> oldValue;
	oldValue.clear();
	GetData(vKey, oldValue);
	operLog = CScriptDBOperLog(vKey, oldValue);
	return SetData(vKey, vValue);
}

bool CScriptDBViewCache::SetTxHashByAddress(const CKeyID &keyId, int nHeight, int nIndex, const string & strTxHash, CScriptDBOperLog &operLog)
{
	vector<unsigned char> vKey = {'A','D','D','R'};
	vector<unsigned char> oldValue;
	oldValue.clear();
	GetData(vKey, oldValue);
	operLog = CScriptDBOperLog(vKey, oldValue);

	CDataStream ds1(SER_DISK, CLIENT_VERSION);
	ds1 << keyId;
	ds1 << nHeight;
	ds1 << nIndex;
	vKey.insert(vKey.end(), ds1.begin(), ds1.end());
	vector<unsigned char> vValue(strTxHash.begin(), strTxHash.end());
	return SetData(vKey, vValue);
}

bool CScriptDBViewCache::GetTxHashByAddress(const CKeyID &keyId, int nHeight, map<vector<unsigned char>, vector<unsigned char> > &vTxHash)
{

	pBase->GetTxHashByAddress(keyId, nHeight, vTxHash);

	vector<unsigned char> vPreKey = {'A','D','D','R'};
	CDataStream ds1(SER_DISK, CLIENT_VERSION);
	ds1 << keyId;
	ds1 << nHeight;
	vPreKey.insert(vPreKey.end(), ds1.begin(), ds1.end());

	map<vector<unsigned char>, vector<unsigned char> >::iterator iterFindKey = mapDatas.upper_bound(vPreKey);
	while (iterFindKey != mapDatas.end()) {
		if (0 == memcmp((char *)&iterFindKey->first[0], (char *)&vPreKey[0], 24)) {
			if(iterFindKey->second.empty())
				vTxHash.erase(iterFindKey->first);
			else {
				vTxHash.insert(make_pair(iterFindKey->first, iterFindKey->second));
			}
			++iterFindKey;
		}
		else {
			break;
		}
	}
	return true;
}


bool CScriptDBViewCache::ReadTxOutPut(const uint256 &txid, vector<CVmOperate> &vOutput) {
	vector<unsigned char> vKey = {'o','u','t','p','u','t'};
	CDataStream ds1(SER_DISK, CLIENT_VERSION);
	ds1 << txid;
	vKey.insert(vKey.end(), ds1.begin(), ds1.end());
	vector<unsigned char> vValue;
	if(!GetData(vKey, vValue))
		return false;
	CDataStream ds(vValue, SER_DISK, CLIENT_VERSION);
	ds >> vOutput;
	return true;
}

bool CScriptDBViewCache::ReadTxIndex(const uint256 &txid, CDiskTxPos &pos){
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << txid;
	vector<unsigned char> vTxHash = {'T'};
	vTxHash.insert(vTxHash.end(), ds.begin(), ds.end());
	vector<unsigned char> vTxPos;

	if(mapDatas.count(vTxHash))  {
		if(mapDatas[vTxHash].empty()) {
			return false;
		}
		vTxPos = mapDatas[vTxHash];
		CDataStream dsPos(vTxPos, SER_DISK, CLIENT_VERSION);
		dsPos >> pos;
	}else {
		if(!GetData(vTxHash, vTxPos))
			return false;
		CDataStream dsPos(vTxPos, SER_DISK, CLIENT_VERSION);
		dsPos >> pos;
	}
	return true;
}
bool CScriptDBViewCache::WriteTxIndex(const vector<pair<uint256, CDiskTxPos> > &list, vector<CScriptDBOperLog> &vTxIndexOperDB){
	for (vector<pair<uint256, CDiskTxPos> >::const_iterator it = list.begin(); it != list.end(); it++) {
		LogPrint("txindex", "txhash:%s dispos: nFile=%d, nPos=%d nTxOffset=%d\n", it->first.GetHex(), it->second.nFile, it->second.nPos, it->second.nTxOffset);
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		ds << it->first;
		vector<unsigned char> vTxHash = { 'T' };
		vTxHash.insert(vTxHash.end(), ds.begin(), ds.end());
		vector<unsigned char> vTxPos;
		CDataStream dsPos(SER_DISK, CLIENT_VERSION);
		dsPos << it->second;
		vTxPos.insert(vTxPos.end(), dsPos.begin(), dsPos.end());
		CScriptDBOperLog txIndexOper;
		txIndexOper.vKey = vTxHash;
		GetData(vTxHash, txIndexOper.vValue);
		vTxIndexOperDB.push_back(txIndexOper);
		if(!SetData(vTxHash, vTxPos))
			return false;
	}
	return true;
}

bool CScriptDBViewCache::GetScript(const vector<unsigned char> &vScriptId, vector<unsigned char> &vValue) {
	vector<unsigned char> scriptKey = { 'd', 'e', 'f' };

	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());
	return GetData(scriptKey, vValue);
}
bool CScriptDBViewCache::GetScript(const CRegID &scriptId, vector<unsigned char> &vValue) {
	return GetScript(scriptId.GetVec6(), vValue);
}

bool CScriptDBViewCache::GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vScriptId,
		const vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData) {
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
	vScriptData = vValue;
//	CDataStream ds(vValue, SER_DISK, CLIENT_VERSION);
//	ds >> vScriptData;
	return true;
}
bool CScriptDBViewCache::GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vScriptId,
		const int &nIndex, vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData) {
	if(0 == nIndex) {
		vector<unsigned char> vKey = { 'd', 'a', 't', 'a' };
		vKey.insert(vKey.end(), vScriptId.begin(), vScriptId.end());
		vKey.push_back('_');
		vector<unsigned char> vDataKey;
		vector<unsigned char> vDataValue;
		vDataKey.clear();
		vDataValue.clear();
		map<vector<unsigned char>, vector<unsigned char> >::iterator iterFindKey = mapDatas.upper_bound(vKey);
		while (iterFindKey != mapDatas.end()) {
			vector<unsigned char> vKeyTemp(vKey.begin(), vKey.begin() + vScriptId.size() + 5);
			vector<unsigned char> vTemp(iterFindKey->first.begin(), iterFindKey->first.begin() + vScriptId.size() + 5);
			if (vKeyTemp == vTemp) {
				if (iterFindKey->second.empty()) {
					++iterFindKey;
					continue;
				} else {
					vDataKey = iterFindKey->first;
					vDataValue = iterFindKey->second;
					break;
				}
			} else {
				break;
			}
		}
		bool bUpLevelRet(false);
		int nIndexTemp = nIndex;
		while((bUpLevelRet = pBase->GetScriptData(nCurBlockHeight, vScriptId, nIndexTemp, vScriptKey, vScriptData))) {
//			LogPrint("INFO", "nCurBlockHeight:%d this addr:0x%x, nIndex:%d, count:%lld\n ScriptKey:%s\n nHeight:%d\n ScriptData:%s\n vDataKey:%s\n vDataValue:%s\n",
//					nCurBlockHeight, this, nIndexTemp, ++llCount, HexStr(vScriptKey), nHeight, HexStr(vScriptData), HexStr(vDataKey), HexStr(vDataValue));
			nIndexTemp = 1;
			vector<unsigned char> dataKeyTemp(vKey.begin(), vKey.end());
			dataKeyTemp.insert(dataKeyTemp.end(), vScriptKey.begin(), vScriptKey.end());
//			LogPrint("INFO", "dataKeyTemp:%s\n vDataKey:%s\n", HexStr(dataKeyTemp), HexStr(vDataKey));
//			if(mapDatas.count(dataKeyTemp) > 0) {//������������ϼ���ѯ�����key
//				if(dataKeyTemp != vDataKey) {  //�������ϼ�����key��ͬ��˵���ϼ���ȡ�������ڱ����ѱ�ɾ��
//					continue;
//				} else {
//					if(vDataValue.empty()) { //�������ϼ�����key��ͬ,�ұ��������Ѿ�ɾ�������´��ϼ���ȡ��һ������
//						LogPrint("INFO", "dataKeyTemp equal vDataKey and vDataValue empty redo getscriptdata()\n");
//						continue;
//					}
//					vScriptKey.clear();
//					vScriptData.clear();
//					vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
//					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//					ds >> nHeight;
//					ds >> vScriptData;
//					return true;
//				}
//			} else {//�������治�����ϼ���ѯ���key
//				if (dataKeyTemp < vDataKey) { //�ϼ���ȡkeyֵС
//					return true;      //�����ϼ��Ĳ�ѯ���
//				} else {              //������ѯ���KeyֵС�����ر�����ѯ���
//					vScriptKey.clear();
//					vScriptData.clear();
//					vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
//					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//					ds >> nHeight;
//					ds >> vScriptData;
//					return true;
//				}
//			}
			if (vDataKey.empty()) {   //������û�з���������key��ֱ�ӷ����ϼ��Ĳ�ѯ���
				if(mapDatas.count(dataKeyTemp) <= 0) {
//					CDataStream ds(vScriptData, SER_DISK, CLIENT_VERSION);
//					ds >> vScriptData;
					return true;
				}
				else {
//					LogPrint("INFO", "local level contains dataKeyTemp,but the value is empty,need redo getscriptdata()\n");
					continue;			 //���´����ݿ��л�ȡ��һ������
				}
			}
			else {
				if (dataKeyTemp < vDataKey) {
					if(mapDatas.count(dataKeyTemp) <= 0) {
						return true;
					}
					else {
//						LogPrint("INFO", "dataKeyTemp less than vDataKey and vDataValue empty redo getscriptdata()\n");
						continue;			 //���´����ݿ��л�ȡ��һ������
					}
				} else {
					if(vDataValue.empty()) { //�������ϼ�����key��ͬ,�ұ��������Ѿ�ɾ�������´��ϼ���ȡ��һ������
//						LogPrint("INFO", "dataKeyTemp equal vDataKey and vDataValue empty redo getscriptdata()\n");
						continue;
					}
					vScriptKey.clear();
					vScriptData.clear();
					vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
					vScriptData = vDataValue;
//					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//					ds >> vScriptData;
					return true;
				}
			}
		}
		if(!bUpLevelRet) {
			if (vDataKey.empty())
				return false;
			else {
				vScriptKey.clear();
				vScriptData.clear();
				vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
				if (vDataValue.empty()) {
					return false;
				}
				vScriptData = vDataValue;
//				CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//				ds >> vScriptData;
				return true;
			}
		}
	}
	else if (1 == nIndex) {
		vector<unsigned char> vKey = { 'd', 'a', 't', 'a' };
		vKey.insert(vKey.end(), vScriptId.begin(), vScriptId.end());
		vKey.push_back('_');
		vector<unsigned char> vPreKey(vKey);
		vPreKey.insert(vPreKey.end(), vScriptKey.begin(), vScriptKey.end());
		map<vector<unsigned char>, vector<unsigned char> >::iterator iterFindKey = mapDatas.upper_bound(vPreKey);
		vector<unsigned char> vDataKey;
		vector<unsigned char> vDataValue;
		vDataValue.clear();
		vDataKey.clear();
		while (iterFindKey != mapDatas.end()) {
			vector<unsigned char> vKeyTemp(vKey.begin(), vKey.begin() + vScriptId.size() + 5);
			vector<unsigned char> vTemp(iterFindKey->first.begin(), iterFindKey->first.begin() + vScriptId.size() + 5);
			if (vKeyTemp == vTemp) {
				if (iterFindKey->second.empty()) {
					++iterFindKey;
					continue;
				} else {
					vDataKey = iterFindKey->first;
					vDataValue = iterFindKey->second;
					break;
				}
			} else {
				break;
				//++iterFindKey;
			}
		}
		bool bUpLevelRet(false);
		while((bUpLevelRet=pBase->GetScriptData(nCurBlockHeight, vScriptId, nIndex, vScriptKey, vScriptData))) {
//			LogPrint("INFO", "nCurBlockHeight:%d this addr:0x%x, nIndex:%d, count:%lld\n ScriptKey:%s\n nHeight:%d\n ScriptData:%s\n vDataKey:%s\n vDataValue:%s\n",
//					nCurBlockHeight, this, nIndex, ++llCount, HexStr(vScriptKey), nHeight, HexStr(vScriptData), HexStr(vDataKey), HexStr(vDataValue));
			vector<unsigned char> dataKeyTemp(vKey.begin(), vKey.end());
			dataKeyTemp.insert(dataKeyTemp.end(), vScriptKey.begin(), vScriptKey.end());
//			LogPrint("INFO", "dataKeyTemp:%s\n vDataKey:%s\n", HexStr(dataKeyTemp), HexStr(vDataKey));
//			if(mapDatas.count(dataKeyTemp) > 0) {//������������ϼ���ѯ�����key
//				if(dataKeyTemp != vDataKey) {  //�������ϼ�����key��ͬ��˵���ϼ���ȡ�������ڱ����ѱ�ɾ��
//					continue;
//				} else {
//					if(vDataValue.empty()) { //�������ϼ�����key��ͬ,�ұ��������Ѿ�ɾ�������´��ϼ���ȡ��һ������
//						LogPrint("INFO", "dataKeyTemp equal vDataKey and vDataValue empty redo getscriptdata()\n");
//						continue;
//					}
//					vScriptKey.clear();
//					vScriptData.clear();
//					vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
//					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//					ds >> nHeight;
//					ds >> vScriptData;
//					return true;
//				}
//			} else {//�������治�����ϼ���ѯ���key
//				if (dataKeyTemp < vDataKey) { //�ϼ���ȡkeyֵС
//					return true;      //�����ϼ��Ĳ�ѯ���
//				} else {              //������ѯ���KeyֵС�����ر�����ѯ���
//					vScriptKey.clear();
//					vScriptData.clear();
//					vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
//					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//					ds >> nHeight;
//					ds >> vScriptData;
//					return true;
//				}
//			}
			if (vDataKey.empty()) {   //������û�з���������key��ֱ�ӷ����ϼ��Ĳ�ѯ���
				if(mapDatas.count(dataKeyTemp) <= 0) {
//					CDataStream ds(vScriptData, SER_DISK, CLIENT_VERSION);
//					ds >> vScriptData;
					return true;
				}
				else {
//					LogPrint("INFO", "local level contains dataKeyTemp,but the value is empty,need redo getscriptdata()\n");
					continue;			 //���´����ݿ��л�ȡ��һ������
				}
			}
			else {
				if (dataKeyTemp < vDataKey) {
					if(mapDatas.count(dataKeyTemp) == 0)
						return true;
					else {
//						LogPrint("INFO", "dataKeyTemp less than vDataKey and vDataValue empty redo getscriptdata()\n");
						continue;			//�ڻ�����dataKeyTemp�Ѿ���ɾ�����ˣ����´����ݿ��л�ȡ��һ������
					}
				} else {
					if(vDataValue.empty()) { //�������ϼ�����key��ͬ,�ұ��������Ѿ�ɾ�������´��ϼ���ȡ��һ������
//						LogPrint("INFO", "dataKeyTemp equal vDataKey and vDataValue empty redo getscriptdata()\n");
						continue;
					}
					vScriptKey.clear();
					vScriptData.clear();
					vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
					vScriptData = vDataValue;
//					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//					ds >> vScriptData;
					return true;
				}
			}
		}
		if(!bUpLevelRet) {
			if (vDataKey.empty())
				return false;
			else {
				vScriptKey.clear();
				vScriptData.clear();
				vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
				if (vDataValue.empty()) {
					return false;
				}
				vScriptData = vDataValue;
//				CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//				ds >> vScriptData;
				return true;
			}
		}
	}
	else {
//		assert(0);
		return ERRORMSG("GetScriptData error");
	}
//	vector<unsigned char> vKey = { 'd', 'a', 't', 'a' };
//	vKey.insert(vKey.end(), vScriptId.begin(), vScriptId.end());
//	vKey.push_back('_');
//	vector<unsigned char> vDataKey;
//	vector<unsigned char> vDataValue;
//	if(0 == nIndex) {
//		vDataKey.clear();
//		vDataValue.clear();
//		for (auto &item : mapDatas) {   //���������������ݣ��ҳ��Ϸ�����С��keyֵ
//			vector<unsigned char> vTemp(item.first.begin(),item.first.begin()+vScriptId.size()+5);
//			if(vKey == vTemp) {
//				if(item.second.empty()) {
//					continue;
//				}
//				vDataKey = item.first;
//				vDataValue = item.second;
//				CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//				ds >> nHeight;
//				if(nHeight <= nCurBlockHeight) { //���ҵ���key��Ӧ�����ݱ���ʱ���Ѿ���ʱ������Ҫɾ�����������������һ������������key
//					CScriptDBOperLog operLog(vDataKey, vDataValue);
//					vDataKey.clear();
//					vDataValue.clear();
//					setOperLog.insert(operLog);
//					continue;
//				}
//				break;
//			}
//		}
//	}
//	else if (1 == nIndex) {
//		vector<unsigned char> vPreKey(vKey);
//		vPreKey.insert(vPreKey.end(), vScriptKey.begin(), vScriptKey.end());
//		map<vector<unsigned char>, vector<unsigned char> >::iterator iterFindKey = mapDatas.upper_bound(vPreKey);
//		vDataValue.clear();
//		vDataKey.clear();
//		while (iterFindKey != mapDatas.end()) {
//			vector<unsigned char> vKeyTemp(vKey.begin(), vKey.begin() + vScriptId.size() + 5);
//			vector<unsigned char> vTemp(iterFindKey->first.begin(), iterFindKey->first.begin() + vScriptId.size() + 5);
//			if (vKeyTemp == vTemp) {
//				if (iterFindKey->second.empty()) {
//					++iterFindKey;
//					continue;
//				} else {
//					vDataKey = iterFindKey->first;
//					vDataValue = iterFindKey->second;
//					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//					ds >> nHeight;
//					if (nHeight <= nCurBlockHeight) { //���ҵ���key��Ӧ�����ݱ���ʱ���Ѿ���ʱ������Ҫɾ�����������������һ������������key
//						CScriptDBOperLog operLog(vDataKey, iterFindKey->second);
//						vDataKey.clear();
//						vDataValue.clear();
//						setOperLog.insert(operLog);
//						++iterFindKey;
//						continue;
//					}
//					break;
//				}
//			} else {
//				++iterFindKey;
//			}
//		}
//	}
//	else {
//		assert(0);
//	}
//	bool bUpLevelRet(false);
//	unsigned long llCount(0);
//	int nIndexTemp = nIndex;
//	while((bUpLevelRet = pBase->GetScriptData(nCurBlockHeight, vScriptId, nIndexTemp, vScriptKey, vScriptData, nHeight, setOperLog))) {
//		LogPrint("INFO", "nCurBlockHeight:%d this addr:%x, nIndex:%d, count:%lld\n ScriptKey:%s\n nHeight:%d\n ScriptData:%s\n vDataKey:%s\n vDataValue:%s\n",
//				nCurBlockHeight, this, nIndexTemp, ++llCount, HexStr(vScriptKey), nHeight, HexStr(vScriptData), HexStr(vDataKey), HexStr(vDataValue));
//		nIndexTemp = 1;
//		for(auto &itemKey : mapDatas) {
//			vector<unsigned char> vKeyTemp(itemKey.first.begin(), itemKey.first.begin() + vKey.size());
//			if(vKeyTemp == vKey) {
//				LogPrint("INFO", "vKey:%s\n vValue:%s\n", HexStr(itemKey.first), HexStr(itemKey.second));
//			}
//		}
//		set<CScriptDBOperLog>::iterator iterOperLog = setOperLog.begin();
//		for (; iterOperLog != setOperLog.end();) { //��ֹ����û��flush cache�������ݿ��г�ʱ�Ľű��������cache�ж��ɾ��������ɾ��ʧ��
//			if (mapDatas.count(iterOperLog->vKey) > 0 && mapDatas[iterOperLog->vKey].empty()) {
//				LogPrint("INFO", "DeleteData key:%s\n", HexStr(iterOperLog->vKey));
//				setOperLog.erase(iterOperLog++);
//			}else {
//				++iterOperLog;
//			}
//		}
//		vector<unsigned char> dataKeyTemp(vKey.begin(), vKey.end());
//		dataKeyTemp.insert(dataKeyTemp.end(), vScriptKey.begin(), vScriptKey.end());
//		LogPrint("INFO", "dataKeyTemp:%s\n vDataKey:%s\n", HexStr(dataKeyTemp), HexStr(vDataKey));
//		if(mapDatas.count(dataKeyTemp) > 0) {//������������ϼ���ѯ�����key
//			if(dataKeyTemp != vDataKey) {  //�������ϼ�����key��ͬ��˵���ϼ���ȡ�������ڱ����ѱ�ɾ��
//				LogPrint("INFO", "dataKeyTemp equal vDataKey and vDataValue empty redo getscriptdata()\n");
//				continue;
//			} else {
//				if(vDataValue.empty()) { //�������ϼ�����key��ͬ,�ұ��������Ѿ�ɾ�������´��ϼ���ȡ��һ������
//					LogPrint("INFO", "dataKeyTemp equal vDataKey and vDataValue empty redo getscriptdata()\n");
//					continue;
//				}
//				vScriptKey.clear();
//				vScriptData.clear();
//				vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
//				CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//				ds >> nHeight;
//				ds >> vScriptData;
//				return true;
//			}
//		} else {//�������治�����ϼ���ѯ���key
//			if (dataKeyTemp < vDataKey) { //�ϼ���ȡkeyֵС
//				return true;      //�����ϼ��Ĳ�ѯ���
//			} else {              //������ѯ���KeyֵС�����ر�����ѯ���
//				vScriptKey.clear();
//				vScriptData.clear();
//				vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
//				CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//				ds >> nHeight;
//				ds >> vScriptData;
//				return true;
//			}
//		}
//		if(!bUpLevelRet) {
//			set<CScriptDBOperLog>::iterator iterOperLog = setOperLog.begin();
//			for (; iterOperLog != setOperLog.end();) { //��ֹ����û��flush cache�������ݿ��г�ʱ�Ľű��������cache�ж��ɾ��������ɾ��ʧ��
//				if (mapDatas.count(iterOperLog->vKey) > 0 && mapDatas[iterOperLog->vKey].empty()) {
//					LogPrint("INFO", "DeleteData key:%s\n", HexStr(iterOperLog->vKey));
//					setOperLog.erase(iterOperLog++);
//				}else{
//					++iterOperLog;
//				}
//			}
//			if (vDataKey.empty())
//				return false;
//			else {
//				vScriptKey.clear();
//				vScriptData.clear();
//				vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
//				if (vDataValue.empty()) {
//					return false;
//				}
//				CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//				ds >> nHeight;
//				ds >> vScriptData;
//				return true;
//			}
//		}
//	}
	return true;
}
bool CScriptDBViewCache::SetScriptData(const vector<unsigned char> &vScriptId, const vector<unsigned char> &vScriptKey,
		const vector<unsigned char> &vScriptData, CScriptDBOperLog &operLog) {
	vector<unsigned char> vKey = { 'd', 'a', 't', 'a' };
	vKey.insert(vKey.end(), vScriptId.begin(), vScriptId.end());
	vKey.push_back('_');
	vKey.insert(vKey.end(), vScriptKey.begin(), vScriptKey.end());
  //  LogPrint("vm","add data:%s",HexStr(vScriptKey).c_str());
//	CDataStream ds(SER_DISK, CLIENT_VERSION);
//	ds << vScriptData;
	vector<unsigned char> vValue(vScriptData.begin(), vScriptData.end());
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
	vector<unsigned char> vValue;
	vValue.clear();
	if(nCount > 0) {
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		ds << nCount;
		vValue.insert(vValue.end(), ds.begin(), ds.end());
	}
	else if (nCount < 0)
	{
//		assert(0);
		return false;
	}
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
		if (!SetScriptCount(--nCount))
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
	vector<unsigned char> vValue;
	vValue.clear();
	if(nCount > 0) {
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		ds << nCount;
		vValue.insert(vValue.end(), ds.begin(), ds.end());
	}
	else if (nCount < 0)
	{
//		assert(0);
		return false;
	}
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
		if(!GetScriptDataCount(vScriptId, nCount)) {
			return false;
		}
		if (!SetScriptDataCount(vScriptId, --nCount)) {
			return false;
		}

	}

	vector<unsigned char> vValue;
	if(!GetData(scriptKey, vValue)) {
		return false;
	}

	operLog = CScriptDBOperLog(scriptKey, vValue);
	if(!EraseKey(scriptKey))
		return false;


	return true;
}

bool CScriptDBViewCache::EraseScriptData(const vector<unsigned char> &vKey) {
	if(vKey.size() < 12) {
		return ERRORMSG("EraseScriptData delete script data key value error!");
//		assert(0);
	}
	vector<unsigned char> vScriptId(vKey.begin()+4, vKey.begin()+10);
	vector<unsigned char> vScriptKey(vKey.begin()+11, vKey.end());
	CScriptDBOperLog operLog;
	return EraseScriptData(vScriptId, vScriptKey, operLog);
}

bool CScriptDBViewCache::HaveScriptData(const vector<unsigned char> &vScriptId, const vector<unsigned char > &vScriptKey) {
	vector<unsigned char> scriptKey = { 'd', 'a', 't', 'a'};
	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());
	scriptKey.push_back('_');
	scriptKey.insert(scriptKey.end(), vScriptKey.begin(), vScriptKey.end());
	return HaveData(scriptKey);
}


bool CScriptDBViewCache::GetScript(const int nIndex, CRegID &scriptId, vector<unsigned char> &vValue) {
	vector<unsigned char> tem;
	if (nIndex != 0) {
		tem = scriptId.GetVec6();
	}
	if (GetScript(nIndex, tem, vValue)) {
		scriptId.SetRegID(tem);
		return true;
	}

	return false;
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
	return  GetScriptDataCount(scriptId.GetVec6(), nCount);
}
bool CScriptDBViewCache::EraseScriptData(const CRegID &scriptId, const vector<unsigned char> &vScriptKey, CScriptDBOperLog &operLog) {
	return EraseScriptData(scriptId.GetVec6(), vScriptKey, operLog);
}
bool CScriptDBViewCache::HaveScriptData(const CRegID &scriptId, const vector<unsigned char > &vScriptKey) {
	return HaveScriptData(scriptId.GetVec6(), vScriptKey);
}
bool CScriptDBViewCache::GetScriptData(const int nCurBlockHeight, const CRegID &scriptId, const vector<unsigned char> &vScriptKey,
			vector<unsigned char> &vScriptData) {
	return GetScriptData(nCurBlockHeight, scriptId.GetVec6(), vScriptKey, vScriptData);
}
bool CScriptDBViewCache::GetScriptData(const int nCurBlockHeight, const CRegID &scriptId, const int &nIndex, vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData)
{
	return GetScriptData(nCurBlockHeight, scriptId.GetVec6(), nIndex, vScriptKey, vScriptData);
}
bool CScriptDBViewCache::SetScriptData(const CRegID &scriptId, const vector<unsigned char> &vScriptKey,
			const vector<unsigned char> &vScriptData, CScriptDBOperLog &operLog) {
	return 	SetScriptData(scriptId.GetVec6(), vScriptKey, vScriptData, operLog);
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
bool CScriptDBViewCache::EraseTxRelAccout(const uint256 &txHash) {
	vector<unsigned char> vKey = {'t','x'};
	vector<unsigned char> vValue;
	vValue.clear();
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << txHash;
	vKey.insert(vKey.end(), ds.begin(), ds.end());
	SetData(vKey, vValue);
	return true;
}
Object CScriptDBViewCache::ToJosnObj() const {
	Object obj;
	Array arrayObj;
//	for (auto& item : mapDatas) {
//		Object obj;
//		obj.push_back(Pair("key", HexStr(item.first)));
//		obj.push_back(Pair("value", HexStr(item.second)));
//		arrayObj.push_back(obj);
//	}
//	obj.push_back(Pair("mapDatas", arrayObj));
	arrayObj.push_back(pBase->ToJosnObj("def"));
	arrayObj.push_back(pBase->ToJosnObj("data"));
	arrayObj.push_back(pBase->ToJosnObj("author"));
	obj.push_back(Pair("mapDatas", arrayObj));
	return obj;
}
void CScriptDBViewCache::SetBaseData(CScriptDBView * pNewBase){
	pBase = pNewBase;
}
string CScriptDBViewCache::ToString(){
	string str("");
	vector<unsigned char> vPrefix = {'d', 'a', 't', 'a'};
	for(auto & item : mapDatas) {
		vector<unsigned char> vTemp(item.first.begin(), item.first.begin()+4);
		if(vTemp ==  vPrefix){
			str = strprintf("vKey=%s\n vData=%s\n", HexStr(item.first), HexStr(item.second));
		}
	}
	return str;
}


uint256 CTransactionDBView::IsContainTx(const uint256 & txHash) { return std::move(uint256()); }
bool CTransactionDBView::IsContainBlock(const CBlock &block) { return false; }
bool CTransactionDBView::AddBlockToCache(const CBlock &block) { return false; }
bool CTransactionDBView::DeleteBlockFromCache(const CBlock &block) { return false; }
bool CTransactionDBView::LoadTransaction(map<uint256, vector<uint256> > &mapTxHashByBlockHash) { return false; }
bool CTransactionDBView::BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHash) { return false; }



CTransactionDBViewBacked::CTransactionDBViewBacked(CTransactionDBView &transactionView) {
	pBase = &transactionView;
}
uint256 CTransactionDBViewBacked::IsContainTx(const uint256 & txHash) {
	return std::move(pBase->IsContainTx(txHash));
}
bool CTransactionDBViewBacked::IsContainBlock(const CBlock &block) {
	return pBase->IsContainBlock(block);
}

bool CTransactionDBViewBacked::AddBlockToCache(const CBlock &block) {
	return pBase->AddBlockToCache(block);
}
bool CTransactionDBViewBacked::DeleteBlockFromCache(const CBlock &block) {
	return pBase->DeleteBlockFromCache(block);
}
bool CTransactionDBViewBacked::LoadTransaction(map<uint256, vector<uint256> > &mapTxHashByBlockHash) {
	return pBase->LoadTransaction(mapTxHashByBlockHash);
}
bool CTransactionDBViewBacked::BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHashIn) {
	return pBase->BatchWrite(mapTxHashByBlockHashIn);
}

CTransactionDBCache::CTransactionDBCache(CTransactionDBView &txCacheDB, bool fDummy) : CTransactionDBViewBacked(txCacheDB){

}
bool CTransactionDBCache::IsContainBlock(const CBlock &block) {
	//(mapTxHashByBlockHash.count(block.GetHash()) > 0 && mapTxHashByBlockHash[block.GetHash()].size() > 0)
	return (IsInMap(mapTxHashByBlockHash,block.GetHash())
			|| pBase->IsContainBlock(block));
}
bool CTransactionDBCache::AddBlockToCache(const CBlock &block) {
	vector<uint256> vTxHash;
	vTxHash.clear();
	for (auto &ptx : block.vptx) {
		vTxHash.push_back(ptx->GetHash());
	}
	mapTxHashByBlockHash[block.GetHash()] = vTxHash;
//	LogPrint("txcache", "CTransactionDBCache:AddBlockToCache() the block height=%d hash=%s is in TxCache\n", block.nHeight, block.GetHash().GetHex());
//	LogPrint("txcache", "mapTxHashByBlockHash size:%d\n", mapTxHashByBlockHash.size());
//	map<int, uint256> mapTxCacheBlockHash;
//	mapTxCacheBlockHash.clear();
//	for (auto &item : mapTxHashByBlockHash) {
//		mapTxCacheBlockHash.insert(make_pair(mapBlockIndex[item.first]->nHeight, item.first));
//	}
//	for(auto &item1 : mapTxCacheBlockHash) {
//		LogPrint("txcache", "block height:%d, hash:%s\n", item1.first, item1.second.GetHex());
//		for (auto &txHash : mapTxHashByBlockHash[item1.second])
//			LogPrint("txcache", "txhash:%s\n", txHash.GetHex());
//	}
	return true;
}
bool CTransactionDBCache::DeleteBlockFromCache(const CBlock &block) {
//	LogPrint("txcache", "CTransactionDBCache::DeleteBlockFromCache() height=%d blockhash=%s \n", block.nHeight, block.GetHash().GetHex());
	if (IsContainBlock(block)) {
		vector<uint256> vTxHash;
		vTxHash.clear();
		mapTxHashByBlockHash[block.GetHash()] = vTxHash;
		return true;
	} else {
		LogPrint("ERROR", "the block hash:%s isn't in TxCache\n", block.GetHash().GetHex());
		return false;
	}
	return true;
}
uint256 CTransactionDBCache::IsContainTx(const uint256 & txHash) {
	for (auto & item : mapTxHashByBlockHash) {
		vector<uint256>::iterator it = find(item.second.begin(), item.second.end(), txHash);
		if (it != item.second.end()) {
			return item.first;
		}
	}
	uint256 blockHash = pBase->IsContainTx(txHash);
	if(IsInMap(mapTxHashByBlockHash,blockHash)){//mapTxHashByBlockHash[blockHash].empty()) { // [] �������ֹ��С�ļ�������������
		return std::move(blockHash);
	}
	return std::move(uint256());
}
map<uint256, vector<uint256> > CTransactionDBCache::GetTxHashCache(void) {
	return mapTxHashByBlockHash;
}
bool CTransactionDBCache::BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHashIn) {
	for(auto & item : mapTxHashByBlockHashIn) {
		mapTxHashByBlockHash[item.first] = item.second;
	}
	return true;
}
bool CTransactionDBCache::Flush() {
	bool bRet = pBase->BatchWrite(mapTxHashByBlockHash);
	if (bRet) {
		map<uint256, vector<uint256> >::iterator iter = mapTxHashByBlockHash.begin();
		for(; iter != mapTxHashByBlockHash.end(); )
		{
			if(iter->second.empty()) {
				mapTxHashByBlockHash.erase(iter++);
			}
			else{
				iter++;
			}
		}
	}
	return bRet;
}
void CTransactionDBCache::AddTxHashCache(const uint256 & blockHash, const vector<uint256> &vTxHash) {
	mapTxHashByBlockHash[blockHash] = vTxHash;
}
bool CTransactionDBCache::LoadTransaction() {
	return pBase->LoadTransaction(mapTxHashByBlockHash);
}
void CTransactionDBCache::Clear() {
	mapTxHashByBlockHash.clear();
}
int CTransactionDBCache::GetSize() {
	int iCount(0);
	for(auto & i : mapTxHashByBlockHash) {
		if(!i.second.empty())
		 ++iCount;
	}
	return iCount;
}
bool CTransactionDBCache::IsInMap(const map<uint256, vector<uint256> >& mMap, const uint256 &hash) const {
	if (hash == uint256())
		return false;
	auto te =mMap.find(hash);
	if(te != mMap.end()){
		return !te->second.empty();
	}

	return false;
}
Object CTransactionDBCache::ToJosnObj() const {
	Array deletedobjArray;
	Array InobjArray;
	for (auto& item : mapTxHashByBlockHash) {
		Object obj;
		obj.push_back(Pair("blockhash", item.first.ToString()));

		Array objTxInBlock;
		for (const auto& itemTx : item.second) {
			Object objTxHash;
			objTxHash.push_back(Pair("txhash", itemTx.ToString()));
			objTxInBlock.push_back(objTxHash);
		}
		obj.push_back(Pair("txHashs", objTxInBlock));
       if(item.second.size() > 0) {
		InobjArray.push_back(std::move(obj));
       }
       else{
    	   deletedobjArray.push_back(std::move(obj));
       }
	}
	Object temobj;
	temobj.push_back(Pair("incachblock", std::move(InobjArray)));
//	temobj.push_back(Pair("removecachblock", std::move(deletedobjArray)));
	Object retobj;
	retobj.push_back(Pair("mapTxHashByBlockHash", std::move(temobj)));
	return std::move(retobj);

}
void CTransactionDBCache::SetBaseData(CTransactionDBView *pNewBase){
	pBase = pNewBase;
}

const map<uint256, vector<uint256> > & CTransactionDBCache::GetCacheMap() {
	return mapTxHashByBlockHash;
}
void CTransactionDBCache::SetCacheMap(const map<uint256, vector<uint256> > &mapCache) {
	mapTxHashByBlockHash = mapCache;
}
bool CScriptDBViewCache::GetScriptAcc(const CRegID& scriptId, const vector<unsigned char> &vAccKey, CAppUserAccout& appAccOut) {
	vector<unsigned char> scriptKey = {'a','c','c','t'};
	vector<unsigned char> vRegId = scriptId.GetVec6();
	scriptKey.insert(scriptKey.end(), vRegId.begin(), vRegId.end());
	scriptKey.push_back( '_');
	scriptKey.insert(scriptKey.end(), vAccKey.begin(), vAccKey.end());
	vector<unsigned char> vValue;

	//LogPrint("vm","%s",HexStr(scriptKey));
	if(!GetData(scriptKey, vValue))
		return false;
	CDataStream ds(vValue, SER_DISK, CLIENT_VERSION);
	ds >> appAccOut;
	return true;
}
bool CScriptDBViewCache::SetScriptAcc(const CRegID& scriptId, const CAppUserAccout& appAccOut, CScriptDBOperLog &operlog) {
	vector<unsigned char> scriptKey = {'a','c','c','t'};
	vector<unsigned char> vRegId = scriptId.GetVec6();
	vector<unsigned char> vAccKey = appAccOut.getaccUserId();
	scriptKey.insert(scriptKey.end(), vRegId.begin(), vRegId.end());
	scriptKey.push_back( '_');
	scriptKey.insert(scriptKey.end(), vAccKey.begin(), vAccKey.end());
	vector<unsigned char> vValue;
	operlog.vKey = scriptKey;
	if(GetData(scriptKey, vValue))
	{
		operlog.vValue = vValue;
	}
	CDataStream ds(SER_DISK, CLIENT_VERSION);

	ds << appAccOut;
	//LogPrint("vm","%s",HexStr(scriptKey));
	vValue.clear();
	vValue.insert(vValue.end(), ds.begin(), ds.end());
	return SetData(scriptKey, vValue);
}
