// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The DACRS developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_NET_H_
#define DACRS_NET_H_

#include "bloom.h"
#include "./compat/compat.h"
#include "hash.h"
#include "limitedmap.h"
#include "mruset.h"
#include "netbase.h"
#include "protocol.h"
#include "sync.h"
#include "uint256.h"
#include "util.h"

#include <deque>
#include <stdint.h>

#ifndef WIN32
#include <arpa/inet.h>
#endif

#include <boost/foreach.hpp>
#include <boost/signals2/signal.hpp>
#include <openssl/rand.h>

class CAddrMan;
class CBlockIndex;
class CNode;

namespace boost {
    class thread_group;
}

/** The maximum number of entries in an 'inv' protocol message */
static const unsigned int g_sMaxInvSz = 50000;

inline unsigned int ReceiveFloodSize() {
	return 1000 * SysCfg().GetArg("-maxreceivebuffer", 5 * 1000);
}

inline unsigned int SendBufferSize() {
	return 1000 * SysCfg().GetArg("-maxsendbuffer", 1 * 1000);
}

void AddOneShot(string strDest);
bool RecvLine(SOCKET hSocket, string& strLine);
bool GetMyExternalIP(CNetAddr& cIpRet);
void AddressCurrentlyConnected(const CService& cAddr);
CNode* FindNode(const CNetAddr& cIp);
CNode* FindNode(const CService& cIp);
CNode* ConnectNode(CAddress cAddrConnect, const char *pszDest = NULL);
void MapPort(bool bUseUPnP);
unsigned short GetListenPort();
bool BindListenPort(const CService &cBindAddr, string& strError=REF(string()));
void StartNode(boost::thread_group& threadGroup);
bool StopNode();
void SocketSendData(CNode *pNode);

typedef int NodeId;

// Signals for message handling
struct ST_NodeSignals {
	boost::signals2::signal<int()> GetHeight;
	boost::signals2::signal<bool(CNode*)> ProcessMessages;
	boost::signals2::signal<bool(CNode*, bool)> SendMessages;
	boost::signals2::signal<void(NodeId, const CNode*)> InitializeNode;
	boost::signals2::signal<void(NodeId)> FinalizeNode;
};

ST_NodeSignals& GetNodeSignals();

enum {
	LOCAL_NONE,   // unknown
	LOCAL_IF,     // address a local interface listens on
	LOCAL_BIND,   // address explicit bound to
	LOCAL_UPNP,   // address reported by UPnP
	LOCAL_HTTP,   // address reported by whatismyip.com and similar
	LOCAL_MANUAL, // address explicitly specified (-externalip=)
	LOCAL_MAX
};

void SetLimited(enum Network netWork, bool bLimited = true);
bool IsLimited(enum Network netWork);
bool IsLimited(const CNetAddr& cNetAddr);
bool AddLocal(const CService& cServiceAddr, int nScore = LOCAL_NONE);
bool AddLocal(const CNetAddr& cNetAddr, int nScore = LOCAL_NONE);
bool SeenLocal(const CService& cServiceAddr);
bool IsLocal(const CService& cServiceAddr);
bool GetLocal(CService &cServiceAddr, const CNetAddr *pAddrPeer = NULL);
bool IsReachable(const CNetAddr &cNetAddr);
void SetReachable(enum Network netWork, bool bFlag = true);
CAddress GetLocalAddress(const CNetAddr *pAddrPeer = NULL);

extern bool g_bDiscover;
extern uint64_t g_ullLocalServices;
extern uint64_t g_ullLocalHostNonce;
extern CAddrMan g_cAddrman;
extern int g_nMaxConnections;

extern vector<CNode*> g_vNodes;
extern CCriticalSection g_cs_vNodes;
extern map<CInv, CDataStream> g_mapRelay;
extern deque<pair<int64_t, CInv> > g_vRelayExpiration;
extern CCriticalSection g_cs_mapRelay;
extern limitedmap<CInv, int64_t> mapAlreadyAskedFor;

