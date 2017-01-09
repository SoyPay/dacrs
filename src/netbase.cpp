// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "netbase.h"

#include "hash.h"
#include "sync.h"
#include "uint256.h"
#include "util.h"

#ifndef WIN32
#include <fcntl.h>
#endif

#include <boost/algorithm/string/case_conv.hpp> // for to_lower()
#include <boost/algorithm/string/predicate.hpp> // for startswith() and endswith()
#include "chainparams.h"

#if !defined(HAVE_MSG_NOSIGNAL) && !defined(MSG_NOSIGNAL)
#define MSG_NOSIGNAL 0
#endif

using namespace std;

// Settings
static proxyType g_sProxyInfo[NET_MAX];
static proxyType g_sNameProxyInfo;
static CCriticalSection g_cs_proxyInfos;
//int nConnectTimeout = 5000;
bool g_bNameLookup = false;

static const unsigned char g_sIPv4[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff };

enum Network ParseNetwork(string strNet) {
	boost::to_lower(strNet);
	if (strNet == "ipv4") {
		return NET_IPV4;
	}
	if (strNet == "ipv6") {
		return NET_IPV6;
	}
	if (strNet == "tor") {
		return NET_TOR;
	}
	return NET_UNROUTABLE;
}

void SplitHostPort(std::string strIn, int &nPortOut, std::string &strHostOut) {
	size_t unColon = strIn.find_last_of(':');
	// if a : is found, and it either follows a [...], or no other : is in the string, treat it as port separator
	bool bHaveColon = unColon != strIn.npos;
	bool bBracketed = bHaveColon && (strIn[0] == '[' && strIn[unColon - 1] == ']'); // if there is a colon, and in[0]=='[', colon is not 0, so in[colon-1] is safe
	bool bMultiColon = bHaveColon && (strIn.find_last_of(':', unColon - 1) != strIn.npos);
	if (bHaveColon && (unColon == 0 || bBracketed || !bMultiColon)) {
		char *pEndp = NULL;
		int n = strtol(strIn.c_str() + unColon + 1, &pEndp, 10);
		if (pEndp && *pEndp == 0 && n >= 0) {
			strIn = strIn.substr(0, unColon);
			if (n > 0 && n < 0x10000) {
				nPortOut = n;
			}
		}
	}
	if (strIn.size() > 0 && strIn[0] == '[' && strIn[strIn.size() - 1] == ']') {
		strHostOut = strIn.substr(1, strIn.size() - 2);
	} else {
		strHostOut = strIn;
	}
}

bool static LookupIntern(const char *pszName, vector<CNetAddr>& vIP, unsigned int nMaxSolutions, bool fAllowLookup)
{
    vIP.clear();

    {
        CNetAddr addr;
        if (addr.SetSpecial(string(pszName))) {
            vIP.push_back(addr);
            return true;
        }
    }

    struct addrinfo tAiHint;
    memset(&tAiHint, 0, sizeof(struct addrinfo));

    tAiHint.ai_socktype = SOCK_STREAM;
    tAiHint.ai_protocol = IPPROTO_TCP;
    tAiHint.ai_family = AF_UNSPEC;
#ifdef WIN32
    tAiHint.ai_flags = fAllowLookup ? 0 : AI_NUMERICHOST;
#else
    tAiHint.ai_flags = fAllowLookup ? AI_ADDRCONFIG : AI_NUMERICHOST;
#endif
    struct addrinfo *pAiRes = NULL;
    int nErr = getaddrinfo(pszName, NULL, &tAiHint, &pAiRes);
	if (nErr) {
		return false;
	}

    struct addrinfo *aiTrav = pAiRes;
    while (aiTrav != NULL && (nMaxSolutions == 0 || vIP.size() < nMaxSolutions))
    {
        if (aiTrav->ai_family == AF_INET)
        {
            assert(aiTrav->ai_addrlen >= sizeof(sockaddr_in));
            vIP.push_back(CNetAddr(((struct sockaddr_in*)(aiTrav->ai_addr))->sin_addr));
        }

        if (aiTrav->ai_family == AF_INET6)
        {
            assert(aiTrav->ai_addrlen >= sizeof(sockaddr_in6));
            vIP.push_back(CNetAddr(((struct sockaddr_in6*)(aiTrav->ai_addr))->sin6_addr));
        }

        aiTrav = aiTrav->ai_next;
    }

    freeaddrinfo(pAiRes);

    return (vIP.size() > 0);
}

bool LookupHost(const char *pszName, vector<CNetAddr>& vcIP, unsigned int nMaxSolutions, bool bAllowLookup) {
	string strHost(pszName);
	if (strHost.empty()) {
		return false;
	}
	if (boost::algorithm::starts_with(strHost, "[") && boost::algorithm::ends_with(strHost, "]")) {
		strHost = strHost.substr(1, strHost.size() - 2);
	}

	return LookupIntern(strHost.c_str(), vcIP, nMaxSolutions, bAllowLookup);
}

bool LookupHostNumeric(const char *pszName, vector<CNetAddr>& vcIP, unsigned int unMaxSolutions) {
	return LookupHost(pszName, vcIP, unMaxSolutions, false);
}

