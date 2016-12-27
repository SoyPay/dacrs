// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#define  BOOST_NO_CXX11_SCOPED_ENUMS
#include "walletdb.h"

#include <fstream>
#include <algorithm>
#include <iterator>
#include <boost/filesystem.hpp>

#include "base58.h"
#include "protocol.h"
#include "serialize.h"
#include "sync.h"
#include "wallet.h"
#include "tx.h"

using namespace std;
using namespace boost;

//
// CWalletDB
//

unsigned int CWalletDB::g_unWalletDBUpdated = 0;

bool ReadKeyValue(CWallet* pwallet, CDataStream& ssKey, CDataStream& ssValue,string& strType, string& strErr, int MinVersion) {
	try {
		// Unserialize
		// Taking advantage of the fact that pair serialization
		// is just the two items serialized one after the other
		ssKey >> strType;

		if (strType == "tx") {
			uint256 hash;
			ssKey >> hash;
			std::shared_ptr<CBaseTransaction> pBaseTx; //= make_shared<CContractTransaction>();
			ssValue >> pBaseTx;
			if (pBaseTx->GetHash() == hash) {
				if (pwallet != NULL) {
					pwallet->m_mapUnConfirmTx[hash] = pBaseTx->GetNewInstance();
				}
			} else {
				strErr = "Error reading wallet database: tx corrupt";
				return false;
			}

		} else if (strType == "keystore") {
			if(-1 != MinVersion) {
				ssKey.SetVersion(MinVersion);
				ssValue.SetVersion(MinVersion);
			}
			CKeyCombi keyCombi;
			CKeyID cKeyid;
			ssKey >> cKeyid;
			ssValue >> keyCombi;
			if(keyCombi.IsContainMainKey()) {
				if (cKeyid != keyCombi.GetCKeyID()) {
					strErr = "Error reading wallet database: keystore corrupt";
					return false;
				}
			}
			if (pwallet != NULL) {
				pwallet->LoadKeyCombi(cKeyid, keyCombi);
			}
		} else if (strType == "ckey") {
			CPubKey pubKey;
			std::vector<unsigned char> vchCryptedSecret;
			ssKey >> pubKey;
			ssValue >> vchCryptedSecret;
			if(pwallet != NULL) {
				pwallet->LoadCryptedKey(pubKey, vchCryptedSecret);
			}
		} else if (strType == "mkey") {
			unsigned int ID;
			CMasterKey kMasterKey;
			ssKey >> ID;
			ssValue >> kMasterKey;
			if(pwallet != NULL) {
				pwallet->m_unMasterKeyMaxID = ID;
				pwallet->m_mapMasterKeys.insert(make_pair(ID, kMasterKey));
			}
		}
		else if (strType == "blocktx") {
			uint256 hash;
			CAccountTx atx;
			CKeyID cKeyid;
			ssKey >> hash;
			ssValue >> atx;
			if (pwallet != NULL) {
				pwallet->m_mapInBlockTx[hash] = std::move(atx);
			}
		} else if (strType == "defaultkey") {
			if (pwallet != NULL) {
				ssValue >> pwallet->m_vchDefaultKey;
			}
		} else if (strType != "version" && "minversion" != strType) {
			ERRORMSG("load wallet error! read invalid key type:%s\n", strType);
			//assert(0);
		}
	} catch (...) {
		return false;
	}

	return true;
}

