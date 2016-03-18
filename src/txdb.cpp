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


CBlockTreeDB::CBlockTreeDB(size_t nCacheSize, bool fMemory, bool fWipe) :
		CLevelDBWrapper(GetDataDir() / "blocks" / "index", nCacheSize, fMemory, fWipe) {
}

bool CBlockTreeDB::WriteBlockIndex(const CDiskBlockIndex& blockindex) {
	return Write(make_pair('b', blockindex.GetBlockHash()), blockindex);
}

bool CBlockTreeDB::EraseBlockIndex(const uint256& blockHash) {
	return Erase(make_pair('b', blockHash));
}

bool CBlockTreeDB::WriteBestInvalidWork(const uint256& bnBestInvalidWork) {
	// Obsolete; only written for backward compatibility.
	return Write('I', bnBestInvalidWork);
}

bool CBlockTreeDB::WriteBlockFileInfo(int nFile, const CBlockFileInfo &info) {
	return Write(make_pair('f', nFile), info);
}

bool CBlockTreeDB::ReadBlockFileInfo(int nFile, CBlockFileInfo &info) {
	return Read(make_pair('f', nFile), info);
}

bool CBlockTreeDB::WriteLastBlockFile(int nFile) {
	return Write('l', nFile);
}

bool CBlockTreeDB::WriteReindexing(bool fReindexing) {
	if (fReindexing)
		return Write('R', '1');
	else
		return Erase('R');
}

bool CBlockTreeDB::ReadReindexing(bool &fReindexing) {
	fReindexing = Exists('R');
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

bool CBlockTreeDB::WriteFlag(const string &name, bool fValue) {
	return Write(make_pair('F', name), fValue ? '1' : '0');
}

bool CBlockTreeDB::ReadFlag(const string &name, bool &fValue) {
	char ch;
	if (!Read(make_pair('F', name), ch))
		return false;
	fValue = ch == '1';
	return true;
}

bool CBlockTreeDB::LoadBlockIndexGuts() {
	leveldb::Iterator *pcursor = NewIterator();

	CDataStream ssKeySet(SER_DISK, CLIENT_VERSION);
	ssKeySet << make_pair('b', uint256());
	pcursor->Seek(ssKeySet.str());

	// Load mapBlockIndex
	while (pcursor->Valid()) {
		boost::this_thread::interruption_point();
		try {
			leveldb::Slice slKey = pcursor->key();
			CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, CLIENT_VERSION);
			char chType;
			ssKey >> chType;
			if (chType == 'b') {
				leveldb::Slice slValue = pcursor->value();
				CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
				CDiskBlockIndex diskindex;
				ssValue >> diskindex;

				// Construct block index object
				CBlockIndex* pindexNew = InsertBlockIndex(diskindex.GetBlockHash());
				pindexNew->pprev = InsertBlockIndex(diskindex.hashPrev);
				pindexNew->nHeight = diskindex.nHeight;
				pindexNew->nFile = diskindex.nFile;
				pindexNew->nDataPos = diskindex.nDataPos;
				pindexNew->nUndoPos = diskindex.nUndoPos;
				pindexNew->nVersion = diskindex.nVersion;
				pindexNew->hashMerkleRoot = diskindex.hashMerkleRoot;
				pindexNew->hashPos = diskindex.hashPos;
				pindexNew->nTime = diskindex.nTime;
				pindexNew->nBits = diskindex.nBits;
				pindexNew->nNonce = diskindex.nNonce;
				pindexNew->nStatus = diskindex.nStatus;
				pindexNew->nTx = diskindex.nTx;
				pindexNew->nFuel = diskindex.nFuel;
				pindexNew->nFuelRate = diskindex.nFuelRate;
				pindexNew->vSignature = diskindex.vSignature;
				pindexNew->dFeePerKb = diskindex.dFeePerKb;

				if (!pindexNew->CheckIndex())
					return ERRORMSG("LoadBlockIndex() : CheckIndex failed: %s", pindexNew->ToString());

				pcursor->Next();
			} else {
				break; // if shutdown requested or finished loading block index
			}
		} catch (std::exception &e) {
			return ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
		}
	}
	delete pcursor;

	return true;
}

CAccountViewDB::CAccountViewDB(size_t nCacheSize, bool fMemory, bool fWipe) :
		db(GetDataDir() / "blocks" / "account", nCacheSize, fMemory, fWipe) {

}
CAccountViewDB::CAccountViewDB(const string& name,size_t nCacheSize, bool fMemory, bool fWipe) :
		db(GetDataDir() / "blocks" / name, nCacheSize, fMemory, fWipe) {

}

