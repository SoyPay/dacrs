#include "database.h"
#include "util.h"
#include "serialize.h"
#include "core.h"
#include "main.h"
#include "chainparams.h"
#include "vm/vmrunevn.h"
#include <algorithm>

bool CAccountView::GetAccount(const CKeyID &cKeyId, CAccount &cAccount) {
	return false;
}

bool CAccountView::SetAccount(const CKeyID &cKeyId, const CAccount &cAccount) {
	return false;
}

bool CAccountView::SetAccount(const vector<unsigned char> &vchAccountId, const CAccount &cAccount) {
	return false;
}

bool CAccountView::HaveAccount(const CKeyID &cKeyId) {
	return false;
}

uint256 CAccountView::GetBestBlock() {
	return uint256();
}

bool CAccountView::SetBestBlock(const uint256 &cHashBlock) {
	return false;
}

bool CAccountView::BatchWrite(const map<CKeyID, CAccount> &mapAccounts,
		const map<vector<unsigned char>, CKeyID> &mapKeyIds, const uint256 &cHashBlock) {
	return false;
}

bool CAccountView::BatchWrite(const vector<CAccount> &vcAccounts) {
	return false;
}

bool CAccountView::EraseAccount(const CKeyID &cKeyId) {
	return false;
}

bool CAccountView::SetKeyId(const vector<unsigned char> &vchAccountId, const CKeyID &cKeyId) {
	return false;
}

bool CAccountView::GetKeyId(const vector<unsigned char> &vchAccountId, CKeyID &cKeyId) {
	return false;
}

bool CAccountView::GetAccount(const vector<unsigned char> &vchAccountId, CAccount &cAccount) {
	return false;
}

bool CAccountView::EraseKeyId(const vector<unsigned char> &vchAccountId) {
	return false;
}

bool CAccountView::SaveAccountInfo(const vector<unsigned char> &vchAccountId, const CKeyID &cKeyId,
		const CAccount &cAccount) {
	return false;
}

Object CAccountView::ToJosnObj(char chPrefix) {
	Object obj;
	return obj;
}

uint64_t CAccountView::TraverseAccount() {
	return 0;
}

CAccountViewBacked::CAccountViewBacked(CAccountView &cAccountView) :
		m_pBaseAccountView(&cAccountView) {
}

bool CAccountViewBacked::GetAccount(const CKeyID &cKeyId, CAccount &cAccount) {
	return m_pBaseAccountView->GetAccount(cKeyId, cAccount);
}

bool CAccountViewBacked::SetAccount(const CKeyID &cKeyId, const CAccount &cAccount) {
	return m_pBaseAccountView->SetAccount(cKeyId, cAccount);
}

bool CAccountViewBacked::SetAccount(const vector<unsigned char> &vchAccountId, const CAccount &cAccount) {
	return m_pBaseAccountView->SetAccount(vchAccountId, cAccount);
}

bool CAccountViewBacked::HaveAccount(const CKeyID &cKeyId) {
	return m_pBaseAccountView->HaveAccount(cKeyId);
}

uint256 CAccountViewBacked::GetBestBlock() {
	return m_pBaseAccountView->GetBestBlock();
}

bool CAccountViewBacked::SetBestBlock(const uint256 &cHashBlock) {
	return m_pBaseAccountView->SetBestBlock(cHashBlock);
}

bool CAccountViewBacked::BatchWrite(const map<CKeyID, CAccount> &mapAccounts,
		const map<std::vector<unsigned char>, CKeyID> &mapKeyIds, const uint256 &cHashBlock) {
	return m_pBaseAccountView->BatchWrite(mapAccounts, mapKeyIds, cHashBlock);
}

bool CAccountViewBacked::BatchWrite(const vector<CAccount> &vcAccounts) {
	return m_pBaseAccountView->BatchWrite(vcAccounts);
}

bool CAccountViewBacked::EraseAccount(const CKeyID &cKeyId) {
	return m_pBaseAccountView->EraseAccount(cKeyId);
}

bool CAccountViewBacked::SetKeyId(const vector<unsigned char> &vchAccountId, const CKeyID &cKeyId) {
	return m_pBaseAccountView->SetKeyId(vchAccountId, cKeyId);
}

bool CAccountViewBacked::GetKeyId(const vector<unsigned char> &vchAccountId, CKeyID &cKeyId) {
	return m_pBaseAccountView->GetKeyId(vchAccountId, cKeyId);
}

bool CAccountViewBacked::EraseKeyId(const vector<unsigned char> &vchAccountId) {
	return m_pBaseAccountView->EraseKeyId(vchAccountId);
}

bool CAccountViewBacked::GetAccount(const vector<unsigned char> &vchAccountId, CAccount &cAccount) {
	return m_pBaseAccountView->GetAccount(vchAccountId, cAccount);
}

bool CAccountViewBacked::SaveAccountInfo(const vector<unsigned char> &vchAccountId, const CKeyID &cKeyId,
		const CAccount &cAccount) {
	return m_pBaseAccountView->SaveAccountInfo(vchAccountId, cKeyId, cAccount);
}

uint64_t CAccountViewBacked::TraverseAccount() {
	return m_pBaseAccountView->TraverseAccount();
}

CAccountViewCache::CAccountViewCache(CAccountView &cAccountView, bool bDummy) :
		CAccountViewBacked(cAccountView), m_cHashBlock(uint256()) {
}

bool CAccountViewCache::GetAccount(const CKeyID &cKeyId, CAccount &cAccount) {
	if (m_mapCacheAccounts.count(cKeyId)) {
		if (m_mapCacheAccounts[cKeyId].keyID != uint160()) {
			cAccount = m_mapCacheAccounts[cKeyId];
			return true;
		} else {
			return false;
		}
	}
	if (m_pBaseAccountView->GetAccount(cKeyId, cAccount)) {
		m_mapCacheAccounts[cKeyId] = cAccount;
		return true;
	}

	return false;
}

bool CAccountViewCache::SetAccount(const CKeyID &cKeyId, const CAccount &cAccount) {
	m_mapCacheAccounts[cKeyId] = cAccount;

	return true;
}

bool CAccountViewCache::SetAccount(const vector<unsigned char> &vchAccountId, const CAccount &cAccount) {
	if (vchAccountId.empty()) {
		return false;
	}
	if (m_mapCacheKeyIds.count(vchAccountId)) {
		m_mapCacheAccounts[m_mapCacheKeyIds[vchAccountId]] = cAccount;
		return true;
	}

	return false;
}

bool CAccountViewCache::HaveAccount(const CKeyID &cKeyId) {
	if (m_mapCacheAccounts.count(cKeyId))
		return true;
	else {
		return m_pBaseAccountView->HaveAccount(cKeyId);
	}
}

uint256 CAccountViewCache::GetBestBlock() {
	if(m_cHashBlock == uint256()) {
		return m_pBaseAccountView->GetBestBlock();
	}

	return m_cHashBlock;
}

bool CAccountViewCache::SetBestBlock(const uint256 &cHashBlockIn) {
	m_cHashBlock = cHashBlockIn;

	return true;
}

bool CAccountViewCache::BatchWrite(const map<CKeyID, CAccount> &mapAccounts,
		const map<vector<unsigned char>, CKeyID> &mapKeyIds, const uint256 &cHashBlockIn) {
	for (map<CKeyID, CAccount>::const_iterator it = mapAccounts.begin(); it != mapAccounts.end(); ++it) {
		if (uint160() == it->second.keyID) {
			m_pBaseAccountView->EraseAccount(it->first);
			m_mapCacheAccounts.erase(it->first);
		} else {
			m_mapCacheAccounts[it->first] = it->second;
		}
	}

	for (map<vector<unsigned char>, CKeyID>::const_iterator itKeyId = mapKeyIds.begin(); itKeyId != mapKeyIds.end();
			++itKeyId) {
		m_mapCacheKeyIds[itKeyId->first] = itKeyId->second;
	}
	m_cHashBlock = cHashBlockIn;

	return true;
}

bool CAccountViewCache::BatchWrite(const vector<CAccount> &vcAccounts) {
	for (vector<CAccount>::const_iterator it = vcAccounts.begin(); it != vcAccounts.end(); ++it) {
		if (it->IsEmptyValue() && !it->IsRegister()) {
			m_mapCacheAccounts[it->keyID] = *it;
			m_mapCacheAccounts[it->keyID].keyID = uint160();
		} else {
			m_mapCacheAccounts[it->keyID] = *it;
		}
	}
	return true;
}

bool CAccountViewCache::EraseAccount(const CKeyID &cKeyId) {
	if (m_mapCacheAccounts.count(cKeyId)) {
		m_mapCacheAccounts[cKeyId].keyID = uint160();
	}
	else {
		CAccount account;
		if (m_pBaseAccountView->GetAccount(cKeyId, account)) {
			account.keyID = uint160();
			m_mapCacheAccounts[cKeyId] = account;
		}
	}

	return true;
}

