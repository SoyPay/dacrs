// Copyright (c) 2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"

#include "hash.h"
#include "uint256.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <string>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>

/* All alphanumeric characters except for "0", "I", "O", and "l" */
static const char* g_pstaBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

bool DecodeBase58(const char *psz, vector<unsigned char>& vch) {
	// Skip leading spaces.
	while (*psz && isspace(*psz)) {
		psz++;
	}

	// Skip and count leading '1's.
	int nZeroes = 0;
	while (*psz == '1') {
		nZeroes++;
		psz++;
	}

	// Allocate enough space in big-endian base256 representation.
	// log(58) / log(256), rounded up.
	vector<unsigned char> b256(strlen(psz) * 733 / 1000 + 1);
	// Process the characters.
	while (*psz && !isspace(*psz)) {
		// Decode base58 character
		const char *ch = strchr(g_pstaBase58, *psz);
		if (ch == NULL) {
			return false;
		}

		// Apply "b256 = b256 * 58 + ch".
		int carry = ch - g_pstaBase58;
		for (vector<unsigned char>::reverse_iterator it = b256.rbegin(); it != b256.rend(); it++) {
			carry += 58 * (*it);
			*it = carry % 256;
			carry /= 256;
		}
		assert(carry == 0);
		psz++;
	}

	// Skip trailing spaces.
	while (isspace(*psz)) {
		psz++;
	}

	if (*psz != 0) {
		return false;
	}

	// Skip leading zeroes in b256.
	vector<unsigned char>::iterator iter = b256.begin();
	while (iter != b256.end() && *iter == 0) {
		iter++;
	}

	// Copy result into output vector.
	vch.reserve(nZeroes + (b256.end() - iter));
	vch.assign(nZeroes, 0x00);
	while (iter != b256.end()) {
		vch.push_back(*(iter++));
	}

	return true;
}

string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend) {
	// Skip & count leading zeroes.
	int nZeroes = 0;
	while (pbegin != pend && *pbegin == 0) {
		pbegin++;
		nZeroes++;
	}
	// Allocate enough space in big-endian base58 representation.
	vector<unsigned char> b58((pend - pbegin) * 138 / 100 + 1); // log(256) / log(58), rounded up.
	// Process the bytes.
	while (pbegin != pend) {
		int carry = *pbegin;
		// Apply "b58 = b58 * 256 + ch".
		for (vector<unsigned char>::reverse_iterator it = b58.rbegin(); it != b58.rend(); it++) {
			carry += 256 * (*it);
			*it = carry % 58;
			carry /= 58;
		}
		assert(carry == 0);
		pbegin++;
	}

	// Skip leading zeroes in base58 result.
	vector<unsigned char>::iterator iter = b58.begin();
	while (iter != b58.end() && *iter == 0) {
		iter++;
	}

	// Translate the result into a string.
	string str;
	str.reserve(nZeroes + (b58.end() - iter));
	str.assign(nZeroes, '1');
	while (iter != b58.end()) {
		str += g_pstaBase58[*(iter++)];
	}

	return str;
}

string EncodeBase58(const vector<unsigned char>& vch) {
	return EncodeBase58(&vch[0], &vch[0] + vch.size());
}

bool DecodeBase58(const string& str, vector<unsigned char>& vchRet) {
	return DecodeBase58(str.c_str(), vchRet);
}

string EncodeBase58Check(const vector<unsigned char>& vchIn) {
	// add 4-byte hash check to the end
	vector<unsigned char> vch(vchIn);
	uint256 hash = Hash(vch.begin(), vch.end());
	vch.insert(vch.end(), (unsigned char*) &hash, (unsigned char*) &hash + 4);
	return EncodeBase58(vch);
}

bool DecodeBase58Check(const char* psz, vector<unsigned char>& vchRet) {
	if (!DecodeBase58(psz, vchRet)) {
		return false;
	}

	if (vchRet.size() < 4) {
		vchRet.clear();
		return false;
	}

	// re-calculate the checksum, insure it matches the included 4-byte checksum
	uint256 hash = Hash(vchRet.begin(), vchRet.end() - 4);
	if (memcmp(&hash, &vchRet.end()[-4], 4) != 0) {
		vchRet.clear();
		return false;
	}
	vchRet.resize(vchRet.size() - 4);

	return true;
}

bool DecodeBase58Check(const string& str, vector<unsigned char>& vchRet) {
	return DecodeBase58Check(str.c_str(), vchRet);
}

CBase58Data::CBase58Data() {
	m_vchVersion.clear();
	m_vchData.clear();
}

void CBase58Data::SetData(const vector<unsigned char> &vchVersionIn, const void* pdata, size_t nSize) {
	m_vchVersion = vchVersionIn;
	m_vchData.resize(nSize);
	if (!m_vchData.empty()) {
		memcpy(&m_vchData[0], pdata, nSize);
	}
}

void CBase58Data::SetData(const vector<unsigned char> &vchVersionIn, const unsigned char *pbegin,
		const unsigned char *pend) {
	SetData(vchVersionIn, (void*) pbegin, pend - pbegin);
}

bool CBase58Data::SetString(const char* psz, unsigned int nVersionBytes) {
	vector<unsigned char> vchTemp;
	DecodeBase58Check(psz, vchTemp);
	if (vchTemp.size() < nVersionBytes) {
		m_vchData.clear();
		m_vchVersion.clear();
		return false;
	}

	m_vchVersion.assign(vchTemp.begin(), vchTemp.begin() + nVersionBytes);
	m_vchData.resize(vchTemp.size() - nVersionBytes);
	if (!m_vchData.empty()) {
		memcpy(&m_vchData[0], &vchTemp[nVersionBytes], m_vchData.size());
	}

	OPENSSL_cleanse(&vchTemp[0], m_vchData.size());
	return true;
}

