// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txdb.h"
#include "util.h"
#include "core.h"
#include "uint256.h"
#include "key.h"
#include <stdint.h>

using namespace std;


void static BatchWriteHashBestChain(CLevelDBBatch &batch, const uint256 &hash) {
	batch.Write('B', hash);
}


CBlockTreeDB::CBlockTreeDB(size_t nCacheSize, bool fMemory, bool fWipe) :
		CLevelDBWrapper(GetDataDir() / "blocks" / "index", nCacheSize, fMemory, fWipe) {
}

bool CBlockTreeDB::WriteBlockIndex(const CDiskBlockIndex& blockindex) {
	return Write(make_pair('b', blockindex.GetBlockHash()), blockindex);
}

bool CBlockTreeDB::WriteBestInvalidWork(const CBigNum& bnBestInvalidWork) {
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

bool CBlockTreeDB::ReadTxIndex(const uint256 &txid, CDiskTxPos &pos) {
	return Read(make_pair('t', txid), pos);
}

bool CBlockTreeDB::WriteTxIndex(const vector<pair<uint256, CDiskTxPos> >&vect) {
	CLevelDBBatch batch;
	for (vector<pair<uint256, CDiskTxPos> >::const_iterator it = vect.begin(); it != vect.end(); it++)
		batch.Write(make_pair('t', it->first), it->second);
	return WriteBatch(batch);
}

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
	ssKeySet << make_pair('b', uint256(0));
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
				pindexNew->nTime = diskindex.nTime;
				pindexNew->nBits = diskindex.nBits;
				pindexNew->nNonce = diskindex.nNonce;
				pindexNew->nStatus = diskindex.nStatus;
				pindexNew->nTx = diskindex.nTx;
				pindexNew->vSignature = diskindex.vSignature;

				if (!pindexNew->CheckIndex())
					return ERROR("LoadBlockIndex() : CheckIndex failed: %s", pindexNew->ToString());

				pcursor->Next();
			} else {
				break; // if shutdown requested or finished loading block index
			}
		} catch (std::exception &e) {
			return ERROR("%s : Deserialize or I/O error - %s", __func__, e.what());
		}
	}
	delete pcursor;

	return true;
}

CAccountViewDB::CAccountViewDB(size_t nCacheSize, bool fMemory, bool fWipe) :
		db(GetDataDir() / "blocks" / "account", nCacheSize, fMemory, fWipe) {

}

bool CAccountViewDB::GetAccount(const CKeyID &keyId, CSecureAccount &secureAccount) {
	return db.Read(make_pair('k', keyId), secureAccount);
}

bool CAccountViewDB::SetAccount(const CKeyID &keyId, const CSecureAccount &secureAccount) {
	return db.Write(make_pair('k', keyId), secureAccount);
}

bool CAccountViewDB::SetAccount(const vector<unsigned char> &accountId, const CSecureAccount &secureAccount) {
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
		return uint256(0);
	return hash;
}

bool CAccountViewDB::SetBestBlock(const uint256 &hashBlock) {
	return db.Write('B', hashBlock);
}

bool CAccountViewDB::BatchWrite(const map<CKeyID, CSecureAccount> &mapAccounts,
		const map<string, CKeyID> &mapKeyIds, const uint256 &hashBlock) {
	CLevelDBBatch batch;
	map<CKeyID, CSecureAccount>::const_iterator iterAccount = mapAccounts.begin();
	for (; iterAccount != mapAccounts.end(); ++iterAccount) {
		if (uint160(0) == iterAccount->second.keyID) {
			batch.Erase(make_pair('k', iterAccount->first));
		} else {
			batch.Write(make_pair('k', iterAccount->first), iterAccount->second);
		}
	}

	map<string, CKeyID>::const_iterator iterKey = mapKeyIds.begin();
	for (; iterKey != mapKeyIds.end(); ++iterKey) {
		if (uint160(0) == iterKey->second) {
			batch.Erase(make_pair('a', ParseHex(iterKey->first)));
		} else {
			batch.Write(make_pair('a', ParseHex(iterKey->first)), iterKey->second);
		}
	}
	if (uint256(0) != hashBlock)
		batch.Write('B', hashBlock);

	return db.WriteBatch(batch, false);
}

