// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "keystore.h"
#include "wallet/wallet.h"
#include "crypter.h"
#include "key.h"
#include "base58.h"
using namespace json_spirit;

Object CKeyCombi::ToJsonObj() const {
	Object reply;
	if (m_cMainCkey.IsValid()) {
		reply.push_back(Pair("address", m_cMainCkey.GetPubKey().GetKeyID().ToAddress()));
		reply.push_back(Pair("mCkey", m_cMainCkey.ToString()));
		reply.push_back(Pair("mCkeyBase58", CDacrsSecret(m_cMainCkey).ToString()));
		reply.push_back(Pair("mMainPk", m_cMainCkey.GetPubKey().ToString()));
	}

	if (m_cMinerCkey.IsValid()) {
		reply.push_back(Pair("m_cMinerCkey", m_cMinerCkey.ToString()));
		reply.push_back(Pair("mMinerCkeyBase58", CDacrsSecret(m_cMinerCkey).ToString()));
		reply.push_back(Pair("mMinerPk", m_cMinerCkey.GetPubKey().ToString()));
	}
	reply.push_back(Pair("m_llCreationTime", m_llCreationTime));
	return std::move(reply);
}

bool CKeyCombi::UnSersailFromJson(const Object& obj) {
	try {
		Object reply;
		const Value& mCKey = find_value(obj, "mCkey");
		if (mCKey.type() != json_spirit::null_type) {
			auto const &tem1 = ::ParseHex(mCKey.get_str());
			m_cMainCkey.Set(tem1.begin(), tem1.end(), true);
		}
		const Value& mMinerKey = find_value(obj, "m_cMinerCkey");
		if (mMinerKey.type() != json_spirit::null_type) {
			auto const &tem2 = ::ParseHex(mMinerKey.get_str());
			m_cMinerCkey.Set(tem2.begin(), tem2.end(), true);
		}
		const Value& nTime = find_value(obj, "m_llCreationTime").get_int64();
		if (nTime.type() != json_spirit::null_type) {
			m_llCreationTime = find_value(obj, "m_llCreationTime").get_int64();
		}
	} catch (...) {
		ERRORMSG("UnSersailFromJson Failed !");
		return false;
	}

	return true;
}

bool CKeyCombi::CleanAll() {
	m_cMainCkey.Clear();
	m_cMinerCkey.Clear();
	m_cMainPKey = CPubKey();
	m_cMinerPKey = CPubKey();
	m_llCreationTime = 0;
	return true;
}

bool CKeyCombi::CleanMainKey() {
	return m_cMainCkey.Clear();
}

CKeyCombi::CKeyCombi(const CKey& inkey, int nVersion) {
	assert(inkey.IsValid());
	CleanAll();
	m_cMainCkey = inkey;
	if (EM_FEATURE_BASE == nVersion) {
		m_cMainPKey = m_cMainCkey.GetPubKey();
	}
	m_llCreationTime = GetTime();
}

CKeyCombi::CKeyCombi(const CKey& inkey, const CKey& minerKey, int nVersion) {
	assert(inkey.IsValid());
	assert(minerKey.IsValid());
	CleanAll();
	m_cMinerCkey = minerKey;
	m_cMainCkey = inkey;
	if (EM_FEATURE_BASE == nVersion) {
		m_cMainPKey = m_cMainCkey.GetPubKey();
		m_cMinerPKey = m_cMinerCkey.GetPubKey();
	}
	m_llCreationTime = GetTime();
}

bool CKeyCombi::GetPubKey(CPubKey& mOutKey, bool IsMine) const {
	if (IsMine == true) {
		if (m_cMinerCkey.IsValid()) {
			mOutKey = m_cMinerCkey.GetPubKey();
			return true;
		}
		return false;
	}
	mOutKey = m_cMainCkey.GetPubKey();
	return true;
}