bool CAccountViewDB::GetAccount(const CKeyID &keyId, CAccount &secureAccount) {
	return db.Read(make_pair('k', keyId), secureAccount);
}

bool CAccountViewDB::SetAccount(const CKeyID &keyId, const CAccount &secureAccount) {
	return db.Write(make_pair('k', keyId), secureAccount);
}

bool CAccountViewDB::SetAccount(const vector<unsigned char> &accountId, const CAccount &secureAccount) {
	CKeyID keyId;
	if (db.Read(make_pair('a', accountId), keyId)) {
		return db.Write(make_pair('k', keyId), secureAccount);
	} else
		return false;
}
bool CAccountViewDB::HaveAccount(const CKeyID &keyId) {
	return db.Exists(keyId);
}

uint256 CAccountViewDB::GetBestBlock() {
	uint256 hash;
	if (!db.Read('B', hash))
		return uint256();
	return hash;
}

bool CAccountViewDB::SetBestBlock(const uint256 &hashBlock) {
	return db.Write('B', hashBlock);
}

bool CAccountViewDB::BatchWrite(const map<CKeyID, CAccount> &mapAccounts,
		const map<vector<unsigned char>, CKeyID> &mapKeyIds, const uint256 &hashBlock) {
	CLevelDBBatch batch;
	map<CKeyID, CAccount>::const_iterator iterAccount = mapAccounts.begin();
	for (; iterAccount != mapAccounts.end(); ++iterAccount) {
		if (iterAccount->second.keyID.IsNull()) {
			batch.Erase(make_pair('k', iterAccount->first));
		} else {
			batch.Write(make_pair('k', iterAccount->first), iterAccount->second);
		}
	}

	map<vector<unsigned char>, CKeyID>::const_iterator iterKey = mapKeyIds.begin();
	for (; iterKey != mapKeyIds.end(); ++iterKey) {
		if (iterKey->second.IsNull()) {
			batch.Erase(make_pair('a', iterKey->first));
		} else {
			batch.Write(make_pair('a', iterKey->first), iterKey->second);
		}
	}
	if (!hashBlock.IsNull())
		batch.Write('B', hashBlock);

	return db.WriteBatch(batch, true);
}

bool CAccountViewDB::BatchWrite(const vector<CAccount> &vAccounts) {
	CLevelDBBatch batch;
	vector<CAccount>::const_iterator iterAccount = vAccounts.begin();
	for (; iterAccount != vAccounts.end(); ++iterAccount) {
		batch.Write(make_pair('k', iterAccount->keyID), *iterAccount);
	}
	return db.WriteBatch(batch, false);
}

bool CAccountViewDB::EraseAccount(const CKeyID &keyId) {
	return db.Erase(make_pair('k', keyId));
}

bool CAccountViewDB::SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId) {
	return db.Write(make_pair('a', accountId), keyId);
}

bool CAccountViewDB::GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId) {
	return db.Read(make_pair('a', accountId), keyId);
}

bool CAccountViewDB::EraseKeyId(const vector<unsigned char> &accountId) {
	return db.Erase(make_pair('a', accountId));
}

bool CAccountViewDB::GetAccount(const vector<unsigned char> &accountId, CAccount &secureAccount) {
	CKeyID keyId;
	if (db.Read(make_pair('a', accountId), keyId)) {
		return db.Read(make_pair('k', keyId), secureAccount);
	}
	return false;
}

bool CAccountViewDB::SaveAccountInfo(const vector<unsigned char> &accountId, const CKeyID &keyId,
		const CAccount &secureAccount) {
	CLevelDBBatch batch;
	batch.Write(make_pair('a', accountId), keyId);
	batch.Write(make_pair('k', keyId), secureAccount);
	return db.WriteBatch(batch, false);
}

uint64_t CAccountViewDB::TraverseAccount() {
	leveldb::Iterator *pcursor = db.NewIterator();

	uint64_t uTotalCoin(0);
	CDataStream ssKeySet(SER_DISK, CLIENT_VERSION);
	ssKeySet << make_pair('k', CKeyID());
	pcursor->Seek(ssKeySet.str());

	// Load mapBlockIndex
	while (pcursor->Valid()) {
		boost::this_thread::interruption_point();
		try {
			leveldb::Slice slKey = pcursor->key();
			CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, CLIENT_VERSION);
			char chType;
			ssKey >> chType;
			if (chType == 'k') {
				leveldb::Slice slValue = pcursor->value();
				CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
				CAccount account;
				ssValue >> account;
				uTotalCoin += account.llValues;
				pcursor->Next();
			} else {
				break; // if shutdown requested or finished loading block index
			}
		} catch (std::exception &e) {
			return ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
		}
	}
	delete pcursor;
	return uTotalCoin;
}

