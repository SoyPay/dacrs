// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CHAIN_PARAMS_H
#define BITCOIN_CHAIN_PARAMS_H
#include <memory>
#include "bignum.h"
#include "uint256.h"
#include "util.h"

#include <vector>

using namespace std;

#define MESSAGE_START_SIZE 4
typedef unsigned char MessageStartChars[MESSAGE_START_SIZE];

class CAddress;
class CBlock;

struct CDNSSeedData {
	string name, host;
	CDNSSeedData(const string &strName, const string &strHost) :
			name(strName), host(strHost) {
	}
};

/**
 * CChainParams defines various tweakable parameters of a given instance of the
 * Bitcoin system. There are three: the main network on which people trade goods
 * and services, the public test network which gets reset from time to time and
 * a regression test mode which is intended for private networks only. It has
 * minimal difficulty to ensure that blocks can be found instantly.
 */
class CBaseParams {
protected:
	bool fDebugAll;
	bool fDebug;
	bool fPrintToConsole;
	bool fPrintToToFile;
	bool fLogTimestamps;
	bool fLogPrintFileLine;
	bool fServer;
	bool fImporting;
	bool fReindex;
	bool fBenchmark;
	bool fTxIndex;
	int64_t nTimeBestReceived;
	unsigned int nScriptCheckThreads;
	int64_t nCoinCacheSize;
	int nTxCacheHeight;
	int nIntervalPos;

public:

	std::shared_ptr<vector<string> > GetMultiArgsMap(string str) {
		std::shared_ptr<vector<string> > temp = make_shared<vector<string> >();
		vector<string> te = m_mapMultiArgs[str];
		temp.get()->assign(te.begin(), te.end());
		return temp;

	}

	virtual bool InitalConfig() {

		fServer = GetBoolArg("-server", false);
		fDebug = !m_mapMultiArgs["-debug"].empty();
		if (fDebug) {
			fDebugAll = GetBoolArg("-logprintall", false);
			fPrintToConsole = GetBoolArg("-printtoconsole", false);
			fLogTimestamps = GetBoolArg("-logtimestamps", true);
			fPrintToToFile = GetBoolArg("-logprintfofile", false);
			fLogPrintFileLine = GetBoolArg("-logprintfileline", false);
		}
		return true;
	}
	virtual string ToString() {
		string te = "";

		for (auto & tep1 : m_mapMultiArgs) {
			te += strprintf("key:%s\n",tep1.first);
			vector<string> tep = tep1.second;
			for(auto const & tep3:tep)
			{
				te += strprintf("value :%s\n",tep3.c_str());
			}
		}

		te += strprintf("fDebug:%s\n",fDebug);
		te += strprintf("fPrintToConsole:%d\n",fPrintToConsole);
		te += strprintf("fPrintToToFile:%d\n",fPrintToToFile);
		te += strprintf("fLogTimestamps:%d\n",fLogTimestamps);
		te += strprintf("fLogPrintFileLine:%d\n",fLogPrintFileLine);
		te += strprintf("fServer:%d\n",fServer);

		return te;
	}

public:
	int getConnectTimeOut() {
		int nConnectTimeout = 5000;
		if (m_mapArgs.count("-timeout")) {
			int nNewTimeout = GetArg("-timeout", 5000);
			if (nNewTimeout > 0 && nNewTimeout < 600000)
				nConnectTimeout = nNewTimeout;
		}
		return nConnectTimeout;
	}
	bool IsDebug() const {
		return fDebug;
	}
	bool IsDebugAll() const {
		return fDebugAll;
	}
	bool IsPrint2Console() const {
		return fPrintToConsole;
	}
	bool IsPrintToFile() const {
		return fPrintToToFile;
	}
	bool IsLogTimestamps() const {
		return fPrintToToFile;
	}
	bool IsLogPrintLine() const {
		return fLogPrintFileLine;
	}
	bool IsServer() const {
		return fServer;
	}
	bool IsImporting() const {
		return fImporting;
	}
	bool IsReindex() const {
		return fReindex;
	}
	bool IsBenchmark() const {
		return fBenchmark;
	}
	bool IsTxIndex() const {
		return fTxIndex;
	}

	int64_t GetBestRecvTime() const {
		return nTimeBestReceived;
	}
	int64_t GetScriptCheckThreads() const {
		return nScriptCheckThreads;
	}
	unsigned int GetCoinCacheSize() const {
		return nCoinCacheSize;
	}
	int GetTxCacheHeight() const {
		return nTxCacheHeight;
	}
	int GetIntervalPos() const {
		return nIntervalPos;
	}
	void SetImporting(bool flag) {
		fImporting = flag;
	}