bool CAccountViewCache::SetKeyId(const vector<unsigned char> &vchAccountId, const CKeyID &cKeyId) {
	if(vchAccountId.empty()) {
		return false;
	}
	m_mapCacheKeyIds[vchAccountId] = cKeyId;

	return true;
}

bool CAccountViewCache::GetKeyId(const vector<unsigned char> &vchAccountId, CKeyID &cKeyId) {
	if (vchAccountId.empty()) {
		return false;
	}
	if (m_mapCacheKeyIds.count(vchAccountId)) {
		cKeyId = m_mapCacheKeyIds[vchAccountId];
		if (cKeyId != uint160()) {
			return true;
		} else {
			return false;
		}
	}
	if (m_pBaseAccountView->GetKeyId(vchAccountId, cKeyId)) {
		m_mapCacheKeyIds[vchAccountId] = cKeyId;
		return true;
	}

	return false;
}

bool CAccountViewCache::EraseKeyId(const vector<unsigned char> &vchAccountId) {
	if (vchAccountId.empty()) {
		return false;
	}
	if (m_mapCacheKeyIds.count(vchAccountId)) {
		m_mapCacheKeyIds[vchAccountId] = uint160();
	} else {
		CKeyID keyId;
		if (m_pBaseAccountView->GetKeyId(vchAccountId, keyId)) {
			m_mapCacheKeyIds[vchAccountId] = uint160();
		}
	}

	return true;
}

bool CAccountViewCache::GetAccount(const vector<unsigned char> &vchAccountId, CAccount &cAccount) {
	if (vchAccountId.empty()) {
		return false;
	}
	if (m_mapCacheKeyIds.count(vchAccountId)) {
		CKeyID keyId(m_mapCacheKeyIds[vchAccountId]);
		if (keyId != uint160()) {
			if (m_mapCacheAccounts.count(keyId)) {
				cAccount = m_mapCacheAccounts[keyId];
				if (cAccount.keyID != uint160()) {  // 判断此帐户是否被删除了
					return true;
				} else {
					return false;   //已删除返回false
				}
			} else {
				return m_pBaseAccountView->GetAccount(keyId, cAccount); //缓存map中没有，从上级存取
			}
		} else {
			return false;  //accountId已删除说明账户信息也已删除
		}
	} else {
		CKeyID keyId;
		if (m_pBaseAccountView->GetKeyId(vchAccountId, keyId)) {
			m_mapCacheKeyIds[vchAccountId] = keyId;
			if (m_mapCacheAccounts.count(keyId) > 0) {
				cAccount = m_mapCacheAccounts[keyId];
				if (cAccount.keyID != uint160()) { // 判断此帐户是否被删除了
					return true;
				} else {
					return false;   //已删除返回false
				}
			}
			bool ret = m_pBaseAccountView->GetAccount(keyId, cAccount);
			if (ret) {
				m_mapCacheAccounts[keyId] = cAccount;
				return true;
			}
		}
	}

	return false;
}

bool CAccountViewCache::SaveAccountInfo(const CRegID &cRegId, const CKeyID &cKeyId, const CAccount &cAccount) {
	m_mapCacheKeyIds[cRegId.GetVec6()] = cKeyId;
	m_mapCacheAccounts[cKeyId] = cAccount;
	return true;
}

uint64_t CAccountViewCache::TraverseAccount() {
	return m_pBaseAccountView->TraverseAccount();
}

bool CAccountViewCache::GetAccount(const CUserID &cUserId, CAccount &cAccount) {
	bool ret = false;
	if (cUserId.type() == typeid(CRegID)) {
		ret = GetAccount(boost::get<CRegID>(cUserId).GetVec6(), cAccount);
//		if(ret) assert(boost::get<CRegID>(userId) == account.regID);
	} else if (cUserId.type() == typeid(CKeyID)) {
		ret = GetAccount(boost::get<CKeyID>(cUserId), cAccount);
//		if(ret) assert(boost::get<CKeyID>(userId) == account.keyID);
	} else if (cUserId.type() == typeid(CPubKey)) {
		ret = GetAccount(boost::get<CPubKey>(cUserId).GetKeyID(), cAccount);
//		if(ret) assert((boost::get<CPubKey>(userId)).GetKeyID() == account.keyID);
	} else if (cUserId.type() == typeid(CNullID)) {
		return ERRORMSG("GetAccount input userid can't be CNullID type");
	}

	return ret;
}

bool CAccountViewCache::GetKeyId(const CUserID &cUserId, CKeyID &cKeyId) {
	if (cUserId.type() == typeid(CRegID)) {
		return GetKeyId(boost::get<CRegID>(cUserId).GetVec6(), cKeyId);
	} else if (cUserId.type() == typeid(CPubKey)) {
		cKeyId = boost::get<CPubKey>(cUserId).GetKeyID();
		return true;
	} else if (cUserId.type() == typeid(CKeyID)) {
		cKeyId = boost::get<CKeyID>(cUserId);
		return true;
	} else if (cUserId.type() == typeid(CNullID)) {
		return ERRORMSG("GetKeyId input userid can't be CNullID type");
	}

	return ERRORMSG("GetKeyId input userid is unknow type");
}

bool CAccountViewCache::SetAccount(const CUserID &cUserId, const CAccount &cAccount) {
	if (cUserId.type() == typeid(CRegID)) {
		return SetAccount(boost::get<CRegID>(cUserId).GetVec6(), cAccount);
	} else if (cUserId.type() == typeid(CKeyID)) {
		return SetAccount(boost::get<CKeyID>(cUserId), cAccount);
	} else if (cUserId.type() == typeid(CPubKey)) {
		return SetAccount(boost::get<CPubKey>(cUserId).GetKeyID(), cAccount);
	} else if (cUserId.type() == typeid(CNullID)) {
		return ERRORMSG("SetAccount input userid can't be CNullID type");
	}
	return ERRORMSG("SetAccount input userid is unknow type");
}

bool CAccountViewCache::SetKeyId(const CUserID &cUserId, const CKeyID &cKeyId) {
	if (cUserId.type() == typeid(CRegID)) {
		return SetKeyId(boost::get<CRegID>(cUserId).GetVec6(), cKeyId);
	} else {
//		assert(0);
	}

	return false;
}

bool CAccountViewCache::EraseAccount(const CUserID &cUserId) {
	if (cUserId.type() == typeid(CKeyID)) {
		return EraseAccount(boost::get<CKeyID>(cUserId));
	} else if (cUserId.type() == typeid(CPubKey)) {
		return EraseAccount(boost::get<CPubKey>(cUserId).GetKeyID());
	} else {
		return ERRORMSG("EraseAccount account type error!");
//		assert(0);
	}

	return false;
}

bool CAccountViewCache::HaveAccount(const CUserID &cUserId) {
	if (cUserId.type() == typeid(CKeyID)) {
		return HaveAccount(boost::get<CKeyID>(cUserId));
	} else {
//		assert(0);
	}

	return false;
}

bool CAccountViewCache::EraseId(const CUserID &cUserId) {
	if (cUserId.type() == typeid(CRegID)) {
		return EraseKeyId(boost::get<CRegID>(cUserId).GetVec6());
	} else {
//		assert(0);
	}

	return false;
}

bool CAccountViewCache::Flush(){
	 bool fOk = m_pBaseAccountView->BatchWrite(m_mapCacheAccounts, m_mapCacheKeyIds, m_cHashBlock);
	 if (fOk) {
		 m_mapCacheAccounts.clear();
		 m_mapCacheKeyIds.clear();
	 }

	 return fOk;
}

bool CAccountViewCache::GetRegId(const CUserID& cUserId, CRegID& cRegId) const {
	CAccountViewCache tempView(*this);
	CAccount account;
	if (cUserId.type() == typeid(CRegID)) {
		cRegId = boost::get<CRegID>(cUserId);
		return true;
	}
	if (tempView.GetAccount(cUserId, account)) {
		cRegId = account.regID;
		return !cRegId.IsEmpty();
	}

	return false;
}

int64_t CAccountViewCache::GetRawBalance(const CUserID& cUserId) const {
	CAccountViewCache tempvew(*this);
	CAccount account;
	if(tempvew.GetAccount(cUserId,account))
	{
		return  account.GetRawBalance();
	}

	return 0;
}

unsigned int CAccountViewCache::GetCacheSize() {
	return ::GetSerializeSize(m_mapCacheAccounts, SER_DISK, CLIENT_VERSION)
			+ ::GetSerializeSize(m_mapCacheKeyIds, SER_DISK, CLIENT_VERSION);
}

Object CAccountViewCache::ToJosnObj() const {
	Object obj;
	Array arrayObj;
	obj.push_back(Pair("hashBlock", m_cHashBlock.ToString()));
	arrayObj.push_back(m_pBaseAccountView->ToJosnObj('a'));
	arrayObj.push_back(m_pBaseAccountView->ToJosnObj('k'));
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

void CAccountViewCache::SetBaseData(CAccountView * pAccountView){
	m_pBaseAccountView = pAccountView;
}

bool CScriptDBView::GetData(const vector<unsigned char> &vchKey, vector<unsigned char> &vchValue) {
	return false;
}

bool CScriptDBView::SetData(const vector<unsigned char> &vchKey, const vector<unsigned char> &vchValue) {
	return false;
}

bool CScriptDBView::BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas) {
	return false;
}

