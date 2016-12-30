// Copyright (c) 2012-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "leveldbwrapper.h"

#include "util.h"

#include <boost/filesystem.hpp>
#include <leveldb/cache.h>
#include <leveldb/env.h>
#include <leveldb/filter_policy.h>
#include <memenv.h>
#include "json/json_spirit_value.h"

void HandleError(const leveldb::Status &status) throw(leveldb_error) {
    if (status.ok()) {
    	 return;
    }
    LogPrint("INFO","%s\n", status.ToString());
    if (status.IsCorruption()) {
    	throw leveldb_error("Database corrupted");
    }
    if (status.IsIOError()) {
    	throw leveldb_error("Database I/O error");
    }
    if (status.IsNotFound()) {
    	throw leveldb_error("Database entry missing");
    }
    throw leveldb_error("Unknown database error");
}

static leveldb::Options GetOptions(size_t nCacheSize) {
    leveldb::Options tOptions;
    tOptions.block_cache 		= leveldb::NewLRUCache(nCacheSize / 2);
    tOptions.write_buffer_size 	= nCacheSize / 4; // up to two write buffers may be held in memory simultaneously
    tOptions.filter_policy 		= leveldb::NewBloomFilterPolicy(10);
    tOptions.compression 		= leveldb::kNoCompression;
    tOptions.max_open_files 	= 64;
    return tOptions;
}

CLevelDBWrapper::CLevelDBWrapper(const boost::filesystem::path &path, size_t unCacheSize, bool bMemory, bool bWipe) {
    m_pEnv 							= NULL;
    m_tReadoptions.verify_checksums = true;
    m_tIteroptions.verify_checksums = true;
    m_tIteroptions.fill_cache 		= false;
    m_tSyncoptions.sync 			= true;
    m_tOptions 						= GetOptions(unCacheSize);
    m_tOptions.create_if_missing 	= true;
    if (bMemory) {
        m_pEnv = leveldb::NewMemEnv(leveldb::Env::Default());
        m_tOptions.env = m_pEnv;
    } else {
        if (bWipe) {
            LogPrint("INFO","Wiping LevelDB in %s\n", path.string());
            leveldb::DestroyDB(path.string(), m_tOptions);
        }
        TryCreateDirectory(path);
        LogPrint("INFO","Opening LevelDB in %s\n", path.string());
    }
    leveldb::Status status = leveldb::DB::Open(m_tOptions, path.string(), &m_pDb);
    HandleError(status);
    LogPrint("INFO","Opened LevelDB successfully\n");
}

CLevelDBWrapper::~CLevelDBWrapper() {
    delete m_pDb;
    m_pDb = NULL;
    delete m_tOptions.filter_policy;
    m_tOptions.filter_policy = NULL;
    delete m_tOptions.block_cache;
    m_tOptions.block_cache = NULL;
    delete m_pEnv;
    m_tOptions.env = NULL;
}

bool CLevelDBWrapper::WriteBatch(CLevelDBBatch &cBatch, bool bSync) throw (leveldb_error) {
	leveldb::Status status = m_pDb->Write(bSync ? m_tSyncoptions : m_tWriteoptions, &cBatch.m_Batch);
	HandleError(status);
	return true;
}

int64_t CLevelDBWrapper::GetDbCount() {
	leveldb::Iterator *pCursor = NewIterator();
	int64_t llRet = 0;
	pCursor->SeekToFirst();
	while (pCursor->Valid()) {
		boost::this_thread::interruption_point();
		try {
			llRet++;
			leveldb::Slice slKey = pCursor->key();
			leveldb::Slice slValue = pCursor->value();
			CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, g_sClientVersion);
			CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, g_sClientVersion);
			LogPrint("db", "Key:%s\n value:%s\n", HexStr(ssKey), HexStr(ssValue));
			pCursor->Next();
		} catch (std::exception &e) {
			if (pCursor) {
				delete pCursor;
			}
			ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
			return 0;
		}
	}
	delete pCursor;
	return llRet;
}
