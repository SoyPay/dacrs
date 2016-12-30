// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txdb.h"
#include "util.h"
#include "core.h"
#include "uint256.h"
#include "key.h"
#include <stdint.h>

using namespace std;


//void static BatchWriteHashBestChain(CLevelDBBatch &batch, const uint256 &hash) {
//	batch.Write('B', hash);
//}

CBlockTreeDB::CBlockTreeDB(size_t unCacheSize, bool bMemory, bool bWipe) :
		CLevelDBWrapper(GetDataDir() / "blocks" / "index", unCacheSize, bMemory, bWipe) {
}

bool CBlockTreeDB::WriteBlockIndex(const CDiskBlockIndex& cBlockindex) {
	return Write(make_pair('b', cBlockindex.GetBlockHash()), cBlockindex);
}

bool CBlockTreeDB::EraseBlockIndex(const uint256 &cBlockHash) {
	return Erase(make_pair('b', cBlockHash));
}

bool CBlockTreeDB::WriteBestInvalidWork(const uint256& cBestInvalidWork) {
	// Obsolete; only written for backward compatibility.
	return Write('I', cBestInvalidWork);
}

bool CBlockTreeDB::WriteBlockFileInfo(int nFile, const CBlockFileInfo &cFileinfo) {
	return Write(make_pair('f', nFile), cFileinfo);
}

bool CBlockTreeDB::ReadBlockFileInfo(int nFile, CBlockFileInfo &cFileinfo) {
	return Read(make_pair('f', nFile), cFileinfo);
}

bool CBlockTreeDB::WriteLastBlockFile(int nFile) {
	return Write('l', nFile);
}

bool CBlockTreeDB::WriteReindexing(bool bReindexing) {
	if (bReindexing) {
		return Write('R', '1');
	} else {
		return Erase('R');
	}
}

bool CBlockTreeDB::ReadReindexing(bool &bReindexing) {
	bReindexing = Exists('R');
	return true;
}

bool CBlockTreeDB::ReadLastBlockFile(int &nFile) {
	return Read('l', nFile);
}

//bool CBlockTreeDB::ReadTxIndex(const uint256 &txid, CDiskTxPos &pos) {
//	return Read(make_pair('t', txid), pos);
//}
//
//bool CBlockTreeDB::WriteTxIndex(const vector<pair<uint256, CDiskTxPos> >&vect) {
//	CLevelDBBatch batch;
//	for (vector<pair<uint256, CDiskTxPos> >::const_iterator it = vect.begin(); it != vect.end(); it++){
//		LogPrint("txindex", "txhash:%s dispos: nFile=%d, nPos=%d nTxOffset=%d\n", it->first.GetHex(), it->second.nFile, it->second.nPos, it->second.nTxOffset);
//		batch.Write(make_pair('t', it->first), it->second);
//	}
//	return WriteBatch(batch);
//}

bool CBlockTreeDB::WriteFlag(const string &strName, bool bValue) {
	return Write(make_pair('F', strName), bValue ? '1' : '0');
}

bool CBlockTreeDB::ReadFlag(const string &strName, bool &bValue) {
	char ch;
	if (!Read(make_pair('F', strName), ch)) {
		return false;
	}
	bValue = ch == '1';
	return true;
}