string CKeyCombi::ToString() const {
	string str("");
	if (m_cMainCkey.IsValid()) {
		str += strprintf(" MainPKey:%s MainKey:%s", m_cMainCkey.GetPubKey().ToString(), m_cMainCkey.ToString());
	}

	if (m_cMinerCkey.IsValid()) {
		str += strprintf(" MinerPKey:%s MinerKey:%s",m_cMinerCkey.GetPubKey().ToString(), m_cMinerCkey.ToString());
	}
	str += strprintf(" CreationTime:%d", m_llCreationTime);
	return str;
}

bool CKeyCombi::GetCKey(CKey& keyOut, bool IsMine) const {
	if (IsMine) {
		keyOut = m_cMinerCkey;
	} else {
		keyOut = m_cMainCkey;
	}
	return keyOut.IsValid();
}

bool CKeyCombi::CreateANewKey() {
	CleanAll();
	m_cMainCkey.MakeNewKey();
	m_llCreationTime = GetTime();
	return true;
}

CKeyCombi::CKeyCombi() {
	CleanAll();
}

int64_t CKeyCombi::GetBirthDay() const {
	return m_llCreationTime;
}

CKeyID CKeyCombi::GetCKeyID() const {
	if (m_cMainCkey.IsValid()) {
		return m_cMainCkey.GetPubKey().GetKeyID();
	} else {
		CKeyID cKeyId;
		return cKeyId;
	}
}

void CKeyCombi::SetMainKey(CKey& mainKey) {
	m_cMainCkey = mainKey;
}

void CKeyCombi::SetMinerKey(CKey & minerKey) {
	m_cMinerCkey = minerKey;
}

bool CKeyCombi::IsContainMinerKey() const {
	return m_cMinerCkey.IsValid();
}

bool CKeyCombi::IsContainMainKey() const {
	return m_cMainCkey.IsValid();
}

bool CKeyStore::GetPubKey(const CKeyID &address, CPubKey &vchPubKeyOut, bool IsMine) const
{
    CKey cKey;
	if (!GetKey(address, cKey, IsMine)) {
		return false;
	}
    vchPubKeyOut = cKey.GetPubKey();
    return true;
}
//
//bool CKeyStore::AddKey(const CKey &key) {
//    return AddKeyPubKey(key, key.GetPubKey());
//}
bool CBasicKeyStore::AddKeyCombi(const CKeyID & keyId, const CKeyCombi &keyCombi)
{
	LOCK(cs_KeyStore);
	mapKeys[keyId] = keyCombi;
	return true;
}

bool CBasicKeyStore::GetKeyCombi(const CKeyID & address, CKeyCombi & keyCombiOut) const {
	{
		LOCK(cs_KeyStore);
		KeyMap::const_iterator mi = mapKeys.find(address);
		if (mi != mapKeys.end()) {
			keyCombiOut = mi->second;
			return true;
		}
	}
	return false;
}

//bool CBasicKeyStore::AddKeyPubKey(const CKey& key, const CPubKey &pubkey)
//{
//    LOCK(cs_KeyStore);
//    mapKeys[pubkey.GetKeyID()] = key;
//    return true;
//}

//bool CBasicKeyStore::AddCScript(const CScript& redeemScript)
//{
//    LOCK(cs_KeyStore);
//    mapScripts[redeemScript.GetID()] = redeemScript;
//    return true;
//}
//
//bool CBasicKeyStore::HaveCScript(const CScriptID& hash) const
//{
//    LOCK(cs_KeyStore);
//    return mapScripts.count(hash) > 0;
//}

//bool CBasicKeyStore::GetCScript(const CScriptID &hash, CScript& redeemScriptOut) const
//{
//    LOCK(cs_KeyStore);
//    ScriptMap::const_iterator mi = mapScripts.find(hash);
//    if (mi != mapScripts.end())
//    {
//        redeemScriptOut = (*mi).second;
//        return true;
//    }
//    return false;
//}

