/*
 * CAccountID.cpp
 *
 *  Created on: 2014Äê11ÔÂ7ÈÕ
 *      Author: ranger.shi
 */

#include "CAccountID.h"
#include "util.h"

unsigned int CID::GetSerializeSize(int nType, int nVersion ) const {
	if (!accId.IsEmpty()) {
		return accId.GetSerializeSize(nType, nVersion);
	}
	return ::GetSerializeSize(KeyId,nType, nVersion);

}

template<typename Stream>
void CID::Serialize(Stream& s, int nType, int nVersion ) const {
	if (!accId.IsEmpty()) {
		accId.Serialize(s, nType, nVersion);
		return;
	}
	::Serialize(s,KeyId, nType, nVersion);
}

template<typename Stream>
void CID::Unserialize(Stream& s, int nType , int nVersion) {
	if (!accId.IsEmpty()) {
		accId.Unserialize(s, nType, nVersion);
		return;
	}
	::Unserialize(s,KeyId, nType, nVersion);
}

const CAccountId &CID::getAccountId() const {
	return accId;
}
const vector<unsigned char> CID::GetKeyId() const {
	return KeyId;
}
const bool CID::IsContainKeyId() const {
	return KeyId.size() == 20;
}