bool CBlockTreeDB::LoadBlockIndexGuts() {
	leveldb::Iterator *pcursor = NewIterator();

	CDataStream ssKeySet(SER_DISK, g_sClientVersion);
	ssKeySet << make_pair('b', uint256());
	pcursor->Seek(ssKeySet.str());

	// Load mapBlockIndex
	while (pcursor->Valid()) {
		boost::this_thread::interruption_point();
		try {
			leveldb::Slice cSliceKey = pcursor->key();
			CDataStream cDSKey(cSliceKey.data(), cSliceKey.data() + cSliceKey.size(), SER_DISK, g_sClientVersion);
			char chType;
			cDSKey >> chType;
			if (chType == 'b') {
				leveldb::Slice cSliceValue = pcursor->value();
				CDataStream cDSValue(cSliceValue.data(), cSliceValue.data() + cSliceValue.size(), SER_DISK, g_sClientVersion);
				CDiskBlockIndex cDiskBlockIndex;
				cDSValue >> cDiskBlockIndex;

				// Construct block index object
				CBlockIndex* pIndexNew 			= InsertBlockIndex(cDiskBlockIndex.GetBlockHash());
				pIndexNew->m_pPrevBlockIndex 				= InsertBlockIndex(cDiskBlockIndex.m_cHashPrev);
				pIndexNew->m_nHeight 			= cDiskBlockIndex.m_nHeight;
				pIndexNew->m_nFile 				= cDiskBlockIndex.m_nFile;
				pIndexNew->m_nDataPos 			= cDiskBlockIndex.m_nDataPos;
				pIndexNew->m_nUndoPos 			= cDiskBlockIndex.m_nUndoPos;
				pIndexNew->m_nVersion 			= cDiskBlockIndex.m_nVersion;
				pIndexNew->m_cHashMerkleRoot 	= cDiskBlockIndex.m_cHashMerkleRoot;
				pIndexNew->m_cHashPos 			= cDiskBlockIndex.m_cHashPos;
				pIndexNew->m_unTime 			= cDiskBlockIndex.m_unTime;
				pIndexNew->m_unBits 			= cDiskBlockIndex.m_unBits;
				pIndexNew->m_unNonce 			= cDiskBlockIndex.m_unNonce;
				pIndexNew->m_unStatus 			= cDiskBlockIndex.m_unStatus;
				pIndexNew->m_unTx 				= cDiskBlockIndex.m_unTx;
				pIndexNew->m_llFuel 			= cDiskBlockIndex.m_llFuel;
				pIndexNew->m_nFuelRate 			= cDiskBlockIndex.m_nFuelRate;
				pIndexNew->m_vchSignature 		= cDiskBlockIndex.m_vchSignature;
				pIndexNew->m_dFeePerKb 			= cDiskBlockIndex.m_dFeePerKb;

				if (!pIndexNew->CheckIndex()) {
					return ERRORMSG("LoadBlockIndex() : CheckIndex failed: %s", pIndexNew->ToString());
				}
				pcursor->Next();
			} else {
				break; 				// if shutdown requested or finished loading block index
			}
		} catch (std::exception &e) {
			return ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
		}
	}
	delete pcursor;

	return true;
}

CAccountViewDB::CAccountViewDB(size_t unCacheSize, bool bMemory, bool bWipe) :
		m_cLevelDBWrapper(GetDataDir() / "blocks" / "account", unCacheSize, bMemory, bWipe) {
}

CAccountViewDB::CAccountViewDB(const string& strName,size_t unCacheSize, bool bMemory, bool bWipe) :
		m_cLevelDBWrapper(GetDataDir() / "blocks" / strName, unCacheSize, bMemory, bWipe) {
}

bool CAccountViewDB::GetAccount(const CKeyID &cKeyId, CAccount &cSecureAccount) {
	return m_cLevelDBWrapper.Read(make_pair('k', cKeyId), cSecureAccount);
}

bool CAccountViewDB::SetAccount(const CKeyID &cKeyId, const CAccount &cSecureAccount) {
	return m_cLevelDBWrapper.Write(make_pair('k', cKeyId), cSecureAccount);
}

bool CAccountViewDB::SetAccount(const vector<unsigned char> &vchAccountId, const CAccount &cSecureAccount) {
	CKeyID cKeyId;
	if (m_cLevelDBWrapper.Read(make_pair('a', vchAccountId), cKeyId)) {
		return m_cLevelDBWrapper.Write(make_pair('k', cKeyId), cSecureAccount);
	} else {
		return false;
	}
}

bool CAccountViewDB::HaveAccount(const CKeyID &cKeyId) {
	return m_cLevelDBWrapper.Exists(cKeyId);
}

uint256 CAccountViewDB::GetBestBlock() {
	uint256 cHash;
	if (!m_cLevelDBWrapper.Read('B', cHash)) {
		return uint256();
	}
	return cHash;
}