bool CScriptDBView::EraseKey(const vector<unsigned char> &vchKey) {
	return false;
}

bool CScriptDBView::HaveData(const vector<unsigned char> &vchKey) {
	return false;
}

bool CScriptDBView::GetScript(const int &nIndex, vector<unsigned char> &vchScriptId, vector<unsigned char> &vchValue) {
	return false;
}

bool CScriptDBView::GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vchScriptId,
		const int &nIndex, vector<unsigned char> &vchScriptKey, vector<unsigned char> &vchScriptData) {
	return false;
}

bool CScriptDBView::ReadTxIndex(const uint256 &cTxId, CDiskTxPos &cDiskTxPos) {
	return false;
}

bool CScriptDBView::WriteTxIndex(const vector<pair<uint256, CDiskTxPos> > &list,
		vector<CScriptDBOperLog> &vTxIndexOperDB) {
	return false;
}

bool CScriptDBView::WriteTxOutPut(const uint256 &cTxId, const vector<CVmOperate> &vcOutput,
		CScriptDBOperLog &cScriptDBOperLog) {
	return false;
}

bool CScriptDBView::ReadTxOutPut(const uint256 &cTxId, vector<CVmOperate> &vcOutput) {
	return false;
}

bool CScriptDBView::GetTxHashByAddress(const CKeyID &cKeyId, int nHeight,
		map<vector<unsigned char>, vector<unsigned char> > &mapTxHash) {
	return false;
}

bool CScriptDBView::SetTxHashByAddress(const CKeyID &cKeyId, int nHeight, int nIndex, const string &strTxHash,
		CScriptDBOperLog &cScriptDBOperLog) {
	return false;
}

bool CScriptDBView::GetAllScriptAcc(const CRegID& cRegId, map<vector<unsigned char>, vector<unsigned char> > &mapAcc) {
	return false;
}

Object CScriptDBView::ToJosnObj(string strPrefix) {
	Object obj;
	return obj;
}

CScriptDBViewBacked::CScriptDBViewBacked(CScriptDBView &cDataBaseView) {
	m_pBase = &cDataBaseView;
}

bool CScriptDBViewBacked::GetData(const vector<unsigned char> &vchKey, vector<unsigned char> &vchValue) {
	return m_pBase->GetData(vchKey, vchValue);
}

bool CScriptDBViewBacked::SetData(const vector<unsigned char> &vchKey, const vector<unsigned char> &vchValue) {
	return m_pBase->SetData(vchKey, vchValue);
}

bool CScriptDBViewBacked::BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas) {
	return m_pBase->BatchWrite(mapDatas);
}

bool CScriptDBViewBacked::EraseKey(const vector<unsigned char> &vchKey) {
	return m_pBase->EraseKey(vchKey);
}

bool CScriptDBViewBacked::HaveData(const vector<unsigned char> &vchKey) {
	return m_pBase->HaveData(vchKey);
}

bool CScriptDBViewBacked::GetScript(const int &nIndex, vector<unsigned char> &vchScriptId,
		vector<unsigned char> &vchValue) {
	return m_pBase->GetScript(nIndex, vchScriptId, vchValue);
}

bool CScriptDBViewBacked::GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vchScriptId,
		const int &nIndex, vector<unsigned char> &vchScriptKey, vector<unsigned char> &vchScriptData) {
	return m_pBase->GetScriptData(nCurBlockHeight, vchScriptId, nIndex, vchScriptKey, vchScriptData);
}

bool CScriptDBViewBacked::ReadTxIndex(const uint256 &cTxId, CDiskTxPos &cDiskTxPos) {
	return m_pBase->ReadTxIndex(cTxId, cDiskTxPos);
}

bool CScriptDBViewBacked::WriteTxIndex(const vector<pair<uint256, CDiskTxPos> > &list,
		vector<CScriptDBOperLog> &vTxIndexOperDB) {
	return m_pBase->WriteTxIndex(list, vTxIndexOperDB);
}

bool CScriptDBViewBacked::WriteTxOutPut(const uint256 &cTxId, const vector<CVmOperate> &vOutput,
		CScriptDBOperLog &cScriptDBOperLog) {
	return m_pBase->WriteTxOutPut(cTxId, vOutput, cScriptDBOperLog);
}

bool CScriptDBViewBacked::ReadTxOutPut(const uint256 &cTxId, vector<CVmOperate> &vOutput) {
	return m_pBase->ReadTxOutPut(cTxId, vOutput);
}

bool CScriptDBViewBacked::GetTxHashByAddress(const CKeyID &cKeyId, int nHeight,
		map<vector<unsigned char>, vector<unsigned char> > &mapTxHash) {
	return m_pBase->GetTxHashByAddress(cKeyId, nHeight, mapTxHash);
}

bool CScriptDBViewBacked::SetTxHashByAddress(const CKeyID &cKeyId, int nHeight, int nIndex, const string &strTxHash,
		CScriptDBOperLog &cScriptDBOperLog) {
	return m_pBase->SetTxHashByAddress(cKeyId, nHeight, nIndex, strTxHash, cScriptDBOperLog);
}

bool CScriptDBViewBacked::GetAllScriptAcc(const CRegID& cRegId,
		map<vector<unsigned char>, vector<unsigned char> > &mapAcc) {
	return m_pBase->GetAllScriptAcc(cRegId, mapAcc);
}

CScriptDBViewCache::CScriptDBViewCache(CScriptDBView &cScriptDBView, bool bDummy) :
		CScriptDBViewBacked(cScriptDBView) {
	m_mapDatas.clear();
}

bool CScriptDBViewCache::GetData(const vector<unsigned char> &vchKey, vector<unsigned char> &vchValue) {
	if (m_mapDatas.count(vchKey) > 0) {
		if (!m_mapDatas[vchKey].empty()) {
			vchValue = m_mapDatas[vchKey];
			return true;
		} else {
			return false;
		}
	}
	if (!m_pBase->GetData(vchKey, vchValue)) {
		return false;
	}
	m_mapDatas[vchKey] = vchValue;

	return true;
}

bool CScriptDBViewCache::SetData(const vector<unsigned char> &vchKey, const vector<unsigned char> &vchValue) {
	m_mapDatas[vchKey] = vchValue;
	return true;
}

bool CScriptDBViewCache::UndoScriptData(const vector<unsigned char> &vchKey, const vector<unsigned char> &vchValue) {
	vector<unsigned char> vPrefix(vchKey.begin(), vchKey.begin() + 4);
	vector<unsigned char> vScriptDataPrefix = { 'd', 'a', 't', 'a' };
	if (vPrefix == vScriptDataPrefix) {
		assert(vchKey.size() > 10);
		if (vchKey.size() < 10) {
			return ERRORMSG("UndoScriptData(): vKey=%s error!\n", HexStr(vchKey));
		}
		vector<unsigned char> vScriptCountKey = { 's', 'd', 'n', 'u', 'm' };
		vector<unsigned char> vScriptId(vchKey.begin() + 4, vchKey.begin() + 10);
		vector<unsigned char> vOldValue;
		if (m_mapDatas.count(vchKey)) {
			vOldValue = m_mapDatas[vchKey];
		} else {
			GetData(vchKey, vOldValue);
		}
		vScriptCountKey.insert(vScriptCountKey.end(), vScriptId.begin(), vScriptId.end());
		CDataStream ds(SER_DISK, CLIENT_VERSION);

		int nCount(0);
		if (vchValue.empty()) {   //key所对应的值由非空设置为空，计数减1
			if (!vOldValue.empty()) {
				if (!GetScriptDataCount(vScriptId, nCount)) {
					return false;
				}
				--nCount;
				if (!SetScriptDataCount(vScriptId, nCount)) {
					return false;
				}
			}
		} else {    //key所对应的值由空设置为非空，计数加1
			if (vOldValue.empty()) {
				GetScriptDataCount(vScriptId, nCount);
				++nCount;
				if (!SetScriptDataCount(vScriptId, nCount)) {
					return false;
				}
			}
		}
	}
	m_mapDatas[vchKey] = vchValue;

	return true;
}

bool CScriptDBViewCache::BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas) {
	for (auto &items : mapDatas) {
		m_mapDatas[items.first] = items.second;
	}

	return true;
}

bool CScriptDBViewCache::EraseKey(const vector<unsigned char> &vchKey) {
	if (m_mapDatas.count(vchKey) > 0) {
		m_mapDatas[vchKey].clear();
	} else {
		vector<unsigned char> vValue;
		if (m_pBase->GetData(vchKey, vValue)) {
			vValue.clear();
			m_mapDatas[vchKey] = vValue;
		} else {
			return false;
		}
	}

	return true;
}

bool CScriptDBViewCache::HaveData(const vector<unsigned char> &vchKey) {
	if (m_mapDatas.count(vchKey) > 0) {
		if (!m_mapDatas[vchKey].empty()) {
			return true;
		} else {
			return false;
		}
	}

	return m_pBase->HaveData(vchKey);
}