extern vector<string> g_vAddedNodes;
extern CCriticalSection g_cs_vAddedNodes;

extern NodeId g_nLastNodeId;
extern CCriticalSection g_cs_nLastNodeId;

struct LocalServiceInfo {
    int nScore;
    int nPort;
};

extern CCriticalSection g_cs_mapLocalHost;
extern map<CNetAddr, LocalServiceInfo> g_mapLocalHost;

class CNodeStats {
 public:
	NodeId 		m_nNodeId;
	uint64_t 	m_ullServices;
	int64_t 	m_llLastSend;
	int64_t 	m_llLastRecv;
	int64_t 	m_llTimeConnected;
	string 		m_strAddrName;
	int 		m_nVersion;
	string 		m_strCleanSubVer;
	bool 		m_bInbound;
	int 		m_nStartingHeight;
	uint64_t 	m_ullSendBytes;
	uint64_t 	m_ullRecvBytes;
	bool 		m_bSyncNode;
	double 		m_dPingTime;
	double 		m_dPingWait;
	string 		m_strAddrLocal;
};

class CNetMessage {
 public:
	CNetMessage(int nTypeIn, int nVersionIn) :
			m_cDSHeaderBuf(nTypeIn, nVersionIn), m_cDSRecv(nTypeIn, nVersionIn) {
		m_cDSHeaderBuf.resize(24);
		m_bInData = false;
		m_unHeaderPos = 0;
		m_unDataPos = 0;
	}

	bool complete() const {
		if (!m_bInData) {
			return false;
		}
		return (m_cMessageHeader.m_unMessageSize == m_unDataPos);
	}

	void SetVersion(int nVersionIn) {
		m_cDSHeaderBuf.SetVersion(nVersionIn);
		m_cDSRecv.SetVersion(nVersionIn);
	}

    int readHeader(const char *pch, unsigned int nBytes);
    int readData(const char *pch, unsigned int nBytes);

    bool 			m_bInData;                  // parsing header (false) or data (true)
	CDataStream 	m_cDSHeaderBuf;             // partially received header
	CMessageHeader 	m_cMessageHeader;        	// complete header
	unsigned int 	m_unHeaderPos;
	CDataStream 	m_cDSRecv;              	// received message data
	unsigned int 	m_unDataPos;
};

/** Information about a peer */
class CNode {
 public:
	// socket
	uint64_t 				m_ullServices;
	SOCKET 					m_hSocket;
	CDataStream 			m_cDSSend;
	size_t 					m_unSendSize; 		// total size of all vSendMsg entries
	size_t 					m_unSendOffset; 	// offset inside the first vSendMsg already sent
	uint64_t 				m_ullSendBytes;
	deque<CSerializeData> 	m_vSendMsg;
	CCriticalSection 		m_cs_vSend;

	deque<CInv> 			m_vRecvGetData;   	// strCommand == "getdata 保存的inv
	deque<CNetMessage> 		m_vRecvMsg;
	CCriticalSection 		m_cs_vRecvMsg;
	uint64_t 				m_ullRecvBytes;
	int 					m_nRecvVersion;

	int64_t 				m_llLastSend;
	int64_t 				m_llLastRecv;
	int64_t 				m_llLastSendEmpty;
	int64_t 				m_llTimeConnected;
	CAddress 				m_cAddress;
	string 					m_strAddrName;
	CService 				m_strAddrLocal;
	int 					m_nVersion;
	// strSubVer is whatever byte array we read from the wire. However, this field is intended
	// to be printed out, displayed to humans in various forms and so on. So we sanitize it and
	// store the sanitized version in cleanSubVer. The original should be used when dealing with
	// the network or wire types and the cleaned string used when displayed or logged.
	string 					m_strSubVer;
	string 					m_strCleanSubVer;
	bool 					m_bOneShot;
	bool 					m_bClient;
	bool 					m_bInbound;
	bool 					m_bNetworkNode;
	bool 					m_bSuccessfullyConnected;
	bool 					m_bDisconnect;
	// We use fRelayTxes for two purposes -
	// a) it allows us to not relay tx invs before receiving the peer's version message
	// b) the peer may tell us in their version message that we should not relay tx invs
	//    until they have initialized their bloom filter.
	bool 					m_bRelayTxes;
	CSemaphoreGrant 		m_cGrantOutbound;
	CCriticalSection 		m_cs_filter;
	CBloomFilter* 			m_pBloomFilter;
	int 					m_nRefCount;
	NodeId 					m_nNodeId;