bool CAccountViewDB::SetBestBlock(const uint256 &cHashBlock) {
	return m_cLevelDBWrapper.Write('B', cHashBlock);
}

bool CAccountViewDB::BatchWrite(const map<CKeyID, CAccount> &mapAccounts,
		const map<vector<unsigned char>, CKeyID> &mapKeyIds, const uint256 &cHashBlock) {
	CLevelDBBatch cLevelDBBatch;
	map<CKeyID, CAccount>::const_iterator iterAccount = mapAccounts.begin();
	for (; iterAccount != mapAccounts.end(); ++iterAccount) {
		if (iterAccount->second.m_cKeyID.IsNull()) {
			cLevelDBBatch.Erase(make_pair('k', iterAccount->first));
		} else {
			cLevelDBBatch.Write(make_pair('k', iterAccount->first), iterAccount->second);
		}
	}

	map<vector<unsigned char>, CKeyID>::const_iterator iterKey = mapKeyIds.begin();
	for (; iterKey != mapKeyIds.end(); ++iterKey) {
		if (iterKey->second.IsNull()) {
			cLevelDBBatch.Erase(make_pair('a', iterKey->first));
		} else {
			cLevelDBBatch.Write(make_pair('a', iterKey->first), iterKey->second);
		}
	}
	if (!cHashBlock.IsNull()) {
		cLevelDBBatch.Write('B', cHashBlock);
	}

	return m_cLevelDBWrapper.WriteBatch(cLevelDBBatch, true);
}

bool CAccountViewDB::BatchWrite(const vector<CAccount> &vcAccounts) {
	CLevelDBBatch cLevelDBBatch;
	vector<CAccount>::const_iterator iterAccount = vcAccounts.begin();
	for (; iterAccount != vcAccounts.end(); ++iterAccount) {
		cLevelDBBatch.Write(make_pair('k', iterAccount->m_cKeyID), *iterAccount);
	}
	return m_cLevelDBWrapper.WriteBatch(cLevelDBBatch, false);
}

bool CAccountViewDB::EraseAccount(const CKeyID &cKeyId) {
	return m_cLevelDBWrapper.Erase(make_pair('k', cKeyId));
}

bool CAccountViewDB::SetKeyId(const vector<unsigned char> &vchAccountId, const CKeyID &cKeyId) {
	return m_cLevelDBWrapper.Write(make_pair('a', vchAccountId), cKeyId);
}

bool CAccountViewDB::GetKeyId(const vector<unsigned char> &vchAccountId, CKeyID &cKeyId) {
	return m_cLevelDBWrapper.Read(make_pair('a', vchAccountId), cKeyId);
}

bool CAccountViewDB::EraseKeyId(const vector<unsigned char> &vchAccountId) {
	return m_cLevelDBWrapper.Erase(make_pair('a', vchAccountId));
}

bool CAccountViewDB::GetAccount(const vector<unsigned char> &vchAccountId, CAccount &cSecureAccount) {
	CKeyID cKeyId;
	if (m_cLevelDBWrapper.Read(make_pair('a', vchAccountId), cKeyId)) {
		return m_cLevelDBWrapper.Read(make_pair('k', cKeyId), cSecureAccount);
	}
	return false;
}

bool CAccountViewDB::SaveAccountInfo(const vector<unsigned char> &vchAccountId, const CKeyID &cKeyId, const CAccount &cSecureAccount) {
	CLevelDBBatch cLevelDBBatch;
	cLevelDBBatch.Write(make_pair('a', vchAccountId), cKeyId);
	cLevelDBBatch.Write(make_pair('k', cKeyId), cSecureAccount);
	return m_cLevelDBWrapper.WriteBatch(cLevelDBBatch, false);
}

