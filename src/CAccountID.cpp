/*
 * CAccountID.cpp
 *
 *  Created on: 2014Äê11ÔÂ7ÈÕ
 *      Author: ranger.shi
 */

#include "CAccountID.h"
#include "util.h"

unsigned int CUserId::GetSerializeSize(int nType, int nVersion ) const {
	if (!regId.IsEmpty()) {
		return regId.GetSerializeSize(nType, nVersion);
	}
	return ::GetSerializeSize(KeyId,nType, nVersion);

}

template<typename Stream>
void CUserId::Serialize(Stream& s, int nType, int nVersion ) const {
	if (!regId.IsEmpty()) {
		regId.Serialize(s, nType, nVersion);
		return;
	}
	::Serialize(s,KeyId, nType, nVersion);
}

template<typename Stream>
void CUserId::Unserialize(Stream& s, int nType , int nVersion) {
	vector<unsigned char> dat;
	::Unserialize(s, dat, nType, nVersion);
	CDataStream ss(nType, nVersion);
	ss << dat;
	if(dat.size() != 20)
	{
		ss >> regId;
		return;
	}
	ss >> KeyId ;
}

const CRegId &CUserId::GetRegId() const {
	return regId;
}
const vector<unsigned char> CUserId::GetKeyId() const {
	return KeyId;
}
const bool CUserId::IsContainKeyId() const {
	return KeyId.size() == 20;
}

