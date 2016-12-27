// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "db.h"

#include <stdint.h>

#ifndef WIN32
#include <sys/stat.h>
#endif

#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/version.hpp>
#include <openssl/rand.h>

#include "addrman.h"
#include "hash.h"
#include "protocol.h"
#include "util.h"
//#include "utilstrencodings.h"

using namespace std;

//
// CDB
//

CDBEnv g_cDacrsDbEnv;

void CDBEnv::EnvShutdown() {
    if (!m_bDbEnvInit)
        return;

    m_bDbEnvInit = false;
    int ret = m_pDbEnv->close(0);
    if (ret != 0) {
    	LogPrint("CDB","CDBEnv::EnvShutdown: Error %d shutting down database environment: %s\n", ret, DbEnv::strerror(ret));
    }
    if (!m_bMockDb) {
    	string te = m_path.string();
    	ret= DbEnv(0).remove(te.c_str(), 0);
        if (ret != 0) {
        	LogPrint("CDB","CDBEnv::remove: Error %d shutting down database environment: %s\n", ret, DbEnv::strerror(ret));
        }
    }
}

void CDBEnv::Reset() {
    delete m_pDbEnv;
    m_pDbEnv = new DbEnv(DB_CXX_NO_EXCEPTIONS);
    m_bDbEnvInit = false;
    m_bMockDb = false;
}

CDBEnv::CDBEnv() : m_pDbEnv(NULL) {
    Reset();
}

CDBEnv::~CDBEnv() {
    EnvShutdown();
    delete m_pDbEnv;
    m_pDbEnv = NULL;
}

void CDBEnv::Close() {
    EnvShutdown();
}

bool CDBEnv::Open(const boost::filesystem::path& pathIn) {
    if (m_bDbEnvInit) {
    	return true;
    }
    boost::this_thread::interruption_point();

    m_path = pathIn;
    boost::filesystem::path pathLogDir = m_path / "database";
    TryCreateDirectory(pathLogDir);
    boost::filesystem::path pathErrorFile = m_path / "db.log";
    LogPrint("CDB","CDBEnv::Open: LogDir=%s ErrorFile=%s\n", pathLogDir.string(), pathErrorFile.string());

    unsigned int nEnvFlags = 0;
    if (SysCfg().GetBoolArg("-privdb", true)) {
    	nEnvFlags |= DB_PRIVATE;
    }
    m_pDbEnv->set_lg_dir(pathLogDir.string().c_str());
    m_pDbEnv->set_cachesize(0, 0x100000, 1); // 1 MiB should be enough for just the wallet
    m_pDbEnv->set_lg_bsize(0x10000);
    m_pDbEnv->set_lg_max(1048576);
    m_pDbEnv->set_lk_max_locks(40000);
    m_pDbEnv->set_lk_max_objects(40000);
    m_pDbEnv->set_errfile(fopen(pathErrorFile.string().c_str(), "a")); /// debug
    m_pDbEnv->set_flags(DB_AUTO_COMMIT, 1);
    m_pDbEnv->set_flags(DB_TXN_WRITE_NOSYNC, 1);
    m_pDbEnv->log_set_config(DB_LOG_AUTO_REMOVE, 1);
    int ret = m_pDbEnv->open(m_path.string().c_str(),
                         DB_CREATE |
                         DB_INIT_LOCK |
                         DB_INIT_LOG |
                         DB_INIT_MPOOL |
                         DB_INIT_TXN |
                         DB_THREAD |
                         DB_RECOVER |
                         nEnvFlags,
                         S_IRUSR | S_IWUSR);
    if (ret != 0) {
    	return ERRORMSG(strprintf("CDBEnv::Open: Error %d opening database environment: %s\n", ret, DbEnv::strerror(ret)).c_str());
    }

    m_bDbEnvInit = true;
    m_bMockDb = false;
    return true;
}