 protected:
	// Denial-of-service detection/prevention
	// Key is IP address, value is banned-until-time
	static map<CNetAddr, int64_t> 	m_sBanned;
	static CCriticalSection 		m_s_cs_Banned;
	// Basic fuzz-testing
	void Fuzz(int nChance); // modifies ssSend

 public:
	uint256 						m_cHashContinue;                    // getblocks the next batch of inventory下一次 盘点的块
	CBlockIndex* 					m_pIndexLastGetBlocksBegin;   		// 上次开始的块  本地节点有的块chainActive.Tip()
	uint256 						m_cHashLastGetBlocksEnd;            // 本地节点保存的孤儿块的根块 hash GetOrphanRoot(hash)
	int 							m_nStartingHeight;   				// Start block sync,currHegiht
	bool 							m_bStartSync;
	// flood relay
	vector<CAddress> 				m_vcAddrToSend;
	mruset<CAddress> 				m_cAddrKnown;
	bool 							m_bGetAddr;
	set<uint256> 					m_setKnown;             			// 存的是 alertHash
	std::set<int> 					m_setcheckPointKnown;  				// checkPoint 的height
	// inventory based relay
	mruset<CInv> 					m_setInventoryKnown;   				//存放已收到的inv
	vector<CInv> 					m_vInventoryToSend;    				//待发送的inv
	CCriticalSection 				m_cs_inventory;
	multimap<int64_t, CInv> 		m_mapAskFor;   						//向网络请求交易 的 时间, a priority queue
	// Ping time measurement
	uint64_t 						m_ullPingNonceSent;
	int64_t 						m_llnPingUsecStart;
	int64_t 						m_llPingUsecTime;
	bool 							m_bPingQueued;

	CNode(SOCKET hSocketIn, CAddress cAddrIn, string strAddrNameIn = "", bool bInboundIn = false) :
			m_cDSSend(SER_NETWORK, g_sInitProtoVersion), m_cAddrKnown(5000) {
		m_ullServices 				= 0;
		m_hSocket 					= hSocketIn;
		m_nRecvVersion 				= g_sInitProtoVersion;
		m_llLastSend 				= 0;
		m_llLastRecv 				= 0;
		m_ullSendBytes 				= 0;
		m_ullRecvBytes 				= 0;
		m_llLastSendEmpty 			= GetTime();
		m_llTimeConnected 			= GetTime();
		m_cAddress 					= cAddrIn;
		m_strAddrName 				= strAddrNameIn == "" ? m_cAddress.ToStringIPPort() : strAddrNameIn;
		m_nVersion 					= 0;
		m_strSubVer 				= "";
		m_bOneShot 					= false;
		m_bClient 					= false; 	// set by version message
		m_bInbound 					= bInboundIn;
		m_bNetworkNode 				= false;
		m_bSuccessfullyConnected 	= false;
		m_bDisconnect 				= false;
		m_nRefCount 				= 0;
		m_unSendSize 				= 0;
		m_unSendOffset 				= 0;
		m_cHashContinue 			= uint256();
		m_pIndexLastGetBlocksBegin 	= 0;
		m_cHashLastGetBlocksEnd 	= uint256();
		m_nStartingHeight 			= -1;
		m_bStartSync 				= false;
		m_bGetAddr 					= false;
		m_bRelayTxes 				= false;
		m_setInventoryKnown.max_size(SendBufferSize() / 1000);
		m_pBloomFilter 				= new CBloomFilter();
		m_ullPingNonceSent 			= 0;
		m_llnPingUsecStart 			= 0;
		m_llPingUsecTime 			= 0;
		m_bPingQueued 				= false;

		{
			LOCK(g_cs_nLastNodeId);
			m_nNodeId = g_nLastNodeId++;
		}

		// Be shy and don't send version until we hear
		if (m_hSocket != INVALID_SOCKET && !m_bInbound) {
			PushVersion();
		}
		GetNodeSignals().InitializeNode(GetId(), this);
	}

