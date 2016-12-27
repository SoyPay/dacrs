// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include "dacrs-config.h"
#endif

#include "net.h"

#include "addrman.h"
#include "chainparams.h"
#include "core.h"
#include "ui_interface.h"
#include "tx.h"

#ifdef WIN32
#include <string.h>
#else
#include <fcntl.h>
#endif

#ifdef USE_UPNP
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/miniwget.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>
#endif

#include <boost/filesystem.hpp>

// Dump addresses to peers.dat every 15 minutes (900s)
#define DUMP_ADDRESSES_INTERVAL 900

#if !defined(HAVE_MSG_NOSIGNAL) && !defined(MSG_NOSIGNAL)
#define MSG_NOSIGNAL 0
#endif

using namespace std;
using namespace boost;

static const int g_sMaxOutbound_connections = 8;

bool OpenNetworkConnection(const CAddress& cAddrConnect, CSemaphoreGrant *pGrantOutbound = NULL, const char *strDest =
		NULL, bool bOneShot = false);


//
// Global state variables
//
bool g_bDiscover = true;
uint64_t g_ullLocalServices = NODE_NETWORK;
CCriticalSection g_cs_mapLocalHost;
map<CNetAddr, LocalServiceInfo> g_mapLocalHost;
static bool g_sReachable[NET_MAX] = {};
static bool g_sLimited[NET_MAX] = {};
static CNode* g_sNodeLocalHost = NULL;
static CNode* g_sNodeSync = NULL;
uint64_t g_ullLocalHostNonce = 0;
static vector<SOCKET> g_shListenSocket;
CAddrMan g_cAddrman;
int g_nMaxConnections = 125;

vector<CNode*> g_vNodes;
CCriticalSection g_cs_vNodes;
map<CInv, CDataStream> g_mapRelay;
deque<pair<int64_t, CInv> > g_vRelayExpiration;
CCriticalSection g_cs_mapRelay;
limitedmap<CInv, int64_t> mapAlreadyAskedFor(g_sMaxInvSz);   //存放发送请求的 MSG_TX ，int64_t是 the earliest time the request can be sent，收到对应交易执行erase

static deque<string> g_vOneShots;
CCriticalSection g_cs_vOneShots;

set<CNetAddr> g_setservAddNodeAddresses;
CCriticalSection g_cs_setservAddNodeAddresses;

vector<string> g_vAddedNodes;
CCriticalSection g_cs_vAddedNodes;

NodeId g_nLastNodeId = 0;
CCriticalSection g_cs_nLastNodeId;

static CSemaphore *g_sSemOutbound = NULL;

// Signals for message handling
static ST_NodeSignals g_signals;
ST_NodeSignals& GetNodeSignals() { return g_signals; }

void AddOneShot(string strDest) {
	LOCK(g_cs_vOneShots);
	g_vOneShots.push_back(strDest);
}

unsigned short GetListenPort() {
	return (unsigned short) (SysCfg().GetArg("-port", SysCfg().GetDefaultPort()));
}

// find 'best' local address for a particular peer
bool GetLocal(CService &cServiceAddr, const CNetAddr *pAddrPeer) {
	if (fNoListen) {
		return false;
	}
	int nBestScore = -1;
	int nBestReachability = -1;
	{
		LOCK(g_cs_mapLocalHost);
		for (map<CNetAddr, LocalServiceInfo>::iterator it = g_mapLocalHost.begin(); it != g_mapLocalHost.end(); it++) {
			int nScore = (*it).second.nScore;
			int nReachability = (*it).first.GetReachabilityFrom(pAddrPeer);
			if (nReachability > nBestReachability || (nReachability == nBestReachability && nScore > nBestScore)) {
				cServiceAddr = CService((*it).first, (*it).second.nPort);
				nBestReachability = nReachability;
				nBestScore = nScore;
			}
		}
	}
	return nBestScore >= 0;
}

// get best local address for a particular peer as a CAddress
CAddress GetLocalAddress(const CNetAddr *pAddrPeer) {
	CAddress cAddresRet(CService("0.0.0.0", 0), 0);
	CService cServiceAddr;
	if (GetLocal(cServiceAddr, pAddrPeer)) {
		cAddresRet = CAddress(cServiceAddr);
		cAddresRet.nServices = g_ullLocalServices;
		cAddresRet.nTime = GetAdjustedTime();
	}
	return cAddresRet;
}

bool RecvLine(SOCKET hSocket, string& strLine) {
	strLine = "";
	while (true) {
		char c;
		int nBytes = recv(hSocket, &c, 1, 0);
		if (nBytes > 0) {
			if (c == '\n') {
				continue;
			}
			if (c == '\r') {
				return true;
			}
			strLine += c;
			if (strLine.size() >= 9000) {
				return true;
			}
		} else if (nBytes <= 0) {
			boost::this_thread::interruption_point();
			if (nBytes < 0) {
				int nErr = WSAGetLastError();
				if (nErr == WSAEMSGSIZE) {
					continue;
				}
				if (nErr == WSAEWOULDBLOCK || nErr == WSAEINTR || nErr == WSAEINPROGRESS) {
					MilliSleep(10);
					continue;
				}
			}
			if (!strLine.empty()) {
				return true;
			}
			if (nBytes == 0) {
				// socket closed
				LogPrint("net", "socket closed\n");
				return false;
			} else {
				// socket error
				int nErr = WSAGetLastError();
				LogPrint("net", "recv failed: %s\n", NetworkErrorString(nErr));
				return false;
			}
		}
	}
}

// used when scores of local addresses may have changed
// pushes better local address to peers
void static AdvertizeLocal() {
	LOCK(g_cs_vNodes);
	for (auto pNode : g_vNodes) {
		if (pNode->m_bSuccessfullyConnected) {
			CAddress cAddrLocal = GetLocalAddress(&pNode->m_cAddress);
			if (cAddrLocal.IsRoutable() && (CService) cAddrLocal != (CService) pNode->m_strAddrLocal) {
				pNode->PushAddress(cAddrLocal);
				pNode->m_strAddrLocal = cAddrLocal;
			}
		}
	}
}

void SetReachable(enum Network netWork, bool bFlag) {
	LOCK(g_cs_mapLocalHost);
	g_sReachable[netWork] = bFlag;
	if (netWork == NET_IPV6 && bFlag) {
		g_sReachable[NET_IPV4] = true;
	}
}

// learn a new local address
bool AddLocal(const CService& cServiceAddr, int nScore) {
	if (!cServiceAddr.IsRoutable()) {
		return false;
	}
	if (!g_bDiscover && nScore < LOCAL_MANUAL) {
		return false;
	}
	if (IsLimited(cServiceAddr)) {
		return false;
	}
	LogPrint("INFO", "AddLocal(%s,%i)\n", cServiceAddr.ToString(), nScore);

	{
		LOCK(g_cs_mapLocalHost);
		bool bAlready = g_mapLocalHost.count(cServiceAddr) > 0;
		LocalServiceInfo &tLocalServiceInfo = g_mapLocalHost[cServiceAddr];
		if (!bAlready || nScore >= tLocalServiceInfo.nScore) {
			tLocalServiceInfo.nScore = nScore + (bAlready ? 1 : 0);
			tLocalServiceInfo.nPort = cServiceAddr.GetPort();
		}
		SetReachable(cServiceAddr.GetNetwork());
	}

	AdvertizeLocal();
	return true;
}

bool AddLocal(const CNetAddr &cNetAddr, int nScore) {
	return AddLocal(CService(cNetAddr, GetListenPort()), nScore);
}

/** Make a particular network entirely off-limits (no automatic connects to it) */
void SetLimited(enum Network netWork, bool bLimited) {
	if (netWork == NET_UNROUTABLE) {
		return;
	}
	LOCK(g_cs_mapLocalHost);
	g_sLimited[netWork] = bLimited;
}

bool IsLimited(enum Network netWork) {
	LOCK(g_cs_mapLocalHost);
	return g_sLimited[netWork];
}

bool IsLimited(const CNetAddr &cNetAddr) {
	return IsLimited(cNetAddr.GetNetwork());
}

/** vote for a local address */
bool SeenLocal(const CService& cServiceAddr) {
	{
		LOCK(g_cs_mapLocalHost);
		if (g_mapLocalHost.count(cServiceAddr) == 0) {
			return false;
		}

		g_mapLocalHost[cServiceAddr].nScore++;
	}
	AdvertizeLocal();
	return true;
}