bool Lookup(const char *pszName, vector<CService>& vcAddr, int nPortDefault, bool bAllowLookup,
		unsigned int unMaxSolutions) {
	if (pszName[0] == 0) {
		return false;
	}
	int nPort = nPortDefault;
	string strHostname = "";
	SplitHostPort(string(pszName), nPort, strHostname);

	vector<CNetAddr> vcIP;
	bool bRet = LookupIntern(strHostname.c_str(), vcIP, unMaxSolutions, bAllowLookup);
	if (!bRet) {
		return false;
	}
	vcAddr.resize(vcIP.size());
	for (unsigned int i = 0; i < vcIP.size(); i++) {
		vcAddr[i] = CService(vcIP[i], nPort);
	}

	return true;
}

bool Lookup(const char *pszName, CService& cAddr, int nPortDefault, bool bAllowLookup) {
	vector<CService> vcService;
	bool bRet = Lookup(pszName, vcService, nPortDefault, bAllowLookup, 1);
	if (!bRet) {
		return false;
	}
	cAddr = vcService[0];

	return true;
}

bool LookupNumeric(const char *pszName, CService& cAddr, int nPortDefault) {
	return Lookup(pszName, cAddr, nPortDefault, false);
}

bool static Socks4(const CService &addrDest, SOCKET& hSocket) {
	LogPrint("INFO", "SOCKS4 connecting %s\n", addrDest.ToString());
	if (!addrDest.IsIPv4()) {
		closesocket(hSocket);
		return ERRORMSG("Proxy destination is not IPv4");
	}
	char pszSocks4IP[] = "\4\1\0\0\0\0\0\0user";
	struct sockaddr_in tAddr;
	socklen_t len = sizeof(tAddr);
	if (!addrDest.GetSockAddr((struct sockaddr*) &tAddr, &len) || tAddr.sin_family != AF_INET) {
		closesocket(hSocket);
		return ERRORMSG("Cannot get proxy destination address");
	}
	memcpy(pszSocks4IP + 2, &tAddr.sin_port, 2);
	memcpy(pszSocks4IP + 4, &tAddr.sin_addr, 4);
	char* pszSocks4 = pszSocks4IP;
	int nSize = sizeof(pszSocks4IP);

	int nRet = send(hSocket, pszSocks4, nSize, MSG_NOSIGNAL);
	if (nRet != nSize) {
		closesocket(hSocket);
		return ERRORMSG("Error sending to proxy");
	}
	char arrchRet[8];
	if (recv(hSocket, arrchRet, 8, 0) != 8) {
		closesocket(hSocket);
		return ERRORMSG("Error reading proxy response");
	}
	if (arrchRet[1] != 0x5a) {
		closesocket(hSocket);
		if (arrchRet[1] != 0x5b) {
			LogPrint("INFO", "ERROR: Proxy returned error %d\n", arrchRet[1]);
		}
		return false;
	}
	LogPrint("INFO", "SOCKS4 connected %s\n", addrDest.ToString());

	return true;
}

bool static Socks5(string strDest, int port, SOCKET& hSocket) {
	LogPrint("INFO", "SOCKS5 connecting %s\n", strDest);
	if (strDest.size() > 255) {
		closesocket(hSocket);
		return ERRORMSG("Hostname too long");
	}
	char pszSocks5Init[] = "\5\1\0";
	ssize_t nSize = sizeof(pszSocks5Init) - 1;

	ssize_t nRet = send(hSocket, pszSocks5Init, nSize, MSG_NOSIGNAL);
	if (nRet != nSize) {
		closesocket(hSocket);
		return ERRORMSG("Error sending to proxy");
	}
	char arrchRet1[2];
	if (recv(hSocket, arrchRet1, 2, 0) != 2) {
		closesocket(hSocket);
		return ERRORMSG("Error reading proxy response");
	}
	if (arrchRet1[0] != 0x05 || arrchRet1[1] != 0x00) {
		closesocket(hSocket);
		return ERRORMSG("Proxy failed to initialize");
	}
	string strSocks5("\5\1");
	strSocks5 += '\000';
	strSocks5 += '\003';
	strSocks5 += static_cast<char>(min((int) strDest.size(), 255));
	strSocks5 += strDest;
	strSocks5 += static_cast<char>((port >> 8) & 0xFF);
	strSocks5 += static_cast<char>((port >> 0) & 0xFF);
	nRet = send(hSocket, strSocks5.c_str(), strSocks5.size(), MSG_NOSIGNAL);
	if (nRet != (ssize_t) strSocks5.size()) {
		closesocket(hSocket);
		return ERRORMSG("Error sending to proxy");
	}
	char arrchRet2[4];
	if (recv(hSocket, arrchRet2, 4, 0) != 4) {
		closesocket(hSocket);
		return ERRORMSG("Error reading proxy response");
	}
	if (arrchRet2[0] != 0x05) {
		closesocket(hSocket);
		return ERRORMSG("Proxy failed to accept request");
	}
	if (arrchRet2[1] != 0x00) {
		closesocket(hSocket);
		switch (arrchRet2[1]) {
		case 0x01: {
			return ERRORMSG("Proxy error: general failure");
		}
		case 0x02: {
			return ERRORMSG("Proxy error: connection not allowed");
		}
		case 0x03: {
			return ERRORMSG("Proxy error: network unreachable");
		}
		case 0x04: {
			return ERRORMSG("Proxy error: host unreachable");
		}
		case 0x05: {
			return ERRORMSG("Proxy error: connection refused");
		}
		case 0x06: {
			return ERRORMSG("Proxy error: TTL expired");
		}
		case 0x07: {
			return ERRORMSG("Proxy error: protocol error");
		}
		case 0x08: {
			return ERRORMSG("Proxy error: address type not supported");
		}
		default: {
			return ERRORMSG("Proxy error: unknown");
		}
		}
	}
	if (arrchRet2[2] != 0x00) {
		closesocket(hSocket);
		return ERRORMSG("Error: malformed proxy response");
	}
	char arrchRet3[256];
	switch (arrchRet2[3]) {
	case 0x01: {
		nRet = recv(hSocket, arrchRet3, 4, 0) != 4;
		break;
	}
	case 0x04: {
		nRet = recv(hSocket, arrchRet3, 16, 0) != 16;
		break;
	}
	case 0x03: {
		nRet = recv(hSocket, arrchRet3, 1, 0) != 1;
		if (nRet) {
			closesocket(hSocket);
			return ERRORMSG("Error reading from proxy");
		}
		int nRecv = arrchRet3[0];
		nRet = recv(hSocket, arrchRet3, nRecv, 0) != nRecv;
		break;
	}
	default: {
		closesocket(hSocket);
		return ERRORMSG("Error: malformed proxy response");
	}
	}
	if (nRet) {
		closesocket(hSocket);
		return ERRORMSG("Error reading from proxy");
	}
	if (recv(hSocket, arrchRet3, 2, 0) != 2) {
		closesocket(hSocket);
		return ERRORMSG("Error reading from proxy");
	}
	LogPrint("INFO", "SOCKS5 connected %s\n", strDest);
	return true;
}