	~CNode() {
		if (m_hSocket != INVALID_SOCKET) {
			closesocket(m_hSocket);
			m_hSocket = INVALID_SOCKET;
		}
		if (m_pBloomFilter) {
			delete m_pBloomFilter;
		}
		GetNodeSignals().FinalizeNode(GetId());
	}

 private:
	// Network usage totals
	static CCriticalSection m_s_cs_totalBytesRecv;
	static CCriticalSection m_s_cs_totalBytesSent;
	static uint64_t m_sTotalBytesRecv;
	static uint64_t m_sTotalBytesSent;

	CNode(const CNode&);
	void operator=(const CNode&);

 public:
	NodeId GetId() const {
		return m_nNodeId;
	}

	int GetRefCount() {
		assert(m_nRefCount >= 0);
		return m_nRefCount;
	}

	// requires LOCK(cs_vRecvMsg)
	unsigned int GetTotalRecvSize() {
		unsigned int unTotal = 0;
		for (const auto &msg : m_vRecvMsg) {
			unTotal += msg.m_cDSRecv.size() + 24;
		}
		return unTotal;
	}

	// requires LOCK(cs_vRecvMsg)
	bool ReceiveMsgBytes(const char *pch, unsigned int nBytes);

	// requires LOCK(cs_vRecvMsg)
	void SetRecvVersion(int nVersionIn) {
		m_nRecvVersion = nVersionIn;
		for (auto &msg : m_vRecvMsg) {
			msg.SetVersion(nVersionIn);
		}
	}

	CNode* AddRef() {
		m_nRefCount++;
		return this;
	}

	void Release() {
		m_nRefCount--;
	}

	void AddAddressKnown(const CAddress& addr) {
		m_cAddrKnown.insert(addr);
	}

	void PushAddress(const CAddress& addr) {
		// Known checking here is only to save space from duplicates.
		// SendMessages will filter it again for knowns that were added
		// after addresses were pushed.
		if (addr.IsValid() && !m_cAddrKnown.count(addr)) {
			m_vcAddrToSend.push_back(addr);
		}
	}

	void AddInventoryKnown(const CInv& inv) {
		{
			LOCK(m_cs_inventory);
			m_setInventoryKnown.insert(inv);
		}
	}

	void PushInventory(const CInv& inv) {
		{
			LOCK(m_cs_inventory);
			if (!m_setInventoryKnown.count(inv)) {
				m_vInventoryToSend.push_back(inv);
			}
		}
	}

	void AskFor(const CInv& cInv) {
		// We're using mapAskFor as a priority queue,
		// the key is the earliest time the request can be sent
		int64_t llRequestTime;
		limitedmap<CInv, int64_t>::const_iterator it = mapAlreadyAskedFor.find(cInv);
		if (it != mapAlreadyAskedFor.end()) {
			llRequestTime = it->second;
		} else {
			llRequestTime = 0;
		}

		LogPrint("net", "askfor %s   %d (%s)\n", cInv.ToString().c_str(), llRequestTime,
				DateTimeStrFormat("%H:%M:%S", llRequestTime / 1000000).c_str());

		// Make sure not to reuse time indexes to keep things in the same order
		int64_t llNow = GetTimeMicros() - 1000000;
		static int64_t llLastTime;
		++llLastTime;
		llNow = max(llNow, llLastTime);
		llLastTime = llNow;
		// Each retry is 2 minutes after the last
		llRequestTime = max(llRequestTime + 2 * 60 * 1000000, llNow);

		if (it != mapAlreadyAskedFor.end()) {
			mapAlreadyAskedFor.update(it, llRequestTime);
		} else {
			mapAlreadyAskedFor.insert(make_pair(cInv, llRequestTime));
		}
		m_mapAskFor.insert(make_pair(llRequestTime, cInv));
	}

