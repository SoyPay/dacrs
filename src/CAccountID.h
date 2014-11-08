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

class CAccountId {
	int nHeight;
	int nIndex;
public:
	CAccountId() {
		nHeight = -1;
		nIndex = -1;
	}
	CAccountId(int high, int index) {
		nHeight = high;
		nIndex = index;
	}

	bool IsEmpty() const {
		return (nHeight == -1 && nIndex == -1);
	}

	string ToString() const{
		return strprintf("nHeight =%d, nIndex=%ld\n",nHeight,nIndex);
	}


	IMPLEMENT_SERIALIZE
	(
	if(fGetSize)
	{
		nSerSize += ::GetSerializeSize(VARINT(nHeight), nType, nVersion);
		nSerSize += ::GetSerializeSize(VARINT(nIndex),nType, nVersion);
	}
	if(fWrite)
	{
		CDataStream ss( nType, nVersion);
		ss << VARINT(nHeight);
		ss << VARINT(nIndex);
		vector<char> te;
		ss >> te;
		READWRITE(te);
	}
	if(fRead)
	{
		CDataStream ss(SER_DISK, 0);
		vector<char> te;
		READWRITE(te);
		ss << te;
		ss >> VARINT(nHeight);
		ss >> VARINT(nIndex);

	}
	)

};

class CKeyId {
public:
	vector<unsigned char> vKeyID; //
	IMPLEMENT_SERIALIZE
	(
			READWRITE(vKeyID);
	)

};
class CID {
	CAccountId accId;
	vector<unsigned char> KeyId;
public:
	CID(const CAccountId &acid) {
		accId = acid;
	}
	CID(const CAccountId &acid,const vector<unsigned char> &kid) {
		accId = acid;
		KeyId = kid;
	}
	CID(const vector<unsigned char> &kid) {
		KeyId = kid;
	}
	CID(int high, int index):accId(high,index)
	{

	}

	string ToString() const;
    unsigned int GetSerializeSize(int nType=0, int nVersion=PROTOCOL_VERSION) const;
    template<typename Stream>
    void Serialize(Stream& s, int nType=0, int nVersion=PROTOCOL_VERSION) const;
    template<typename Stream>
    void Unserialize(Stream& s, int nType=0, int nVersion=PROTOCOL_VERSION);
    const CAccountId &getAccountId() const ;
	const vector<unsigned char> GetKeyId() const ;
	const bool IsContainKeyId() const ;
};

#endif /* CACCOUNTID_H_ */