uint64_t CAccountViewDB::TraverseAccount() {
	leveldb::Iterator *pCursor = m_cLevelDBWrapper.NewIterator();

	uint64_t ullTotalCoin(0);
	CDataStream cDSKeySet(SER_DISK, g_sClientVersion);
	cDSKeySet << make_pair('k', CKeyID());
	pCursor->Seek(cDSKeySet.str());

	// Load mapBlockIndex
	while (pCursor->Valid()) {
		boost::this_thread::interruption_point();
		try {
			leveldb::Slice cSliceKey = pCursor->key();
			CDataStream cDSKey(cSliceKey.data(), cSliceKey.data() + cSliceKey.size(), SER_DISK, g_sClientVersion);
			char chType;
			cDSKey >> chType;
			if (chType == 'k') {
				leveldb::Slice cSliceValue = pCursor->value();
				CDataStream cDSValue(cSliceValue.data(), cSliceValue.data() + cSliceValue.size(), SER_DISK, g_sClientVersion);
				CAccount cAccount;
				cDSValue >> cAccount;
				ullTotalCoin += cAccount.m_ullValues;
				pCursor->Next();
			} else {
				break; 				// if shutdown requested or finished loading block index
			}
		} catch (std::exception &e) {
			return ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
		}
	}
	delete pCursor;
	return ullTotalCoin;
}

CTransactionDB::CTransactionDB(size_t unCacheSize, bool bMemory, bool bWipe) :
		m_LevelDBWrapper(GetDataDir() / "blocks" / "txcache", unCacheSize, bMemory, bWipe) {
}

bool CTransactionDB::SetTxCache(const uint256 &cBlockHash, const vector<uint256> &vcHashTxSet) {
	return m_LevelDBWrapper.Write(make_pair('h', cBlockHash), vcHashTxSet);
}

bool CTransactionDB::GetTxCache(const uint256 &blockHash, vector<uint256> &vHashTx) {
	return m_LevelDBWrapper.Read(make_pair('h', blockHash), vHashTx);
}

bool CTransactionDB::BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHash) {
	CLevelDBBatch cLevelDBBatch;
	for (auto & item : mapTxHashByBlockHash) {
		if(item.second.empty()) {
			cLevelDBBatch.Erase(make_pair('h', item.first));
		} else {
			if(!m_LevelDBWrapper.Exists(make_pair('h', item.first))) {
				cLevelDBBatch.Write(make_pair('h', item.first), item.second);
			}
		}
	}
	return m_LevelDBWrapper.WriteBatch(cLevelDBBatch, true);
}

bool CTransactionDB::LoadTransaction(map<uint256, vector<uint256> > &mapTxHashByBlockHash) {
	leveldb::Iterator *pCursor = m_LevelDBWrapper.NewIterator();
	CDataStream cDSKeySet(SER_DISK, g_sClientVersion);
	cDSKeySet << make_pair('h', uint256());
	pCursor->Seek(cDSKeySet.str());
	// Load mapBlockIndex
	while (pCursor->Valid()) {
		boost::this_thread::interruption_point();
		try {
			leveldb::Slice cSliceKey = pCursor->key();
			CDataStream cDSKey(cSliceKey.data(), cSliceKey.data() + cSliceKey.size(), SER_DISK, g_sClientVersion);
			char chType;
			cDSKey >> chType;
			if (chType == 'h') {
				leveldb::Slice cSliceValue = pCursor->value();
				CDataStream cDSValue(cSliceValue.data(), cSliceValue.data() + cSliceValue.size(), SER_DISK, g_sClientVersion);
				vector<uint256> vcTxhash;
				uint256 cBlockHash;
				cDSValue >> vcTxhash;
				cDSKey >> cBlockHash;
				mapTxHashByBlockHash[cBlockHash] = vcTxhash;
				pCursor->Next();
			} else {
				break; // if shutdown requested or finished loading block index
			}
		} catch (std::exception &e) {
			return ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
		}
	}
	delete pCursor;
	return true;
}

CScriptDB::CScriptDB(const string& strName,size_t unCacheSize, bool bMemory, bool bWipe) :
		m_LevelDBWrapper(GetDataDir() / "blocks" / strName, unCacheSize, bMemory, bWipe){
}

CScriptDB::CScriptDB(size_t unCacheSize, bool bMemory, bool bWipe) :
		m_LevelDBWrapper(GetDataDir() / "blocks" / "script", unCacheSize, bMemory, bWipe){
}