bool CScriptDBViewCache::GetScript(const int nIndex, vector<unsigned char> &vchScriptId,
		vector<unsigned char> &vchValue) {
	if (0 == nIndex) {
		vector<unsigned char> vchScriptKey = { 'd', 'e', 'f' };
		vector<unsigned char> vchDataKey;
		vector<unsigned char> vchDataValue;
		vchDataKey.clear();
		vchDataValue.clear();
		for (auto &item : m_mapDatas) {   //遍历本级缓存数据，找出合法的最小的key值
			vector<unsigned char> vTemp(item.first.begin(), item.first.begin() + 3);
			if (vchScriptKey == vTemp) {
				if (item.second.empty()) {
					continue;
				}
				vchDataKey = item.first;
				vchDataValue = item.second;
				break;
			}
		}
		if (!m_pBase->GetScript(nIndex, vchScriptId, vchValue)) { //上级没有获取符合条件的key值
			if (vchDataKey.empty()) {
				return false;
			} else { //返回本级缓存的查询结果
				vchScriptId.clear();
				vchValue.clear();
				vchScriptId.assign(vchDataKey.begin() + 3, vchDataKey.end());
				vchValue = vchDataValue;
				return true;
			}
		} else { //上级获取到符合条件的key值
			if (vchDataKey.empty()) {  //缓存中没有符合条件的key，直接返回上级的查询结果
				return true;
			}
			vector<unsigned char> vchDataKeyTemp = { 'd', 'e', 'f' };
			vchDataKeyTemp.insert(vchDataKeyTemp.end(), vchScriptId.begin(), vchScriptId.end()); //上级得到的key值
			if (vchDataKeyTemp < vchDataKey) {  //若上级查询的key小于本级缓存的key,且此key在缓存中没有，则直接返回数据库中查询的结果
				if (m_mapDatas.count(vchDataKeyTemp) == 0) {
					return true;
				} else {
					m_mapDatas[vchDataKeyTemp].clear();  //在缓存中dataKeyTemp已经被删除过了，重新将此key对应的value清除
					return GetScript(nIndex, vchScriptId, vchValue); //重新从数据库中获取下一条数据
				}
			} else {  //若上级查询的key大于等于本级缓存的key,返回本级的数据
				vchScriptId.clear();
				vchValue.clear();
				vchScriptId.assign(vchDataKey.begin() + 3, vchDataKey.end());
				vchValue = vchDataValue;
				return true;
			}
		}
	} else if (1 == nIndex) {
		vector<unsigned char> vchKey = { 'd', 'e', 'f' };
		vchKey.insert(vchKey.end(), vchScriptId.begin(), vchScriptId.end());
		vector<unsigned char> vchPreKey(vchKey);
		map<vector<unsigned char>, vector<unsigned char> >::iterator iterFindKey = m_mapDatas.upper_bound(vchPreKey);
		vector<unsigned char> vchDataKey;
		vector<unsigned char> vchDataValue;
		vchDataKey.clear();
		vchDataValue.clear();
		vector<unsigned char> vchKeyTemp = { 'd', 'e', 'f' };
		while (iterFindKey != m_mapDatas.end()) {
			vector<unsigned char> vchTemp(iterFindKey->first.begin(), iterFindKey->first.begin() + 3);
			if (vchKeyTemp == vchTemp) {
				if (iterFindKey->second.empty()) {
					++iterFindKey;
					continue;
				} else {
					vchDataKey = iterFindKey->first;
					vchDataValue = iterFindKey->second;
					break;
				}
			} else {
				++iterFindKey;
			}
		}
		if (!m_pBase->GetScript(nIndex, vchScriptId, vchValue)) { //从BASE获取指定键值之后的下一个值
			if (vchDataKey.empty()) {
				return false;
			} else {
				vchScriptId.clear();
				vchValue.clear();
				vchScriptId.assign(vchDataKey.begin() + 3, vchDataKey.end());
				vchValue = vchDataValue;
				return true;
			}
		} else {
			if (vchDataKey.empty()) {   //缓存中没有符合条件的key，直接返回上级的查询结果
				return true;
			}
			vector<unsigned char> vchDataKeyTemp = { 'd', 'e', 'f' };
			vchDataKeyTemp.insert(vchDataKeyTemp.end(), vchScriptId.begin(), vchScriptId.end()); //上级得到的key值
			if (vchDataKeyTemp < vchDataKey) {
				if (m_mapDatas.count(vchDataKeyTemp) == 0) {
					return true;
				} else {
					m_mapDatas[vchDataKeyTemp].clear();  //在缓存中dataKeyTemp已经被删除过了，重新将此key对应的value清除
					return GetScript(nIndex, vchScriptId, vchValue); //重新从数据库中获取下一条数据
				}
			} else { //若上级查询的key大于等于本级缓存的key,返回本级的数据
				vchScriptId.clear();
				vchValue.clear();
				vchScriptId.assign(vchDataKey.begin() + 3, vchDataKey.end());
				vchValue = vchDataValue;
				return true;
			}
		}
	} else {
//		assert(0);
	}

	return true;
}

bool CScriptDBViewCache::SetScript(const vector<unsigned char> &vchScriptId, const vector<unsigned char> &vchValue) {
	vector<unsigned char> vchScriptKey = { 'd', 'e', 'f' };
	vchScriptKey.insert(vchScriptKey.end(), vchScriptId.begin(), vchScriptId.end());

	if (!HaveScript(vchScriptId)) {
		int nCount(0);
		GetScriptCount(nCount);
		++nCount;
		if (!SetScriptCount(nCount)) {
			return false;
		}
	}

	return SetData(vchScriptKey, vchValue);
}

bool CScriptDBViewCache::Flush() {
	bool ok = m_pBase->BatchWrite(m_mapDatas);
	if (ok) {
		m_mapDatas.clear();
	}

	return ok;
}

unsigned int CScriptDBViewCache::GetCacheSize() {
	return ::GetSerializeSize(m_mapDatas, SER_DISK, CLIENT_VERSION);
}

bool CScriptDBViewCache::WriteTxOutPut(const uint256 &cTxId, const vector<CVmOperate> &vcOutput,
		CScriptDBOperLog &cScriptDBOperLog) {
	vector<unsigned char> vchKey = { 'o', 'u', 't', 'p', 'u', 't' };
	CDataStream cDS1(SER_DISK, CLIENT_VERSION);
	cDS1 << cTxId;
	vchKey.insert(vchKey.end(), cDS1.begin(), cDS1.end());

	vector<unsigned char> vchValue;
	CDataStream cDS(SER_DISK, CLIENT_VERSION);
	cDS << vcOutput;
	vchValue.assign(cDS.begin(), cDS.end());

	vector<unsigned char> vchOldValue;
	vchOldValue.clear();
	GetData(vchKey, vchOldValue);
	cScriptDBOperLog = CScriptDBOperLog(vchKey, vchOldValue);

	return SetData(vchKey, vchValue);
}

bool CScriptDBViewCache::SetTxHashByAddress(const CKeyID &keyId, int nHeight, int nIndex, const string &strTxHash,
		CScriptDBOperLog &cScriptDBOperLog) {
	vector<unsigned char> vchKey = { 'A', 'D', 'D', 'R' };
	vector<unsigned char> vchOldValue;
	vchOldValue.clear();
	GetData(vchKey, vchOldValue);
	cScriptDBOperLog = CScriptDBOperLog(vchKey, vchOldValue);

	CDataStream cDS1(SER_DISK, CLIENT_VERSION);
	cDS1 << keyId;
	cDS1 << nHeight;
	cDS1 << nIndex;
	vchKey.insert(vchKey.end(), cDS1.begin(), cDS1.end());
	vector<unsigned char> vchValue(strTxHash.begin(), strTxHash.end());

	return SetData(vchKey, vchValue);
}

bool CScriptDBViewCache::GetTxHashByAddress(const CKeyID &cKeyId, int nHeight,
		map<vector<unsigned char>, vector<unsigned char> > &mapTxHash) {
	m_pBase->GetTxHashByAddress(cKeyId, nHeight, mapTxHash);

	vector<unsigned char> vchPreKey = { 'A', 'D', 'D', 'R' };
	CDataStream cDataStream(SER_DISK, CLIENT_VERSION);
	cDataStream << cKeyId;
	cDataStream << nHeight;
	vchPreKey.insert(vchPreKey.end(), cDataStream.begin(), cDataStream.end());

	map<vector<unsigned char>, vector<unsigned char> >::iterator iterFindKey = m_mapDatas.upper_bound(vchPreKey);
	while (iterFindKey != m_mapDatas.end()) {
		if (0 == memcmp((char *) &iterFindKey->first[0], (char *) &vchPreKey[0], 24)) {
			if (iterFindKey->second.empty()) {
				mapTxHash.erase(iterFindKey->first);
			} else {
				mapTxHash.insert(make_pair(iterFindKey->first, iterFindKey->second));
			}
			++iterFindKey;
		} else {
			break;
		}
	}

	return true;
}

