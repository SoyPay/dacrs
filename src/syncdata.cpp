/*
 * CSyncData.cpp
 *
 *  Created on: Jun 14, 2014
 *      Author: ranger.shi
 */

#include "syncdata.h"

namespace SyncData {

CSyncData::CSyncData()
{
}

CSyncData::~CSyncData()
{
}

bool CSyncData::CheckSignature(const std::string& pubKey)
{
	CPubKey key(ParseHex(pubKey));
    if (!key.Verify(Hash(m_vchMsg.begin(), m_vchMsg.end()), m_vchSig))
    {
        return ERRORMSG("CSyncCheckpoint::CheckSignature : verify signature failed");
    }
    return true;
}

bool CSyncData::Sign(const std::vector<unsigned char>& priKey, const std::vector<unsigned char>& syncData)
{
    CKey key;
	m_vchMsg.assign(syncData.begin(), syncData.end());
    key.Set(priKey.begin(), priKey.end(), true);
    if (!key.Sign(Hash(m_vchMsg.begin(), m_vchMsg.end()), m_vchSig))
    {
    	return ERRORMSG("CSyncCheckpoint::Sign: Unable to sign checkpoint, check private key?");
    }
	return true;
}

} /* namespace SyncData */
