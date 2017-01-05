// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_WALLET_DB_H_
#define DACRS_WALLET_DB_H_

#include <map>
#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>

#include <db_cxx.h>

#include "clientversion.h"
#include "serialize.h"
//#include "streams.h"
#include "sync.h"
#include "version.h"

class CDiskBlockIndex;
class COutPoint;

void ThreadFlushWalletDB(const std::string& kstrWalletFile);

class CDBEnv {
 public:
    CDBEnv();
    ~CDBEnv();
    void Reset();
    void MakeMock();
    bool IsMock() { return m_bMockDb; }

    /**
     * Verify that database file strFile is OK. If it is not,
     * call the callback to try to recover.
     * This must be called BEFORE strFile is opened.
     * Returns true if strFile is OK.
     */
    enum m_emVerifyResult {
    	EM_VERIFY_OK,
    	EM_RECOVER_OK,
        EM_RECOVER_FAIL
    };
    m_emVerifyResult Verify(std::string strFile, bool (*recoverFunc)(CDBEnv& dbenv, std::string strFile));
    /**
     * Salvage data from a file that Verify says is bad.
     * fAggressive sets the DB_AGGRESSIVE flag (see berkeley DB->verify() method documentation).
     * Appends binary key/value pairs to vResult, returns true if successful.
     * NOTE: reads the entire database into memory, so cannot be used
     * for huge databases.
     */
    typedef std::pair<std::vector<unsigned char>, std::vector<unsigned char> > KeyValPair;
    bool Salvage(std::string strFile, bool bAggressive, std::vector<KeyValPair>& vecResult);

    bool Open(const boost::filesystem::path& path);
    void Close();
    void Flush(bool bShutdown);
    void CheckpointLSN(const std::string& strFile);

    void CloseDb(const std::string& strFile);
    bool RemoveDb(const std::string& strFile);

    DbTxn* TxnBegin(int flags = DB_TXN_WRITE_NOSYNC) {
        DbTxn* ptxn = NULL;
        int nRet = m_pDbEnv->txn_begin(NULL, &ptxn, flags);
        if (!ptxn || nRet != 0) {
            return NULL;
        }
        return ptxn;
    }

    mutable CCriticalSection m_cs_Db;
    DbEnv *m_pDbEnv;
    std::map<std::string, int> m_mapFileUseCount;
    std::map<std::string, Db*> m_mapDb;

 private:
    void EnvShutdown();

    bool m_bDbEnvInit;
    bool m_bMockDb;
    boost::filesystem::path m_path;
};

extern CDBEnv g_cDacrsDbEnv;

/** RAII class that provides access to a Berkeley database */
class CDB {
 public:
    void Flush();
    void Close();

 public:
     bool TxnBegin() {
         if (!m_pdb || m_pActiveTxn) {
        	 return false;
         }
         DbTxn* ptxn = g_cDacrsDbEnv.TxnBegin();
         if (!ptxn) {
        	 return false;
         }
         m_pActiveTxn = ptxn;
         return true;
     }

     bool TxnCommit() {
         if (!m_pdb || !m_pActiveTxn) {
        	 return false;
         }
         int nRet = m_pActiveTxn->commit(0);
         m_pActiveTxn = NULL;
         return (nRet == 0);
     }

     bool TxnAbort() {
         if (!m_pdb || !m_pActiveTxn) {
        	 return false;
         }
         int nRet = m_pActiveTxn->abort();
         m_pActiveTxn = NULL;
         return (nRet == 0);
     }

     bool ReadVersion(int& nVersion) {
         nVersion = 0;
         return Read(std::string("version"), nVersion);
     }

     bool WriteVersion(int nVersion) {
         return Write(std::string("version"), nVersion);
     }

     bool static Rewrite(const std::string& strFile, const char* pszSkip = NULL);

 protected:
    explicit CDB(const std::string& strFilename, const char* pszMode = "r+", bool bFlushOnCloseIn=true);
    ~CDB() { Close(); }

    Db* m_pdb;
    std::string m_strFile;
    DbTxn* m_pActiveTxn;
    bool m_bReadOnly;
    bool m_bFlushOnClose;

 protected:
    template <typename K, typename T>
    bool Read(const K& key, T& value, int nVersion = g_sClientVersion) {
        if (!m_pdb) {
        	return false;
        }

        // Key
        CDataStream ssKey(SER_DISK, g_sClientVersion);
        ssKey.reserve(1000);
        ssKey << key;
        Dbt cDatKey(&ssKey[0], ssKey.size());

        // Read
        Dbt cDatValue;
        cDatValue.set_flags(DB_DBT_MALLOC);
        int nRet = m_pdb->get(m_pActiveTxn, &cDatKey, &cDatValue, 0);
        memset(cDatKey.get_data(), 0, cDatKey.get_size());
        if (cDatValue.get_data() == NULL) {
        	return false;
        }

        // Unserialize value
        try {
            CDataStream ssValue((char*)cDatValue.get_data(), (char*)cDatValue.get_data() + cDatValue.get_size(), SER_DISK, nVersion);
            ssValue >> value;
        } catch (const std::exception&) {
            return false;
        }

        // Clear and free memory
        memset(cDatValue.get_data(), 0, cDatValue.get_size());
        free(cDatValue.get_data());
        return (nRet == 0);
    }