bool CScriptDBViewCache::GetAllScriptAcc(const CRegID& cRegId,
		map<vector<unsigned char>, vector<unsigned char> > &mapAcc) {
	return m_pBase->GetAllScriptAcc(cRegId, mapAcc);
}


bool CScriptDBViewCache::ReadTxOutPut(const uint256 &cTxId, vector<CVmOperate> &vcOutput) {
	vector<unsigned char> vchKey = { 'o', 'u', 't', 'p', 'u', 't' };
	CDataStream cDataStream(SER_DISK, CLIENT_VERSION);
	cDataStream << cTxId;
	vchKey.insert(vchKey.end(), cDataStream.begin(), cDataStream.end());
	vector<unsigned char> vValue;
	if (!GetData(vchKey, vValue)) {
		return false;
	}

	CDataStream cDS(vValue, SER_DISK, CLIENT_VERSION);
	cDS >> vcOutput;

	return true;
}


bool CScriptDBViewCache::ReadTxIndex(const uint256 &cTxId, CDiskTxPos &cDiskTxPos) {
	CDataStream cDS(SER_DISK, CLIENT_VERSION);
	cDS << cTxId;
	vector<unsigned char> vchTxHash = { 'T' };
	vchTxHash.insert(vchTxHash.end(), cDS.begin(), cDS.end());
	vector<unsigned char> vTxPos;

	if (m_mapDatas.count(vchTxHash)) {
		if (m_mapDatas[vchTxHash].empty()) {
			return false;
		}
		vTxPos = m_mapDatas[vchTxHash];
		CDataStream dsPos(vTxPos, SER_DISK, CLIENT_VERSION);
		dsPos >> cDiskTxPos;
	} else {
		if (!GetData(vchTxHash, vTxPos)) {
			return false;
		}
		CDataStream dsPos(vTxPos, SER_DISK, CLIENT_VERSION);
		dsPos >> cDiskTxPos;
	}

	return true;
}

bool CScriptDBViewCache::WriteTxIndex(const vector<pair<uint256, CDiskTxPos> > &list,
		vector<CScriptDBOperLog> &vTxIndexOperDB) {
	for (vector<pair<uint256, CDiskTxPos> >::const_iterator it = list.begin(); it != list.end(); it++) {
		LogPrint("txindex", "txhash:%s dispos: nFile=%d, nPos=%d nTxOffset=%d\n", it->first.GetHex(), it->second.nFile,
				it->second.nPos, it->second.nTxOffset);
		CDataStream cDS(SER_DISK, CLIENT_VERSION);
		cDS << it->first;
		vector<unsigned char> vchTxHash = { 'T' };
		vchTxHash.insert(vchTxHash.end(), cDS.begin(), cDS.end());
		vector<unsigned char> vchTxPos;
		CDataStream cDSPos(SER_DISK, CLIENT_VERSION);
		cDSPos << it->second;
		vchTxPos.insert(vchTxPos.end(), cDSPos.begin(), cDSPos.end());
		CScriptDBOperLog cTxIndexOper;
		cTxIndexOper.vKey = vchTxHash;
		GetData(vchTxHash, cTxIndexOper.vValue);
		vTxIndexOperDB.push_back(cTxIndexOper);
		if (!SetData(vchTxHash, vchTxPos)) {
			return false;
		}
	}

	return true;
}


bool CScriptDBViewCache::GetScript(const vector<unsigned char> &vchScriptId, vector<unsigned char> &vchValue) {
	vector<unsigned char> vchScriptKey = { 'd', 'e', 'f' };
	vchScriptKey.insert(vchScriptKey.end(), vchScriptId.begin(), vchScriptId.end());

	return GetData(vchScriptKey, vchValue);
}

bool CScriptDBViewCache::GetScript(const CRegID &cRegId, vector<unsigned char> &vchValue) {
	return GetScript(cRegId.GetVec6(), vchValue);
}

bool CScriptDBViewCache::GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vchScriptId,
		const vector<unsigned char> &vchScriptKey, vector<unsigned char> &vchScriptData) {
//	assert(vScriptKey.size() == 8);
	vector<unsigned char> vchKey = { 'd', 'a', 't', 'a' };

	vchKey.insert(vchKey.end(), vchScriptId.begin(), vchScriptId.end());
	vchKey.push_back('_');
	vchKey.insert(vchKey.end(), vchScriptKey.begin(), vchScriptKey.end());
	vector<unsigned char> vchValue;
	if (!GetData(vchKey, vchValue)) {
		return false;
	}
	if (vchValue.empty()) {
		return false;
	}
	vchScriptData = vchValue;
//	CDataStream ds(vValue, SER_DISK, CLIENT_VERSION);
//	ds >> vScriptData;

	return true;
}