/** check whether a given address is potentially local */
bool IsLocal(const CService& cServiceAddr) {
	LOCK(g_cs_mapLocalHost);
	return g_mapLocalHost.count(cServiceAddr) > 0;
}

/** check whether a given address is in a network we can probably connect to */
bool IsReachable(const CNetAddr& cNetAddr) {
	LOCK(g_cs_mapLocalHost);
	enum Network net = cNetAddr.GetNetwork();
	return g_sReachable[net] && !g_sLimited[net];
}

bool GetMyExternalIP2(const CService& cAddrConnect, const char* pszGet, const char* pszKeyword, CNetAddr& cIpRet) {
	LogPrint("GETMYIP", "GetMyExternalIP2 addrConnect:%s \n", cAddrConnect.ToString());
	SOCKET hSocket;
	if (!ConnectSocket(cAddrConnect, hSocket)) {
		return ERRORMSG("GetMyExternalIP() : connection to %s failed", cAddrConnect.ToString());
	}

	send(hSocket, pszGet, strlen(pszGet), MSG_NOSIGNAL);
	LogPrint("GETMYIP", "GetMyExternalIP2 SendData:%s\n", pszGet);
	//set timeout interval
	u_long ulNetTimeout = 5000;
	if (0 != setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (char *) &ulNetTimeout, sizeof(u_long))) {
		closesocket(hSocket);
		return false;
	}

	string strLine;
	while (RecvLine(hSocket, strLine)) {
		if (strLine.empty()) // HTTP response is separated from headers by blank line
		{
			while (true) {
				if (!RecvLine(hSocket, strLine)) {
					closesocket(hSocket);
					return false;
				}
				if (pszKeyword == NULL) {
					break;
				}
				if (strLine.find(pszKeyword) != string::npos) {
					strLine = strLine.substr(strLine.find(pszKeyword) + strlen(pszKeyword));
					break;
				}
			}
			closesocket(hSocket);
			if (strLine.find("<") != string::npos) {
				strLine = strLine.substr(0, strLine.find("<"));
			}
			strLine = strLine.substr(strspn(strLine.c_str(), " \t\n\r"));
			while (strLine.size() > 0 && isspace(strLine[strLine.size() - 1])) {
				strLine.resize(strLine.size() - 1);
			}
			CService cServiceAddr(strLine, 0, true);
			LogPrint("GETMYIP", "GetMyExternalIP() received [%s] %s\n", strLine, cServiceAddr.ToString());
			if (!cServiceAddr.IsValid() || !cServiceAddr.IsRoutable()) {
				return false;
			}

			cIpRet.SetIP(cServiceAddr);
			return true;
		}
	}
	closesocket(hSocket);
	return ERRORMSG("GetMyExternalIP() : connection closed");
}

bool GetMyExternalIP(CNetAddr& cIpRet) {
	CService cAddrConnect;
	string strSZGet;
	const char* pszKeyword;

	for (int nLookup = 0; nLookup <= 1; nLookup++)
		for (int nHost = 1; nHost <= 2; nHost++) {
			// We should be phasing out our use of sites like these. If we need
			// replacements, we should ask for volunteers to put this simple
			// php file on their web server that prints the client IP:
			//  <?php echo $_SERVER["REMOTE_ADDR"]; ?>
			if (nHost == 1) {
				cAddrConnect = CService("91.198.22.70", 80); // checkip.dyndns.org
				strSZGet = string("GET / HTTP/1.1\r\n"
						"Host: 91.198.22.70\r\n"
						"User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1)\r\n"
						"Connection: close\r\n"
						"\r\n");
				if (nLookup == 1) {
					CService addrIP("checkip.dyndns.org", 80, true);
					if (addrIP.IsValid()) {
						cAddrConnect = addrIP;
					}
					strSZGet.replace(strSZGet.find("91.198.22.70"), sizeof("91.198.22.70"), "checkip.dyndns.org");
				}
				pszKeyword = "Address:";
			} else if (nHost == 2) {
				cAddrConnect = CService("74.208.43.192", 80); // www.showmyip.com

				strSZGet = string("GET /simple/ HTTP/1.1\r\n"
						"Host: 74.208.43.192\r\n"
						"User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1)\r\n"
						"Connection: close\r\n"
						"\r\n");
				if (nLookup == 1) {
					CService cAddrIP("www.showmyip.com", 80, true);
					if (cAddrIP.IsValid()) {
						cAddrConnect = cAddrIP;
					}
					strSZGet.replace(strSZGet.find("74.208.43.192"), sizeof("74.208.43.192"), "www.showmyip.com");
				}
				pszKeyword = NULL; // Returns just IP address
			}

			if (GetMyExternalIP2(cAddrConnect, strSZGet.c_str(), pszKeyword, cIpRet)) {
				return true;
			}
		}
	return false;
}

void ThreadGetMyExternalIP() {
	CNetAddr cAddrLocalHost;
	if (GetMyExternalIP(cAddrLocalHost)) {
		LogPrint("INFO", "GetMyExternalIP() returned %s\n", cAddrLocalHost.ToStringIP());
		AddLocal(cAddrLocalHost, LOCAL_HTTP);
	}
	LogPrint("INFO", "ThreadGetMyExternalIP END\n");
}

void AddressCurrentlyConnected(const CService& cAddr) {
	g_cAddrman.Connected(cAddr);
}

uint64_t CNode::m_sTotalBytesRecv = 0;
uint64_t CNode::m_sTotalBytesSent = 0;
CCriticalSection CNode::m_s_cs_totalBytesRecv;
CCriticalSection CNode::m_s_cs_totalBytesSent;

CNode* FindNode(const CNetAddr& cIp) {
	LOCK(g_cs_vNodes);
	for (auto pNode : g_vNodes) {
		if ((CNetAddr) pNode->m_cAddress == cIp) {
			return (pNode);
		}
	}

	return NULL;
}

CNode* FindNode(string strAddrName) {
	LOCK(g_cs_vNodes);
	for (auto pNode : g_vNodes) {
		if (pNode->m_strAddrName == strAddrName) {
			return (pNode);
		}
	}

	return NULL;
}

CNode* FindNode(const CService& cIp) {
	LOCK(g_cs_vNodes);
	for (auto pNode : g_vNodes) {
		if ((CService) pNode->m_cAddress == cIp) {
			return (pNode);
		}
	}

	return NULL;
}

CNode* ConnectNode(CAddress cAddrConnect, const char *pszDest) {
	if (pszDest == NULL) {
		if (IsLocal (cAddrConnect)) {
			return NULL;
		}
		// Look for an existing connection
		CNode* pNode = FindNode((CService) cAddrConnect);
		if (pNode) {
			pNode->AddRef();
			return pNode;
		}
	}

	/// debug print
	LogPrint("net", "trying connection %s lastseen=%.1fhrs\n", pszDest ? pszDest : cAddrConnect.ToString(),
			pszDest ? 0 : (double )(GetAdjustedTime() - cAddrConnect.nTime) / 3600.0);

	// Connect
	SOCKET hSocket;
	if (pszDest ?
			ConnectSocketByName(cAddrConnect, hSocket, pszDest, SysCfg().GetDefaultPort()) :
			ConnectSocket(cAddrConnect, hSocket)) {
		g_cAddrman.Attempt(cAddrConnect);
		LogPrint("net", "connected %s\n", pszDest ? pszDest : cAddrConnect.ToString());

		// Set to non-blocking
#ifdef WIN32
		u_long nOne = 1;
		if (ioctlsocket(hSocket, FIONBIO, &nOne) == SOCKET_ERROR)
			LogPrint("INFO", "ConnectSocket() : ioctlsocket non-blocking setting failed, error %s\n",
					NetworkErrorString(WSAGetLastError()));
#else
		if (fcntl(hSocket, F_SETFL, O_NONBLOCK) == SOCKET_ERROR)
		LogPrint("INFO","ConnectSocket() : fcntl non-blocking setting failed, error %s\n", NetworkErrorString(errno));
#endif

		// Add node
		CNode* pNode = new CNode(hSocket, cAddrConnect, pszDest ? pszDest : "", false);
		pNode->AddRef();

		{
			LOCK(g_cs_vNodes);
			g_vNodes.push_back(pNode);
		}

		pNode->m_llTimeConnected = GetTime();
		return pNode;
	} else {
		return NULL;
	}
}