bool CScriptDB::GetData(const vector<unsigned char> &vchKey, vector<unsigned char> &vchValue) {
	return m_LevelDBWrapper.Read(vchKey, vchValue);
}

bool CScriptDB::SetData(const vector<unsigned char> &vchKey, const vector<unsigned char> &vchValue) {
	return m_LevelDBWrapper.Write(vchKey, vchValue);
}

bool CScriptDB::BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas) {
	CLevelDBBatch cBatch;
	for (auto & item : mapDatas) {
		if (item.second.empty()) {
			cBatch.Erase(item.first);
		} else {
			cBatch.Write(item.first, item.second);
		}
	}
	return m_LevelDBWrapper.WriteBatch(cBatch, true);
}

bool CScriptDB::EraseKey(const vector<unsigned char> &vchKey) {
	return m_LevelDBWrapper.Erase(vchKey);
}

bool CScriptDB::HaveData(const vector<unsigned char> &vchKey) {
	return m_LevelDBWrapper.Exists(vchKey);
}

bool CScriptDB::GetScript(const int &nIndex, vector<unsigned char> &vchScriptId, vector<unsigned char> &vchValue) {
	assert(nIndex >= 0 && nIndex <=1);
	leveldb::Iterator* pCursor = m_LevelDBWrapper.NewIterator();
	CDataStream cDSKeySet(SER_DISK, g_sClientVersion);
	string strTempPrefix("def");
	cDSKeySet.insert(cDSKeySet.end(), &strTempPrefix[0], &strTempPrefix[3]);

	int i(0);
	if(1 == nIndex ) {
		if(vchScriptId.empty()) {
			return ERRORMSG("GetScript() : nIndex is 1, and vScriptId is empty");
		}
		vector<char> vchId(vchScriptId.begin(), vchScriptId.end());
		cDSKeySet.insert(cDSKeySet.end(), vchId.begin(), vchId.end());
		vector<unsigned char> vchKey(cDSKeySet.begin(), cDSKeySet.end());
		if(HaveData(vchKey)) {   //判断传过来的key,数据库中是否已经存在
			pCursor->Seek(cDSKeySet.str());
			i = nIndex;
		} else {
			pCursor->Seek(cDSKeySet.str());
		}
	} else {
		pCursor->Seek(cDSKeySet.str());
	}
	while(pCursor->Valid() && i-->=0) {
		boost::this_thread::interruption_point();
		try {
			leveldb::Slice cSliceKey = pCursor->key();
			string strScriptKey(cSliceKey.data(), 0, cSliceKey.size());
//			ssKey >> strScriptKey;
			string strPrefix = strScriptKey.substr(0,3);
			if (strPrefix == "def") {
				if(-1 == i) {
					leveldb::Slice slValue = pCursor->value();
					CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, g_sClientVersion);
					ssValue >> vchValue;
					vchScriptId.clear();
					vchScriptId.insert(vchScriptId.end(), cSliceKey.data()+3, cSliceKey.data()+cSliceKey.size());
				}
				pCursor->Next();
			} else {
				delete pCursor;
				return false;
			}
		}catch (std::exception &e) {
			return ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
		}
	}
	delete pCursor;
	if(i >= 0) {
		return false;
	}

	return true;
}