CTransactionDB::CTransactionDB(size_t nCacheSize, bool fMemory, bool fWipe) :
		db(GetDataDir() / "blocks" / "txcache", nCacheSize, fMemory, fWipe) {
}

bool CTransactionDB::SetTxCache(const uint256 &blockHash, const vector<uint256> &vHashTx) {
	return db.Write(make_pair('h', blockHash), vHashTx);
}

bool CTransactionDB::GetTxCache(const uint256 &blockHash, vector<uint256> &vHashTx) {
	return db.Read(make_pair('h', blockHash), vHashTx);
}

bool CTransactionDB::BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHash) {
	CLevelDBBatch batch;
	for (auto & item : mapTxHashByBlockHash) {
		if(item.second.empty()) {
			batch.Erase(make_pair('h', item.first));
		} else {
			if(!db.Exists(make_pair('h', item.first)))
				batch.Write(make_pair('h', item.first), item.second);
		}
	}
	return db.WriteBatch(batch, true);
}

bool CTransactionDB::LoadTransaction(map<uint256, vector<uint256> > &mapTxHashByBlockHash) {

	leveldb::Iterator *pcursor = db.NewIterator();

	CDataStream ssKeySet(SER_DISK, CLIENT_VERSION);
	ssKeySet << make_pair('h', uint256());
	pcursor->Seek(ssKeySet.str());

	// Load mapBlockIndex
	while (pcursor->Valid()) {
		boost::this_thread::interruption_point();
		try {
			leveldb::Slice slKey = pcursor->key();
			CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, CLIENT_VERSION);
			char chType;
			ssKey >> chType;
			if (chType == 'h') {
				leveldb::Slice slValue = pcursor->value();
				CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
				vector<uint256> vTxhash;
				uint256 blockHash;
				ssValue >> vTxhash;
				ssKey >> blockHash;
				mapTxHashByBlockHash[blockHash] = vTxhash;
				pcursor->Next();
			} else {
				break; // if shutdown requested or finished loading block index
			}
		} catch (std::exception &e) {
			return ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
		}
	}
	delete pcursor;
	return true;
}

CScriptDB::CScriptDB(const string&name,size_t nCacheSize, bool fMemory, bool fWipe) :
		db(GetDataDir() / "blocks" / name, nCacheSize, fMemory, fWipe){
}
CScriptDB::CScriptDB(size_t nCacheSize, bool fMemory, bool fWipe) :
		db(GetDataDir() / "blocks" / "script", nCacheSize, fMemory, fWipe){
}
bool CScriptDB::GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue) {
	return db.Read(vKey, vValue);
}