void CNode::CloseSocketDisconnect() {
	m_bDisconnect = true;
	if (m_hSocket != INVALID_SOCKET) {
		LogPrint("net", "disconnecting node %s\n", m_strAddrName);
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
	}

	// in case this fails, we'll empty the recv buffer when the CNode is deleted
	TRY_LOCK(m_cs_vRecvMsg, lockRecv);
	if (lockRecv) {
		m_vRecvMsg.clear();
	}
	// if this was the sync node, we'll need a new one
	if (this == g_sNodeSync) {
		g_sNodeSync = NULL;
	}
}

void CNode::Cleanup() {
}

void CNode::PushVersion() {
	int nBestHeight = g_signals.GetHeight().get_value_or(0);

#ifdef WIN32
	string strSystem("windows");
#else
	string strSystem("linux");
#endif
	vector<string> vstrComments;
	vstrComments.push_back(strSystem);
	/// when NTP implemented, change to just nTime = GetAdjustedTime()
	int64_t llTime = (m_bInbound ? GetAdjustedTime() : GetTime());
	CAddress cAddrYou = (m_cAddress.IsRoutable() && !IsProxy(m_cAddress) ? m_cAddress : CAddress(CService("0.0.0.0", 0)));
	CAddress cAddrMe = GetLocalAddress(&m_cAddress);
	RAND_bytes((unsigned char*) &g_ullLocalHostNonce, sizeof(g_ullLocalHostNonce));
	LogPrint("net", "send version message: version %d, blocks=%d, us=%s, them=%s, peer=%s\n", PROTOCOL_VERSION,
			nBestHeight, cAddrMe.ToString(), cAddrYou.ToString(), m_cAddress.ToString());
	PushMessage("version", PROTOCOL_VERSION, g_ullLocalServices, llTime, cAddrYou, cAddrMe, g_ullLocalHostNonce,
			FormatSubVersion(CLIENT_NAME, CLIENT_VERSION, vstrComments), nBestHeight, true);
}

map<CNetAddr, int64_t> CNode::m_sBanned;
CCriticalSection CNode::m_s_cs_Banned;

void CNode::ClearBanned() {
	m_sBanned.clear();
}

bool CNode::IsBanned(CNetAddr cNetIp) {
	bool bResult = false;
	{
		LOCK(m_s_cs_Banned);
		map<CNetAddr, int64_t>::iterator i = m_sBanned.find(cNetIp);
		if (i != m_sBanned.end()) {
			int64_t t = (*i).second;
			if (GetTime() < t) {
				bResult = true;
			}
		}
	}
	return bResult;
}

bool CNode::Ban(const CNetAddr &cNetIp) {
    int64_t llBanTime = GetTime() + SysCfg().GetArg("-bantime", 60*60*24);  // Default 24-hour ban
    {
        LOCK(m_s_cs_Banned);
        if (m_sBanned[cNetIp] < llBanTime) {
        	m_sBanned[cNetIp] = llBanTime;
        }
    }
    return true;
}

#undef X
#define X(name) stats.name = name
void CNode::copyStats(CNodeStats &stats) {
    stats.m_nNodeId = this->GetId();
    X(m_ullServices);
    X(m_llLastSend);
    X(m_llLastRecv);
    X(m_llTimeConnected);
    X(m_strAddrName);
    X(m_nVersion);
    X(m_strCleanSubVer);
    X(m_bInbound);
    X(m_nStartingHeight);
    X(m_ullSendBytes);
    X(m_ullRecvBytes);
    stats.m_bSyncNode = (this == g_sNodeSync);

    // It is common for nodes with good ping times to suddenly become lagged,
    // due to a new block arriving or other large transfer.
    // Merely reporting pingtime might fool the caller into thinking the node was still responsive,
    // since pingtime does not update until the ping is complete, which might take a while.
    // So, if a ping is taking an unusually long time in flight,
    // the caller can immediately detect that this is happening.
    int64_t llPingUsecWait = 0;
    if ((0 != m_ullPingNonceSent) && (0 != m_llnPingUsecStart)) {
        llPingUsecWait = GetTimeMicros() - m_llnPingUsecStart;
    }

    // Raw ping time is in microseconds, but show it to user as whole seconds (Dacrs users should be well used to small numbers with many decimal places by now :)
    stats.m_dPingTime = (((double)m_llPingUsecTime) / 1e6);
    stats.m_dPingWait = (((double)llPingUsecWait) / 1e6);

    // Leave string empty if addrLocal invalid (not filled in yet)
    stats.m_strAddrLocal = m_strAddrLocal.IsValid() ? m_strAddrLocal.ToString() : "";
}
#undef X

// requires LOCK(cs_vRecvMsg)
bool CNode::ReceiveMsgBytes(const char *pch, unsigned int nBytes) {
	while (nBytes > 0) {
		// get current incomplete message, or create a new one
		if (m_vRecvMsg.empty() || m_vRecvMsg.back().complete()) {
			m_vRecvMsg.push_back(CNetMessage(SER_NETWORK, m_nRecvVersion));
		}
		CNetMessage& cNetMsg = m_vRecvMsg.back();

		// absorb network data
		int nHandled;
		if (!cNetMsg.m_bInData) {
			nHandled = cNetMsg.readHeader(pch, nBytes);
		} else {
			nHandled = cNetMsg.readData(pch, nBytes);
		}
		if (nHandled < 0) {
			return false;
		}

		pch += nHandled;
		nBytes -= nHandled;
	}

	return true;
}

int CNetMessage::readHeader(const char *pch, unsigned int nBytes) {
	// copy data to temporary parsing buffer
	unsigned int nRemaining = 24 - m_unHeaderPos;
	unsigned int nCopy = min(nRemaining, nBytes);

	memcpy(&m_cDSHeaderBuf[m_unHeaderPos], pch, nCopy);
	m_unHeaderPos += nCopy;

	// if header incomplete, exit
	if (m_unHeaderPos < 24) {
		return nCopy;
	}
	// deserialize to CMessageHeader
	try {
		m_cDSHeaderBuf >> m_cMessageHeader;
	} catch (std::exception &e) {
		return -1;
	}

	// reject messages larger than MAX_SIZE
	if (m_cMessageHeader.nMessageSize > MAX_SIZE) {
		return -1;
	}
	// switch state to reading message data
	m_bInData = true;
	m_cDSRecv.resize(m_cMessageHeader.nMessageSize);

	return nCopy;
}

int CNetMessage::readData(const char *pch, unsigned int nBytes) {
	unsigned int nRemaining = m_cMessageHeader.nMessageSize - m_unDataPos;
	unsigned int nCopy = min(nRemaining, nBytes);

	memcpy(&m_cDSRecv[m_unDataPos], pch, nCopy);
	m_unDataPos += nCopy;

	return nCopy;
}

// requires LOCK(cs_vSend)
void SocketSendData(CNode *pNode) {
	deque<CSerializeData>::iterator it = pNode->m_vSendMsg.begin();

	while (it != pNode->m_vSendMsg.end()) {
		const CSerializeData &cSerializeData = *it;
		assert(cSerializeData.size() > pNode->m_unSendOffset);
		int nBytes = send(pNode->m_hSocket, &cSerializeData[pNode->m_unSendOffset], cSerializeData.size() - pNode->m_unSendOffset,
				MSG_NOSIGNAL | MSG_DONTWAIT);
		if (nBytes > 0) {
			pNode->m_llLastSend = GetTime();
			pNode->m_ullSendBytes += nBytes;
			pNode->m_unSendOffset += nBytes;
			pNode->RecordBytesSent(nBytes);
			if (pNode->m_unSendOffset == cSerializeData.size()) {
				pNode->m_unSendOffset = 0;
				pNode->m_unSendSize -= cSerializeData.size();
				it++;
			} else {
				// could not send full message; stop sending more
				break;
			}
		} else {
			if (nBytes < 0) {
				// error
				int nErr = WSAGetLastError();
				if (nErr != WSAEWOULDBLOCK && nErr != WSAEMSGSIZE && nErr != WSAEINTR && nErr != WSAEINPROGRESS) {
					LogPrint("INFO", "socket send error %s\n", NetworkErrorString(nErr));
					pNode->CloseSocketDisconnect();
				}
			}
			// couldn't send anything at all
			break;
		}
	}

	if (it == pNode->m_vSendMsg.end()) {
		assert(pNode->m_unSendOffset == 0);
		assert(pNode->m_unSendSize == 0);
	}
	pNode->m_vSendMsg.erase(pNode->m_vSendMsg.begin(), it);
}

static list<CNode*> g_vNodesDisconnected;

