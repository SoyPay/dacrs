/*
 * CSyncData.cpp
 *
 *  Created on: Jun 14, 2014
 *      Author: ranger.shi
 */

#include "syncdata.h"

namespace SyncData {

CSyncData::CSyncData() {
}

CSyncData::~CSyncData() {
}

bool CSyncData::CheckSignature(const std::string& strPubKey) {
	CPubKey cPubKey(ParseHex(strPubKey));
	LogPrint("CHECKPOINT", "CheckSignature PubKey:%s\n", strPubKey.c_str());
	LogPrint("CHECKPOINT", "CheckSignature Msg:%s\n", HexStr(m_vchMsg).c_str());
	LogPrint("CHECKPOINT", "CheckSignature Sig:%s\n", HexStr(m_vchSig).c_str());
	if (!cPubKey.Verify(Hash(m_vchMsg.begin(), m_vchMsg.end()), m_vchSig)) {
		return ERRORMSG("CSyncCheckpoint::CheckSignature : verify signature failed");
	}
	return true;
}

bool CSyncData::Sign(const std::vector<unsigned char>& vchPriKey, const std::vector<unsigned char>& vchSyncData) {
	CKey cKey;
	m_vchMsg.assign(vchSyncData.begin(), vchSyncData.end());
	cKey.Set(vchPriKey.begin(), vchPriKey.end(), true);
	if (!cKey.Sign(Hash(m_vchMsg.begin(), m_vchMsg.end()), m_vchSig)) {
		return ERRORMSG("CSyncCheckpoint::Sign: Unable to sign checkpoint, check private key?");
	}
	return true;
}

bool CSyncData::Sign(const CKey& cPriKey, const std::vector<unsigned char>& vchSyncData) {
	m_vchMsg.assign(vchSyncData.begin(), vchSyncData.end());
	if (!cPriKey.Sign(Hash(m_vchMsg.begin(), m_vchMsg.end()), m_vchSig)) {
		return ERRORMSG("CSyncCheckpoint::Sign: Unable to sign checkpoint, check private key?");
	}
	return true;
}

json_spirit::Object CSyncData::ToJsonObj() {
	Object obj;
	obj.push_back(Pair("msg", HexStr(m_vchMsg)));
	obj.push_back(Pair("sig", HexStr(m_vchSig)));
	return obj;
}

} /* namespace SyncData */