bool CBase58Data::SetString(const string& str) {
	return SetString(str.c_str());
}

string CBase58Data::ToString() const {
	vector<unsigned char> vch = m_vchVersion;
	vch.insert(vch.end(), m_vchData.begin(), m_vchData.end());
	return EncodeBase58Check(vch);
}

int CBase58Data::CompareTo(const CBase58Data& b58) const {
	if (m_vchVersion < b58.m_vchVersion) {
		return -1;
	}

	if (m_vchVersion > b58.m_vchVersion) {
		return 1;
	}

	if (m_vchData < b58.m_vchData) {
		return -1;
	}

	if (m_vchData > b58.m_vchData) {
		return 1;
	}

	return 0;
}

namespace {
class CDacrsAddressVisitor: public boost::static_visitor<bool> {
 public:
	CDacrsAddressVisitor(CDacrsAddress *addrIn) :
		m_cDacrsAddr(addrIn) {
	}

	bool operator()(const CKeyID &id) const {
		return m_cDacrsAddr->Set(id);
	}

	bool operator()(const CNoDestination &no) const {
		return false;
	}

 private:
	CDacrsAddress *m_cDacrsAddr;
};
};

bool CDacrsAddress::Set(const CKeyID &id) {
	SetData(SysCfg().Base58Prefix(CBaseParams::EM_PUBKEY_ADDRESS), &id, 20);
	return true;
}

bool CDacrsAddress::Set(const CTxDestination &dest) {
	return boost::apply_visitor(CDacrsAddressVisitor(this), dest);
}

bool CDacrsAddress::IsValid() const {
	bool bValid = false;
	{
		bool fCorrectSize = m_vchData.size() == 20;
		bool fKnownVersion = m_vchVersion == SysCfg().Base58Prefix(CBaseParams::EM_PUBKEY_ADDRESS);
		bValid = fCorrectSize && fKnownVersion;
	}

	if (!bValid) {
		vector<unsigned char> vid;
		vid.push_back(CBaseParams::EM_ACC_ADDRESS);
		if (m_vchData.size() == 26 && m_vchVersion == vid) {
			bValid = true;
		}
	}

	return bValid;
}

CTxDestination CDacrsAddress::Get() const {
	if (!IsValid()) {
		return CNoDestination();
	}

	if (m_vchData.size() == 20) {
		uint160 id;
		memcpy(&id, &m_vchData[0], 20);

		if (m_vchVersion == SysCfg().Base58Prefix(CBaseParams::EM_PUBKEY_ADDRESS)) {
			return CKeyID(id);
		} else {
			return CNoDestination();
		}
	} else {
		return CNoDestination();
	}
}

bool CDacrsAddress::GetKeyID(CKeyID &keyID) const {
	uint160 id;

	if (m_vchVersion == SysCfg().Base58Prefix(CBaseParams::EM_PUBKEY_ADDRESS) && m_vchData.size() == 20) {
		memcpy(&id, &m_vchData[0], 20);
		keyID = CKeyID(id);
		return true;
	}

	vector<unsigned char> vid;
	vid.push_back(CBaseParams::EM_ACC_ADDRESS);
	if (m_vchData.size() == 26 && m_vchVersion == vid) {
		memcpy(keyID.begin(), &m_vchData[0], 20);
		return true;
	}

	return false;
}

//bool CDacrsAddress::GetRegID(CRegID &Regid) const {
//
////	vector<unsigned char> vid;
////	vid.push_back(CBaseParams::ACC_ADDRESS);
//	if (vchData.size() == 26 && vchVersion == vector<unsigned char>(CBaseParams::ACC_ADDRESS)) {
//		Regid.SetRegID(vector<unsigned char>(vchData.end()-6,vchData.end()));
//		return true;
//	}
//	return false;
//}

bool CDacrsAddress::IsScript() const {
	return IsValid() && m_vchVersion == SysCfg().Base58Prefix(CBaseParams::EM_SCRIPT_ADDRESS);
}

void CDacrsSecret::SetKey(const CKey& vchSecret) {
	assert(vchSecret.IsValid());
	SetData(SysCfg().Base58Prefix(CBaseParams::EM_SECRET_KEY), vchSecret.begin(), vchSecret.size());
	if (vchSecret.IsCompressed()) {
		m_vchData.push_back(1);
	}
}

CKey CDacrsSecret::GetKey() {
	CKey ret;
	ret.Set(&m_vchData[0], &m_vchData[32], m_vchData.size() > 32 && m_vchData[32] == 1);
	return ret;
}

bool CDacrsSecret::IsValid() const {
	bool fExpectedFormat = m_vchData.size() == 32 || (m_vchData.size() == 33 && m_vchData[32] == 1);
	bool fCorrectVersion = m_vchVersion == SysCfg().Base58Prefix(CBaseParams::EM_SECRET_KEY);
	return fExpectedFormat && fCorrectVersion;
}

bool CDacrsSecret::SetString(const char* pszSecret) {
	return CBase58Data::SetString(pszSecret) && IsValid();
}

bool CDacrsSecret::SetString(const string& strSecret) {
	return SetString(strSecret.c_str());
}