void ThreadSocketHandler() {
	unsigned int unPrevNodeCount = 0;
	while (true) {
		// Disconnect nodes
		{
			LOCK(g_cs_vNodes);
			// Disconnect unused nodes
			vector<CNode*> vNodesCopy = g_vNodes;
			for (auto pNode : vNodesCopy) {
				if (pNode->m_bDisconnect
						|| (pNode->GetRefCount() <= 0 && pNode->m_vRecvMsg.empty() && pNode->m_unSendSize == 0
								&& pNode->m_cDSSend.empty())) {
					// remove from vNodes
					g_vNodes.erase(remove(g_vNodes.begin(), g_vNodes.end(), pNode), g_vNodes.end());

					// release outbound grant (if any)
					pNode->m_cGrantOutbound.Release();
					// close socket and cleanup
					pNode->CloseSocketDisconnect();
					pNode->Cleanup();
					// hold in disconnected pool until all refs are released
					if (pNode->m_bNetworkNode || pNode->m_bInbound) {
						pNode->Release();
					}

					g_vNodesDisconnected.push_back(pNode);
				}
			}
		}
		{
			// Delete disconnected nodes
			list<CNode*> vNodesDisconnectedCopy = g_vNodesDisconnected;
			for (auto pnode : vNodesDisconnectedCopy) {
				// wait until threads are done using it
				if (pnode->GetRefCount() <= 0) {
					bool bDelete = false;
					{
						TRY_LOCK(pnode->m_cs_vSend, lockSend);
						if (lockSend) {
							TRY_LOCK(pnode->m_cs_vRecvMsg, lockRecv);
							if (lockRecv) {
								TRY_LOCK(pnode->m_cs_inventory, lockInv);
								if (lockInv)
									bDelete = true;
							}
						}
					}
					if (bDelete) {
						g_vNodesDisconnected.remove(pnode);
						delete pnode;
					}
				}
			}
		}
		if (g_vNodes.size() != unPrevNodeCount) {
			unPrevNodeCount = g_vNodes.size();
			uiInterface.NotifyNumConnectionsChanged(unPrevNodeCount);
			uiInterface.NotifyMessage(strprintf("connections=%d", unPrevNodeCount));
		}

		//
		// Find which sockets have data to receive
		//
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 50000; // frequency to poll pnode->vSend

		fd_set fdsetRecv;
		fd_set fdsetSend;
		fd_set fdsetError;
		FD_ZERO(&fdsetRecv);
		FD_ZERO(&fdsetSend);
		FD_ZERO(&fdsetError);
		SOCKET hSocketMax = 0;
		bool bHave_fds = false;

		for (auto hListenSocket : g_shListenSocket) {
			FD_SET(hListenSocket, &fdsetRecv);
			hSocketMax = max(hSocketMax, hListenSocket);
			bHave_fds = true;
		}
		{
			LOCK(g_cs_vNodes);
			for (auto pNode : g_vNodes) {
				if (pNode->m_hSocket == INVALID_SOCKET) {
					continue;
				}

				FD_SET(pNode->m_hSocket, &fdsetError);
				hSocketMax = max(hSocketMax, pNode->m_hSocket);
				bHave_fds = true;

				// Implement the following logic:
				// * If there is data to send, select() for sending data. As this only
				//   happens when optimistic write failed, we choose to first drain the
				//   write buffer in this case before receiving more. This avoids
				//   needlessly queueing received data, if the remote peer is not themselves
				//   receiving data. This means properly utilizing TCP flow control signalling.
				// * Otherwise, if there is no (complete) message in the receive buffer,
				//   or there is space left in the buffer, select() for receiving data.
				// * (if neither of the above applies, there is certainly one message
				//   in the receiver buffer ready to be processed).
				// Together, that means that at least one of the following is always possible,
				// so we don't deadlock:
				// * We send some data.
				// * We wait for data to be received (and disconnect after timeout).
				// * We process a message in the buffer (message handler thread).
				{
					TRY_LOCK(pNode->m_cs_vSend, lockSend);
					if (lockSend && !pNode->m_vSendMsg.empty()) {
						FD_SET(pNode->m_hSocket, &fdsetSend);
						continue;
					}
				}
				{
					TRY_LOCK(pNode->m_cs_vRecvMsg, lockRecv);
					if (lockRecv
							&& (pNode->m_vRecvMsg.empty() || !pNode->m_vRecvMsg.front().complete()
									|| pNode->GetTotalRecvSize() <= ReceiveFloodSize()))
						FD_SET(pNode->m_hSocket, &fdsetRecv);
				}
			}
		}

		int nSelect = select(bHave_fds ? hSocketMax + 1 : 0, &fdsetRecv, &fdsetSend, &fdsetError, &timeout);
		boost::this_thread::interruption_point();

		if (nSelect == SOCKET_ERROR) {
			if (bHave_fds) {
				int nErr = WSAGetLastError();
				LogPrint("INFO", "socket select error %s\n", NetworkErrorString(nErr));
				for (unsigned int i = 0; i <= hSocketMax; i++) {
					FD_SET(i, &fdsetRecv);
				}
			}
			FD_ZERO(&fdsetSend);
			FD_ZERO(&fdsetError);
			MilliSleep(timeout.tv_usec / 1000);
		}

		// Accept new connections
		for (auto hListenSocket : g_shListenSocket)
			if (hListenSocket != INVALID_SOCKET && FD_ISSET(hListenSocket, &fdsetRecv)) {
				struct sockaddr_storage sockaddr;
				socklen_t len = sizeof(sockaddr);
				SOCKET hSocket = accept(hListenSocket, (struct sockaddr*) &sockaddr, &len);
				CAddress cAddress;
				int nInbound = 0;

				if (hSocket != INVALID_SOCKET)
					if (!cAddress.SetSockAddr((const struct sockaddr*) &sockaddr))
						LogPrint("INFO", "Warning: Unknown socket family\n");
				{
					LOCK(g_cs_vNodes);
					for (auto pnode : g_vNodes)
						if (pnode->m_bInbound)
							nInbound++;
				}

				if (hSocket == INVALID_SOCKET) {
					int nErr = WSAGetLastError();
					if (nErr != WSAEWOULDBLOCK)
						LogPrint("INFO", "socket error accept failed: %s\n", NetworkErrorString(nErr));
				} else if (nInbound >= g_nMaxConnections - g_sMaxOutbound_connections) {
					closesocket(hSocket);
				} else if (CNode::IsBanned(cAddress)) {
					LogPrint("INFO", "connection from %s dropped (banned)\n", cAddress.ToString());
					closesocket(hSocket);
				} else {
					LogPrint("net", "accepted connection %s\n", cAddress.ToString());
					CNode* pNode = new CNode(hSocket, cAddress, "", true);
					pNode->AddRef();
					{
						LOCK(g_cs_vNodes);
						g_vNodes.push_back(pNode);
					}
				}
			}
		// Service each socket
		vector<CNode*> vNodesCopy;
		{
			LOCK(g_cs_vNodes);
			vNodesCopy = g_vNodes;
			for (auto pnode : vNodesCopy)
				pnode->AddRef();
		}
		for (auto pNode : vNodesCopy) {
			boost::this_thread::interruption_point();
			// Receive
			if (pNode->m_hSocket == INVALID_SOCKET) {
				continue;
			}
			if (FD_ISSET(pNode->m_hSocket, &fdsetRecv) || FD_ISSET(pNode->m_hSocket, &fdsetError)) {
				TRY_LOCK(pNode->m_cs_vRecvMsg, lockRecv);
				if (lockRecv) {
					{
						// typical socket buffer is 8K-64K
						char pchBuf[0x10000];
						int nBytes = recv(pNode->m_hSocket, pchBuf, sizeof(pchBuf), MSG_DONTWAIT);
						if (nBytes > 0) {
							if (!pNode->ReceiveMsgBytes(pchBuf, nBytes))
								pNode->CloseSocketDisconnect();
							pNode->m_llLastRecv = GetTime();
							pNode->m_ullRecvBytes += nBytes;
							pNode->RecordBytesRecv(nBytes);
						} else if (nBytes == 0) {
							// socket closed gracefully
							if (!pNode->m_bDisconnect) {
								LogPrint("net", "socket closed\n");
							}
							pNode->CloseSocketDisconnect();
						} else if (nBytes < 0) {
							// error
							int nErr = WSAGetLastError();
							if (nErr != WSAEWOULDBLOCK && nErr != WSAEMSGSIZE && nErr != WSAEINTR
									&& nErr != WSAEINPROGRESS) {
								if (!pNode->m_bDisconnect) {
									LogPrint("INFO", "socket recv error %s\n", NetworkErrorString(nErr));
								}
								pNode->CloseSocketDisconnect();
							}
						}
					}
				}
			}
			// Send
			if (pNode->m_hSocket == INVALID_SOCKET) {
				continue;
			}
			if (FD_ISSET(pNode->m_hSocket, &fdsetSend)) {
				TRY_LOCK(pNode->m_cs_vSend, lockSend);
				if (lockSend) {
					SocketSendData(pNode);
				}
			}
			// Inactivity checking
			if (pNode->m_vSendMsg.empty()) {
				pNode->m_llLastSendEmpty = GetTime();
			}
			if (GetTime() - pNode->m_llTimeConnected > 60) {
				if (pNode->m_llLastRecv == 0 || pNode->m_llLastSend == 0) {
					LogPrint("net", "socket no message in first 60 seconds, %d %d\n", pNode->m_llLastRecv != 0,
							pNode->m_llLastSend != 0);
					pNode->m_bDisconnect = true;
				} else if (GetTime() - pNode->m_llLastSend > 90 * 60
						&& GetTime() - pNode->m_llLastSendEmpty > 90 * 60) {
					LogPrint("INFO", "socket not sending\n");
					pNode->m_bDisconnect = true;
				} else if (GetTime() - pNode->m_llLastRecv > 90 * 60) {
					LogPrint("INFO", "socket inactivity timeout\n");
					pNode->m_bDisconnect = true;
				}
			}
		}
		{
			LOCK(g_cs_vNodes);
			for (auto pNode : vNodesCopy) {
				pNode->Release();
			}
		}

		MilliSleep(10);
	}
}