bool CAccountViewDB::BatchWrite(const vector<CSecureAccount> &vAccounts) {
	CLevelDBBatch batch;
	vector<CSecureAccount>::const_iterator iterAccount = vAccounts.begin();
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

bool CAccountViewDB::GetAccount(const vector<unsigned char> &accountId, CSecureAccount &secureAccount) {
	CKeyID keyId;
	if (db.Read(make_pair('a', accountId), keyId)) {
		return db.Read(make_pair('k', keyId), secureAccount);
	}
	return false;
}

bool CAccountViewDB::SaveAccountInfo(const vector<unsigned char> &accountId, const CKeyID &keyId,
		const CSecureAccount &secureAccount) {
	CLevelDBBatch batch;
	batch.Write(make_pair('a', accountId), keyId);
	batch.Write(make_pair('k', keyId), secureAccount);
	return db.WriteBatch(batch, false);
}

CTransactionCacheDB::CTransactionCacheDB(size_t nCacheSize, bool fMemory, bool fWipe) :
		CLevelDBWrapper(GetDataDir() / "blocks" / "txcache", nCacheSize, fMemory, fWipe) {
}

bool CTransactionCacheDB::SetRelayTx(const uint256 &prevhash, const vector<uint256> &vHashTx) {
	return Write(make_pair('p', prevhash), vHashTx);
}

bool CTransactionCacheDB::GetRelayTx(const uint256 &prevhash, vector<uint256> &vHashTx) {
	return Read(make_pair('p', prevhash), vHashTx);
}

bool CTransactionCacheDB::SetTxCache(const uint256 &blockHash, const vector<uint256> &vHashTx) {
	return Write(make_pair('h', blockHash), vHashTx);
}

bool CTransactionCacheDB::GetTxCache(const uint256 &blockHash, vector<uint256> &vHashTx) {
	return Read(make_pair('h', blockHash), vHashTx);
}

bool CTransactionCacheDB::Flush(const map<uint256, vector<uint256> > &mapTxHashByBlockHash,
		const map<uint256, vector<uint256> > &mapTxHashCacheByPrev) {
	CLevelDBBatch batch;
	for (auto & item : mapTxHashByBlockHash) {
		if(item.second.empty()) {
			batch.Erase(make_pair('h', item.first));
		} else {
			batch.Write(make_pair('h', item.first), item.second);
		}
	}

	for (auto & item : mapTxHashCacheByPrev) {
		if (item.second.empty()) {
			batch.Erase(make_pair('p', item.first));
		} else {
			batch.Write(make_pair('p', item.first), item.second);
		}
	}
	return WriteBatch(batch, false);
}

bool CTransactionCacheDB::LoadTransaction(map<uint256, vector<uint256> > &mapTxHashByBlockHash,
		map<uint256, vector<uint256> > &mapTxHashCacheByPrev) {

	leveldb::Iterator *pcursor = NewIterator();

	CDataStream ssKeySet(SER_DISK, CLIENT_VERSION);
	ssKeySet << make_pair('h', uint256(0));
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
			} else if (chType == 'p') {
				leveldb::Slice slValue = pcursor->value();
				CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
				vector<uint256> hashTxSet;
				uint256 preTxHash;
				ssValue >> hashTxSet;
				ssKey >> preTxHash;
				mapTxHashCacheByPrev[preTxHash] = hashTxSet;
				pcursor->Next();
			} else {
				break; // if shutdown requested or finished loading block index
			}
		} catch (std::exception &e) {
			return ERROR("%s : Deserialize or I/O error - %s", __func__, e.what());
		}
	}
	delete pcursor;
	return true;
}

CScriptDB::CScriptDB(size_t nCacheSize, bool fMemory, bool fWipe) :
		CLevelDBWrapper(GetDataDir() / "blocks" / "script", nCacheSize, fMemory, fWipe) {
}