bool CScriptDB::GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vchScriptId, const int &nIndex,
		vector<unsigned char> &vchScriptKey, vector<unsigned char> &vchScriptData) {
	const int nPrefixLen 	= 4;
	const int nScriptIdLen 	= 6;
	const int nSpaceLen 	= 1;
	assert(nIndex >= 0 && nIndex <=1);
	leveldb::Iterator* pCursor = m_LevelDBWrapper.NewIterator();
	CDataStream cDSKeySet(SER_DISK, g_sClientVersion);

	string strTempPrefix("data");
	cDSKeySet.insert(cDSKeySet.end(), &strTempPrefix[0], &strTempPrefix[4]);
	vector<char> vchId(vchScriptId.begin(), vchScriptId.end());
	cDSKeySet.insert(cDSKeySet.end(), vchId.begin(), vchId.end());
	cDSKeySet.insert(cDSKeySet.end(),'_');
	
	int i(0);
	if (1 == nIndex) {
		if(vchScriptKey.empty()) {
			return ERRORMSG("GetScriptData() : nIndex is 1, and vScriptKey is empty");
		}
		vector<char> vchKey(vchScriptKey.begin(), vchScriptKey.end());
		cDSKeySet.insert(cDSKeySet.end(), vchKey.begin(), vchKey.end());
		vector<unsigned char> vKey(cDSKeySet.begin(), cDSKeySet.end());
		if(HaveData(vKey)) {   					//判断传过来的key,数据库中是否已经存在
			pCursor->Seek(cDSKeySet.str());
			i = nIndex;
		} else {
			pCursor->Seek(cDSKeySet.str());
		}
	} else {
		pCursor->Seek(cDSKeySet.str());
	}

	while (pCursor->Valid() && i-- >= 0) {
		boost::this_thread::interruption_point();
		try {
			leveldb::Slice cSliceKey = pCursor->key();
			CDataStream cDSKey(cSliceKey.data(), cSliceKey.data() + cSliceKey.size(), SER_DISK, g_sClientVersion);
			if (0 == memcmp((char *)&cDSKey[0], (char *)&cDSKeySet[0], 11)) {
				if (-1 == i) {
					leveldb::Slice cSliceValue = pCursor->value();
					CDataStream cDSValue(cSliceValue.data(), cSliceValue.data() + cSliceValue.size(), SER_DISK, g_sClientVersion);
					cDSValue >> vchScriptData;
					vchScriptKey.clear();
					vchScriptKey.insert(vchScriptKey.end(), cSliceKey.data() + nPrefixLen + nScriptIdLen + nSpaceLen,
							cSliceKey.data() + cSliceKey.size());
				}
				pCursor->Next();
			} else {
				delete pCursor;
				return false;
			}
		} catch (std::exception &e) {
			return ERRORMSG("%s : Deserialize or I/O error - %s\n", __func__, e.what());
		}
	}
	delete pCursor;
	if(i >= 0) {
		return false;
	}

	return true;
}

bool CScriptDB::GetTxHashByAddress(const CKeyID &cKeyId, int nHeight,
		map<vector<unsigned char>, vector<unsigned char> > &mapTxHash) {
	leveldb::Iterator* pCursor = m_LevelDBWrapper.NewIterator();
	CDataStream cDSKeySet(SER_DISK, g_sClientVersion);

	string strTempPrefix("ADDR");
	cDSKeySet.insert(cDSKeySet.end(), &strTempPrefix[0], &strTempPrefix[4]);
	cDSKeySet << cKeyId;
	cDSKeySet << nHeight;
	pCursor->Seek(cDSKeySet.str());

	while (pCursor->Valid()) {
		boost::this_thread::interruption_point();
		try {
			leveldb::Slice cSliceKey = pCursor->key();
			leveldb::Slice cSliceValue = pCursor->value();
			CDataStream cDSKey(cSliceKey.data(), cSliceKey.data() + cSliceKey.size(), SER_DISK, g_sClientVersion);

			if (0 == memcmp((char *) &cDSKey[0], (char *) &cDSKeySet[0], 24)) {
				vector<unsigned char> vchValue;
				vector<unsigned char> vchKey;
				CDataStream ssValue(cSliceValue.data(), cSliceValue.data() + cSliceValue.size(), SER_DISK,
						g_sClientVersion);
				ssValue >> vchValue;
				vchKey.insert(vchKey.end(), cSliceKey.data(), cSliceKey.data() + cSliceKey.size());
				mapTxHash.insert(make_pair(vchKey, vchValue));
				pCursor->Next();
			} else {
				break;
			}
		} catch (std::exception &e) {
			return ERRORMSG("%s : Deserialize or I/O error - %s\n", __func__, e.what());
		}
	}
	delete pCursor;
	return true;
}