	void SetReIndex(bool flag) {
		fReindex = flag;
	}
	void SetBenchMark(bool flag) {
		fBenchmark = flag;
	}
	void SetTxIndex(bool flag) {
		fTxIndex = flag;
	}
	void SetBestRecvTime(int64_t nTime) {
		nTimeBestReceived = nTime;
	}
	void SetScriptCheckThreads(int64_t nNum) {
		nScriptCheckThreads = nNum;
	}
	void SetCoinCacheSize(unsigned int nSize) {
		nCoinCacheSize = nSize;
	}
	void SetTxCacheHeight(int nHeight) {
		nTxCacheHeight = nHeight;
	}
	void SetIntervalPos(int nPos) {
		nIntervalPos = nPos;
	}

public:
	enum Network {
		MAIN, TESTNET, REGTEST,

		MAX_NETWORK_TYPES
	};

	enum Base58Type {
		PUBKEY_ADDRESS, SCRIPT_ADDRESS, SECRET_KEY, EXT_PUBLIC_KEY, EXT_SECRET_KEY, ACC_ADDRESS,

		MAX_BASE58_TYPES
	};

	const uint256& HashGenesisBlock() const {
		return hashGenesisBlock;
	}
	const MessageStartChars& MessageStart() const {
		return pchMessageStart;
	}
	const vector<unsigned char>& AlertKey() const {
		return vAlertPubKey;
	}
	int GetDefaultPort() const {
		return nDefaultPort;
	}
	const CBigNum& ProofOfWorkLimit() const {
		return bnProofOfStakeLimit;
	}
	int SubsidyHalvingInterval() const {
		return nSubsidyHalvingInterval;
	}
	virtual const CBlock& GenesisBlock() const = 0;
	virtual bool RequireRPCPassword() const {
		return true;
	}
	const string& DataDir() const {
		return strDataDir;
	}
	virtual Network NetworkID() const = 0;
	const vector<CDNSSeedData>& DNSSeeds() const {
		return vSeeds;
	}
	const vector<unsigned char> &Base58Prefix(Base58Type type) const {
		return base58Prefixes[type];
	}
	virtual const vector<CAddress>& FixedSeeds() const = 0;
	int RPCPort() const {
		return nRPCPort;
	}

	/******************************paras**************************************/
	static bool IntialParams(int argc, const char* const argv[]);
	static int64_t GetArg(const string& strArg, int64_t nDefault);
	static string GetArg(const string& strArg, const string& strDefault);
	static bool GetBoolArg(const string& strArg, bool fDefault);
	static bool SoftSetArg(const string& strArg, const string& strValue);
	static bool SoftSetBoolArg(const string& strArg, bool fValue);
	static bool IsArgCount(const string& strArg);

	virtual ~CBaseParams() {
	}

	virtual bool CreateGenesisRewardTx(vector<std::shared_ptr<CBaseTransaction> > &vRewardTx) = 0;
	static void ParseParameters(int argc, const char* const argv[]);

	static const vector<string> &GetMultiArgs(const string& strArg);
protected:

	static map<string, string> m_mapArgs;
	static map<string, vector<string> > m_mapMultiArgs;
	/********************************************************************/

protected:
	CBaseParams() {
		fImporting = false;
		fReindex = false;
		fBenchmark = false;
		fTxIndex = false;
		nIntervalPos = 1;
		nTxCacheHeight = 500;
		nTimeBestReceived = 0;
		nScriptCheckThreads = 0;
		nCoinCacheSize = 2000000;
	}

	uint256 hashGenesisBlock;
	MessageStartChars pchMessageStart;
	// Raw pub key bytes for the broadcast alert signing key.
	vector<unsigned char> vAlertPubKey;
	int nDefaultPort;
	int nRPCPort;
	CBigNum bnProofOfStakeLimit;
	int nSubsidyHalvingInterval;
	string strDataDir;
	vector<CDNSSeedData> vSeeds;
	vector<unsigned char> base58Prefixes[MAX_BASE58_TYPES];
};

/**
 * Return the currently selected parameters. This won't change after app startup
 * outside of the unit tests.
 */
CBaseParams &Params();

/** Sets the params returned by Params() to those for the given network. */
void SelectParams(CBaseParams::Network network);

/**
 * Looks for -regtest or -testnet and then calls SelectParams as appropriate.
 * Returns false if an invalid combination is given.
 */
bool SelectParamsFromCommandLine();

inline bool TestNet() {
	// Note: it's deliberate that this returns "false" for regression test mode.
	return Params().NetworkID() == CBaseParams::TESTNET;
}

inline bool RegTest() {
	return Params().NetworkID() == CBaseParams::REGTEST;
}

extern const CBaseParams &SysParams();

//write for test code
extern const CBaseParams &SysParamsMain();

//write for test code
extern const CBaseParams &SysParamsTest();

//write for test code
extern const CBaseParams &SysParamsReg();

#endif