void CDBEnv::MakeMock() {
    if (m_bDbEnvInit) {
    	throw runtime_error("CDBEnv::MakeMock: Already initialized");
    }
    boost::this_thread::interruption_point();

    LogPrint("db", "CDBEnv::MakeMock\n");

    m_pDbEnv->set_cachesize(1, 0, 1);
    m_pDbEnv->set_lg_bsize(10485760 * 4);
    m_pDbEnv->set_lg_max(10485760);
    m_pDbEnv->set_lk_max_locks(10000);
    m_pDbEnv->set_lk_max_objects(10000);
    m_pDbEnv->set_flags(DB_AUTO_COMMIT, 1);
    m_pDbEnv->log_set_config(DB_LOG_IN_MEMORY, 1);
    int ret = m_pDbEnv->open(NULL,
                         DB_CREATE |
                         DB_INIT_LOCK |
                         DB_INIT_LOG |
                         DB_INIT_MPOOL |
                         DB_INIT_TXN |
                         DB_THREAD |
                         DB_PRIVATE,
                         S_IRUSR | S_IWUSR);
    if (ret > 0) {
    	throw runtime_error(strprintf("CDBEnv::MakeMock: Error %d opening database environment.", ret));
    }

    m_bDbEnvInit = true;
    m_bMockDb = true;
}

CDBEnv::m_emVerifyResult CDBEnv::Verify(std::string strFile, bool (*recoverFunc)(CDBEnv& dbenv, std::string strFile)) {
	LOCK(m_cs_Db);
    assert(m_mapFileUseCount.count(strFile) == 0);

    Db db(m_pDbEnv, 0);
    int result = db.verify(strFile.c_str(), NULL, NULL, 0);
    if (result == 0) {
    	return EM_VERIFY_OK;
    }
    else if (recoverFunc == NULL) {
    	return EM_RECOVER_FAIL;
    }

    // Try to recover:
    bool fRecovered = (*recoverFunc)(*this, strFile);
    return (fRecovered ? EM_RECOVER_OK : EM_RECOVER_FAIL);
}

bool CDBEnv::Salvage(std::string strFile, bool bAggressive, std::vector<CDBEnv::KeyValPair>& vecResult) {
    LOCK(m_cs_Db);
    assert(m_mapFileUseCount.count(strFile) == 0);

    u_int32_t flags = DB_SALVAGE;
    if (bAggressive) {
    	flags |= DB_AGGRESSIVE;
    }

    stringstream strDump;
    Db db(m_pDbEnv, 0);
    int result = db.verify(strFile.c_str(), NULL, &strDump, flags);
    if (result == DB_VERIFY_BAD) {
        LogPrint("CDB","CDBEnv::Salvage: Database salvage found errors, all data may not be recoverable.\n");
        if (!bAggressive) {
            LogPrint("CDB","CDBEnv::Salvage: Rerun with aggressive mode to ignore errors and continue.\n");
            return false;
        }
    }
    if (result != 0 && result != DB_VERIFY_BAD) {
        LogPrint("CDB","CDBEnv::Salvage: Database salvage failed with result %d.\n", result);
        return false;
    }

    // Format of bdb dump is ascii lines:
    // header lines...
    // HEADER=END
    // hexadecimal key
    // hexadecimal value
    // ... repeated
    // DATA=END

    string strLine;
    while (!strDump.eof() && strLine != "HEADER=END") {
    	getline(strDump, strLine); // Skip past header
    }

    std::string keyHex, valueHex;
    while (!strDump.eof() && keyHex != "DATA=END") {
        getline(strDump, keyHex);
        if (keyHex != "DATA_END") {
            getline(strDump, valueHex);
            vecResult.push_back(make_pair(ParseHex(keyHex), ParseHex(valueHex)));
        }
    }

    return (result == 0);
}

void CDBEnv::CheckpointLSN(const std::string& strFile) {
	m_pDbEnv->txn_checkpoint(0, 0, 0);
    if (m_bMockDb) {
    	return;
    }

    m_pDbEnv->lsn_reset(strFile.c_str(), 0);
}