emDBErrors CWalletDB::LoadWallet(CWallet* pwallet) {
	pwallet->m_vchDefaultKey = CPubKey();
	bool bNoncriticalErrors = false;
	emDBErrors result = EM_DB_LOAD_OK;

	try {
			LOCK(pwallet->m_cs_wallet);
	        int nMinVersion = 0;
			if (Read((string)"minversion", nMinVersion)) {
				 if (nMinVersion > CLIENT_VERSION) {
					 return EM_DB_TOO_NEW;
				 }
				 pwallet->LoadMinVersion(nMinVersion);
			}

	        // Get cursor
	        Dbc* pcursor = GetCursor();
	        if (!pcursor) {
	            LogPrint("INFO","Error getting wallet database cursor\n");
	            return EM_DB_CORRUPT;
	        }

	        while (true) {
	            // Read next record
	            CDataStream ssKey(SER_DISK, CLIENT_VERSION);
	            CDataStream ssValue(SER_DISK, CLIENT_VERSION);
	            int ret = ReadAtCursor(pcursor, ssKey, ssValue);
	            if (ret == DB_NOTFOUND) {
	            	break;
	            }
	            else if (ret != 0) {
	                LogPrint("INFO","Error reading next record from wallet database\n");
	                return EM_DB_CORRUPT;
	            }

	            // Try to be tolerant of single corrupt records:
	            string strType, strErr;
	            if (!ReadKeyValue(pwallet, ssKey, ssValue, strType, strErr, GetMinVersion())) {
	                // losing keys is considered a catastrophic error, anything else
	                // we assume the user can live with:
	            	// if (IsKeyType(strType))
//	                    result = DB_CORRUPT;
				if (strType == "keystore") {
					return EM_DB_CORRUPT;
				} else {
	                	// Leave other errors alone, if we try to fix them we might make things worse.
						bNoncriticalErrors = true; // ... but do warn the user there is something wrong.
//	                    if (strType == "acctx")
//	                        // Rescan if there is a bad transaction record:
//	                        SoftSetBoolArg("-rescan", true);
	                }
	            }
	            if (!strErr.empty()) {
	            	LogPrint("INFO","%s\n", strErr);
	            }
	        }
	        pcursor->close();
	    }
	    catch (boost::thread_interrupted) {
	        throw;
	    }
	    catch (...) {
	        result = EM_DB_CORRUPT;
	    }

	    if (bNoncriticalErrors && result == EM_DB_LOAD_OK) {
	    	result = EM_DB_NONCRITICAL_ERROR;
	    }

	    // Any wallet corruption at all: skip any rewriting or
	    // upgrading, we don't want to make it worse.
	    if (result != EM_DB_LOAD_OK) {
	    	return result;
	    }

	    LogPrint("INFO","nFileVersion = %d\n", GetMinVersion());

//	    // nTimeFirstKey is only reliable if all keys have metadata
//	    if ((wss.nKeys + wss.nCKeys) != wss.nKeyMeta)
//	        pwallet->nTimeFirstKey = 1; // 0 would be considered 'no value'
//
//	    for (auto hash : wss.vWalletUpgrade)
//	        WriteAccountTx(hash, pwallet->mapWalletTx[hash]);

//	    // Rewrite encrypted wallets of versions 0.4.0 and 0.5.0rc:
//	    if (wss.fIsEncrypted && (wss.nFileVersion == 40000 || wss.nFileVersion == 50000))
//	        return DB_NEED_REWRITE;

	    if ( GetMinVersion()< CLIENT_VERSION) {
	    	WriteVersion( GetMinVersion());
	    }

	    if (pwallet->IsEmpty()) {
	    	CKey mCkey;
	    	mCkey.MakeNewKey();
	    	if (!pwallet->AddKey(mCkey)) {
			throw runtime_error("add key failed ");
	    	}
	    }

	    return result;
}


//
// Try to (very carefully!) recover wallet.dat if there is a problem.
//
bool CWalletDB::Recover(CDBEnv& dbenv, string filename, bool fOnlyKeys) {
    // Recovery procedure:
    // move wallet.dat to wallet.timestamp.bak
    // Call Salvage with fAggressive=true to
    // get as much data as possible.
    // Rewrite salvaged data to wallet.dat
    // Set -rescan so any missing transactions will be
    // found.
    int64_t now = GetTime();
    string newFilename = strprintf("wallet.%d.bak", now);

    int nResult = dbenv.m_pDbEnv->dbrename(NULL, filename.c_str(), NULL,
                                      newFilename.c_str(), DB_AUTO_COMMIT);
    if (nResult == 0) {
    	LogPrint("INFO","Renamed %s to %s\n", filename, newFilename);
    } else {
        LogPrint("INFO","Failed to rename %s to %s\n", filename, newFilename);
        return false;
    }

    vector<CDBEnv::KeyValPair> salvagedData;
    bool bAllOK = dbenv.Salvage(newFilename, true, salvagedData);
    if (salvagedData.empty()) {
        LogPrint("INFO","Salvage(aggressive) found no records in %s.\n", newFilename);
        return false;
    }
    LogPrint("INFO","Salvage(aggressive) found %u records\n", salvagedData.size());

    bool bSuccess = bAllOK;
    boost::scoped_ptr<Db> pdbCopy(new Db(dbenv.m_pDbEnv, 0));
    int ret = pdbCopy->open(NULL,               // Txn pointer
                            filename.c_str(),   // Filename
                            "main",             // Logical db name
                            DB_BTREE,           // Database type
                            DB_CREATE,          // Flags
                            0);
    if (ret > 0) {
        LogPrint("INFO","Cannot create database file %s\n", filename);
        return false;
    }
    DbTxn* ptxn = dbenv.TxnBegin();
    for (auto& row : salvagedData) {
        if (fOnlyKeys) {
            CDataStream ssKey(row.first, SER_DISK, CLIENT_VERSION);
            CDataStream ssValue(row.second, SER_DISK, CLIENT_VERSION);
            string strType, strErr;
            bool fReadOK = ReadKeyValue(NULL, ssKey, ssValue, strType, strErr, -1);
            if (strType != "keystore") {
            	continue;
            }
            if (!fReadOK) {
            	LogPrint("INFO","WARNING: CWalletDB::Recover skipping %s: %s\n", strType, strErr);
            	                continue;
            }
        }
        Dbt datKey(&row.first[0], row.first.size());
        Dbt datValue(&row.second[0], row.second.size());
        int ret2 = pdbCopy->put(ptxn, &datKey, &datValue, DB_NOOVERWRITE);
        if (ret2 > 0) {
        	bSuccess = false;
        }
    }
    ptxn->commit(0);
    pdbCopy->close(0);

    return bSuccess;
}

