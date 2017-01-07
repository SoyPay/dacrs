// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __cplusplus
# error This header can only be compiled as C++.
#endif

#ifndef DACRS_PROTOCOL_H__
#define DACRS_PROTOCOL_H__

#include "chainparams.h"
#include "netbase.h"
#include "serialize.h"
#include "uint256.h"

#include <stdint.h>
#include <string>

/** Message header.
 * (4) message start.
 * (12) command.
 * (4) size.
 * (4) checksum.
 */
class CMessageHeader {
 public:
	CMessageHeader();
	CMessageHeader(const char* pszCommand, unsigned int nMessageSizeIn);

	std::string GetCommand() const;
	bool IsValid() const;

	IMPLEMENT_SERIALIZE
	(
			READWRITE(FLATDATA(m_pchMessageStart));
			READWRITE(FLATDATA(m_pchCommand));
			READWRITE(m_unMessageSize);
			READWRITE(m_unChecksum);
	)
	// TODO: make private (improves encapsulation)

 public:
	enum {
		COMMAND_SIZE = 12, MESSAGE_SIZE_SIZE = sizeof(int), CHECKSUM_SIZE = sizeof(int),

		MESSAGE_SIZE_OFFSET = MESSAGE_START_SIZE + COMMAND_SIZE, CHECKSUM_OFFSET = MESSAGE_SIZE_OFFSET
				+ MESSAGE_SIZE_SIZE, HEADER_SIZE = MESSAGE_START_SIZE + COMMAND_SIZE + MESSAGE_SIZE_SIZE + CHECKSUM_SIZE
	};

	char m_pchMessageStart[MESSAGE_START_SIZE];
	char m_pchCommand[COMMAND_SIZE];
	unsigned int m_unMessageSize;
	unsigned int m_unChecksum;
};

/** nServices flags */
enum {
	NODE_NETWORK = (1 << 0),
};

/** A CService with information about it as peer */
class CAddress: public CService {
 public:
	CAddress();
	explicit CAddress(CService ipIn, uint64_t nServicesIn = NODE_NETWORK);
	void Init();

	IMPLEMENT_SERIALIZE
	(
			CAddress* pthis = const_cast<CAddress*>(this);
			CService* pip = (CService*)pthis;
			if (fRead)
			pthis->Init();
			if (nType & SER_DISK)
			READWRITE(nVersion);
			if ((nType & SER_DISK) /*|| (nVersion >= CADDR_TIME_VERSION && !(nType & SER_GETHASH))*/)
			READWRITE(m_ullTime);
			READWRITE(m_ullServices);
			READWRITE(*pip);
	)

	void print() const;

	// TODO: make private (improves encapsulation)

public:
	uint64_t m_ullServices;
	// disk and network only
	unsigned int m_ullTime;
	// memory only
	int64_t m_llLastTry;
};

/** inv message data */
class CInv {
 public:
	CInv();
	CInv(int nTypeIn, const uint256& cHashIn);
	CInv(const std::string& strType, const uint256& cHashIn);

	IMPLEMENT_SERIALIZE
	(
			READWRITE(m_nType);
			READWRITE(m_cHash);
	)

	friend bool operator<(const CInv& cInvA, const CInv& cInvB);
	bool IsKnownType() const;
	const char* GetCommand() const;
	std::string ToString() const;
	void print() const;

	// TODO: make private (improves encapsulation)

 public:
	int m_nType;
	uint256 m_cHash;
};

enum {
	MSG_TX = 1,
	MSG_BLOCK,
	// Nodes may always request a MSG_FILTERED_BLOCK in a getdata, however,
	// MSG_FILTERED_BLOCK should not appear in any invs except as a part of getdata.
	MSG_FILTERED_BLOCK,
};

#endif // DACRS_PROTOCOL_H__