	// TODO: Document the postcondition of this function.  Is cs_vSend locked?
	void BeginMessage(const char* pszCommand) EXCLUSIVE_LOCK_FUNCTION(m_cs_vSend)
	{
		ENTER_CRITICAL_SECTION(m_cs_vSend);
		assert(m_cDSSend.size() == 0);
		m_cDSSend << CMessageHeader(pszCommand, 0);
		LogPrint("net", "sending: %s ", pszCommand);
	}

	// TODO: Document the precondition of this function.  Is cs_vSend locked?
	void AbortMessage() UNLOCK_FUNCTION(m_cs_vSend)
	{
		m_cDSSend.clear();
		LEAVE_CRITICAL_SECTION(m_cs_vSend);
		LogPrint("net", "(aborted)\n");
	}

	// TODO: Document the precondition of this function.  Is cs_vSend locked?
	void EndMessage() UNLOCK_FUNCTION(m_cs_vSend)
	{
		// The -*messagestest options are intentionally not documented in the help message,
		// since they are only used during development to debug the networking code and are
		// not intended for end-users.
		if (SysCfg().IsArgCount("-dropmessagestest") && GetRand(SysCfg().GetArg("-dropmessagestest", 2)) == 0) {
			LogPrint("net", "dropmessages DROPPING SEND MESSAGE\n");
			AbortMessage();
			return;
		}
		if (SysCfg().IsArgCount("-fuzzmessagestest")) {
			Fuzz(SysCfg().GetArg("-fuzzmessagestest", 10));
		}
		if (m_cDSSend.size() == 0) {
			return;
		}

		// Set the size
		unsigned int unSize = m_cDSSend.size() - CMessageHeader::HEADER_SIZE;
		memcpy((char*) &m_cDSSend[CMessageHeader::MESSAGE_SIZE_OFFSET], &unSize, sizeof(unSize));
		// Set the checksum
		uint256 cHash = Hash(m_cDSSend.begin() + CMessageHeader::HEADER_SIZE, m_cDSSend.end());
		unsigned int unChecksum = 0;
		memcpy(&unChecksum, &cHash, sizeof(unChecksum));
		assert(m_cDSSend.size() >= CMessageHeader::CHECKSUM_OFFSET + sizeof(unChecksum));
		memcpy((char*) &m_cDSSend[CMessageHeader::CHECKSUM_OFFSET], &unChecksum, sizeof(unChecksum));

		LogPrint("net", "(%d bytes)\n", unSize);

		deque<CSerializeData>::iterator it = m_vSendMsg.insert(m_vSendMsg.end(), CSerializeData());
		m_cDSSend.GetAndClear(*it);
		m_unSendSize += (*it).size();

		// If write queue empty, attempt "optimistic write"
		if (it == m_vSendMsg.begin()) {
			SocketSendData(this);
		}

		LEAVE_CRITICAL_SECTION(m_cs_vSend);
	}

	void PushVersion();

	void PushMessage(const char* pszCommand) {
		try {
			BeginMessage(pszCommand);
			EndMessage();
		} catch (...) {
			AbortMessage();
			throw;
		}
	}

	template<typename T1>
	void PushMessage(const char* pszCommand, const T1& a1) {
		try {
			BeginMessage(pszCommand);
			m_cDSSend << a1;
			EndMessage();
		} catch (...) {
			AbortMessage();
			throw;
		}
	}