bool CWalletDB::Recover(CDBEnv& dbenv, string filename) {
    return CWalletDB::Recover(dbenv, filename, false);
}

bool CWalletDB::WriteBlockTx(const uint256 &hash, const CAccountTx& atx) {
	g_unWalletDBUpdated++;
	return Write(make_pair(string("blocktx"), hash), atx);
}

bool CWalletDB::EraseBlockTx(const uint256 &hash) {
	g_unWalletDBUpdated++;
	return Erase(make_pair(string("blocktx"), hash));
}

bool CWalletDB::WriteKeyStoreValue(const CKeyID &keyId, const CKeyCombi& KeyCombi, int nVersion) {
	g_unWalletDBUpdated++;
	//	cout << "keystore:" << "Keyid=" << keyId.ToAddress() << " KeyCombi:" << KeyCombi.ToString() << endl;
	return Write(make_pair(string("keystore"), keyId), KeyCombi, true, nVersion);
}

bool CWalletDB::EraseKeyStoreValue(const CKeyID& keyId) {
	g_unWalletDBUpdated++;
	return Erase(make_pair(string("keystore"), keyId));
}

bool CWalletDB::WriteCryptedKey(const CPubKey& pubkey, const std::vector<unsigned char>& vchCryptedSecret) {
	g_unWalletDBUpdated++;
	if (!Write(std::make_pair(std::string("ckey"), pubkey), vchCryptedSecret, true)) {
		return false;
	}
	CKeyCombi keyCombi;
	CKeyID keyId = pubkey.GetKeyID();
	int nVersion(0);
	Read(string("minversion"), nVersion);
	if(Read(make_pair(string("keystore"), keyId), keyCombi, nVersion))
	{
		keyCombi.CleanMainKey();
		if(!Write(make_pair(string("keystore"), keyId), keyCombi, true)) {
			return false;
		}
	}

	return true;
}

bool CWalletDB::WriteUnComFirmedTx(const uint256& hash, const std::shared_ptr<CBaseTransaction>& tx) {
	g_unWalletDBUpdated++;
	return Write(make_pair(string("tx"), hash),tx);
}
bool CWalletDB::EraseUnComFirmedTx(const uint256& hash) {
	g_unWalletDBUpdated++;
	return Erase(make_pair(string("tx"), hash));
}

bool CWalletDB::WriteVersion(const int version) {
	g_unWalletDBUpdated++;
	return Write(string("version"), version);
}

int CWalletDB::GetVersion(void) {
	int verion;
	return Read(string("version"),verion);
}

bool CWalletDB::WriteMinVersion(const int version) {
	g_unWalletDBUpdated++;
	return Write(string("minversion"), version);
}

int CWalletDB::GetMinVersion(void) {
	int verion;
	return Read(string("minversion"),verion);
}

bool CWalletDB::WriteMasterKey(unsigned int nID, const CMasterKey& kMasterKey) {
    g_unWalletDBUpdated++;
    return Write(std::make_pair(std::string("mkey"), nID), kMasterKey, true);
}

bool CWalletDB::EraseMasterKey(unsigned int nID) {
    g_unWalletDBUpdated++;
    return Erase(std::make_pair(std::string("mkey"), nID));
}

