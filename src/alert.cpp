// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "alert.h"

#include "key.h"
#include "net.h"
#include "ui_interface.h"
#include "util.h"

#include <stdint.h>
#include <algorithm>
#include <map>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace std;

map<uint256, CAlert> g_mapAlerts;
CCriticalSection g_cs_mapAlerts;

void CUnsignedAlert::SetNull() {
	m_nVersion 		= 1;
	m_llRelayUntil 	= 0;
	m_llExpiration	= 0;
	m_nID			= 0;
	m_nCancel 		= 0;
	m_nMinVer 		= 0;
	m_nMaxVer 		= 0;
	m_nPriority 	= 0;

	m_setnCancel.clear();
	m_setstrSubVer.clear();

	m_strComment.clear();
	m_strStatusBar.clear();
	m_strReserved.clear();
}

string CUnsignedAlert::ToString() const {
	string strSetCancel;
	for (auto n : m_setnCancel)
		strSetCancel += strprintf("%d ", n);

	string strSetSubVer;
	for (auto str : m_setstrSubVer) {
		strSetSubVer += "\"" + str + "\" ";
	}

	return strprintf(
	"CAlert(\n"
	"    nVersion     = %d\n"
	"    nRelayUntil  = %d\n"
	"    nExpiration  = %d\n"
	"    nID          = %d\n"
	"    nCancel      = %d\n"
	"    setCancel    = %s\n"
	"    nMinVer      = %d\n"
	"    nMaxVer      = %d\n"
	"    setSubVer    = %s\n"
	"    nPriority    = %d\n"
	"    strComment   = \"%s\"\n"
	"    strStatusBar = \"%s\"\n"
	")\n",
	m_nVersion,
	m_llRelayUntil,
	m_llExpiration,
	m_nID,
	m_nCancel,
	strSetCancel,
	m_nMinVer,
	m_nMaxVer,
	strSetSubVer,
	m_nPriority,
	m_strComment,
	m_strStatusBar);
}

void CUnsignedAlert::print() const {
	LogPrint("INFO", "%s", ToString());
}

void CAlert::SetNull() {
	CUnsignedAlert::SetNull();
	m_vchMsg.clear();
	m_vchSig.clear();
}

bool CAlert::IsNull() const {
	return (m_llExpiration == 0);
}

uint256 CAlert::GetHash() const {
	return Hash(this->m_vchMsg.begin(), this->m_vchMsg.end());
}

bool CAlert::IsInEffect() const {
	return (GetAdjustedTime() < m_llExpiration);
}

bool CAlert::Cancels(const CAlert& alert) const {
	if (!IsInEffect()) {
		return false; // this was a no-op before 31403
	}

	return (alert.m_nID <= m_nCancel || m_setnCancel.count(alert.m_nID));
}

bool CAlert::AppliesTo(int nVersion, string strSubVerIn) const {
	// TODO: rework for client-version-embedded-in-strSubVer ?
	return (IsInEffect() && m_nMinVer <= nVersion && nVersion <= m_nMaxVer
			&& (m_setstrSubVer.empty() || m_setstrSubVer.count(strSubVerIn)));
}

bool CAlert::AppliesToMe() const {
	return AppliesTo(g_sProtocolVersion, FormatSubVersion(g_strClientName, g_sClientVersion, vector<string>()));
}

bool CAlert::RelayTo(CNode* pnode) const {
	if (!IsInEffect()) {
		return false;
	}

	// returns true if wasn't already contained in the set
	if (pnode->m_setKnown.insert(GetHash()).second) {
		if (AppliesTo(pnode->m_nVersion, pnode->m_strSubVer) || AppliesToMe() || GetAdjustedTime() < m_llRelayUntil) {
			pnode->PushMessage("alert", *this);
			return true;
		}
	}
	return false;
}

bool CAlert::CheckSignature() const {
	CPubKey key(SysCfg().AlertKey());
	if (!key.Verify(Hash(m_vchMsg.begin(), m_vchMsg.end()), m_vchSig)) {
		return ERRORMSG("CAlert::CheckSignature() : verify signature failed");
	}

	// Now unserialize the data
	CDataStream sMsg(m_vchMsg, SER_NETWORK, g_sProtocolVersion);
	sMsg >> *(CUnsignedAlert*) this;
	return true;
}

CAlert CAlert::getAlertByHash(const uint256 &hash) {
	CAlert retval;
	{
		LOCK(g_cs_mapAlerts);
		map<uint256, CAlert>::iterator mi = g_mapAlerts.find(hash);
		if (mi != g_mapAlerts.end()) {
			retval = mi->second;
		}
	}
	return retval;
}

bool CAlert::ProcessAlert(bool fThread) {
	if (!CheckSignature()) {
		return false;
	}

	if (!IsInEffect()) {
		return false;
	}

	// alert.nID=max is reserved for if the alert key is
	// compromised. It must have a pre-defined message,
	// must never expire, must apply to all versions,
	// and must cancel all previous
	// alerts or it will be ignored (so an attacker can't
	// send an "everything is OK, don't panic" version that
	// cannot be overridden):
	int maxInt = numeric_limits<int>::max();
	if (m_nID == maxInt) {
		if (!(m_llExpiration == maxInt && m_nCancel == (maxInt - 1) && m_nMinVer == 0 && m_nMaxVer == maxInt && m_setstrSubVer.empty()
				&& m_nPriority == maxInt && m_strStatusBar == "URGENT: Alert key compromised, upgrade required")) {
			return false;
		}
	}

	{
		LOCK(g_cs_mapAlerts);
		// Cancel previous alerts
		for (map<uint256, CAlert>::iterator mi = g_mapAlerts.begin(); mi != g_mapAlerts.end();) {
			const CAlert& alert = (*mi).second;
			if (Cancels(alert)) {
				LogPrint("alert", "cancelling alert %d\n", alert.m_nID);
				g_cUIInterface.NotifyAlertChanged((*mi).first, CT_DELETED);
				g_mapAlerts.erase(mi++);
			} else if (!alert.IsInEffect()) {
				LogPrint("alert", "expiring alert %d\n", alert.m_nID);
				g_cUIInterface.NotifyAlertChanged((*mi).first, CT_DELETED);
				g_mapAlerts.erase(mi++);
			} else {
				mi++;
			}
		}

		// Check if this alert has been cancelled
		for (const auto &item : g_mapAlerts) {
			const CAlert& alert = item.second;
			if (alert.Cancels(*this)) {
				LogPrint("alert", "alert already cancelled by %d\n", alert.m_nID);
				return false;
			}
		}

		// Add to mapAlerts
		g_mapAlerts.insert(make_pair(GetHash(), *this));
		// Notify UI and -alertnotify if it applies to me
		if (AppliesToMe()) {
			g_cUIInterface.NotifyAlertChanged(GetHash(), CT_NEW);
			string strCmd = SysCfg().GetArg("-alertnotify", "");
			if (!strCmd.empty()) {
				// Alert text should be plain ascii coming from a trusted source, but to
				// be safe we first strip anything not in safeChars, then add single quotes around
				// the whole string before passing it to the shell:
				string singleQuote("'");
				string safeStatus = SanitizeString(m_strStatusBar);
				safeStatus = singleQuote + safeStatus + singleQuote;
				boost::replace_all(strCmd, "%s", safeStatus);

				if (fThread) {
					boost::thread t(runCommand, strCmd); // thread runs free
				} else {
					runCommand(strCmd);
				}
			}
		}
	}

	LogPrint("alert", "accepted alert %d, AppliesToMe()=%d\n", m_nID, AppliesToMe());

	return true;
}