#ifdef USE_UPNP
void ThreadMapPort() {
    string port = strprintf("%u", GetListenPort());
    const char * multicastif = 0;
    const char * minissdpdpath = 0;
    struct UPNPDev * devlist = 0;
    char lanaddr[64];

#ifndef UPNPDISCOVER_SUCCESS
    /* miniupnpc 1.5 */
    devlist = upnpDiscover(2000, multicastif, minissdpdpath, 0);
#else
    /* miniupnpc 1.6 */
    int error = 0;
#ifdef MAC_OSX
    devlist = upnpDiscover(2000, multicastif, minissdpdpath, 0, 0, 0, &error);
#else
	devlist = upnpDiscover(2000, multicastif, minissdpdpath, 0, 0, &error);
#endif
#endif

    struct UPNPUrls urls;
    struct IGDdatas data;
    int r;

    r = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr));
    if (r == 1)
    {
        if (g_bDiscover) {
            char externalIPAddress[40];
            r = UPNP_GetExternalIPAddress(urls.controlURL, data.first.servicetype, externalIPAddress);
            if(r != UPNPCOMMAND_SUCCESS)
                LogPrint("INFO","UPnP: GetExternalIPAddress() returned %d\n", r);
            else
            {
                if(externalIPAddress[0])
                {
                    LogPrint("INFO","UPnP: ExternalIPAddress = %s\n", externalIPAddress);
                    AddLocal(CNetAddr(externalIPAddress), LOCAL_UPNP);
                }
                else
                    LogPrint("INFO","UPnP: GetExternalIPAddress failed.\n");
            }
        }

        string strDesc = "Bitcoin " + FormatFullVersion();

        try {
            while (true) {
#ifndef UPNPDISCOVER_SUCCESS
                /* miniupnpc 1.5 */
                r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype,
                                    port.c_str(), port.c_str(), lanaddr, strDesc.c_str(), "TCP", 0);
#else
                /* miniupnpc 1.6 */
                r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype,
                                    port.c_str(), port.c_str(), lanaddr, strDesc.c_str(), "TCP", 0, "0");
#endif

                if(r!=UPNPCOMMAND_SUCCESS)
                    LogPrint("INFO","AddPortMapping(%s, %s, %s) failed with code %d (%s)\n",
                        port, port, lanaddr, r, strupnperror(r));
                else
                    LogPrint("INFO","UPnP Port Mapping successful.\n");;

                MilliSleep(20*60*1000); // Refresh every 20 minutes
            }
        }
        catch (boost::thread_interrupted)
        {
            r = UPNP_DeletePortMapping(urls.controlURL, data.first.servicetype, port.c_str(), "TCP", 0);
            LogPrint("INFO","UPNP_DeletePortMapping() returned : %d\n", r);
            freeUPNPDevlist(devlist); devlist = 0;
            FreeUPNPUrls(&urls);
            throw;
        }
    } else {
        LogPrint("INFO","No valid UPnP IGDs found\n");
        freeUPNPDevlist(devlist); devlist = 0;
        if (r != 0)
            FreeUPNPUrls(&urls);
    }
}

void MapPort(bool bUseUPnP) {
	static boost::thread* upnp_thread = NULL;
	if (bUseUPnP) {
		if (upnp_thread) {
			upnp_thread->interrupt();
			upnp_thread->join();
			delete upnp_thread;
		}
		upnp_thread = new boost::thread(boost::bind(&TraceThread<void (*)()>, "upnp", &ThreadMapPort));
	} else if (upnp_thread) {
		upnp_thread->interrupt();
		upnp_thread->join();
		delete upnp_thread;
		upnp_thread = NULL;
	}
}

#else
void MapPort(bool)
{
    // Intentionally left blank.
}
#endif

void ThreadDNSAddressSeed() {
	const vector<CDNSSeedData> &vcSeeds = SysCfg().DNSSeeds();
	int nFound = 0;
	LogPrint("INFO", "Loading addresses from DNS seeds (could take a while)\n");
	for (const auto &seed : vcSeeds) {
		if (HaveNameProxy()) {
			AddOneShot(seed.strHost);
		} else {
			vector<CNetAddr> vcIPs;
			vector<CAddress> vcAdd;
			if (LookupHost(seed.strHost.c_str(), vcIPs)) {
				for (auto & ip : vcIPs) {
					int nOneDay = 24 * 3600;
					CAddress cAddr = CAddress(CService(ip, SysCfg().GetDefaultPort()));
					cAddr.nTime = GetTime() - 3 * nOneDay - GetRand(4 * nOneDay); // use a random age between 3 and 7 days old
					vcAdd.push_back(cAddr);
					nFound++;
				}
			}
			g_cAddrman.Add(vcAdd, CNetAddr(seed.strName, true));
		}
	}

	LogPrint("INFO", "%d addresses found from DNS seeds\n", nFound);
}


void DumpAddresses() {
	int64_t llStart = GetTimeMillis();

	CAddrDB cAddrDB;
	cAddrDB.Write(g_cAddrman);
	LogPrint("net", "Flushed %d addresses to peers.dat  %dms\n", g_cAddrman.size(), GetTimeMillis() - llStart);
}

void static ProcessOneShot() {
	string strDest;
	{
		LOCK(g_cs_vOneShots);
		if (g_vOneShots.empty()) {
			return;
		}
		strDest = g_vOneShots.front();
		g_vOneShots.pop_front();
	}
	CAddress cAddr;
	CSemaphoreGrant grant(*g_sSemOutbound, true);
	if (grant) {
		if (!OpenNetworkConnection(cAddr, &grant, strDest.c_str(), true)) {
			AddOneShot(strDest);
		}
	}
}