bool static ConnectSocketDirectly(const CService &addrConnect, SOCKET& hSocketRet, int nTimeout) {
    hSocketRet = INVALID_SOCKET;

    struct sockaddr_storage sockaddr;
    socklen_t nLen = sizeof(sockaddr);
    if (!addrConnect.GetSockAddr((struct sockaddr*)&sockaddr, &nLen)) {
        LogPrint("INFO","Cannot connect to %s: unsupported network\n", addrConnect.ToString());
        return false;
    }

    SOCKET hSocket = socket(((struct sockaddr*)&sockaddr)->sa_family, SOCK_STREAM, IPPROTO_TCP);
    if (hSocket == INVALID_SOCKET) {
    	return false;
    }

#ifdef SO_NOSIGPIPE
    int set = 1;
    setsockopt(hSocket, SOL_SOCKET, SO_NOSIGPIPE, (void*)&set, sizeof(int));
#endif

#ifdef WIN32
    u_long fNonblock = 1;
    if (ioctlsocket(hSocket, FIONBIO, &fNonblock) == SOCKET_ERROR)
#else
    int fFlags = fcntl(hSocket, F_GETFL, 0);
    if (fcntl(hSocket, F_SETFL, fFlags | O_NONBLOCK) == -1)
#endif
    {
        closesocket(hSocket);
        return false;
    }

    if (connect(hSocket, (struct sockaddr*)&sockaddr, nLen) == SOCKET_ERROR) {
        // WSAEINVAL is here because some legacy version of winsock uses it
        if (WSAGetLastError() == WSAEINPROGRESS || WSAGetLastError() == WSAEWOULDBLOCK || WSAGetLastError() == WSAEINVAL) {
            struct timeval tTimeout;
            tTimeout.tv_sec  = nTimeout / 1000;
            tTimeout.tv_usec = (nTimeout % 1000) * 1000;

            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(hSocket, &fdset);
            int nRet = select(hSocket + 1, NULL, &fdset, NULL, &tTimeout);
			if (nRet == 0) {
				LogPrint("net", "connection to %s tTimeout\n", addrConnect.ToString());
				closesocket(hSocket);
				return false;
			}
			if (nRet == SOCKET_ERROR) {
				LogPrint("net", "select() for %s failed: %s\n", addrConnect.ToString(),
						NetworkErrorString(WSAGetLastError()));
				closesocket(hSocket);
				return false;
			}
            socklen_t nRetSize = sizeof(nRet);
#ifdef WIN32
            if (getsockopt(hSocket, SOL_SOCKET, SO_ERROR, (char*)(&nRet), &nRetSize) == SOCKET_ERROR)
#else
            if (getsockopt(hSocket, SOL_SOCKET, SO_ERROR, &nRet, &nRetSize) == SOCKET_ERROR)
#endif
            {
                LogPrint("net","getsockopt() for %s failed: %s\n", addrConnect.ToString(), NetworkErrorString(WSAGetLastError()));
                closesocket(hSocket);
                return false;
            }
			if (nRet != 0) {
				LogPrint("net", "connect() to %s failed after select(): %s\n", addrConnect.ToString(),
						NetworkErrorString(nRet));
				closesocket(hSocket);
				return false;
			}
        }
#ifdef WIN32
        else if (WSAGetLastError() != WSAEISCONN)
#else
        else
#endif
        {
            LogPrint("INFO","connect() to %s failed: %s\n", addrConnect.ToString(), NetworkErrorString(WSAGetLastError()));
            closesocket(hSocket);
            return false;
        }
    }

    // this isn't even strictly necessary
    // CNode::ConnectNode immediately turns the socket back to non-blocking
    // but we'll turn it back to blocking just in case
#ifdef WIN32
    fNonblock = 0;
    if (ioctlsocket(hSocket, FIONBIO, &fNonblock) == SOCKET_ERROR)
#else
    fFlags = fcntl(hSocket, F_GETFL, 0);
    if (fcntl(hSocket, F_SETFL, fFlags & ~O_NONBLOCK) == SOCKET_ERROR)
#endif
    {
        closesocket(hSocket);
        return false;
    }

    hSocketRet = hSocket;
    return true;
}

