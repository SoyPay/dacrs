// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "walletdb.h"

#include "base58.h"
#include "protocol.h"
#include "serialize.h"
#include "sync.h"
#include "wallet.h"
#include "tx.h"

#include <fstream>
#include <algorithm>
#include <iterator>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost;


//
// CWalletDB
//






bool ReadKeyValue(CWallet* pwallet, CDataStream& ssKey, CDataStream& ssValue,string& strType, string& strErr)
{
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
            if(pBaseTx->GetHash() == hash)
            {
            	pwallet->UnConfirmTx[hash]=pBaseTx->GetNewInstance();
            }else {
            strErr = "Error reading wallet database: tx corrupt";
            return false;
            }

        }
        else if (strType == "keystore") {
			CKeyStoreValue vKeyStore;
			CKeyID cKeyid;
			ssKey >> cKeyid;
			ssValue >> vKeyStore;
			if(vKeyStore.SelfCheck() != true ||cKeyid != vKeyStore.GetCKeyID())
			{
				strErr = "Error reading wallet database: keystore corrupt";
				return false;
			}
			pwallet->mKeyPool[cKeyid]= std::move(vKeyStore);

        }
        else if (strType == "blocktx") {
        	 uint256 hash;
        	 CAccountTx atx;
   			 CKeyID cKeyid;
   			 ssKey >> hash;
   			 ssValue >> atx;
   			 pwallet->mapInBlockTx[hash] = std::move(atx);
           }
        else if (strType == "defaultkey")
        {
            ssValue >> pwallet->vchDefaultKey;
        }
        else
        {
        	assert(0);
        }
    } catch (...)
    {
        return false;
    }
    return true;
}



DBErrors CWalletDB::LoadWallet(CWallet* pwallet)
{
    pwallet->vchDefaultKey = CPubKey();
    bool fNoncriticalErrors = false;
    DBErrors result = DB_LOAD_OK;

    try {
        LOCK(pwallet->cs_wallet);
    	leveldb::Iterator *pcursor = db.NewIterator();
    	// Load mapBlockIndex
       	pcursor->SeekToFirst();
    	while (pcursor->Valid()) {
    		boost::this_thread::interruption_point();
    		try {
    			leveldb::Slice slKey = pcursor->key();
    			CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, CLIENT_VERSION);
    			leveldb::Slice slValue = pcursor->value();
    			CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
      		     // Try to be tolerant of single corrupt records:
       		      string strType, strErr;
    			if(!ReadKeyValue(pwallet,ssKey,ssValue,strType,strErr))
    				{
    				   if (strType == "keystore")
    					 	delete pcursor;
    				    return  DB_CORRUPT;
    				};
    			pcursor->Next();
    			}
    	 catch (std::exception &e) {
    			delete pcursor;
    			 ERROR("%s : Deserialize or I/O error - %s", __func__, e.what());
    			 return DB_CORRUPT;
    		}
    	}
    	delete pcursor;

    }
    catch (boost::thread_interrupted) {
        throw;
    }
    catch (...) {
        result = DB_CORRUPT;
    }
    return result;

}
bool CWalletDB::WriteBlockTx(const uint256 &hash, const CAccountTx& atx)
{
	nWalletDBUpdated++;
	return db.Write(make_pair(string("blocktx"), hash), atx);
}
bool CWalletDB::EraseBlockTx(const uint256 &hash)
{
	nWalletDBUpdated++;
	return db.Erase(make_pair(string("blocktx"), hash));
}

bool CWalletDB::WriteKeyStoreValue(const CKeyID &keyId, const CKeyStoreValue& KeyStoreValue)
{
	nWalletDBUpdated++;
	return db.Write(make_pair(string("keystore"), keyId), KeyStoreValue,true);
}

bool CWalletDB::EraseKeyStoreValue(const CKeyID& keyId) {
	nWalletDBUpdated++;
	return db.Erase(make_pair(string("keystore"), keyId),true);
}




bool CWalletDB::WriteUnComFirmedTx(const uint256& hash, const std::shared_ptr<CBaseTransaction>& tx) {
	nWalletDBUpdated++;
	return db.Write(make_pair(string("tx"), hash),tx);
}


bool CWalletDB::EraseUnComFirmedTx(const uint256& hash) {
	nWalletDBUpdated++;
	return db.Erase(make_pair(string("tx"), hash));
}

bool CWalletDB::WriteMasterKey(const CMasterKey& kMasterKey)
{
    nWalletDBUpdated++;
    return db.Write(string("mkey"), kMasterKey, true);
}

bool CWalletDB::EraseMasterKey() {
    nWalletDBUpdated++;
    return db.Erase(string("mkey"),  true);
}

CWalletDB::CWalletDB(const string& strFilename):db(GetDataDir() / strFilename, 1024*15, false, false) {
	nWalletDBUpdated = 0 ;

}