//extern CDBEnv bitdb;
void ThreadFlushWalletDB(const string& kstrWalletFile) {
    // Make this thread recognisable as the wallet flushing thread
    RenameThread("dacrs-wallet");
    static bool fOneThread;
    if (fOneThread) {
    	return;
    }
    fOneThread = true;
    if (!SysCfg().GetBoolArg("-flushwallet", true)) {
    	return;
    }

    unsigned int unLastSeen = CWalletDB::g_unWalletDBUpdated;
    unsigned int unLastFlushed = CWalletDB::g_unWalletDBUpdated;
    int64_t llLastWalletUpdate = GetTime();
    while (true) {
        MilliSleep(500);

        if (unLastSeen != CWalletDB::g_unWalletDBUpdated) {
            unLastSeen = CWalletDB::g_unWalletDBUpdated;
            llLastWalletUpdate = GetTime();
        }
        if (unLastFlushed != CWalletDB::g_unWalletDBUpdated && GetTime() - llLastWalletUpdate >= 2) {
            TRY_LOCK(g_cDacrsDbEnv.m_cs_Db,lockDb);
            if (lockDb) {
                // Don't do this if any databases are in use
                int nRefCount = 0;
                map<string, int>::iterator iter = g_cDacrsDbEnv.m_mapFileUseCount.begin();
                while (iter != g_cDacrsDbEnv.m_mapFileUseCount.end()) {
                    nRefCount += (*iter).second;
                    iter++;
                }

                if (nRefCount == 0) {
                    boost::this_thread::interruption_point();
                    map<string, int>::iterator iter = g_cDacrsDbEnv.m_mapFileUseCount.find(kstrWalletFile);
                    if (iter != g_cDacrsDbEnv.m_mapFileUseCount.end()) {
                        LogPrint("db", "Flushing wallet.dat\n");
                        unLastFlushed = CWalletDB::g_unWalletDBUpdated;
                        int64_t llStart = GetTimeMillis();

                        // Flush wallet.dat so it's self contained
                        g_cDacrsDbEnv.CloseDb(kstrWalletFile);
                        g_cDacrsDbEnv.CheckpointLSN(kstrWalletFile);

                        g_cDacrsDbEnv.m_mapFileUseCount.erase(iter++);
                        LogPrint("db", "Flushed wallet.dat %dms\n", GetTimeMillis() - llStart);
                    }
                }
            }
        }
    }
}

void ThreadRelayTx(CWallet* pWallet) {
	RenameThread("relay-tx");
	while(pWallet) {
		MilliSleep(60*1000);
		map<uint256, std::shared_ptr<CBaseTransaction> >::iterator iterTx =  pWallet->m_mapUnConfirmTx.begin();
		for ( ; iterTx != pWallet->m_mapUnConfirmTx.end(); ++iterTx) {
			if (mempool.exists(iterTx->first)) {
				RelayTransaction(iterTx->second.get(), iterTx->first);
				LogPrint("sendtx", "ThreadRelayTx resend tx hash:%s time:%ld\n", iterTx->first.GetHex(), GetTime());
			}
		}
	}
}

bool BackupWallet(const CWallet& wallet, const string& strDest) {
    while (true) {
        {
            LOCK(g_cDacrsDbEnv.m_cs_Db);
            if (!g_cDacrsDbEnv.m_mapFileUseCount.count(wallet.m_strWalletFile) || g_cDacrsDbEnv.m_mapFileUseCount[wallet.m_strWalletFile] == 0) {
                // Flush log data to the dat file
                g_cDacrsDbEnv.CloseDb(wallet.m_strWalletFile);
                g_cDacrsDbEnv.CheckpointLSN(wallet.m_strWalletFile);
                g_cDacrsDbEnv.m_mapFileUseCount.erase(wallet.m_strWalletFile);

                // Copy wallet.dat
                boost::filesystem::path pathSrc = GetDataDir() / wallet.m_strWalletFile;
                boost::filesystem::path pathDest(strDest);
                if (boost::filesystem::is_directory(pathDest)) {
                	pathDest /= wallet.m_strWalletFile;
                }
                try {
#if BOOST_VERSION >= 104000
                    boost::filesystem::copy_file(pathSrc, pathDest, boost::filesystem::copy_option::overwrite_if_exists);
#else
                    boost::filesystem::copy_file(pathSrc, pathDest);
#endif
                    LogPrint("INFO", "copied wallet.dat to %s\n", pathDest.string());
                    return true;
                } catch (const boost::filesystem::filesystem_error& e) {
                    LogPrint("ERROR", "error copying wallet.dat to %s - %s\n", pathDest.string(), e.what());
                    return false;
                }
            }
        }
        MilliSleep(100);
    }

    return false;
}