bool SetProxy(enum Network netWork, CService cAddrProxy, int nSocksVersion) {
    assert(netWork >= 0 && netWork < NET_MAX);
    if (nSocksVersion != 0 && nSocksVersion != 4 && nSocksVersion != 5) {
    	 return false;
    }
    if (nSocksVersion != 0 && !cAddrProxy.IsValid()) {
    	 return false;
    }
    LOCK(g_cs_proxyInfos);
    g_sProxyInfo[netWork] = make_pair(cAddrProxy, nSocksVersion);
    return true;
}

bool GetProxy(enum Network netWork, proxyType &proxyInfoOut) {
    assert(netWork >= 0 && netWork < NET_MAX);
    LOCK(g_cs_proxyInfos);
    if (!g_sProxyInfo[netWork].second) {
    	 return false;
    }

    proxyInfoOut = g_sProxyInfo[netWork];
    return true;
}

bool SetNameProxy(CService cAddrProxy, int nSocksVersion) {
    if (nSocksVersion != 0 && nSocksVersion != 5) {
    	 return false;
    }
    if (nSocksVersion != 0 && !cAddrProxy.IsValid()) {
    	return false;
    }

    LOCK(g_cs_proxyInfos);
    g_sNameProxyInfo = make_pair(cAddrProxy, nSocksVersion);
    return true;
}

bool GetNameProxy(proxyType &nameproxyInfoOut) {
    LOCK(g_cs_proxyInfos);
    if (!g_sNameProxyInfo.second) {
    	return false;
    }

    nameproxyInfoOut = g_sNameProxyInfo;
    return true;
}

bool HaveNameProxy() {
    LOCK(g_cs_proxyInfos);
    return g_sNameProxyInfo.second != 0;
}

bool IsProxy(const CNetAddr &cNetAddr) {
    LOCK(g_cs_proxyInfos);
    for (int i = 0; i < NET_MAX; i++) {
        if (g_sProxyInfo[i].second && (cNetAddr == (CNetAddr)g_sProxyInfo[i].first)) {
        	return true;
        }
    }
    return false;
}

int GetConnectTime() {
	return SysCfg().getConnectTimeOut();
}

bool ConnectSocket(const CService &cAddrDest, SOCKET& hSocketRet, int nTimeout) {
	proxyType proxy;
	// no proxy needed
	if (!GetProxy(cAddrDest.GetNetwork(), proxy)) {
		return ConnectSocketDirectly(cAddrDest, hSocketRet, nTimeout);
	}

	SOCKET hSocket = INVALID_SOCKET;
	// first connect to proxy server
	if (!ConnectSocketDirectly(proxy.first, hSocket, nTimeout)) {
		return false;
	}

	// do socks negotiation
	switch (proxy.second) {
	case 4:
		if (!Socks4(cAddrDest, hSocket)) {
			return false;
		}
		break;
	case 5:
		if (!Socks5(cAddrDest.ToStringIP(), cAddrDest.GetPort(), hSocket)) {
			return false;
		}
		break;
	default:
		closesocket(hSocket);
		return false;
	}

	hSocketRet = hSocket;
	return true;
}

bool ConnectSocketByName(CService &cAddr, SOCKET& hSocketRet, const char *pszDest, int nPortDefault, int nTimeout) {
	string strDest;
	int nPort = nPortDefault;
	SplitHostPort(string(pszDest), nPort, strDest);

	SOCKET hSocket = INVALID_SOCKET;

	proxyType nameproxy;
	GetNameProxy(nameproxy);

	CService cAddrResolved(CNetAddr(strDest, g_bNameLookup && !nameproxy.second), nPort);
	if (cAddrResolved.IsValid()) {
		cAddr = cAddrResolved;
		return ConnectSocket(cAddr, hSocketRet, nTimeout);
	}
	cAddr = CService("0.0.0.0:0");
	if (!nameproxy.second) {
		return false;
	}
	if (!ConnectSocketDirectly(nameproxy.first, hSocket, nTimeout)) {
		return false;
	}

	switch (nameproxy.second) {
	default:
	case 4: {
		closesocket(hSocket);
		return false;
	}
	case 5: {
		if (!Socks5(strDest, nPort, hSocket)) {
			return false;
		}
		break;
	}
	}

	hSocketRet = hSocket;
	return true;
}

void CNetAddr::Init() {
	memset(m_chIp, 0, sizeof(m_chIp));
}

void CNetAddr::SetIP(const CNetAddr& cIpIn) {
	memcpy(m_chIp, cIpIn.m_chIp, sizeof(m_chIp));
}

