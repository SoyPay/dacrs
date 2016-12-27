// Copyright (c) 2012-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_LEVELDBWRAPPER_H_
#define DACRS_LEVELDBWRAPPER_H_

#include "serialize.h"
#include "util.h"
#include "version.h"

#include <boost/filesystem/path.hpp>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include "json/json_spirit_value.h"
using namespace json_spirit;

class leveldb_error: public runtime_error {
 public:
	leveldb_error(const string &msg) :
			runtime_error(msg) {
	}
};

void HandleError(const leveldb::Status &status) throw(leveldb_error);

// Batch of changes queued to be written to a CLevelDBWrapper
class CLevelDBBatch {
 public:
	template<typename K, typename V> void Write(const K& key, const V& value) {
		leveldb::Slice cSliceKey;
		CDataStream cDSKey(SER_DISK, CLIENT_VERSION);
		cDSKey.reserve(cDSKey.GetSerializeSize(key));
		cDSKey << key;

		if (typeid(key) == typeid(std::vector<unsigned char>)) {
			CDataStream cDSKeyTemp(cDSKey.begin(), cDSKey.end(), SER_DISK, CLIENT_VERSION);
			vector<unsigned char> vchKey;
			cDSKeyTemp >> vchKey;
			int nStartPos = 0;
			int nVKeySize = vchKey.size();
			nStartPos = GetSizeOfCompactSize(nVKeySize);
			cSliceKey = leveldb::Slice(&cDSKey[nStartPos], cDSKey.size() - nStartPos);
		} else {
			cSliceKey = leveldb::Slice(&cDSKey[0], cDSKey.size());
		}
		CDataStream cDSValue(SER_DISK, CLIENT_VERSION);
		cDSValue.reserve(cDSValue.GetSerializeSize(value));
		cDSValue << value;
		leveldb::Slice slValue(&cDSValue[0], cDSValue.size());
		m_Batch.Put(cSliceKey, slValue);
	}

	template<typename K> void Erase(const K& key) {
		leveldb::Slice cSliceKey;
		CDataStream cDSKey(SER_DISK, CLIENT_VERSION);
		cDSKey.reserve(cDSKey.GetSerializeSize(key));
		cDSKey << key;
		if (typeid(key) == typeid(std::vector<unsigned char>)) {
			CDataStream cDSKeyTemp(cDSKey.begin(), cDSKey.end(), SER_DISK, CLIENT_VERSION);
			vector<unsigned char> vchKey;
			cDSKeyTemp >> vchKey;
			int nStartPos = 0;
			int nVKeySize = vchKey.size();
			nStartPos = GetSizeOfCompactSize(nVKeySize);
			cSliceKey = leveldb::Slice(&cDSKey[nStartPos], cDSKey.size() - nStartPos);
		} else {
			cSliceKey = leveldb::Slice(&cDSKey[0], cDSKey.size());
		}

		m_Batch.Delete(cSliceKey);
	}
	friend class CLevelDBWrapper;

 private:
 	leveldb::WriteBatch m_Batch;
};

class CLevelDBWrapper {
 public:
    CLevelDBWrapper(const boost::filesystem::path &path, size_t unCacheSize, bool bMemory = false, bool bWipe = false);
    ~CLevelDBWrapper();