    template <typename K, typename T>
    bool Write(const K& key, const T& value, bool fOverwrite = true, int nVersion = g_sClientVersion) {
        if (!m_pdb) {
        	return false;
        }
        if (m_bReadOnly) {
        	assert(!"Write called on database in read-only mode");
        }

        // Key
        CDataStream cSsKey(SER_DISK, g_sClientVersion);
        cSsKey.reserve(1000);
        cSsKey << key;
        Dbt cDatKey(&cSsKey[0], cSsKey.size());

        // Value
        CDataStream cSsValue(SER_DISK, nVersion);
        cSsValue.reserve(10000);
        cSsValue << value;
        Dbt cDatValue(&cSsValue[0], cSsValue.size());

        // Write
        int nRet = m_pdb->put(m_pActiveTxn, &cDatKey, &cDatValue, (fOverwrite ? 0 : DB_NOOVERWRITE));

        // Clear memory in case it was a private key
        memset(cDatKey.get_data(), 0, cDatKey.get_size());
        memset(cDatValue.get_data(), 0, cDatValue.get_size());
        return (nRet == 0);
    }

    template <typename K>
    bool Erase(const K& key) {
        if (!m_pdb) {
        	return false;
        }
        if (m_bReadOnly) {
        	assert(!"Erase called on database in read-only mode");
        }

        // Key
        CDataStream ssKey(SER_DISK, g_sClientVersion);
        ssKey.reserve(1000);
        ssKey << key;
        Dbt cDatKey(&ssKey[0], ssKey.size());

        // Erase
        int nRet = m_pdb->del(m_pActiveTxn, &cDatKey, 0);

        // Clear memory
        memset(cDatKey.get_data(), 0, cDatKey.get_size());
        return (nRet == 0 || nRet == DB_NOTFOUND);
    }

    template <typename K>
    bool Exists(const K& key) {
        if (!m_pdb) {
            return false;
        }

        // Key
        CDataStream ssKey(SER_DISK, g_sClientVersion);
        ssKey.reserve(1000);
        ssKey << key;
        Dbt cDatKey(&ssKey[0], ssKey.size());

        // Exists
        int nRet = m_pdb->exists(m_pActiveTxn, &cDatKey, 0);

        // Clear memory
        memset(cDatKey.get_data(), 0, cDatKey.get_size());
        return (nRet == 0);
    }

    Dbc* GetCursor() {
        if (!m_pdb) {
        	return NULL;
        }

        Dbc* pcursor = NULL;
        int nRet = m_pdb->cursor(NULL, &pcursor, 0);
        if (nRet != 0) {
        	 return NULL;
        }

        return pcursor;
    }

    int ReadAtCursor(Dbc* pcursor, CDataStream& ssKey, CDataStream& ssValue, unsigned int fFlags = DB_NEXT) {
        // Read at cursor
        Dbt cDatKey;
        if (fFlags == DB_SET || fFlags == DB_SET_RANGE || fFlags == DB_GET_BOTH || fFlags == DB_GET_BOTH_RANGE) {
            cDatKey.set_data(&ssKey[0]);
            cDatKey.set_size(ssKey.size());
        }
        Dbt cDatValue;
        if (fFlags == DB_GET_BOTH || fFlags == DB_GET_BOTH_RANGE) {
            cDatValue.set_data(&ssValue[0]);
            cDatValue.set_size(ssValue.size());
        }
        cDatKey.set_flags(DB_DBT_MALLOC);
        cDatValue.set_flags(DB_DBT_MALLOC);
        int nRet = pcursor->get(&cDatKey, &cDatValue, fFlags);
        if (nRet != 0) {
        	return nRet;
        }
        else if (cDatKey.get_data() == NULL || cDatValue.get_data() == NULL) {
        	return 99999;
        }

        // Convert to streams
        ssKey.SetType(SER_DISK);
        ssKey.clear();
        ssKey.write((char*)cDatKey.get_data(), cDatKey.get_size());
        ssValue.SetType(SER_DISK);
        ssValue.clear();
        ssValue.write((char*)cDatValue.get_data(), cDatValue.get_size());

        // Clear and free memory
        memset(cDatKey.get_data(), 0, cDatKey.get_size());
        memset(cDatValue.get_data(), 0, cDatValue.get_size());
        free(cDatKey.get_data());
        free(cDatValue.get_data());
        return 0;
    }

 private:
     CDB(const CDB&);
     void operator=(const CDB&);
};

#endif // DACRS_WALLET_DB_H_