CDB::CDB(const std::string& strFilename, const char* pszMode, bool bFlushOnCloseIn) : m_pdb(NULL), m_pActiveTxn(NULL) {
    int ret;
    m_bReadOnly = (!strchr(pszMode, '+') && !strchr(pszMode, 'w'));
    m_bFlushOnClose = bFlushOnCloseIn;
    if (strFilename.empty()) {
    	 return;
    }

    bool fCreate = strchr(pszMode, 'c') != NULL;
    unsigned int nFlags = DB_THREAD;
    if (fCreate) {
    	nFlags |= DB_CREATE;
    }

    {
        LOCK(g_cDacrsDbEnv.m_cs_Db);
        if (!g_cDacrsDbEnv.Open(GetDataDir())) {
        	throw runtime_error("CDB: Failed to open database environment.");
        }

        m_strFile = strFilename;
        ++g_cDacrsDbEnv.m_mapFileUseCount[m_strFile];
        m_pdb = g_cDacrsDbEnv.m_mapDb[m_strFile];
        if (m_pdb == NULL) {
            m_pdb = new Db(g_cDacrsDbEnv.m_pDbEnv, 0);

            bool fMockDb = g_cDacrsDbEnv.IsMock();
            if (fMockDb) {
                DbMpoolFile* mpf = m_pdb->get_mpf();
                ret = mpf->set_flags(DB_MPOOL_NOFILE, 1);
                if (ret != 0) {
                	throw runtime_error(strprintf("CDB: Failed to configure for no temp file backing for database %s", m_strFile));
                }
            }

            ret = m_pdb->open(NULL,                               	// Txn pointer
                            fMockDb ? NULL : m_strFile.c_str(),   	// Filename
                            fMockDb ? m_strFile.c_str() : "main", 	// Logical db name
                            DB_BTREE,                           	// Database type
                            nFlags,                             	// Flags
                            0);

            if (ret != 0) {
                delete m_pdb;
                m_pdb = NULL;
                --g_cDacrsDbEnv.m_mapFileUseCount[m_strFile];
                m_strFile = "";
                throw runtime_error(strprintf("CDB: Error %d, can't open database %s", ret, m_strFile));
            }

            if (fCreate && !Exists(string("version"))) {
                bool fTmp = m_bReadOnly;
                m_bReadOnly = false;
                WriteVersion(CLIENT_VERSION);
                m_bReadOnly = fTmp;
            }

            g_cDacrsDbEnv.m_mapDb[m_strFile] = m_pdb;
        }
    }
}

void CDB::Flush() {
    if (m_pActiveTxn) {
    	return;
    }

    // Flush database activity from memory pool to disk log
    unsigned int nMinutes = 0;
    if (m_bReadOnly) {
    	nMinutes = 1;
    }

    g_cDacrsDbEnv.m_pDbEnv->txn_checkpoint(nMinutes ? SysCfg().GetArg("-dblogsize", 100) * 1024 : 0, nMinutes, 0);
}

void CDB::Close() {
    if (!m_pdb) {
    	 return;
    }
    if (m_pActiveTxn) {
    	m_pActiveTxn->abort();
    }
    m_pActiveTxn = NULL;
    m_pdb = NULL;

    if (m_bFlushOnClose) {
    	Flush();
    }

    {
        LOCK(g_cDacrsDbEnv.m_cs_Db);
        --g_cDacrsDbEnv.m_mapFileUseCount[m_strFile];
    }
}

void CDBEnv::CloseDb(const string& strFile) {
    {
        LOCK(m_cs_Db);
        if (m_mapDb[strFile] != NULL) {
            // Close the database handle
            Db* pdb = m_mapDb[strFile];
            pdb->close(0);
            delete pdb;
            m_mapDb[strFile] = NULL;
        }
    }
}

bool CDBEnv::RemoveDb(const string& strFile) {
    this->CloseDb(strFile);

    LOCK(m_cs_Db);
    int rc = m_pDbEnv->dbremove(NULL, strFile.c_str(), NULL, DB_AUTO_COMMIT);
    return (rc == 0);
}