    template<typename K, typename V> bool Read(const K& key, V& value) throw(leveldb_error) {
    	leveldb::Slice cSliceKey;
    	CDataStream cDSKey(SER_DISK, CLIENT_VERSION);

		cDSKey.reserve(cDSKey.GetSerializeSize(key));
		cDSKey << key;
		if (typeid(key) == typeid(std::vector<unsigned char>)) {
			CDataStream cDSKeyTemp(cDSKey.begin(), cDSKey.end(), SER_DISK, CLIENT_VERSION);
			vector<unsigned char> vchKey;
			cDSKeyTemp >> vchKey;
			int nStartPos = 0;
			int nVKeySize = vchKey.size();
			nStartPos = GetSizeOfCompactSize(nVKeySize);
			cSliceKey = leveldb::Slice(&cDSKey[nStartPos], cDSKey.size() - nStartPos);
		} else {
			cSliceKey = leveldb::Slice(&cDSKey[0], cDSKey.size());
		}
        string strValue;
        leveldb::Status cStatus = m_pDb->Get(m_tReadoptions, cSliceKey, &strValue);
        if (!cStatus.ok()) {
            if (cStatus.IsNotFound()) {
            	return false;
            }
            LogPrint("INFO","LevelDB read failure: %s\n", cStatus.ToString().c_str());
            HandleError(cStatus);
        }
        try {
            CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
            ssValue >> value;
        } catch(std::exception &e) {
            return false;
        }
        return true;
    }

    template<typename K, typename V> bool Write(const K& key, const V& value, bool bSync = false) throw(leveldb_error) {
        CLevelDBBatch cBatch;
        cBatch.Write(key, value);
        return WriteBatch(cBatch, bSync);
    }

    template<typename K> bool Exists(const K& key) throw(leveldb_error) {
    	leveldb::Slice cSliceKey;
    	CDataStream cDSKey(SER_DISK, CLIENT_VERSION);
        cDSKey.reserve(cDSKey.GetSerializeSize(key));
        cDSKey << key;
		if (typeid(key) == typeid(std::vector<unsigned char>)) {
			CDataStream cDSKeyTemp(cDSKey.begin(), cDSKey.end(), SER_DISK, CLIENT_VERSION);
			vector<unsigned char> vchKey;
			cDSKeyTemp >> vchKey;
			int nStartPos = 0;
			int nVKeySize = vchKey.size();
			nStartPos = GetSizeOfCompactSize(nVKeySize);
			cSliceKey = leveldb::Slice(&cDSKey[nStartPos], cDSKey.size() - nStartPos);
		} else {
			cSliceKey = leveldb::Slice(&cDSKey[0], cDSKey.size());
		}

        string strValue;
        leveldb::Status nStatus = m_pDb->Get(m_tReadoptions, cSliceKey, &strValue);
        if (!nStatus.ok()) {
            if (nStatus.IsNotFound()) {
            	return false;
            }
            LogPrint("INFO","LevelDB read failure: %s\n", nStatus.ToString().c_str());
            HandleError(nStatus);
        }
        return true;
    }

    template<typename K> bool Erase(const K& key, bool bSync = false) throw(leveldb_error) {
        CLevelDBBatch cBatch;
        cBatch.Erase(key);
        return WriteBatch(cBatch, bSync);
    }

    bool WriteBatch(CLevelDBBatch &cBatch, bool bSync = false) throw(leveldb_error);

    // not available for LevelDB; provide for compatibility with BDB
    bool Flush() {
        return true;
    }

    bool Sync() throw(leveldb_error) {
        CLevelDBBatch cBatch;
        return WriteBatch(cBatch, true);
    }

    // not exactly clean encapsulation, but it's easiest for now
    leveldb::Iterator *NewIterator() {
        return m_pDb->NewIterator(m_tIteroptions);
    }

    int64_t GetDbCount();
   // Object ToJosnObj();

 private:
     // custom environment this database is using (may be NULL in case of default environment)
     leveldb::Env *m_pEnv;

     // database options used
     leveldb::Options m_tOptions;

     // options used when reading from the database
     leveldb::ReadOptions m_tReadoptions;

     // options used when iterating over values of the database
     leveldb::ReadOptions m_tIteroptions;

     // options used when writing to the database
     leveldb::WriteOptions m_tWriteoptions;

     // options used when sync writing to the database
     leveldb::WriteOptions m_tSyncoptions;

     // the database itself
     leveldb::DB *m_pDb;
};

#endif // DACRS_LEVELDBWRAPPER_H_
