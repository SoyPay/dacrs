// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "protocol.h"

#include "util.h"

#ifndef WIN32
# include <arpa/inet.h>
#endif

static const char* g_spszTypeName[] = { "ERROR", "tx", "block", "filtered block" };

CMessageHeader::CMessageHeader() {
	memcpy(m_pchMessageStart, SysCfg().MessageStart(), MESSAGE_START_SIZE);
	memset(m_pchCommand, 0, sizeof(m_pchCommand));
	m_pchCommand[1] = 1;
	m_unMessageSize = -1;
	m_unChecksum = 0;
}

CMessageHeader::CMessageHeader(const char* pszCommand, unsigned int nMessageSizeIn) {
	memcpy(m_pchMessageStart, SysCfg().MessageStart(), MESSAGE_START_SIZE);
	strncpy(m_pchCommand, pszCommand, COMMAND_SIZE);
	m_unMessageSize = nMessageSizeIn;
	m_unChecksum = 0;
}

std::string CMessageHeader::GetCommand() const {
	if (m_pchCommand[COMMAND_SIZE - 1] == 0) {
		return std::string(m_pchCommand, m_pchCommand + strlen(m_pchCommand));
	} else {
		return std::string(m_pchCommand, m_pchCommand + COMMAND_SIZE);
	}
}

bool CMessageHeader::IsValid() const {
	// Check start string
	if (memcmp(m_pchMessageStart, SysCfg().MessageStart(), MESSAGE_START_SIZE) != 0) {
		return false;
	}
	// Check the command string for errors
	for (const char* p1 = m_pchCommand; p1 < m_pchCommand + COMMAND_SIZE; p1++) {
		if (*p1 == 0) {
			// Must be all zeros after the first zero
			for (; p1 < m_pchCommand + COMMAND_SIZE; p1++) {
				if (*p1 != 0) {
					return false;
				}
			}
		} else if (*p1 < ' ' || *p1 > 0x7E) {
			return false;
		}
	}

	// Message size
	if (m_unMessageSize > g_sMaxSize) {
		LogPrint("INFO", "CMessageHeader::IsValid() : (%s, %u bytes) nMessageSize > MAX_SIZE\n", GetCommand(),
				m_unMessageSize);
		return false;
	}

	return true;
}

CAddress::CAddress() : CService() {
	Init();
}

CAddress::CAddress(CService ipIn, uint64_t nServicesIn) : CService(ipIn) {
	Init();
	m_ullServices = nServicesIn;
}

void CAddress::Init() {
	m_ullServices = NODE_NETWORK;
	m_ullTime = 100000000;
	m_llLastTry = 0;
}

CInv::CInv() {
	m_nType = 0;
	m_cHash.SetNull();
}

CInv::CInv(int nTypeIn, const uint256& cHashIn) {
	m_nType = nTypeIn;
	m_cHash = cHashIn;
}

CInv::CInv(const std::string& strType, const uint256& cHashIn) {
	unsigned int i;
	for (i = 1; i < ARRAYLEN(g_spszTypeName); i++) {
		if (strType == g_spszTypeName[i]) {
			m_nType = i;
			break;
		}
	}
	if (i == ARRAYLEN(g_spszTypeName)) {
		throw std::out_of_range(strprintf("CInv::CInv(string, uint256) : unknown type '%s'", strType));
	}

	m_cHash = cHashIn;
}

bool operator<(const CInv& cInvA, const CInv& cInvB) {
	return (cInvA.m_nType < cInvB.m_nType || (cInvA.m_nType == cInvB.m_nType && cInvA.m_cHash < cInvB.m_cHash));
}

bool CInv::IsKnownType() const {
	return (m_nType >= 1 && m_nType < (int) ARRAYLEN(g_spszTypeName));
}

const char* CInv::GetCommand() const {
	if (!IsKnownType())
		throw std::out_of_range(strprintf("CInv::GetCommand() : type=%d unknown type", m_nType));
		return g_spszTypeName[m_nType];
	}

std::string CInv::ToString() const {
	return strprintf("%s %s", GetCommand(), m_cHash.ToString());
}

void CInv::print() const {
	LogPrint("INFO", "CInv(%s)\n", ToString());
}