void ThreadOpenConnections() {
	// Connect to specific addresses
	if (SysCfg().IsArgCount("-connect") && SysCfg().GetMultiArgs("-connect").size() > 0) {
		for (int64_t llLoop = 0;; llLoop++) {
			ProcessOneShot();
			vector<string> vstrTemp = SysCfg().GetMultiArgs("-connect");
			for (auto strAddr : vstrTemp) {
				CAddress cAddr;
				OpenNetworkConnection(cAddr, NULL, strAddr.c_str());
				for (int i = 0; i < 10 && i < llLoop; i++) {
					MilliSleep(500);
				}
			}
			MilliSleep(500);
		}
	}

	// Initiate network connections
	int64_t llStart = GetTime();
	while (true) {
		ProcessOneShot();

		MilliSleep(500);

		CSemaphoreGrant grant(*g_sSemOutbound);
		boost::this_thread::interruption_point();

		// Add seed nodes if DNS seeds are all down (an infrastructure attack?).
		if (g_cAddrman.size() == 0 && (GetTime() - llStart > 60)) {
			static bool bDone = false;
			if (!bDone) {
				LogPrint("INFO", "Adding fixed seed nodes as DNS doesn't seem to be available.\n");
				g_cAddrman.Add(SysCfg().FixedSeeds(), CNetAddr("127.0.0.1"));
				bDone = true;
			}
		}
		// Choose an address to connect to based on most recently seen
		CAddress cAddrConnect;

		// Only connect out to one peer per network group (/16 for IPv4).
		// Do this here so we don't have to critsect vNodes inside mapAddresses critsect.
		int nOutbound = 0;
		set<vector<unsigned char> > setConnected;
		{
			LOCK(g_cs_vNodes);
			for (auto pNode : g_vNodes) {
				if (!pNode->m_bInbound) {
					setConnected.insert(pNode->m_cAddress.GetGroup());
					nOutbound++;
				}
			}
		}

		int64_t llANow = GetAdjustedTime();

		int nTries = 0;
		while (true) {
			// use an nUnkBias between 10 (no outgoing connections) and 90 (8 outgoing connections)
			CAddress cAddr = g_cAddrman.Select(10 + min(nOutbound, 8) * 10);

			// if we selected an invalid address, restart
			if (!cAddr.IsValid() || (setConnected.count(cAddr.GetGroup()) && !SysCfg().IsInFixedSeeds(cAddr))
					|| IsLocal(cAddr))
				break;

			// If we didn't find an appropriate destination after trying 100 addresses fetched from addrman,
			// stop this loop, and let the outer loop run again (which sleeps, adds seed nodes, recalculates
			// already-connected network ranges, ...) before trying new addrman addresses.
			nTries++;
			if (nTries > 100) {
				break;
			}
			if (IsLimited(cAddr)) {
				continue;
			}
			// only consider very recently tried nodes after 30 failed attempts
			if (llANow - cAddr.nLastTry < 600 && nTries < 30) {
				continue;
			}

			// do not allow non-default ports, unless after 50 invalid addresses selected already
			if (cAddr.GetPort() != SysCfg().GetDefaultPort() && nTries < 50) {
				continue;
			}

			cAddrConnect = cAddr;
			break;
		}

		if (cAddrConnect.IsValid()) {
			OpenNetworkConnection(cAddrConnect, &grant);
		}
	}
}

void ThreadOpenAddedConnections() {
	{
		LOCK(g_cs_vAddedNodes);
		g_vAddedNodes = SysCfg().GetMultiArgs("-addnode");
	}

	if (HaveNameProxy()) {
		while (true) {
			list<string> lAddresses(0);
			{
				LOCK(g_cs_vAddedNodes);
				for (auto& strAddNode : g_vAddedNodes) {
					lAddresses.push_back(strAddNode);
				}
			}
			for (auto & strAddNode : lAddresses) {
				CAddress cAddr;
				CSemaphoreGrant grant(*g_sSemOutbound);
				OpenNetworkConnection(cAddr, &grant, strAddNode.c_str());
				MilliSleep(500);
			}
			MilliSleep(120000); // Retry every 2 minutes
		}
	}

	for (unsigned int i = 0; true; i++) {
		list<string> lAddresses(0);
		{
			LOCK(g_cs_vAddedNodes);
			for (auto & strAddNode : g_vAddedNodes)
				lAddresses.push_back(strAddNode);
		}

		list<vector<CService> > lservAddressesToAdd(0);
		for (auto& strAddNode : lAddresses) {
			vector<CService> vservNode(0);
			if (Lookup(strAddNode.c_str(), vservNode, SysCfg().GetDefaultPort(), fNameLookup, 0)) {
				lservAddressesToAdd.push_back(vservNode);
				{
					LOCK(g_cs_setservAddNodeAddresses);
					for (auto& serv : vservNode)
						g_setservAddNodeAddresses.insert(serv);
				}
			}
		}
		// Attempt to connect to each IP for each addnode entry until at least one is successful per addnode entry
		// (keeping in mind that addnode entries can have many IPs if fNameLookup)
		{
			LOCK(g_cs_vNodes);
			for (auto pnode : g_vNodes)
				for (list<vector<CService> >::iterator it = lservAddressesToAdd.begin();
						it != lservAddressesToAdd.end(); it++)
					for (auto& addrNode : *(it))
						if (pnode->m_cAddress == addrNode) {
							it = lservAddressesToAdd.erase(it);
							it--;
							break;
						}
		}
		for (auto& vserv : lservAddressesToAdd) {
			CSemaphoreGrant grant(*g_sSemOutbound);
			OpenNetworkConnection(CAddress(vserv[i % vserv.size()]), &grant);
			MilliSleep(500);
		}
		MilliSleep(120000); // Retry every 2 minutes
	}
}

// if successful, this moves the passed grant to the constructed node
bool OpenNetworkConnection(const CAddress& cAddrConnect, CSemaphoreGrant *pGrantOutbound, const char *strDest,
		bool bOneShot) {
	//
	// Initiate outbound network connection
	//
	boost::this_thread::interruption_point();
	if (!strDest)
		if (IsLocal(cAddrConnect) || FindNode((CNetAddr) cAddrConnect) || CNode::IsBanned(cAddrConnect)
				|| FindNode(cAddrConnect.ToStringIPPort().c_str())) {
			return false;
		}
	if (strDest && FindNode(strDest)) {
		return false;
	}

	CNode* pNode = ConnectNode(cAddrConnect, strDest);
	boost::this_thread::interruption_point();

	if (!pNode) {
		return false;
	}
	if (pGrantOutbound) {
		pGrantOutbound->MoveTo(pNode->m_cGrantOutbound);
	}
	pNode->m_bNetworkNode = true;
	if (bOneShot) {
		pNode->m_bOneShot = true;
	}

	return true;
}

// for now, use a very simple selection metric: the node from which we received
// most recently
static int64_t NodeSyncScore(const CNode *pNode) {
    return pNode->m_llLastRecv;
}

void static StartSync(const vector<CNode*> &vNodes) {
    CNode *pNodeNewSync = NULL;
    int64_t llBestScore = 0;

    int nBestHeight = g_signals.GetHeight().get_value_or(0);

    // Iterate over all nodes
    for (auto pNode : vNodes) {
        // check preconditions for allowing a sync
        if (!pNode->m_bClient && !pNode->m_bOneShot &&
            !pNode->m_bDisconnect && pNode->m_bSuccessfullyConnected &&
            (pNode->m_nStartingHeight > (nBestHeight - 144)) /*&& (pnode->nVersion < NOBLKS_VERSION_START || pnode->nVersion >= NOBLKS_VERSION_END)*/
            ) {
            // if ok, compare node's score with the best so far
            int64_t llScore = NodeSyncScore(pNode);
            if (pNodeNewSync == NULL || llScore > llBestScore) {
                pNodeNewSync = pNode;
                llBestScore = llScore;
            }
        }
    }
    // if a new sync candidate was found, start sync!
    if (pNodeNewSync) {
        pNodeNewSync->m_bStartSync = true;
        g_sNodeSync = pNodeNewSync;
    }
}

void ThreadMessageHandler() {
	SetThreadPriority(THREAD_PRIORITY_BELOW_NORMAL);
	while (true) {
		bool bHaveSyncNode = false;
		vector<CNode*> vNodesCopy;
		{
			LOCK(g_cs_vNodes);
			vNodesCopy = g_vNodes;
			for (auto pNode : vNodesCopy) {
				pNode->AddRef();
				if (pNode == g_sNodeSync) {
					bHaveSyncNode = true;
				}
			}
		}
		if (!bHaveSyncNode) {
			StartSync(vNodesCopy);
		}
		// Poll the connected nodes for messages
		CNode* pnodeTrickle = NULL;
		if (!vNodesCopy.empty()) {
			pnodeTrickle = vNodesCopy[GetRand(vNodesCopy.size())];
		}
		bool bSleep = true;

		for (auto pNode : vNodesCopy) {
			if (pNode->m_bDisconnect) {
				continue;
			}
			// Receive messages
			{
				TRY_LOCK(pNode->m_cs_vRecvMsg, lockRecv);
				if (lockRecv) {
					if (!g_signals.ProcessMessages(pNode)) {
						pNode->CloseSocketDisconnect();
					}
					if (pNode->m_unSendSize < SendBufferSize()) {
						if (!pNode->m_vRecvGetData.empty()
								|| (!pNode->m_vRecvMsg.empty() && pNode->m_vRecvMsg[0].complete())) {
							bSleep = false;
						}
					}
				}
			}
			boost::this_thread::interruption_point();

			// Send messages
			{
				TRY_LOCK(pNode->m_cs_vSend, lockSend);
				if (lockSend) {
					g_signals.SendMessages(pNode, pNode == pnodeTrickle);
				}
			}
			boost::this_thread::interruption_point();
		}

		{
			LOCK(g_cs_vNodes);
			for (auto pNode : vNodesCopy) {
				pNode->Release();
			}
		}
		if (bSleep) {
			MilliSleep(100);
		}
	}
}