	template<typename T1, typename T2>
	void PushMessage(const char* pszCommand, const T1& a1, const T2& a2) {
		try {
			BeginMessage(pszCommand);
			m_cDSSend << a1 << a2;
			EndMessage();
		} catch (...) {
			AbortMessage();
			throw;
		}
	}

	template<typename T1, typename T2, typename T3>
	void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3) {
		try {
			BeginMessage(pszCommand);
			m_cDSSend << a1 << a2 << a3;
			EndMessage();
		} catch (...) {
			AbortMessage();
			throw;
		}
	}

	template<typename T1, typename T2, typename T3, typename T4>
	void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4) {
		try {
			BeginMessage(pszCommand);
			m_cDSSend << a1 << a2 << a3 << a4;
			EndMessage();
		} catch (...) {
			AbortMessage();
			throw;
		}
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5>
	void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5) {
		try {
			BeginMessage(pszCommand);
			m_cDSSend << a1 << a2 << a3 << a4 << a5;
			EndMessage();
		} catch (...) {
			AbortMessage();
			throw;
		}
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5,
			const T6& a6) {
		try {
			BeginMessage(pszCommand);
			m_cDSSend << a1 << a2 << a3 << a4 << a5 << a6;
			EndMessage();
		} catch (...) {
			AbortMessage();
			throw;
		}
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5,
			const T6& a6, const T7& a7) {
		try {
			BeginMessage(pszCommand);
			m_cDSSend << a1 << a2 << a3 << a4 << a5 << a6 << a7;
			EndMessage();
		} catch (...) {
			AbortMessage();
			throw;
		}
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5,
			const T6& a6, const T7& a7, const T8& a8) {
		try {
			BeginMessage(pszCommand);
			m_cDSSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8;
			EndMessage();
		} catch (...) {
			AbortMessage();
			throw;
		}
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8,
			typename T9>
	void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5,
			const T6& a6, const T7& a7, const T8& a8, const T9& a9) {
		try {
			BeginMessage(pszCommand);
			m_cDSSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9;
			EndMessage();
		} catch (...) {
			AbortMessage();
			throw;
		}
	}

	bool IsSubscribed(unsigned int nChannel);
	void Subscribe(unsigned int nChannel, unsigned int nHops = 0);
	void CancelSubscribe(unsigned int nChannel);
	void CloseSocketDisconnect();
	void Cleanup();

	// Denial-of-service detection/prevention
	// The idea is to detect peers that are behaving
	// badly and disconnect/ban them, but do it in a
	// one-coding-mistake-won't-shatter-the-entire-network
	// way.
	// IMPORTANT:  There should be nothing I can give a
	// node that it will forward on that will make that
	// node's peers drop it. If there is, an attacker
	// can isolate a node and/or try to split the network.
	// Dropping a node for sending stuff that is invalid
	// now but might be valid in a later version is also
	// dangerous, because it can cause a network split
	// between nodes running old code and nodes running
	// new code.
	static void ClearBanned(); // needed for unit testing
	static bool IsBanned(CNetAddr cNetIp);
	static bool Ban(const CNetAddr &cNetIp);
	void copyStats(CNodeStats &stats);

	// Network stats
	static void RecordBytesRecv(uint64_t ullBytes);
	static void RecordBytesSent(uint64_t ullBytes);
	static uint64_t GetTotalBytesRecv();
	static uint64_t GetTotalBytesSent();
};

class CTransaction;
void RelayTransaction(CBaseTransaction *pBaseTx, const uint256& cHash);
void RelayTransaction(CBaseTransaction *pBaseTx, const uint256& cHash, const CDataStream& cDS);

/** Access to the (IP) address database (peers.dat) */
class CAddrDB {
 public:
	CAddrDB();
	bool Write(const CAddrMan& cAddr);
	bool Read(CAddrMan& cAddr);

 private:
 	boost::filesystem::path m_pathAddr;
};

#endif