bool CScriptDB::SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue) {
	return db.Write(vKey, vValue);
}
bool CScriptDB::BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas) {
	CLevelDBBatch batch;
	for (auto & item : mapDatas) {
		if (item.second.empty()) {
			batch.Erase(item.first);
		} else {
			batch.Write(item.first, item.second);
		}
	}
	return db.WriteBatch(batch, true);
}
bool CScriptDB::EraseKey(const vector<unsigned char> &vKey) {
	return db.Erase(vKey);
}
bool CScriptDB::HaveData(const vector<unsigned char> &vKey) {
	return db.Exists(vKey);
}
bool CScriptDB::GetScript(const int &nIndex, vector<unsigned char> &vScriptId, vector<unsigned char> &vValue) {
	assert(nIndex >= 0 && nIndex <=1);
	leveldb::Iterator* pcursor = db.NewIterator();
	CDataStream ssKeySet(SER_DISK, CLIENT_VERSION);
	string strPrefixTemp("def");
	//ssKeySet.insert(ssKeySet.end(), 9);
	ssKeySet.insert(ssKeySet.end(), &strPrefixTemp[0], &strPrefixTemp[3]);
	int i(0);
	if(1 == nIndex ) {
		if(vScriptId.empty()) {
			return ERRORMSG("GetScript() : nIndex is 1, and vScriptId is empty");
		}
		vector<char> vId(vScriptId.begin(), vScriptId.end());
		ssKeySet.insert(ssKeySet.end(), vId.begin(), vId.end());
		vector<unsigned char> vKey(ssKeySet.begin(), ssKeySet.end());
		if(HaveData(vKey)) {   //�жϴ�������key,���ݿ����Ƿ��Ѿ�����
			pcursor->Seek(ssKeySet.str());
			i = nIndex;
		}
		else {
			pcursor->Seek(ssKeySet.str());
		}
	} else {
		pcursor->Seek(ssKeySet.str());
	}
	while(pcursor->Valid() && i-->=0) {
		boost::this_thread::interruption_point();
		try {
			leveldb::Slice slKey = pcursor->key();
//			CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, CLIENT_VERSION);
			string strScriptKey(slKey.data(), 0, slKey.size());
//			ssKey >> strScriptKey;
			string strPrefix = strScriptKey.substr(0,3);
			if (strPrefix == "def") {
				if(-1 == i) {
					leveldb::Slice slValue = pcursor->value();
					CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
					ssValue >> vValue;
					vScriptId.clear();
					vScriptId.insert(vScriptId.end(), slKey.data()+3, slKey.data()+slKey.size());
				}
				pcursor->Next();
			}
			else
			{
				delete pcursor;
				return false;
			}
		}catch (std::exception &e) {
				return ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
		}
	}
	delete pcursor;
	if(i >= 0)
		return false;
	return true;
}
bool CScriptDB::GetScriptData(const int curBlockHeight, const vector<unsigned char> &vScriptId, const int &nIndex,
		vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData) {
	const int iPrefixLen = 4;
	const int iScriptIdLen = 6;
	const int iSpaceLen = 1;
	assert(nIndex >= 0 && nIndex <=1);
	leveldb::Iterator* pcursor = db.NewIterator();
	CDataStream ssKeySet(SER_DISK, CLIENT_VERSION);

	string strPrefixTemp("data");
	ssKeySet.insert(ssKeySet.end(), &strPrefixTemp[0], &strPrefixTemp[4]);
	vector<char> vId(vScriptId.begin(), vScriptId.end());
	ssKeySet.insert(ssKeySet.end(), vId.begin(), vId.end());
	ssKeySet.insert(ssKeySet.end(),'_');
	int i(0);
	if (1 == nIndex) {
		if(vScriptKey.empty()) {
			return ERRORMSG("GetScriptData() : nIndex is 1, and vScriptKey is empty");
		}
		vector<char> vsKey(vScriptKey.begin(), vScriptKey.end());
		ssKeySet.insert(ssKeySet.end(), vsKey.begin(), vsKey.end());
		vector<unsigned char> vKey(ssKeySet.begin(), ssKeySet.end());
		if(HaveData(vKey)) {   //�жϴ�������key,���ݿ����Ƿ��Ѿ�����
			pcursor->Seek(ssKeySet.str());
			i = nIndex;
		}
		else {
			pcursor->Seek(ssKeySet.str());
		}
	}else {
		pcursor->Seek(ssKeySet.str());
	}
	while (pcursor->Valid() && i-- >= 0) {
		boost::this_thread::interruption_point();
		try {
			leveldb::Slice slKey = pcursor->key();
			CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, CLIENT_VERSION);
			if (0 == memcmp((char *)&ssKey[0], (char *)&ssKeySet[0], 11)) {
				if (-1 == i) {
					leveldb::Slice slValue = pcursor->value();
					CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
					ssValue >> vScriptData;
					vScriptKey.clear();
					vScriptKey.insert(vScriptKey.end(), slKey.data() + iPrefixLen + iScriptIdLen + iSpaceLen,
							slKey.data() + slKey.size());
				}
				pcursor->Next();
			} else {
				delete pcursor;
				return false;
			}
		} catch (std::exception &e) {
			return ERRORMSG("%s : Deserialize or I/O error - %s\n", __func__, e.what());
		}
	}
	delete pcursor;
	if(i >= 0)
		return false;
	return true;
}