bool BindListenPort(const CService &cBindAddr, string& strError) {
    strError = "";
    int nOne = 1;

    // Create socket for listening for incoming connections
    struct sockaddr_storage sockaddr;
    socklen_t len = sizeof(sockaddr);
	if (!cBindAddr.GetSockAddr((struct sockaddr*) &sockaddr, &len)) {
		strError = strprintf("Error: bind address family for %s not supported", cBindAddr.ToString());
		LogPrint("INFO","%s\n", strError);
		return false;
	}

    SOCKET hListenSocket = socket(((struct sockaddr*)&sockaddr)->sa_family, SOCK_STREAM, IPPROTO_TCP);
	if (hListenSocket == INVALID_SOCKET) {
		strError = strprintf("Error: Couldn't open socket for incoming connections (socket returned error %s)", NetworkErrorString(WSAGetLastError()));
		LogPrint("INFO","%s\n", strError);
		return false;
	}

#ifdef SO_NOSIGPIPE
    // Different way of disabling SIGPIPE on BSD
    setsockopt(hListenSocket, SOL_SOCKET, SO_NOSIGPIPE, (void*)&nOne, sizeof(int));
#endif

#ifndef WIN32
    // Allow binding if the port is still in TIME_WAIT state after
    // the program was closed and restarted.  Not an issue on windows.
    setsockopt(hListenSocket, SOL_SOCKET, SO_REUSEADDR, (void*)&nOne, sizeof(int));
#endif


#ifdef WIN32
    // Set to non-blocking, incoming connections will also inherit this
    if (ioctlsocket(hListenSocket, FIONBIO, (u_long*)&nOne) == SOCKET_ERROR)
#else
    if (fcntl(hListenSocket, F_SETFL, O_NONBLOCK) == SOCKET_ERROR)
#endif
    {
        strError = strprintf("Error: Couldn't set properties on socket for incoming connections (error %s)", NetworkErrorString(WSAGetLastError()));
        LogPrint("INFO","%s\n", strError);
        return false;
    }

    // some systems don't have IPV6_V6ONLY but are always v6only; others do have the option
    // and enable it by default or not. Try to enable it, if possible.
    if (cBindAddr.IsIPv6()) {
#ifdef IPV6_V6ONLY
#ifdef WIN32
        setsockopt(hListenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&nOne, sizeof(int));
#else
        setsockopt(hListenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&nOne, sizeof(int));
#endif
#endif
#ifdef WIN32
        int nProtLevel = 10 /* PROTECTION_LEVEL_UNRESTRICTED */;
        int nParameterId = 23 /* IPV6_PROTECTION_LEVEl */;
        // this call is allowed to fail
        setsockopt(hListenSocket, IPPROTO_IPV6, nParameterId, (const char*)&nProtLevel, sizeof(int));
#endif
    }

    if (::bind(hListenSocket, (struct sockaddr*)&sockaddr, len) == SOCKET_ERROR)
    {
        int nErr = WSAGetLastError();
        if (nErr == WSAEADDRINUSE)
            strError = strprintf(_("Unable to bind to %s on this computer. Dacrs Core is probably already running."), cBindAddr.ToString());
        else
            strError = strprintf(_("Unable to bind to %s on this computer (bind returned error %s)"), cBindAddr.ToString(), NetworkErrorString(nErr));
        LogPrint("INFO","%s\n", strError);
        return false;
    }
    LogPrint("INFO","Bound to %s\n", cBindAddr.ToString());

    // Listen for incoming connections
    if (listen(hListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        strError = strprintf(_("Error: Listening for incoming connections failed (listen returned error %s)"), NetworkErrorString(WSAGetLastError()));
        LogPrint("INFO","%s\n", strError);
        return false;
    }

    g_shListenSocket.push_back(hListenSocket);

    if (cBindAddr.IsRoutable() && g_bDiscover)
        AddLocal(cBindAddr, LOCAL_BIND);

    return true;
}

void static Discover(boost::thread_group& threadGroup) {
    if (!g_bDiscover) {
    	return;
    }

#ifdef WIN32
    // Get local host IP
    char pszHostName[1000] = "";
	if (gethostname(pszHostName, sizeof(pszHostName)) != SOCKET_ERROR) {
		vector<CNetAddr> vaddr;
		if (LookupHost(pszHostName, vaddr)) {
			for (const auto &addr : vaddr) {
				AddLocal(addr, LOCAL_IF);
			}
		}
	}
#else
    // Get local host ip
    struct ifaddrs* myaddrs;
    if (getifaddrs(&myaddrs) == 0)
    {
        for (struct ifaddrs* ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr == NULL) continue;
            if ((ifa->ifa_flags & IFF_UP) == 0) continue;
            if (strcmp(ifa->ifa_name, "lo") == 0) continue;
            if (strcmp(ifa->ifa_name, "lo0") == 0) continue;
            if (ifa->ifa_addr->sa_family == AF_INET)
            {
                struct sockaddr_in* s4 = (struct sockaddr_in*)(ifa->ifa_addr);
                CNetAddr addr(s4->sin_addr);
                if (AddLocal(addr, LOCAL_IF))
                    LogPrint("INFO","IPv4 %s: %s\n", ifa->ifa_name, addr.ToString());
            }
            else if (ifa->ifa_addr->sa_family == AF_INET6)
            {
                struct sockaddr_in6* s6 = (struct sockaddr_in6*)(ifa->ifa_addr);
                CNetAddr addr(s6->sin6_addr);
                if (AddLocal(addr, LOCAL_IF))
                    LogPrint("INFO","IPv6 %s: %s\n", ifa->ifa_name, addr.ToString());
            }
        }
        freeifaddrs(myaddrs);
    }
#endif

    // Don't use external IPv4 discovery, when -onlynet="IPv6"
    if (!IsLimited(NET_IPV4)) {
    	threadGroup.create_thread(boost::bind(&TraceThread<void (*)()>, "ext-ip", &ThreadGetMyExternalIP));
    }
}

void StartNode(boost::thread_group& threadGroup) {
    if (g_sSemOutbound == NULL) {
        // initialize semaphore
        int nMaxOutbound = min(g_sMaxOutbound_connections, g_nMaxConnections);
        g_sSemOutbound = new CSemaphore(nMaxOutbound);
    }
    if (g_sNodeLocalHost == NULL) {
    	 g_sNodeLocalHost = new CNode(INVALID_SOCKET, CAddress(CService("127.0.0.1", 0), g_ullLocalServices));
    }

    Discover(threadGroup);

    // Start threads
    if (!SysCfg().GetBoolArg("-dnsseed", true)) {
    	LogPrint("INFO","DNS seeding disabled\n");
    } else {
    	threadGroup.create_thread(boost::bind(&TraceThread<void (*)()>, "dnsseed", &ThreadDNSAddressSeed));
    }

#ifdef USE_UPNP
    // Map ports with UPnP
    MapPort(SysCfg().GetBoolArg("-upnp", USE_UPNP));
#endif

    // Send and receive from sockets, accept connections
    threadGroup.create_thread(boost::bind(&TraceThread<void (*)()>, "net", &ThreadSocketHandler));

    // Initiate outbound connections from -addnode
    threadGroup.create_thread(boost::bind(&TraceThread<void (*)()>, "addcon", &ThreadOpenAddedConnections));

    // Initiate outbound connections
    threadGroup.create_thread(boost::bind(&TraceThread<void (*)()>, "opencon", &ThreadOpenConnections));

    // Process messages
    threadGroup.create_thread(boost::bind(&TraceThread<void (*)()>, "msghand", &ThreadMessageHandler));

    // Dump network addresses
    threadGroup.create_thread(boost::bind(&LoopForever<void (*)()>, "dumpaddr", &DumpAddresses, DUMP_ADDRESSES_INTERVAL * 1000));
}

bool StopNode() {
	LogPrint("INFO", "StopNode()\n");
	MapPort(false);
	if (g_sSemOutbound) {
		for (int i = 0; i < g_sMaxOutbound_connections; i++) {
			g_sSemOutbound->post();
		}
	}
	MilliSleep(50);
	DumpAddresses();

	return true;
}