static const unsigned char pchOnionCat[] = {0xFD,0x87,0xD8,0x7E,0xEB,0x43};

bool CNetAddr::SetSpecial(const string &strName) {
	if (strName.size() > 6 && strName.substr(strName.size() - 6, 6) == ".onion") {
		vector<unsigned char> vchAddr = DecodeBase32(strName.substr(0, strName.size() - 6).c_str());
		if (vchAddr.size() != 16 - sizeof(pchOnionCat)) {
			return false;
		}

		memcpy(m_chIp, pchOnionCat, sizeof(pchOnionCat));
		for (unsigned int i = 0; i < 16 - sizeof(pchOnionCat); i++) {
			m_chIp[i + sizeof(pchOnionCat)] = vchAddr[i];
		}
		return true;
	}
	return false;
}

CNetAddr::CNetAddr() {
	Init();
}

CNetAddr::CNetAddr(const struct in_addr& ipv4Addr) {
	memcpy(m_chIp, g_sIPv4, 12);
	memcpy(m_chIp + 12, &ipv4Addr, 4);
}

CNetAddr::CNetAddr(const struct in6_addr& ipv6Addr) {
	memcpy(m_chIp, &ipv6Addr, 16);
}

CNetAddr::CNetAddr(const char *pszIp, bool bAllowLookup) {
	Init();
	vector<CNetAddr> vcIP;
	if (LookupHost(pszIp, vcIP, 1, bAllowLookup)) {
		*this = vcIP[0];
	}
}

CNetAddr::CNetAddr(const string &strIp, bool bAllowLookup) {
	Init();
	vector<CNetAddr> vcIP;
	if (LookupHost(strIp.c_str(), vcIP, 1, bAllowLookup)) {
		*this = vcIP[0];
	}
}

unsigned int CNetAddr::GetByte(int n) const {
	return m_chIp[15 - n];
}

bool CNetAddr::IsIPv4() const {
	return (memcmp(m_chIp, g_sIPv4, sizeof(g_sIPv4)) == 0);
}

bool CNetAddr::IsIPv6() const {
	return (!IsIPv4() && !IsTor());
}

bool CNetAddr::IsRFC1918() const {
	return IsIPv4()
			&& (GetByte(3) == 10 || (GetByte(3) == 192 && GetByte(2) == 168)
					|| (GetByte(3) == 172 && (GetByte(2) >= 16 && GetByte(2) <= 31)));
}

bool CNetAddr::IsRFC3927() const {
    return IsIPv4() && (GetByte(3) == 169 && GetByte(2) == 254);
}

bool CNetAddr::IsRFC3849() const {
    return GetByte(15) == 0x20 && GetByte(14) == 0x01 && GetByte(13) == 0x0D && GetByte(12) == 0xB8;
}

bool CNetAddr::IsRFC3964() const {
    return (GetByte(15) == 0x20 && GetByte(14) == 0x02);
}

bool CNetAddr::IsRFC6052() const {
    static const unsigned char pchRFC6052[] = {0,0x64,0xFF,0x9B,0,0,0,0,0,0,0,0};
    return (memcmp(m_chIp, pchRFC6052, sizeof(pchRFC6052)) == 0);
}

bool CNetAddr::IsRFC4380() const {
    return (GetByte(15) == 0x20 && GetByte(14) == 0x01 && GetByte(13) == 0 && GetByte(12) == 0);
}

bool CNetAddr::IsRFC4862() const {
    static const unsigned char pchRFC4862[] = {0xFE,0x80,0,0,0,0,0,0};
    return (memcmp(m_chIp, pchRFC4862, sizeof(pchRFC4862)) == 0);
}

bool CNetAddr::IsRFC4193() const {
    return ((GetByte(15) & 0xFE) == 0xFC);
}

bool CNetAddr::IsRFC6145() const {
    static const unsigned char pchRFC6145[] = {0,0,0,0,0,0,0,0,0xFF,0xFF,0,0};
    return (memcmp(m_chIp, pchRFC6145, sizeof(pchRFC6145)) == 0);
}

bool CNetAddr::IsRFC4843() const {
    return (GetByte(15) == 0x20 && GetByte(14) == 0x01 && GetByte(13) == 0x00 && (GetByte(12) & 0xF0) == 0x10);
}

bool CNetAddr::IsTor() const {
    return (memcmp(m_chIp, pchOnionCat, sizeof(pchOnionCat)) == 0);
}