bool CDB::Rewrite(const string& strFile, const char* pszSkip) {
    while (true) {
        {
            LOCK(g_cDacrsDbEnv.m_cs_Db);
            if (!g_cDacrsDbEnv.m_mapFileUseCount.count(strFile) || g_cDacrsDbEnv.m_mapFileUseCount[strFile] == 0) {
                // Flush log data to the dat file
                g_cDacrsDbEnv.CloseDb(strFile);
                g_cDacrsDbEnv.CheckpointLSN(strFile);
                g_cDacrsDbEnv.m_mapFileUseCount.erase(strFile);

                bool fSuccess = true;
                LogPrint("CDB","CDB::Rewrite: Rewriting %s...\n", strFile);
                string strFileRes = strFile + ".rewrite";
                { // surround usage of db with extra {}
                    CDB db(strFile.c_str(), "r");
                    Db* pdbCopy = new Db(g_cDacrsDbEnv.m_pDbEnv, 0);

                    int ret = pdbCopy->open(NULL,               // Txn pointer
                                            strFileRes.c_str(), // Filename
                                            "main",             // Logical db name
                                            DB_BTREE,           // Database type
                                            DB_CREATE,          // Flags
                                            0);
                    if (ret > 0) {
                        LogPrint("CDB","CDB::Rewrite: Can't create database file %s\n", strFileRes);
                        fSuccess = false;
                    }

                    Dbc* pcursor = db.GetCursor();
                    if (pcursor)
                        while (fSuccess) {
                            CDataStream ssKey(SER_DISK, CLIENT_VERSION);
                            CDataStream ssValue(SER_DISK, CLIENT_VERSION);
                            int ret = db.ReadAtCursor(pcursor, ssKey, ssValue, DB_NEXT);
                            if (ret == DB_NOTFOUND) {
                                pcursor->close();
                                break;
                            } else if (ret != 0) {
                                pcursor->close();
                                fSuccess = false;
                                break;
                            }
                            if (pszSkip &&
                                strncmp(&ssKey[0], pszSkip, std::min(ssKey.size(), strlen(pszSkip))) == 0) {
                            	continue;
                            }
                            if (strncmp(&ssKey[0], "\x07version", 8) == 0) {
                                // Update version:
                                ssValue.clear();
                                ssValue << CLIENT_VERSION;
                            }
                            Dbt datKey(&ssKey[0], ssKey.size());
                            Dbt datValue(&ssValue[0], ssValue.size());
                            int ret2 = pdbCopy->put(NULL, &datKey, &datValue, DB_NOOVERWRITE);
                            if (ret2 > 0) {
                            	fSuccess = false;
                            }
                        }
                    if (fSuccess) {
                        db.Close();
                        g_cDacrsDbEnv.CloseDb(strFile);
                        if (pdbCopy->close(0)) {
                        	fSuccess = false;
                        }
                        delete pdbCopy;
                    }
                }
				if (fSuccess) {
					Db dbA(g_cDacrsDbEnv.m_pDbEnv, 0);
					if (dbA.remove(strFile.c_str(), NULL, 0)) {
						fSuccess = false;
					}
					Db dbB(g_cDacrsDbEnv.m_pDbEnv, 0);
					if (dbB.rename(strFileRes.c_str(), NULL, strFile.c_str(), 0)) {
						fSuccess = false;
					}
				}
                if (!fSuccess) {
                	LogPrint("CDB","CDB::Rewrite: Failed to rewrite database file %s\n", strFileRes);
                }
                return fSuccess;
            }
        }
        MilliSleep(100);
    }
    return false;
}

void CDBEnv::Flush(bool fShutdown) {
    int64_t nStart = GetTimeMillis();
    // Flush log data to the actual data file on all files that are not in use
    LogPrint("db", "CDBEnv::Flush: Flush(%s)%s\n", fShutdown ? "true" : "false", m_bDbEnvInit ? "" : " database not started");
    if (!m_bDbEnvInit) {
    	return;
    }

    {
        LOCK(m_cs_Db);
        map<string, int>::iterator mi = m_mapFileUseCount.begin();
        while (mi != m_mapFileUseCount.end()) {
            string strFile = (*mi).first;
            int nRefCount = (*mi).second;
            LogPrint("db", "CDBEnv::Flush: Flushing %s (refcount = %d)...\n", strFile, nRefCount);
            if (nRefCount == 0) {
                // Move log data to the dat file
                CloseDb(strFile);
                LogPrint("db", "CDBEnv::Flush: %s checkpoint\n", strFile);
                m_pDbEnv->txn_checkpoint(0, 0, 0);
                LogPrint("db", "CDBEnv::Flush: %s detach\n", strFile);
                if (!m_bMockDb) {
                	m_pDbEnv->lsn_reset(strFile.c_str(), 0);
                }
                LogPrint("db", "CDBEnv::Flush: %s closed\n", strFile);
                m_mapFileUseCount.erase(mi++);
            } else {
            	mi++;
            }
        }
        LogPrint("db", "CDBEnv::Flush: Flush(%s)%s took %15dms\n", fShutdown ? "true" : "false", m_bDbEnvInit ? "" : " database not started", GetTimeMillis() - nStart);
        if (fShutdown) {
            char** listp;
            if (m_mapFileUseCount.empty()) {
            	m_pDbEnv->log_archive(&listp, DB_ARCH_REMOVE);
                Close();
                if (!m_bMockDb) {
                	boost::filesystem::remove_all(m_path / "database");
                }
            }
        }
    }
}