bool CScriptDB::SetArbitrator(const vector<unsigned char> &scriptId, const set<string> &setArbitrator) {
	return Write(make_pair('a', scriptId), setArbitrator);
//	string strScriptId = HexStr(scriptId.begin(), scriptId.end());
//	CContractScript ascript;
//	if (!mapScript.count(strScriptId)) {
//		return false;
//	}
//
//	ascript = mapScript[strScriptId];
//	ascript.setArbitratorAccId = setArbitrator;
//	mapScript[strScriptId] = ascript;
//	return true;
}
bool CScriptDB::SetScript(const vector<unsigned char> &scriptId, const vector<unsigned char> &vScript) {
	return Write(make_pair('s', scriptId), vScript);
//	string strScriptId = HexStr(scriptId.begin(), scriptId.end());
//	CContractScript ascript;
//	if (!mapScript.count(strScriptId)) {
//		ascript.scriptContent = vScript;
//		ascript.scriptId = scriptId;
//		ascript.setArbitratorAccId.clear();
//		mapScript[strScriptId] = ascript;
//		return true;
//	}
//	ascript = mapScript[strScriptId];
//	ascript.scriptContent = vScript;
//	mapScript[strScriptId] = ascript;
//	return true;
}
bool CScriptDB::GetArbitrator(const vector<unsigned char> &scriptId, set<string> &setArbitrator) {
	return Read(make_pair('a', scriptId), setArbitrator);
//	string strScriptId = HexStr(scriptId.begin(), scriptId.end());
//	CContractScript ascript;
//	if (!mapScript.count(strScriptId)) {
//		return false;
//	}
//
//	ascript = mapScript[strScriptId];
//	setArbitrator = ascript.setArbitratorAccId;
//	return true;
}
bool CScriptDB::GetScript(const vector<unsigned char> &scriptId, vector<unsigned char> &vScript) {
	return Read(make_pair('s', scriptId), vScript);
//	string strScriptId = HexStr(scriptId.begin(), scriptId.end());
//	CContractScript ascript;
//	if (!mapScript.count(strScriptId)) {
//		return false;
//	}
//
//	ascript = mapScript[strScriptId];
//	vScript = ascript.scriptContent;
//	return true;
}
bool CScriptDB::EraseScript(const vector<unsigned char> & scriptId) {
	CLevelDBBatch batch;
	batch.Erase(make_pair('s', scriptId));
	batch.Erase(make_pair('a', scriptId));
	return WriteBatch(batch, false);
//	string strScriptId = HexStr(scriptId.begin(), scriptId.end());
//	CContractScript ascript;
//	if (!mapScript.count(strScriptId)) {
//		return false;
//	}
//
//	ascript = mapScript[strScriptId];
//	ascript.scriptId.clear();
//	mapScript[strScriptId] = ascript;
//	return true;
}

bool CScriptDB::GetContractScript(const vector<unsigned char> &scriptId, CContractScript &contractScript) {
	contractScript.scriptId = scriptId;
	if(!GetScript(scriptId, contractScript.scriptContent))
		return false;
	if(!GetArbitrator(scriptId, contractScript.setArbitratorAccId))
		return false;
	return true;
}

bool CScriptDB::Flush(const map<string, CContractScript> &mapScriptCache){
	CLevelDBBatch batch;
	vector<unsigned char> vScriptID;
	map<string, CContractScript>::const_iterator iterScript =  mapScriptCache.begin();
	for(; iterScript != mapScriptCache.end(); ++iterScript) {
		vScriptID = ParseHex(iterScript->first.c_str());
		if(iterScript->second.scriptId.empty() ){
			batch.Erase(make_pair('s', vScriptID));
			batch.Erase(make_pair('a', vScriptID));
		}else{
			batch.Write(make_pair('s', vScriptID), iterScript->second.scriptContent);
			batch.Write(make_pair('a', vScriptID), iterScript->second.setArbitratorAccId);
		}
	}

	return WriteBatch(batch, false);
}
bool CScriptDB::LoadRegScript(map<string, CContractScript> &mapScriptCache) {

	leveldb::Iterator *pcursor = NewIterator();
	vector<unsigned char> vTemp;
	CDataStream ssKeySet(SER_DISK, CLIENT_VERSION);
	ssKeySet << make_pair('a', vTemp);
	pcursor->Seek(ssKeySet.str());

	// Load mapBlockIndex
	while (pcursor->Valid()) {
		boost::this_thread::interruption_point();
		try {
			leveldb::Slice slKey = pcursor->key();
			CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, CLIENT_VERSION);
			char chType;
			ssKey >> chType;
			CContractScript ascript;
			ssKey >> ascript.scriptId;
			string strScriptId = HexStr(ascript.scriptId.begin(), ascript.scriptId.end());
			if (mapScriptCache.count(strScriptId)) {
				ascript = mapScriptCache[strScriptId];
			}
			if (chType == 'a') {
				leveldb::Slice slValue = pcursor->value();
				CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
				set<string> setArbitrator;
				ssValue >> setArbitrator;
				ascript.setArbitratorAccId.insert(setArbitrator.begin(), setArbitrator.end());
				pcursor->Next();
			} else if (chType == 's') {
				leveldb::Slice slValue = pcursor->value();
				CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
				vector<unsigned char> vScript;
				ssValue >> vScript;
				ascript.scriptContent = vScript;
				pcursor->Next();
			} else {
				break; // if shutdown requested or finished loading block index
			}
			mapScriptCache[strScriptId] = ascript;
		} catch (std::exception &e) {
			return ERROR("%s : Deserialize or I/O error - %s", __func__, e.what());
		}
	}
	delete pcursor;

	return true;
}