bool CNetAddr::IsLocal() const {
    // IPv4 loopback
   if (IsIPv4() && (GetByte(3) == 127 || GetByte(3) == 0)) {
	   return true;
   }
   // IPv6 loopback (::1/128)
   static const unsigned char pchLocal[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
   if (memcmp(m_chIp, pchLocal, 16) == 0) {
	   return true;
   }

   return false;
}

bool CNetAddr::IsMulticast() const
{
    return    (IsIPv4() && (GetByte(3) & 0xF0) == 0xE0)
           || (GetByte(15) == 0xFF);
}

bool CNetAddr::IsValid() const {
    // Cleanup 3-byte shifted addresses caused by garbage in size field
    // of addr messages from versions before 0.2.9 checksum.
    // Two consecutive addr messages look like this:
    // header20 vectorlen3 addr26 addr26 addr26 header20 vectorlen3 addr26 addr26 addr26...
    // so if the first length field is garbled, it reads the second batch
    // of addr misaligned by 3 bytes.
    if (memcmp(m_chIp, g_sIPv4+3, sizeof(g_sIPv4)-3) == 0) {
    	return false;
    }
    // unspecified IPv6 address (::/128)
    unsigned char ipNone[16] = {};
    if (memcmp(m_chIp, ipNone, 16) == 0) {
    	return false;
    }
    // documentation IPv6 address
    if (IsRFC3849()) {
    	return false;
    }
    if (IsIPv4()) {
        // INADDR_NONE
        uint32_t uIpNone = INADDR_NONE;
        if (memcmp(m_chIp+12, &uIpNone, 4) == 0) {
        	return false;
        }
        // 0
        uIpNone = 0;
        if (memcmp(m_chIp+12, &uIpNone, 4) == 0) {
        	return false;
        }
    }

    return true;
}

bool CNetAddr::IsRoutable() const {
    return IsValid() && !(IsRFC1918() || IsRFC3927() || IsRFC4862() || (IsRFC4193() && !IsTor()) || IsRFC4843() || IsLocal());
}

enum Network CNetAddr::GetNetwork() const {
    if (!IsRoutable()) {
    	return NET_UNROUTABLE;
    }
    if (IsIPv4()) {
    	return NET_IPV4;
    }
    if (IsTor()) {
    	return NET_TOR;
    }

    return NET_IPV6;
}

string CNetAddr::ToStringIP() const {
    if (IsTor()) {
    	return EncodeBase32(&m_chIp[6], 10) + ".onion";
    }
    CService cService(*this, 0);
    struct sockaddr_storage tSockaddr;
    socklen_t socklen = sizeof(tSockaddr);
    if (cService.GetSockAddr((struct sockaddr*)&tSockaddr, &socklen)) {
        char szName[1025] = "";
        if (!getnameinfo((const struct sockaddr*)&tSockaddr, socklen, szName, sizeof(szName), NULL, 0, NI_NUMERICHOST)) {
        	return string(szName);
        }
    }
    if (IsIPv4()) {
    	return strprintf("%u.%u.%u.%u", GetByte(3), GetByte(2), GetByte(1), GetByte(0));
    } else {
    	return strprintf("%x:%x:%x:%x:%x:%x:%x:%x",
    	                  GetByte(15) << 8 | GetByte(14), GetByte(13) << 8 | GetByte(12),
    	                  GetByte(11) << 8 | GetByte(10), GetByte(9) << 8 | GetByte(8),
    	                  GetByte(7) << 8 | GetByte(6), GetByte(5) << 8 | GetByte(4),
    	                  GetByte(3) << 8 | GetByte(2), GetByte(1) << 8 | GetByte(0));
    }

}

string CNetAddr::ToString() const {
	return ToStringIP();
}

bool operator==(const CNetAddr& cNetAddrA, const CNetAddr& cNetAddrB) {
	return (memcmp(cNetAddrA.m_chIp, cNetAddrB.m_chIp, 16) == 0);
}

bool operator!=(const CNetAddr& cNetAddrA, const CNetAddr& cNetAddrB) {
	return (memcmp(cNetAddrA.m_chIp, cNetAddrB.m_chIp, 16) != 0);
}

bool operator<(const CNetAddr& cNetAddrA, const CNetAddr& cNetAddrB) {
	return (memcmp(cNetAddrA.m_chIp, cNetAddrB.m_chIp, 16) < 0);
}

bool CNetAddr::GetInAddr(struct in_addr* pipv4Addr) const {
	if (!IsIPv4()) {
		return false;
	}
	memcpy(pipv4Addr, m_chIp + 12, 4);
	return true;
}

bool CNetAddr::GetIn6Addr(struct in6_addr* pipv6Addr) const {
	memcpy(pipv6Addr, m_chIp, 16);
	return true;
}

// get canonical identifier of an address' group
// no two connections will be attempted to addresses with the same group
vector<unsigned char> CNetAddr::GetGroup() const
{
    vector<unsigned char> vchRet;
    int nClass = NET_IPV6;
    int nStartByte = 0;
    int nBits = 16;
    // all local addresses belong to the same group
	if (IsLocal()) {
		nClass = 255;
		nBits = 0;
	}
	// all unroutable addresses belong to the same group
	if (!IsRoutable()) {
		nClass = NET_UNROUTABLE;
		nBits = 0;
	}
	// for IPv4 addresses, '1' + the 16 higher-order bits of the IP
	// includes mapped IPv4, SIIT translated IPv4, and the well-known prefix
	else if (IsIPv4() || IsRFC6145() || IsRFC6052()) {
		nClass = NET_IPV4;
		nStartByte = 12;
	}
	// for 6to4 tunnelled addresses, use the encapsulated IPv4 address
	else if (IsRFC3964()) {
		nClass = NET_IPV4;
		nStartByte = 2;
	}
	// for Teredo-tunnelled IPv6 addresses, use the encapsulated IPv4 address
	else if (IsRFC4380()) {
		vchRet.push_back(NET_IPV4);
		vchRet.push_back(GetByte(3) ^ 0xFF);
		vchRet.push_back(GetByte(2) ^ 0xFF);
		return vchRet;
	} else if (IsTor()) {
		nClass = NET_TOR;
		nStartByte = 6;
		nBits = 4;
	}
	// for he.net, use /36 groups
	else if (GetByte(15) == 0x20 && GetByte(14) == 0x01 && GetByte(13) == 0x04 && GetByte(12) == 0x70)
		nBits = 36;
	// for the rest of the IPv6 network, use /32 groups
	else {
		nBits = 32;
	}

    vchRet.push_back(nClass);
	while (nBits >= 8) {
		vchRet.push_back(GetByte(15 - nStartByte));
		nStartByte++;
		nBits -= 8;
	}
    if (nBits > 0) {
    	vchRet.push_back(GetByte(15 - nStartByte) | ((1 << nBits) - 1));
    }

    return vchRet;
}

uint64_t CNetAddr::GetHash() const {
	uint256 cHash = Hash(&m_chIp[0], &m_chIp[16]);
	uint64_t ullRet;
	memcpy(&ullRet, &cHash, sizeof(ullRet));
	return ullRet;
}

void CNetAddr::print() const {
	LogPrint("INFO", "CNetAddr(%s)\n", ToString());
}

// private extensions to enum Network, only returned by GetExtNetwork,
// and only used in GetReachabilityFrom
static const int NET_UNKNOWN = NET_MAX + 0;
static const int NET_TEREDO  = NET_MAX + 1;
int static GetExtNetwork(const CNetAddr *pAddr) {
	if (pAddr == NULL) {
		return NET_UNKNOWN;
	}
	if (pAddr->IsRFC4380()) {
		return NET_TEREDO;
	}

	return pAddr->GetNetwork();
}

/** Calculates a metric for how reachable (*this) is from a given partner */
int CNetAddr::GetReachabilityFrom(const CNetAddr *paddrPartner) const
{
    enum emReachability {
        EM_REACH_UNREACHABLE,
        EM_REACH_DEFAULT,
        EM_REACH_TEREDO,
        EM_REACH_IPV6_WEAK,
        EM_REACH_IPV4,
        EM_REACH_IPV6_STRONG,
        EM_REACH_PRIVATE
    };

    if (!IsRoutable()) {
    	return EM_REACH_UNREACHABLE;
    }

    int nOurNet 	= GetExtNetwork(this);
    int nTheirNet 	= GetExtNetwork(paddrPartner);
    bool bTunnel 	= IsRFC3964() || IsRFC6052() || IsRFC6145();

    switch(nTheirNet) {
    case NET_IPV4:
        switch(nOurNet) {
        default:       return EM_REACH_DEFAULT;
        case NET_IPV4: return EM_REACH_IPV4;
        }
    case NET_IPV6:
        switch(nOurNet) {
        default:         return EM_REACH_DEFAULT;
        case NET_TEREDO: return EM_REACH_TEREDO;
        case NET_IPV4:   return EM_REACH_IPV4;
        case NET_IPV6:   return bTunnel ? EM_REACH_IPV6_WEAK : EM_REACH_IPV6_STRONG; // only prefer giving our IPv6 address if it's not tunnelled
        }
    case NET_TOR:
        switch(nOurNet) {
        default:         return EM_REACH_DEFAULT;
        case NET_IPV4:   return EM_REACH_IPV4; // Tor users can connect to IPv4 as well
        case NET_TOR:    return EM_REACH_PRIVATE;
        }
    case NET_TEREDO:
        switch(nOurNet) {
        default:          return EM_REACH_DEFAULT;
        case NET_TEREDO:  return EM_REACH_TEREDO;
        case NET_IPV6:    return EM_REACH_IPV6_WEAK;
        case NET_IPV4:    return EM_REACH_IPV4;
        }
    case NET_UNKNOWN:
    case NET_UNROUTABLE:
    default:
        switch(nOurNet) {
        default:          return EM_REACH_DEFAULT;
        case NET_TEREDO:  return EM_REACH_TEREDO;
        case NET_IPV6:    return EM_REACH_IPV6_WEAK;
        case NET_IPV4:    return EM_REACH_IPV4;
        case NET_TOR:     return EM_REACH_PRIVATE; // either from Tor, or don't care about our address
        }
    }
}

void CService::Init()
{
    m_ushPort = 0;
}

CService::CService() {
	Init();
}

CService::CService(const CNetAddr& cIp, unsigned short ushPortIn) :
		CNetAddr(cIp), m_ushPort(ushPortIn) {
}

CService::CService(const struct in_addr& ipv4Addr, unsigned short ushPortIn) :
		CNetAddr(ipv4Addr), m_ushPort(ushPortIn) {
}

CService::CService(const struct in6_addr& tIpv6Addr, unsigned short ushPortIn) :
		CNetAddr(tIpv6Addr), m_ushPort(ushPortIn) {
}

CService::CService(const struct sockaddr_in& tAddr) :
		CNetAddr(tAddr.sin_addr), m_ushPort(ntohs(tAddr.sin_port)) {
	assert(tAddr.sin_family == AF_INET);
}

CService::CService(const struct sockaddr_in6 &tAddr) :
		CNetAddr(tAddr.sin6_addr), m_ushPort(ntohs(tAddr.sin6_port)) {
	assert(tAddr.sin6_family == AF_INET6);
}

bool CService::SetSockAddr(const struct sockaddr *pAddr) {
	switch (pAddr->sa_family) {
	case AF_INET: {
		*this = CService(*(const struct sockaddr_in*) pAddr);
		return true;
	}
	case AF_INET6: {
		*this = CService(*(const struct sockaddr_in6*) pAddr);
		return true;
	}
	default: {
		return false;
	}
	}
}

CService::CService(const char *pszIpPort, bool bAllowLookup) {
	Init();
	CService ip;
	if (Lookup(pszIpPort, ip, 0, bAllowLookup)) {
		*this = ip;
	}
}

CService::CService(const char *pszIpPort, int nPortDefault, bool bAllowLookup) {
	Init();
	CService ip;
	if (Lookup(pszIpPort, ip, nPortDefault, bAllowLookup)) {
		*this = ip;
	}
}

CService::CService(const string &strIpPort, bool bAllowLookup) {
	Init();
	CService ip;
	if (Lookup(strIpPort.c_str(), ip, 0, bAllowLookup)) {
		*this = ip;
	}
}

CService::CService(const string &strIpPort, int nPortDefault, bool bAllowLookup) {
	Init();
	CService ip;
	if (Lookup(strIpPort.c_str(), ip, nPortDefault, bAllowLookup)) {
		*this = ip;
	}
}


unsigned short CService::GetPort() const {
	return m_ushPort;
}

bool operator==(const CService& cServiceA, const CService& cServiceB) {
	return (CNetAddr) cServiceA == (CNetAddr) cServiceB && cServiceA.m_ushPort == cServiceB.m_ushPort;
}

bool operator!=(const CService& cServiceA, const CService& cServiceB) {
	return (CNetAddr) cServiceA != (CNetAddr) cServiceB || cServiceA.m_ushPort != cServiceB.m_ushPort;
}

bool operator<(const CService& cServiceA, const CService& cServiceB) {
	return (CNetAddr) cServiceA < (CNetAddr) cServiceB
			|| ((CNetAddr) cServiceA == (CNetAddr) cServiceB && cServiceA.m_ushPort < cServiceB.m_ushPort);
}

bool CService::GetSockAddr(struct sockaddr* pAddr, socklen_t *pAddrlen) const {
	if (IsIPv4()) {
		if (*pAddrlen < (socklen_t) sizeof(struct sockaddr_in)) {
			return false;
		}
		*pAddrlen = sizeof(struct sockaddr_in);
		struct sockaddr_in *paddrin = (struct sockaddr_in*) pAddr;
		memset(paddrin, 0, *pAddrlen);
		if (!GetInAddr(&paddrin->sin_addr)) {
			return false;
		}
		paddrin->sin_family = AF_INET;
		paddrin->sin_port = htons(m_ushPort);
		return true;
	}
	if (IsIPv6()) {
		if (*pAddrlen < (socklen_t) sizeof(struct sockaddr_in6)) {
			return false;
		}
		*pAddrlen = sizeof(struct sockaddr_in6);
		struct sockaddr_in6 *paddrin6 = (struct sockaddr_in6*) pAddr;
		memset(paddrin6, 0, *pAddrlen);
		if (!GetIn6Addr(&paddrin6->sin6_addr)) {
			return false;
		}
		paddrin6->sin6_family = AF_INET6;
		paddrin6->sin6_port = htons(m_ushPort);
		return true;
	}
	return false;
}


vector<unsigned char> CService::GetKey() const {
	vector<unsigned char> vchKey;
	vchKey.resize(18);
	memcpy(&vchKey[0], m_chIp, 16);
	vchKey[16] = m_ushPort / 0x100;
	vchKey[17] = m_ushPort & 0x0FF;
	return vchKey;
}


string CService::ToStringPort() const {
	return strprintf("%u", m_ushPort);
}


string CService::ToStringIPPort() const {
	if (IsIPv4() || IsTor()) {
		return ToStringIP() + ":" + ToStringPort();
	} else {
		return "[" + ToStringIP() + "]:" + ToStringPort();
	}
}


string CService::ToString() const {
	return ToStringIPPort();
}


void CService::print() const {
	LogPrint("INFO", "CService(%s)\n", ToString());
}

void CService::SetPort(unsigned short ushPortIn) {
	m_ushPort = ushPortIn;
}

#ifdef WIN32
string NetworkErrorString(int nErr) {
	char szBuf[256];
	szBuf[0] = 0;
	if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
	NULL, nErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szBuf, sizeof(szBuf), NULL)) {
		return strprintf("%s (%d)", szBuf, nErr);
	} else {
		return strprintf("Unknown error (%d)", nErr);
	}
}
#else
string NetworkErrorString(int err)
{
    char buf[256];
    const char *s = buf;
    buf[0] = 0;
    /* Too bad there are two incompatible implementations of the
     * thread-safe strerror. */
#ifdef STRERROR_R_CHAR_P /* GNU variant can return a pointer outside the passed buffer */
    s = strerror_r(err, buf, sizeof(buf));
#else /* POSIX variant always returns message in buffer */
    (void) strerror_r(err, buf, sizeof(buf));
#endif
    return strprintf("%s (%d)", s, err);
}
#endif

