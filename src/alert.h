// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_ALERT_H_
#define DACRS_ALERT_H_ 1

#include "serialize.h"
#include "sync.h"

#include <map>
#include <set>
#include <stdint.h>
#include <string>
using namespace std;

class CAlert;
class CNode;
class uint256;

extern map<uint256, CAlert> g_mapAlerts;
extern CCriticalSection g_cs_mapAlerts;

/** Alerts are for notifying old versions if they become too obsolete and
 * need to upgrade.  The message is displayed in the status bar.
 * Alert messages are broadcast as a vector of signed data.  Unserializing may
 * not read the entire buffer if the alert is for a newer version, but older
 * versions can still relay the original data.
 */
class CUnsignedAlert {
 public:
	int m_nVersion;
	int64_t m_llRelayUntil;      // when newer nodes stop relaying to newer nodes
	int64_t m_llExpiration;
	int m_nID;
	int m_nCancel;
	set<int> m_setnCancel;
	int m_nMinVer;            // lowest version inclusive
	int m_nMaxVer;            // highest version inclusive
	set<string> m_setstrSubVer;  // empty matches all
	int m_nPriority;

	// Actions
	string m_strComment;
	string m_strStatusBar;
	string m_strReserved;

	IMPLEMENT_SERIALIZE
	(
		READWRITE(this->m_nVersion);
		nVersion = this->m_nVersion;
		READWRITE(m_llRelayUntil);
		READWRITE(m_llExpiration);
		READWRITE(m_nID);
		READWRITE(m_nCancel);
		READWRITE(m_setnCancel);
		READWRITE(m_nMinVer);
		READWRITE(m_nMaxVer);
		READWRITE(m_setstrSubVer);
		READWRITE(m_nPriority);

		READWRITE(m_strComment);
		READWRITE(m_strStatusBar);
		READWRITE(m_strReserved);
	)

	void SetNull();
	string ToString() const;
	void print() const;
};

/** An alert is a combination of a serialized CUnsignedAlert and a signature. */
class CAlert: public CUnsignedAlert {
 public:
	vector<unsigned char> m_vchMsg;
	vector<unsigned char> m_vchSig;

	CAlert() {
		SetNull();
	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(m_vchMsg);
			READWRITE(m_vchSig);
	)

	void SetNull();
	bool IsNull() const;
	uint256 GetHash() const;
	bool IsInEffect() const;
	bool Cancels(const CAlert& alert) const;
	bool AppliesTo(int nVersion, string strSubVerIn) const;
	bool AppliesToMe() const;
	bool RelayTo(CNode* pnode) const;
	bool CheckSignature() const;
	bool ProcessAlert(bool fThread = true);

	/*
	 * Get copy of (active) alert object by hash. Returns a null alert if it is not found.
	 */
	static CAlert getAlertByHash(const uint256 &hash);
};

#endif
