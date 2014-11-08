/*
 * CAccountID.h
 *
 *  Created on: 2014Äê11ÔÂ7ÈÕ
 *      Author: ranger.shi
 */

#ifndef CACCOUNTID_H_
#define CACCOUNTID_H_


#define WRITEDATA(s, obj)   s.write((char*)&(obj), sizeof(obj))
#define READDATA(s, obj)    s.read((char*)&(obj), sizeof(obj))

#include "allocators.h"
#include "hash.h"
#include "serialize.h"
#include "uint256.h"

#include <stdexcept>
#include <vector>
#include "util.h"

class CRegId {
	int nHeight;
	int nIndex;
	vector<unsigned char> vRegID;
public:
	CRegId() {
		nHeight = -1;
		nIndex = -1;
		vRegID.clear();
	}
	CRegId(int high, int index) {
		nHeight = high;
		nIndex = index;
		CDataStream ss(SER_DISK, 0);
		ss << VARINT(nHeight);
		ss << VARINT(nIndex);
		ss >> vRegID;
	}

	bool IsEmpty() const {
		return (nHeight == -1 && nIndex == -1);
	}

	string ToString() const{
		return strprintf("nHeight =%d, nIndex=%ld\n",nHeight,nIndex);
	}


	IMPLEMENT_SERIALIZE
	(
		READWRITE(vRegID);
	)

};


class CUserId {
	CRegId regId;
	vector<unsigned char> KeyId;
public:
	CUserId(const CRegId &acid) {
		regId = acid;
	}
	CUserId(const CRegId &acid,const vector<unsigned char> &kid) {
		regId = acid;
		KeyId = kid;
	}
	CUserId(const vector<unsigned char> &kid) {
		KeyId = kid;
	}
	CUserId(int high, int index):regId(high,index)
	{

	}

	string ToString() const;
    unsigned int GetSerializeSize(int nType=0, int nVersion=PROTOCOL_VERSION) const;
    template<typename Stream>
    void Serialize(Stream& s, int nType=0, int nVersion=PROTOCOL_VERSION) const;
    template<typename Stream>
    void Unserialize(Stream& s, int nType=0, int nVersion=PROTOCOL_VERSION);
    const CRegId &GetRegId() const ;
	const vector<unsigned char> GetKeyId() const ;
	const bool IsContainKeyId() const ;
};

#endif /* CACCOUNTID_H_ */