Object CScriptDB::ToJosnObj(string strPrefix) {
	Object obj;
	Array arrayObj;

	leveldb::Iterator *pCursor = m_LevelDBWrapper.NewIterator();
	CDataStream cDSKeySet(SER_DISK, g_sClientVersion);
	cDSKeySet.insert(cDSKeySet.end(), &strPrefix[0], &strPrefix[strPrefix.length()]);
	pCursor->Seek(cDSKeySet.str());

	while (pCursor->Valid()) {
		boost::this_thread::interruption_point();
		try {
			leveldb::Slice cSliceKey = pCursor->key();
			CDataStream cDSKey(cSliceKey.data(), cSliceKey.data() + cSliceKey.size(), SER_DISK, g_sClientVersion);
			string strScriptKey(cSliceKey.data(), 0, cSliceKey.size());
			string strPrefix = strScriptKey.substr(0, strPrefix.length());
			leveldb::Slice cSliceValue = pCursor->value();
			CDataStream cDSValue(cSliceValue.data(), cSliceValue.data() + cSliceValue.size(), SER_DISK, g_sClientVersion);
			Object obj;
			if (strPrefix == strPrefix) {
				if (strPrefix == "def") {
					obj.push_back(Pair("scriptid", HexStr(cDSKey)));
					obj.push_back(Pair("value", HexStr(cDSValue)));
				} else if (strPrefix == "data") {
					obj.push_back(Pair("key", HexStr(cDSKey)));
					obj.push_back(Pair("value", HexStr(cDSValue)));
				} else if (strPrefix == "acct") {
					obj.push_back(Pair("acctkey", HexStr(cDSKey)));
					obj.push_back(Pair("acctvalue", HexStr(cDSValue)));
				}

			} else {
				obj.push_back(Pair("unkown key", HexStr(cDSKey)));
				obj.push_back(Pair("unkown value", HexStr(cDSValue)));
			}
			arrayObj.push_back(obj);
			pCursor->Next();
		} catch (std::exception &e) {
			if (pCursor) {
				delete pCursor;
			}
			LogPrint("ERROR", "line:%d,%s : Deserialize or I/O error - %s\n", __LINE__, __func__, e.what());
		}
	}
	delete pCursor;
	obj.push_back(Pair("scriptdb", arrayObj));
	return obj;
}

Object CAccountViewDB::ToJosnObj(char chPrefix) {
	Object obj;
	Array arrayObj;
	leveldb::Iterator *pCursor = m_cLevelDBWrapper.NewIterator();
	CDataStream cDSKeySet(SER_DISK, g_sClientVersion);
	if (chPrefix == 'a') {
		vector<unsigned char> vchAccount;
		cDSKeySet << make_pair('a', vchAccount);
	} else {
		CKeyID cKeyid;
		cDSKeySet << make_pair('k', cKeyid);
	}
	pCursor->Seek(cDSKeySet.str());

	// Load mapBlockIndex
	while (pCursor->Valid()) {
		boost::this_thread::interruption_point();
		try {
			leveldb::Slice cSliceKey = pCursor->key();
			CDataStream cDSKey(cSliceKey.data(), cSliceKey.data() + cSliceKey.size(), SER_DISK, g_sClientVersion);
			char chType;
			cDSKey >> chType;
			if (chType == chPrefix) {
				leveldb::Slice cSliceValue = pCursor->value();
				CDataStream cDSValue(cSliceValue.data(), cSliceValue.data() + cSliceValue.size(), SER_DISK, g_sClientVersion);
				Object obj;
				if (chPrefix == 'a') {
					obj.push_back(Pair("accountid:", HexStr(cDSKey)));
					obj.push_back(Pair("keyid", HexStr(cDSValue)));
				} else if (chPrefix == 'k') {
					obj.push_back(Pair("keyid:", HexStr(cDSKey)));
					CAccount account;
					cDSValue >> account;
					obj.push_back(Pair("account", account.ToJosnObj()));
				}
				arrayObj.push_back(obj);
				pCursor->Next();
			} else {
				break; 					// if shutdown requested or finished loading block index
			}
		} catch (std::exception &e) {
			LogPrint("ERROR", "line:%d,%s : Deserialize or I/O error - %s\n", __LINE__, __func__, e.what());
		}
	}
	delete pCursor;
	obj.push_back(Pair("scriptdb", arrayObj));
	return obj;
}