bool CScriptDBViewCache::GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vchScriptId,
		const int &nIndex, vector<unsigned char> &vchScriptKey, vector<unsigned char> &vchScriptData) {
	if (0 == nIndex) {
		vector<unsigned char> vchKey = { 'd', 'a', 't', 'a' };
		vchKey.insert(vchKey.end(), vchScriptId.begin(), vchScriptId.end());
		vchKey.push_back('_');
		vector<unsigned char> vchDataKey;
		vector<unsigned char> vDataValue;
		vchDataKey.clear();
		vDataValue.clear();
		map<vector<unsigned char>, vector<unsigned char> >::iterator iterFindKey = m_mapDatas.upper_bound(vchKey);
		while (iterFindKey != m_mapDatas.end()) {
			vector<unsigned char> vKeyTemp(vchKey.begin(), vchKey.begin() + vchScriptId.size() + 5);
			vector<unsigned char> vTemp(iterFindKey->first.begin(),
					iterFindKey->first.begin() + vchScriptId.size() + 5);
			if (vKeyTemp == vTemp) {
				if (iterFindKey->second.empty()) {
					++iterFindKey;
					continue;
				} else {
					vchDataKey = iterFindKey->first;
					vDataValue = iterFindKey->second;
					break;
				}
			} else {
				break;
			}
		}
		bool bUpLevelRet(false);
		int nIndexTemp = nIndex;
		while ((bUpLevelRet = m_pBase->GetScriptData(nCurBlockHeight, vchScriptId, nIndexTemp, vchScriptKey,
				vchScriptData))) {
//			LogPrint("INFO", "nCurBlockHeight:%d this addr:0x%x, nIndex:%d, count:%lld\n ScriptKey:%s\n nHeight:%d\n ScriptData:%s\n vDataKey:%s\n vDataValue:%s\n",
//					nCurBlockHeight, this, nIndexTemp, ++llCount, HexStr(vScriptKey), nHeight, HexStr(vScriptData), HexStr(vDataKey), HexStr(vDataValue));
			nIndexTemp = 1;
			vector<unsigned char> dataKeyTemp(vchKey.begin(), vchKey.end());
			dataKeyTemp.insert(dataKeyTemp.end(), vchScriptKey.begin(), vchScriptKey.end());
//			LogPrint("INFO", "dataKeyTemp:%s\n vDataKey:%s\n", HexStr(dataKeyTemp), HexStr(vDataKey));
//			if(mapDatas.count(dataKeyTemp) > 0) {//本级缓存包含上级查询结果的key
//				if(dataKeyTemp != vDataKey) {  //本级和上级查找key不同，说明上级获取的数据在本级已被删除
//					continue;
//				} else {
//					if(vDataValue.empty()) { //本级和上级数据key相同,且本级数据已经删除，重新从上级获取下一条数据
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
//			} else {//本级缓存不包含上级查询结果key
//				if (dataKeyTemp < vDataKey) { //上级获取key值小
//					return true;      //返回上级的查询结果
//				} else {              //本级查询结果Key值小，返回本级查询结果
//					vScriptKey.clear();
//					vScriptData.clear();
//					vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
//					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//					ds >> nHeight;
//					ds >> vScriptData;
//					return true;
//				}
//			}
			if (vchDataKey.empty()) {   //缓存中没有符合条件的key，直接返回上级的查询结果
				if (m_mapDatas.count(dataKeyTemp) <= 0) {
//					CDataStream ds(vScriptData, SER_DISK, CLIENT_VERSION);
//					ds >> vScriptData;
					return true;
				} else {
//					LogPrint("INFO", "local level contains dataKeyTemp,but the value is empty,need redo getscriptdata()\n");
					continue;//重新从数据库中获取下一条数据
				}
			} else {
				if (dataKeyTemp < vchDataKey) {
					if (m_mapDatas.count(dataKeyTemp) <= 0) {
						return true;
					} else {
//						LogPrint("INFO", "dataKeyTemp less than vDataKey and vDataValue empty redo getscriptdata()\n");
						continue;//重新从数据库中获取下一条数据
					}
				} else {
					if (vDataValue.empty()) { //本级和上级数据key相同,且本级数据已经删除，重新从上级获取下一条数据
//						LogPrint("INFO", "dataKeyTemp equal vDataKey and vDataValue empty redo getscriptdata()\n");
						continue;
					}
					vchScriptKey.clear();
					vchScriptData.clear();
					vchScriptKey.insert(vchScriptKey.end(), vchDataKey.begin() + 11, vchDataKey.end());
					vchScriptData = vDataValue;
//					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//					ds >> vScriptData;
					return true;
				}
			}
		}
		if (!bUpLevelRet) {
			if (vchDataKey.empty())
				return false;
			else {
				vchScriptKey.clear();
				vchScriptData.clear();
				vchScriptKey.insert(vchScriptKey.end(), vchDataKey.begin() + 11, vchDataKey.end());
				if (vDataValue.empty()) {
					return false;
				}
				vchScriptData = vDataValue;
//				CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//				ds >> vScriptData;
				return true;
			}
		}
	} else if (1 == nIndex) {
		vector<unsigned char> vKey = { 'd', 'a', 't', 'a' };
		vKey.insert(vKey.end(), vchScriptId.begin(), vchScriptId.end());
		vKey.push_back('_');
		vector<unsigned char> vPreKey(vKey);
		vPreKey.insert(vPreKey.end(), vchScriptKey.begin(), vchScriptKey.end());
		map<vector<unsigned char>, vector<unsigned char> >::iterator iterFindKey = m_mapDatas.upper_bound(vPreKey);
		vector<unsigned char> vDataKey;
		vector<unsigned char> vDataValue;
		vDataValue.clear();
		vDataKey.clear();
		while (iterFindKey != m_mapDatas.end()) {
			vector<unsigned char> vKeyTemp(vKey.begin(), vKey.begin() + vchScriptId.size() + 5);
			vector<unsigned char> vTemp(iterFindKey->first.begin(),
					iterFindKey->first.begin() + vchScriptId.size() + 5);
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
		while ((bUpLevelRet = m_pBase->GetScriptData(nCurBlockHeight, vchScriptId, nIndex, vchScriptKey, vchScriptData))) {
//			LogPrint("INFO", "nCurBlockHeight:%d this addr:0x%x, nIndex:%d, count:%lld\n ScriptKey:%s\n nHeight:%d\n ScriptData:%s\n vDataKey:%s\n vDataValue:%s\n",
//					nCurBlockHeight, this, nIndex, ++llCount, HexStr(vScriptKey), nHeight, HexStr(vScriptData), HexStr(vDataKey), HexStr(vDataValue));
			vector<unsigned char> dataKeyTemp(vKey.begin(), vKey.end());
			dataKeyTemp.insert(dataKeyTemp.end(), vchScriptKey.begin(), vchScriptKey.end());
//			LogPrint("INFO", "dataKeyTemp:%s\n vDataKey:%s\n", HexStr(dataKeyTemp), HexStr(vDataKey));
//			if(mapDatas.count(dataKeyTemp) > 0) {//本级缓存包含上级查询结果的key
//				if(dataKeyTemp != vDataKey) {  //本级和上级查找key不同，说明上级获取的数据在本级已被删除
//					continue;
//				} else {
//					if(vDataValue.empty()) { //本级和上级数据key相同,且本级数据已经删除，重新从上级获取下一条数据
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
//			} else {//本级缓存不包含上级查询结果key
//				if (dataKeyTemp < vDataKey) { //上级获取key值小
//					return true;      //返回上级的查询结果
//				} else {              //本级查询结果Key值小，返回本级查询结果
//					vScriptKey.clear();
//					vScriptData.clear();
//					vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
//					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//					ds >> nHeight;
//					ds >> vScriptData;
//					return true;
//				}
//			}
			if (vDataKey.empty()) {   //缓存中没有符合条件的key，直接返回上级的查询结果
				if (m_mapDatas.count(dataKeyTemp) <= 0) {
//					CDataStream ds(vScriptData, SER_DISK, CLIENT_VERSION);
//					ds >> vScriptData;
					return true;
				} else {
//					LogPrint("INFO", "local level contains dataKeyTemp,but the value is empty,need redo getscriptdata()\n");
					continue;//重新从数据库中获取下一条数据
				}
			} else {
				if (dataKeyTemp < vDataKey) {
					if (m_mapDatas.count(dataKeyTemp) == 0)
						return true;
					else {
//						LogPrint("INFO", "dataKeyTemp less than vDataKey and vDataValue empty redo getscriptdata()\n");
						continue;//在缓存中dataKeyTemp已经被删除过了，重新从数据库中获取下一条数据
					}
				} else {
					if (vDataValue.empty()) { //本级和上级数据key相同,且本级数据已经删除，重新从上级获取下一条数据
//						LogPrint("INFO", "dataKeyTemp equal vDataKey and vDataValue empty redo getscriptdata()\n");
						continue;
					}
					vchScriptKey.clear();
					vchScriptData.clear();
					vchScriptKey.insert(vchScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
					vchScriptData = vDataValue;
//					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//					ds >> vScriptData;
					return true;
				}
			}
		}
		if (!bUpLevelRet) {
			if (vDataKey.empty())
				return false;
			else {
				vchScriptKey.clear();
				vchScriptData.clear();
				vchScriptKey.insert(vchScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
				if (vDataValue.empty()) {
					return false;
				}
				vchScriptData = vDataValue;
//				CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//				ds >> vScriptData;
				return true;
			}
		}
	} else {
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
//		for (auto &item : mapDatas) {   //遍历本级缓存数据，找出合法的最小的key值
//			vector<unsigned char> vTemp(item.first.begin(),item.first.begin()+vScriptId.size()+5);
//			if(vKey == vTemp) {
//				if(item.second.empty()) {
//					continue;
//				}
//				vDataKey = item.first;
//				vDataValue = item.second;
//				CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//				ds >> nHeight;
//				if(nHeight <= nCurBlockHeight) { //若找到的key对应的数据保存时间已经超时，则需要删除该数据项，继续找下一个符合条件的key
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
//					if (nHeight <= nCurBlockHeight) { //若找到的key对应的数据保存时间已经超时，则需要删除该数据项，继续找下一个符合条件的key
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
//		for (; iterOperLog != setOperLog.end();) { //防止由于没有flush cache，对数据库中超时的脚本数据项，在cache中多次删除，引起删除失败
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
//		if(mapDatas.count(dataKeyTemp) > 0) {//本级缓存包含上级查询结果的key
//			if(dataKeyTemp != vDataKey) {  //本级和上级查找key不同，说明上级获取的数据在本级已被删除
//				LogPrint("INFO", "dataKeyTemp equal vDataKey and vDataValue empty redo getscriptdata()\n");
//				continue;
//			} else {
//				if(vDataValue.empty()) { //本级和上级数据key相同,且本级数据已经删除，重新从上级获取下一条数据
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
//		} else {//本级缓存不包含上级查询结果key
//			if (dataKeyTemp < vDataKey) { //上级获取key值小
//				return true;      //返回上级的查询结果
//			} else {              //本级查询结果Key值小，返回本级查询结果
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
//			for (; iterOperLog != setOperLog.end();) { //防止由于没有flush cache，对数据库中超时的脚本数据项，在cache中多次删除，引起删除失败
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
bool CScriptDBViewCache::SetScriptData(const vector<unsigned char> &vchScriptId,
		const vector<unsigned char> &vchScriptKey, const vector<unsigned char> &vchScriptData,
		CScriptDBOperLog &cScriptDBOperLog) {
	vector<unsigned char> vchKey = { 'd', 'a', 't', 'a' };
	vchKey.insert(vchKey.end(), vchScriptId.begin(), vchScriptId.end());
	vchKey.push_back('_');
	vchKey.insert(vchKey.end(), vchScriptKey.begin(), vchScriptKey.end());
	//  LogPrint("vm","add data:%s",HexStr(vScriptKey).c_str());
//	CDataStream ds(SER_DISK, CLIENT_VERSION);
//	ds << vScriptData;
	vector<unsigned char> vchValue(vchScriptData.begin(), vchScriptData.end());
	if (!HaveScriptData(vchScriptId, vchScriptKey)) {
		int nCount(0);
		GetScriptDataCount(vchScriptId, nCount);
		++nCount;
		if (!SetScriptDataCount(vchScriptId, nCount))
			return false;
	}
	vector<unsigned char> vchOldValue;
	vchOldValue.clear();
	GetData(vchKey, vchOldValue);
	cScriptDBOperLog = CScriptDBOperLog(vchKey, vchOldValue);

	return SetData(vchKey, vchValue);
}

bool CScriptDBViewCache::HaveScript(const vector<unsigned char> &vchScriptId) {
	vector<unsigned char> vchScriptKey = { 'd', 'e', 'f' };
	vchScriptKey.insert(vchScriptKey.end(), vchScriptId.begin(), vchScriptId.end());

	return HaveData(vchScriptKey);
}

bool CScriptDBViewCache::GetScriptCount(int &nCount) {
	vector<unsigned char> vchScriptKey = { 's', 'n', 'u', 'm' };
	vector<unsigned char> vchValue;
	if (!GetData(vchScriptKey, vchValue)) {
		return false;
	}
	CDataStream cDS(vchValue, SER_DISK, CLIENT_VERSION);
	cDS >> nCount;

	return true;
}

bool CScriptDBViewCache::SetScriptCount(const int nCount) {
	vector<unsigned char> vchScriptKey = { 's', 'n', 'u', 'm' };
	vector<unsigned char> vchValue;
	vchValue.clear();
	if (nCount > 0) {
		CDataStream cDS(SER_DISK, CLIENT_VERSION);
		cDS << nCount;
		vchValue.insert(vchValue.end(), cDS.begin(), cDS.end());
	} else if (nCount < 0) {
		return false;
	}
	if (!SetData(vchScriptKey, vchValue)) {
		return false;
	}

	return true;
}

bool CScriptDBViewCache::EraseScript(const vector<unsigned char> &vchScriptId) {
	vector<unsigned char> vchScriptKey = { 'd', 'e', 'f' };
	vchScriptKey.insert(vchScriptKey.end(), vchScriptId.begin(), vchScriptId.end());
	if (HaveScript(vchScriptId)) {
		int nCount(0);
		if (!GetScriptCount(nCount)) {
			return false;
		}
		if (!SetScriptCount(--nCount)) {
			return false;
		}
	}

	return EraseKey(vchScriptKey);
}

bool CScriptDBViewCache::GetScriptDataCount(const vector<unsigned char> &vchScriptId, int &nCount) {
	vector<unsigned char> vchScriptKey = { 's', 'd', 'n', 'u', 'm' };
	vchScriptKey.insert(vchScriptKey.end(), vchScriptId.begin(), vchScriptId.end());
	vector<unsigned char> vchValue;
	if (!GetData(vchScriptKey, vchValue)) {
		return false;
	}
	CDataStream cDS(vchValue, SER_DISK, CLIENT_VERSION);
	cDS >> nCount;

	return true;
}
bool CScriptDBViewCache::SetScriptDataCount(const vector<unsigned char> &vchScriptId, int nCount) {
	vector<unsigned char> vchScriptKey = { 's', 'd', 'n', 'u', 'm' };
	vchScriptKey.insert(vchScriptKey.end(), vchScriptId.begin(), vchScriptId.end());
	vector<unsigned char> vchValue;
	vchValue.clear();
	if (nCount > 0) {
		CDataStream cDS(SER_DISK, CLIENT_VERSION);
		cDS << nCount;
		vchValue.insert(vchValue.end(), cDS.begin(), cDS.end());
	} else if (nCount < 0) {
		return false;
	}
	if (!SetData(vchScriptKey, vchValue)) {
		return false;
	}

	return true;
}

bool CScriptDBViewCache::EraseScriptData(const vector<unsigned char> &vchScriptId,
		const vector<unsigned char> &vchScriptKey, CScriptDBOperLog &cScriptDBOperLog) {
	vector<unsigned char> vchLocalScriptKey = { 'd', 'a', 't', 'a' };
	vchLocalScriptKey.insert(vchLocalScriptKey.end(), vchScriptId.begin(), vchScriptId.end());
	vchLocalScriptKey.push_back('_');
	vchLocalScriptKey.insert(vchLocalScriptKey.end(), vchScriptKey.begin(), vchScriptKey.end());

	if (HaveScriptData(vchScriptId, vchScriptKey)) {
		int nCount(0);
		if (!GetScriptDataCount(vchScriptId, nCount)) {
			return false;
		}
		if (!SetScriptDataCount(vchScriptId, --nCount)) {
			return false;
		}
	}

	vector<unsigned char> vchValue;
	if (!GetData(vchLocalScriptKey, vchValue)) {
		return false;
	}
	cScriptDBOperLog = CScriptDBOperLog(vchLocalScriptKey, vchValue);
	if (!EraseKey(vchLocalScriptKey)) {
		return false;
	}

	return true;
}

bool CScriptDBViewCache::EraseScriptData(const vector<unsigned char> &vchKey) {
	if (vchKey.size() < 12) {
		return ERRORMSG("EraseScriptData delete script data key value error!");
	}
	vector<unsigned char> vchScriptId(vchKey.begin() + 4, vchKey.begin() + 10);
	vector<unsigned char> vchScriptKey(vchKey.begin() + 11, vchKey.end());
	CScriptDBOperLog cScriptDBOperLog;

	return EraseScriptData(vchScriptId, vchScriptKey, cScriptDBOperLog);
}

bool CScriptDBViewCache::HaveScriptData(const vector<unsigned char> &vchScriptId,
		const vector<unsigned char> &vchScriptKey) {
	vector<unsigned char> vchLocalScriptKey = { 'd', 'a', 't', 'a' };
	vchLocalScriptKey.insert(vchLocalScriptKey.end(), vchScriptId.begin(), vchScriptId.end());
	vchLocalScriptKey.push_back('_');
	vchLocalScriptKey.insert(vchLocalScriptKey.end(), vchScriptKey.begin(), vchScriptKey.end());

	return HaveData(vchLocalScriptKey);
}

bool CScriptDBViewCache::GetScript(const int nIndex, CRegID &cRegId, vector<unsigned char> &vchValue) {
	vector<unsigned char> vchTemp;
	if (nIndex != 0) {
		vchTemp = cRegId.GetVec6();
	}
	if (GetScript(nIndex, vchTemp, vchValue)) {
		cRegId.SetRegID(vchTemp);
		return true;
	}

	return false;
}

bool CScriptDBViewCache::SetScript(const CRegID &cRegId, const vector<unsigned char> &vchValue) {
	return SetScript(cRegId.GetVec6(), vchValue);
}

bool CScriptDBViewCache::HaveScript(const CRegID &cRegId) {
	return HaveScript(cRegId.GetVec6());
}

bool CScriptDBViewCache::EraseScript(const CRegID &cRegId) {
	return EraseScript(cRegId.GetVec6());
}

bool CScriptDBViewCache::GetScriptDataCount(const CRegID &cRegId, int &nCount) {
	return GetScriptDataCount(cRegId.GetVec6(), nCount);
}

bool CScriptDBViewCache::EraseScriptData(const CRegID &cRegId, const vector<unsigned char> &vchScriptKey,
		CScriptDBOperLog &cScriptDBOperLog) {
	return EraseScriptData(cRegId.GetVec6(), vchScriptKey, cScriptDBOperLog);
}

bool CScriptDBViewCache::HaveScriptData(const CRegID &cRegId, const vector<unsigned char> &vchScriptKey) {
	return HaveScriptData(cRegId.GetVec6(), vchScriptKey);
}
bool CScriptDBViewCache::GetScriptData(const int nCurBlockHeight, const CRegID &cRegId,
		const vector<unsigned char> &vchScriptKey, vector<unsigned char> &vchScriptData) {
	return GetScriptData(nCurBlockHeight, cRegId.GetVec6(), vchScriptKey, vchScriptData);
}

bool CScriptDBViewCache::GetScriptData(const int nCurBlockHeight, const CRegID &cRegId, const int &nIndex,
		vector<unsigned char> &vchScriptKey, vector<unsigned char> &vchScriptData) {
	return GetScriptData(nCurBlockHeight, cRegId.GetVec6(), nIndex, vchScriptKey, vchScriptData);
}

bool CScriptDBViewCache::SetScriptData(const CRegID &cRegID, const vector<unsigned char> &vchScriptKey,
		const vector<unsigned char> &vchScriptData, CScriptDBOperLog &cScriptDBOperLog) {
	return SetScriptData(cRegID.GetVec6(), vchScriptKey, vchScriptData, cScriptDBOperLog);
}

bool CScriptDBViewCache::SetTxRelAccout(const uint256 &cTxHash, const set<CKeyID> &RelAccount) {
	vector<unsigned char> vchKey = { 't', 'x' };
	vector<unsigned char> vchValue;
	CDataStream cDS(SER_DISK, CLIENT_VERSION);
	cDS << cTxHash;
	vchKey.insert(vchKey.end(), cDS.begin(), cDS.end());
	cDS.clear();
	cDS << RelAccount;
	vchValue.assign(cDS.begin(), cDS.end());

	return SetData(vchKey, vchValue);
}

bool CScriptDBViewCache::GetTxRelAccount(const uint256 &cTxHash, set<CKeyID> &RelAccount) {
	vector<unsigned char> vchKey = { 't', 'x' };
	vector<unsigned char> vchValue;
	CDataStream cDS(SER_DISK, CLIENT_VERSION);
	cDS << cTxHash;
	vchKey.insert(vchKey.end(), cDS.begin(), cDS.end());
	if (!GetData(vchKey, vchValue)) {
		return false;
	}
	cDS.clear();
	vector<char> vchTemp;
	vchTemp.assign(vchValue.begin(), vchValue.end());
	cDS.insert(cDS.end(), vchTemp.begin(), vchTemp.end());
	cDS >> RelAccount;

	return true;
}

bool CScriptDBViewCache::EraseTxRelAccout(const uint256 &cTxHash) {
	vector<unsigned char> vchKey = { 't', 'x' };
	vector<unsigned char> vchValue;
	vchValue.clear();
	CDataStream cDS(SER_DISK, CLIENT_VERSION);
	cDS << cTxHash;
	vchKey.insert(vchKey.end(), cDS.begin(), cDS.end());
	SetData(vchKey, vchValue);

	return true;
}

Object CScriptDBViewCache::ToJosnObj() const {
	Object obj;
	Array arrayObj;
	arrayObj.push_back(m_pBase->ToJosnObj("def"));
	arrayObj.push_back(m_pBase->ToJosnObj("data"));
	arrayObj.push_back(m_pBase->ToJosnObj("author"));
	obj.push_back(Pair("mapDatas", arrayObj));

	return obj;
}

void CScriptDBViewCache::SetBaseData(CScriptDBView *pNewBase) {
	m_pBase = pNewBase;
}

string CScriptDBViewCache::ToString() {
	string str("");
	vector<unsigned char> vchPrefix = { 'd', 'a', 't', 'a' };
	for (auto & item : m_mapDatas) {
		vector<unsigned char> vchTemp(item.first.begin(), item.first.begin() + 4);
		if (vchTemp == vchPrefix) {
			str = strprintf("vKey=%s\n vData=%s\n", HexStr(item.first), HexStr(item.second));
		}
	}
	
	return str;
}

uint256 CTransactionDBView::IsContainTx(const uint256 & cTxHash) {
	return std::move(uint256());
}

bool CTransactionDBView::IsContainBlock(const CBlock &cBlock) {
	return false;
}

bool CTransactionDBView::AddBlockToCache(const CBlock &cBlock) {
	return false;
}

bool CTransactionDBView::DeleteBlockFromCache(const CBlock &cBlock) {
	return false;
}

bool CTransactionDBView::LoadTransaction(map<uint256, vector<uint256> > &mapTxHashByBlockHash) {
	return false;
}

bool CTransactionDBView::BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHash) {
	return false;
}

CTransactionDBViewBacked::CTransactionDBViewBacked(CTransactionDBView &cTransactionView) {
	m_pBase = &cTransactionView;
}

uint256 CTransactionDBViewBacked::IsContainTx(const uint256 & cTxHash) {
	return std::move(m_pBase->IsContainTx(cTxHash));
}

bool CTransactionDBViewBacked::IsContainBlock(const CBlock &cBlock) {
	return m_pBase->IsContainBlock(cBlock);
}

bool CTransactionDBViewBacked::AddBlockToCache(const CBlock &cBlock) {
	return m_pBase->AddBlockToCache(cBlock);
}

bool CTransactionDBViewBacked::DeleteBlockFromCache(const CBlock &cBlock) {
	return m_pBase->DeleteBlockFromCache(cBlock);
}

bool CTransactionDBViewBacked::LoadTransaction(map<uint256, vector<uint256> > &mapTxHashByBlockHash) {
	return m_pBase->LoadTransaction(mapTxHashByBlockHash);
}

bool CTransactionDBViewBacked::BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHashIn) {
	return m_pBase->BatchWrite(mapTxHashByBlockHashIn);
}

CTransactionDBCache::CTransactionDBCache(CTransactionDBView &cTxCacheDBView, bool bDummy) :
		CTransactionDBViewBacked(cTxCacheDBView) {

}

bool CTransactionDBCache::IsContainBlock(const CBlock &cBlock) {
	//(mapTxHashByBlockHash.count(block.GetHash()) > 0 && mapTxHashByBlockHash[block.GetHash()].size() > 0)
	return (IsInMap(mapTxHashByBlockHash, cBlock.GetHash()) || m_pBase->IsContainBlock(cBlock));
}

bool CTransactionDBCache::AddBlockToCache(const CBlock &cBlock) {
	vector<uint256> vchTxHash;
	vchTxHash.clear();
	for (auto &ptx : cBlock.vptx) {
		vchTxHash.push_back(ptx->GetHash());
	}
	mapTxHashByBlockHash[cBlock.GetHash()] = vchTxHash;
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

bool CTransactionDBCache::DeleteBlockFromCache(const CBlock &cBlock) {
//	LogPrint("txcache", "CTransactionDBCache::DeleteBlockFromCache() height=%d blockhash=%s \n", block.nHeight, block.GetHash().GetHex());
	if (IsContainBlock (cBlock)) {
		vector<uint256> vcTxHash;
		vcTxHash.clear();
		mapTxHashByBlockHash[cBlock.GetHash()] = vcTxHash;
		return true;
	} else {
		LogPrint("ERROR", "the block hash:%s isn't in TxCache\n", cBlock.GetHash().GetHex());
		return false;
	}

	return true;
}

uint256 CTransactionDBCache::IsContainTx(const uint256 & cTxHash) {
	for (auto & item : mapTxHashByBlockHash) {
		vector<uint256>::iterator it = find(item.second.begin(), item.second.end(), cTxHash);
		if (it != item.second.end()) {
			return item.first;
		}
	}
	uint256 cBlockHash = m_pBase->IsContainTx(cTxHash);
	if (IsInMap(mapTxHashByBlockHash, cBlockHash)) { //mapTxHashByBlockHash[blockHash].empty()) { // [] 运算符防止不小心加入了垃圾数据
		return std::move(cBlockHash);
	}

	return std::move(uint256());
}

map<uint256, vector<uint256> > CTransactionDBCache::GetTxHashCache(void) {
	return mapTxHashByBlockHash;
}

bool CTransactionDBCache::BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHashIn) {
	for (auto & item : mapTxHashByBlockHashIn) {
		mapTxHashByBlockHash[item.first] = item.second;
	}

	return true;
}

bool CTransactionDBCache::Flush() {
	bool bRet = m_pBase->BatchWrite(mapTxHashByBlockHash);
	if (bRet) {
		map<uint256, vector<uint256> >::iterator iter = mapTxHashByBlockHash.begin();
		for (; iter != mapTxHashByBlockHash.end();) {
			if (iter->second.empty()) {
				mapTxHashByBlockHash.erase(iter++);
			} else {
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
	return m_pBase->LoadTransaction(mapTxHashByBlockHash);
}

void CTransactionDBCache::Clear() {
	mapTxHashByBlockHash.clear();
}

int CTransactionDBCache::GetSize() {
	int iCount(0);
	for (auto & i : mapTxHashByBlockHash) {
		if (!i.second.empty()) {
			++iCount;
		}
	}
	return iCount;
}

bool CTransactionDBCache::IsInMap(const map<uint256, vector<uint256> >& mMap, const uint256 &hash) const {
	if (hash == uint256()) {
		return false;
	}
	auto te = mMap.find(hash);
	if (te != mMap.end()) {
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
		if (item.second.size() > 0) {
			InobjArray.push_back(std::move(obj));
		} else {
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

void CTransactionDBCache::SetBaseData(CTransactionDBView *pNewBase) {
	m_pBase = pNewBase;
}


const map<uint256, vector<uint256> > & CTransactionDBCache::GetCacheMap() {
	return mapTxHashByBlockHash;
}

void CTransactionDBCache::SetCacheMap(const map<uint256, vector<uint256> > &mapCache) {
	mapTxHashByBlockHash = mapCache;
}

bool CScriptDBViewCache::GetScriptAcc(const CRegID &cRegId, const vector<unsigned char> &vchAccKey,
		CAppUserAccout& cAppUserAccOut) {
	vector<unsigned char> vchScriptKey = { 'a', 'c', 'c', 't' };
	vector<unsigned char> vchRegId = cRegId.GetVec6();
	vchScriptKey.insert(vchScriptKey.end(), vchRegId.begin(), vchRegId.end());
	vchScriptKey.push_back('_');
	vchScriptKey.insert(vchScriptKey.end(), vchAccKey.begin(), vchAccKey.end());
	vector<unsigned char> vchValue;

	if (!GetData(vchScriptKey, vchValue)) {
		return false;
	}
	CDataStream cDS(vchValue, SER_DISK, CLIENT_VERSION);
	cDS >> cAppUserAccOut;

	return true;
}

bool CScriptDBViewCache::SetScriptAcc(const CRegID &cRegId, const CAppUserAccout& cAppUserAccIn,
		CScriptDBOperLog &cScriptDBOperLog) {
	vector<unsigned char> vchScriptKey = { 'a', 'c', 'c', 't' };
	vector<unsigned char> vchRegId = cRegId.GetVec6();
	vector<unsigned char> vchAccKey = cAppUserAccIn.getaccUserId();
	vchScriptKey.insert(vchScriptKey.end(), vchRegId.begin(), vchRegId.end());
	vchScriptKey.push_back('_');
	vchScriptKey.insert(vchScriptKey.end(), vchAccKey.begin(), vchAccKey.end());
	vector<unsigned char> vValue;
	cScriptDBOperLog.vKey = vchScriptKey;
	if (GetData(vchScriptKey, vValue)) {
		cScriptDBOperLog.vValue = vValue;
	}
	CDataStream cDS(SER_DISK, CLIENT_VERSION);
	cDS << cAppUserAccIn;
	vValue.clear();
	vValue.insert(vValue.end(), cDS.begin(), cDS.end());

	return SetData(vchScriptKey, vValue);
}