class CNetCleanup {
 public:
    CNetCleanup()
    {
    }
    ~CNetCleanup()
    {
        // Close sockets
        for (auto pnode : g_vNodes)
            if (pnode->m_hSocket != INVALID_SOCKET)
                closesocket(pnode->m_hSocket);
        for (auto hListenSocket : g_shListenSocket)
            if (hListenSocket != INVALID_SOCKET)
                if (closesocket(hListenSocket) == SOCKET_ERROR)
                    LogPrint("INFO","closesocket(hListenSocket) failed with error %s\n", NetworkErrorString(WSAGetLastError()));

        // clean up some globals (to help leak detection)
        for (auto pnode : g_vNodes)
            delete pnode;
        for (auto pnode : g_vNodesDisconnected)
            delete pnode;
        g_vNodes.clear();
        g_vNodesDisconnected.clear();
        delete g_sSemOutbound;
        g_sSemOutbound = NULL;
        delete g_sNodeLocalHost;
        g_sNodeLocalHost = NULL;

#ifdef WIN32
        // Shutdown Windows Sockets
        WSACleanup();
#endif
    }
}
instance_of_cnetcleanup;

void CNode::RecordBytesRecv(uint64_t ullBytes) {
	LOCK(m_s_cs_totalBytesRecv);
	m_sTotalBytesRecv += ullBytes;
}

void CNode::RecordBytesSent(uint64_t ullBytes) {
	LOCK(m_s_cs_totalBytesSent);
	m_sTotalBytesSent += ullBytes;
}

uint64_t CNode::GetTotalBytesRecv() {
	LOCK(m_s_cs_totalBytesRecv);
	return m_sTotalBytesRecv;
}

uint64_t CNode::GetTotalBytesSent() {
	LOCK(m_s_cs_totalBytesSent);
	return m_sTotalBytesSent;
}

void CNode::Fuzz(int nChance) {
	if (!m_bSuccessfullyConnected) {
		return; // Don't fuzz initial handshake
	}
	if (GetRand(nChance) != 0) {
		return; // Fuzz 1 of every nChance messages
	}

	switch (GetRand(3)) {
	case 0:
		// xor a random byte with a random value:
		if (!m_cDSSend.empty()) {
			CDataStream::size_type pos = GetRand(m_cDSSend.size());
			m_cDSSend[pos] ^= (unsigned char) (GetRand(256));
		}
		break;
	case 1:
		// delete a random byte:
		if (!m_cDSSend.empty()) {
			CDataStream::size_type pos = GetRand(m_cDSSend.size());
			m_cDSSend.erase(m_cDSSend.begin() + pos);
		}
		break;
	case 2:
		// insert a random byte at a random position
	{
		CDataStream::size_type pos = GetRand(m_cDSSend.size());
		char ch = (char) GetRand(256);
		m_cDSSend.insert(m_cDSSend.begin() + pos, ch);
	}
		break;
	}
	// Chance of more than one change half the time:
	// (more changes exponentially less likely):
	Fuzz(2);
}

void RelayTransaction(CBaseTransaction *pBaseTx, const uint256& cHash) {
	CDataStream cDS(SER_NETWORK, PROTOCOL_VERSION);
	cDS.reserve(10000);
	std::shared_ptr<CBaseTransaction> pTx = pBaseTx->GetNewInstance();
	cDS << pTx;
	RelayTransaction(pBaseTx, cHash, cDS);
}

void RelayTransaction(CBaseTransaction *pBaseTx, const uint256& cHash, const CDataStream& cDS) {
	CInv inv(MSG_TX, cHash);
	{
		LOCK(g_cs_mapRelay);
		// Expire old relay messages
		while (!g_vRelayExpiration.empty() && g_vRelayExpiration.front().first < GetTime()) {
			g_mapRelay.erase(g_vRelayExpiration.front().second);
			g_vRelayExpiration.pop_front();
		}
		// Save original serialized message so newer versions are preserved
		g_mapRelay.insert(make_pair(inv, cDS));
		g_vRelayExpiration.push_back(make_pair(GetTime() + 15 * 60, inv));
	}
	LOCK(g_cs_vNodes);
	for (auto pnode : g_vNodes) {
		if (!pnode->m_bRelayTxes) {
			continue;
		}
		LOCK(pnode->m_cs_filter);
		if (pnode->m_pBloomFilter) {
			if (pnode->m_pBloomFilter->IsRelevantAndUpdate(pBaseTx, cHash)) {
				pnode->PushInventory(inv);
				LogPrint("sendtx", "hash:%s time:%ld\n", inv.hash.GetHex(), GetTime());
			}
		} else {
			pnode->PushInventory(inv);
			LogPrint("sendtx", "hash:%s time:%ld\n", inv.hash.GetHex(), GetTime());
		}
	}
}

// CAddrDB
CAddrDB::CAddrDB() {
	m_pathAddr = GetDataDir() / "peers.dat";
}

bool CAddrDB::Write(const CAddrMan& cAddr) {
	// Generate random temporary filename
	unsigned short randv = 0;
	RAND_bytes((unsigned char *) &randv, sizeof(randv));
	string strTempFileName = strprintf("peers.dat.%04x", randv);

	// serialize addresses, checksum data up to that point, then append csum
	CDataStream cDSPeers(SER_DISK, CLIENT_VERSION);
	cDSPeers << FLATDATA(SysCfg().MessageStart());
	cDSPeers << cAddr;
	uint256 cHash = Hash(cDSPeers.begin(), cDSPeers.end());
	cDSPeers << cHash;

	// open temp output file, and associate with CAutoFile
	boost::filesystem::path tempPath = GetDataDir() / strTempFileName;
	FILE *pFile = fopen(tempPath.string().c_str(), "wb");
	CAutoFile cAutoFileout = CAutoFile(pFile, SER_DISK, CLIENT_VERSION);
	if (!cAutoFileout) {
		return ERRORMSG("%s : Failed to open file %s", __func__, tempPath.string());
	}
	// Write and commit header, data
	try {
		cAutoFileout << cDSPeers;
	} catch (std::exception &e) {
		return ERRORMSG("%s : Serialize or I/O error - %s", __func__, e.what());
	}
	FileCommit(cAutoFileout);
	cAutoFileout.fclose();

	// replace existing peers.dat, if any, with new peers.dat.XXXX
	if (!RenameOver(tempPath, m_pathAddr)) {
		return ERRORMSG("%s : Rename-into-place failed", __func__);
	}

	return true;
}

bool CAddrDB::Read(CAddrMan& cAddr) {
	// open input file, and associate with CAutoFile
	FILE *PFile = fopen(m_pathAddr.string().c_str(), "rb");
	CAutoFile cAutoFilein = CAutoFile(PFile, SER_DISK, CLIENT_VERSION);
	if (!cAutoFilein) {
		return ERRORMSG("%s : Failed to open file %s", __func__, m_pathAddr.string());
	}

	// use file size to size memory buffer
	int nFileSize = boost::filesystem::file_size(m_pathAddr);
	int nDataSize = nFileSize - sizeof(uint256);
	// Don't try to resize to a negative number if file is small
	if (nDataSize < 0) {
		nDataSize = 0;
	}
	vector<unsigned char> vchData;
	vchData.resize(nDataSize);
	uint256 cHashIn;

	// read data and checksum from file
	try {
		cAutoFilein.read((char *) &vchData[0], nDataSize);
		cAutoFilein >> cHashIn;
	} catch (std::exception &e) {
		return ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
	}
	cAutoFilein.fclose();

	CDataStream cDSPeers(vchData, SER_DISK, CLIENT_VERSION);
	// verify stored checksum matches input data
	uint256 hashTmp = Hash(cDSPeers.begin(), cDSPeers.end());
	if (cHashIn != hashTmp) {
		return ERRORMSG("%s : Checksum mismatch, data corrupted", __func__);
	}

	unsigned char pchMsgTmp[4];
	try {
		// de-serialize file header (network specific magic number) and ..
		cDSPeers >> FLATDATA(pchMsgTmp);
		// ... verify the network matches ours
		if (memcmp(pchMsgTmp, SysCfg().MessageStart(), sizeof(pchMsgTmp))) {
			return ERRORMSG("%s : Invalid network magic number", __func__);
		}
		// de-serialize address data into one CAddrMan object
		cDSPeers >> cAddr;
	} catch (std::exception &e) {
		return ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
	}

	return true;
}