bool CScriptDB::GetTxHashByAddress(const CKeyID &keyId, int nHeight, map<vector<unsigned char>, vector<unsigned char> > &mapTxHash)
{
	leveldb::Iterator* pcursor = db.NewIterator();
	CDataStream ssKeySet(SER_DISK, CLIENT_VERSION);

	string strPrefixTemp("ADDR");
	ssKeySet.insert(ssKeySet.end(), &strPrefixTemp[0], &strPrefixTemp[4]);
	ssKeySet << keyId;
	ssKeySet << nHeight;
	pcursor->Seek(ssKeySet.str());

	while (pcursor->Valid()) {
		boost::this_thread::interruption_point();
		try {
			leveldb::Slice slKey = pcursor->key();
			leveldb::Slice slValue = pcursor->value();
			CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, CLIENT_VERSION);
			if (0 == memcmp((char *)&ssKey[0], (char *)&ssKeySet[0], 24)) {
				vector<unsigned char> vValue;
				vector<unsigned char> vKey;
				CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
				ssValue >> vValue;
				vKey.insert(vKey.end(), slKey.data(), slKey.data() + slKey.size());
				mapTxHash.insert(make_pair(vKey, vValue));
				pcursor->Next();
			} else {
				break;
			}
		} catch (std::exception &e) {
			return ERRORMSG("%s : Deserialize or I/O error - %s\n", __func__, e.what());
		}
	}
	delete pcursor;
	return true;
}
Object CScriptDB::ToJosnObj(string Prefix) {

	Object obj;
	Array arrayObj;

	leveldb::Iterator *pcursor = db.NewIterator();
	CDataStream ssKeySet(SER_DISK, CLIENT_VERSION);
	ssKeySet.insert(ssKeySet.end(), &Prefix[0], &Prefix[Prefix.length()]);
	pcursor->Seek(ssKeySet.str());

	while (pcursor->Valid()) {
		boost::this_thread::interruption_point();
		try {
			leveldb::Slice slKey = pcursor->key();
			CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, CLIENT_VERSION);
			string strScriptKey(slKey.data(), 0, slKey.size());
			string strPrefix = strScriptKey.substr(0,Prefix.length());
			leveldb::Slice slValue = pcursor->value();
			CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
			Object obj;
			if (strPrefix == Prefix) {
				if(Prefix == "def"){
				obj.push_back(Pair("scriptid", HexStr(ssKey)));
				obj.push_back(Pair("value", HexStr(ssValue)));
				}
				else if(Prefix == "data"){
					obj.push_back(Pair("key", HexStr(ssKey)));
					obj.push_back(Pair("value", HexStr(ssValue)));
				}
				else if(Prefix == "acct"){
					obj.push_back(Pair("acctkey", HexStr(ssKey)));
					obj.push_back(Pair("acctvalue", HexStr(ssValue)));

				}

			} else {
				obj.push_back(Pair("unkown key", HexStr(ssKey)));
				obj.push_back(Pair("unkown value", HexStr(ssValue)));
			}
			arrayObj.push_back(obj);
			pcursor->Next();
		} catch (std::exception &e) {
			if(pcursor)
						delete pcursor;
			LogPrint("ERROR","line:%d,%s : Deserialize or I/O error - %s\n",__LINE__, __func__, e.what());
		}
	}
	delete pcursor;
	obj.push_back(Pair("scriptdb", arrayObj));
	return obj;
}

Object CAccountViewDB::ToJosnObj(char Prefix) {
		Object obj;
		Array arrayObj;
		leveldb::Iterator *pcursor =db.NewIterator();
		CDataStream ssKeySet(SER_DISK, CLIENT_VERSION);
		if(Prefix == 'a'){
			vector<unsigned char>account;
			ssKeySet << make_pair('a',account);
		}else{
			CKeyID keyid;
			ssKeySet << make_pair('k',keyid);
		}
		pcursor->Seek(ssKeySet.str());
		// Load mapBlockIndex
		while (pcursor->Valid()) {
			boost::this_thread::interruption_point();
			try {
				leveldb::Slice slKey = pcursor->key();
				CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, CLIENT_VERSION);
				char chType;
				ssKey >> chType;
				if (chType == Prefix) {
					leveldb::Slice slValue = pcursor->value();
					CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
					Object obj;
					if(Prefix == 'a'){
						obj.push_back(Pair("accountid:", HexStr(ssKey)));
						obj.push_back(Pair("keyid", HexStr(ssValue)));
					}else if(Prefix == 'k'){
						obj.push_back(Pair("keyid:", HexStr(ssKey)));
						CAccount account;
						ssValue >> account;
						obj.push_back(Pair("account", account.ToJosnObj()));
					}
					arrayObj.push_back(obj);
					pcursor->Next();
					} else {
					break; // if shutdown requested or finished loading block index
				}
			} catch (std::exception &e) {
				LogPrint("ERROR","line:%d,%s : Deserialize or I/O error - %s\n",__LINE__, __func__, e.what());
			 }
		}
		delete pcursor;
		obj.push_back(Pair("scriptdb", arrayObj));
		return obj;
}
