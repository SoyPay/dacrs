// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_CHAINPARAMS_H_
#define DACRS_CHAINPARAMS_H_

#include <memory>
#include "bignum.h"
#include "uint256.h"
#include "arith_uint256.h"
#include "util.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <vector>

using namespace std;

#define MESSAGE_START_SIZE 4
typedef unsigned char MessageStartChars[MESSAGE_START_SIZE];

class CAddress;
class CBlock;

struct CDNSSeedData {
	string strName;
	string strHost;
	CDNSSeedData(const string &strName, const string &strHost) :
		strName(strName), strHost(strHost) {
	}
};

/**
 * CChainParams defines various tweakable parameters of a given instance of the
 * Dacrs system. There are three: the main network on which people trade goods
 * and services, the public test network which gets reset from time to time and
 * a regression test mode which is intended for private networks only. It has
 * minimal difficulty to ensure that blocks can be found instantly.
 */
class CBaseParams {
 public:
	std::shared_ptr<vector<string> > GetMultiArgsMap(string str) const {
		std::shared_ptr<vector<string> > vstrTemp = std::make_shared<vector<string> >();
		vector<string> te = m_mapMultiArgs[str];
		vstrTemp.get()->assign(te.begin(), te.end());

		return vstrTemp;
	}

	virtual bool InitalConfig() {
		m_bServer = GetBoolArg("-server", false);

		m_mapMultiArgs["-debug"].push_back("ERROR"); //add froce ERROR to log
		m_bDebug = !m_mapMultiArgs["-debug"].empty();
		if (m_bDebug) {
			m_bDebugAll = GetBoolArg("-logprintall", false);
			m_bPrintToConsole = GetBoolArg("-printtoconsole", false);
			m_bLogTimestamps = GetBoolArg("-logtimestamps", true);
			m_bPrintToToFile = GetBoolArg("-logprintfofile", false);
			m_bLogPrintFileLine = GetBoolArg("-logprintfileline", false);
		}
		int64_t llTransactionFee;
		if (ParseMoney(GetArg("-paytxfee", ""), llTransactionFee) && llTransactionFee > 0) {
			m_llpaytxfee = llTransactionFee;
		}

		m_nIntervalPos = GetArg("-intervalpos", 1440);
		m_nLogmaxsize = GetArg("-logmaxsize", 100) * 1024 * 1024;
		m_bOutPut = GetBoolArg("-output", false);
		m_bAddressToTx = GetBoolArg("-addresstotx", false);

		return true;
	}

	virtual string ToString() const {
		string strTep = "";

		for (auto & tep1 : m_mapMultiArgs) {
			strTep += strprintf("key:%s\n",tep1.first);
			vector<string> tep = tep1.second;
			for (auto const & tep3:tep) {
				strTep += strprintf("value :%s\n",tep3.c_str());
			}
		}
		strTep += strprintf("m_bDebugAll:%s\n",m_bDebugAll);
		strTep += strprintf("m_bDebug:%s\n",m_bDebug);
		strTep += strprintf("m_bPrintToConsole:%d\n",m_bPrintToConsole);
		strTep += strprintf("m_bPrintToToFile:%d\n",m_bPrintToToFile);
		strTep += strprintf("m_bLogTimestamps:%d\n",m_bLogTimestamps);
		strTep += strprintf("m_bLogPrintFileLine:%d\n",m_bLogPrintFileLine);
		strTep += strprintf("m_bServer:%d\n",m_bServer);

		strTep += strprintf("m_bImporting:%d\n",m_bImporting);
		strTep += strprintf("m_bReindex:%d\n",m_bReindex);
		strTep += strprintf("m_bBenchmark:%d\n",m_bBenchmark);
		strTep += strprintf("m_bTxIndex:%d\n",m_bTxIndex);
		strTep += strprintf("m_llTimeBestReceived:%d\n",m_llTimeBestReceived);
		strTep += strprintf("m_llpaytxfee:%d\n",m_llpaytxfee);
		strTep += strprintf("m_llTargetSpacing:%d\n",m_llTargetSpacing);
		strTep += strprintf("m_llTargetTimespan:%d\n",m_llTargetTimespan);
		strTep += strprintf("m_unScriptCheckThreads:%d\n",m_unScriptCheckThreads);
		strTep += strprintf("m_llViewCacheSize:%d\n",m_llViewCacheSize);
		strTep += strprintf("m_nTxCacheHeight:%d\n",m_nTxCacheHeight);
		strTep += strprintf("m_nIntervalPos:%d\n",m_nIntervalPos);
		strTep += strprintf("m_nLogmaxsize:%d\n",m_nLogmaxsize);

		return strTep;
	}
	virtual int GetBlockMaxNonce() const {
		return 1000;
	}
    int64_t GetTxFee() const;
    int64_t SetDeflautTxFee(int64_t fee) const;
	virtual string GetDefaultTestDataPath() const {
		char chFindchar;
#ifdef WIN32
		chFindchar = '\\';
#else
		chFindchar = '/';
#endif
		string strCurDir = boost::filesystem::initial_path<boost::filesystem::path>().string();
		int nIndex = strCurDir.find_last_of(chFindchar);
		int nCount = 3;
		while (nCount--) {
			nIndex = strCurDir.find_last_of(chFindchar);
			strCurDir = strCurDir.substr(0, nIndex);
		}

#ifdef WIN32
		strCurDir += "\\dacrs_test\\data\\";
		return strCurDir;
#else
		strCurDir +="/dacrs_test/data/";
		return strCurDir;
#endif
	}

 public:
	int getConnectTimeOut() const {
		int nConnectTimeout = 5000;
		if (m_mapArgs.count("-timeout")) {
			int nNewTimeout = GetArg("-timeout", 5000);
			if (nNewTimeout > 0 && nNewTimeout < 600000) {
				nConnectTimeout = nNewTimeout;
			}
		}
		return nConnectTimeout;
	}
	bool IsDebug() const {
		return m_bDebug;
	}
	bool IsDebugAll() const {
		return m_bDebugAll;
	}
	bool IsPrint2Console() const {
		return m_bPrintToConsole;
	}
	bool IsPrintToFile() const {
		return m_bPrintToToFile;
	}
	bool IsLogTimestamps() const {
		return m_bPrintToToFile;
	}
	bool IsLogPrintLine() const {
		return m_bLogPrintFileLine;
	}
	bool IsServer() const {
		return m_bServer;
	}
	bool IsImporting() const {
		return m_bImporting;
	}
	bool IsReindex() const {
		return m_bReindex;
	}
	bool IsBenchmark() const {
		return m_bBenchmark;
	}
	bool IsTxIndex() const {
		return m_bTxIndex;
	}
	int64_t GetTargetSpacing() const {
		return m_llTargetSpacing;
	}
	int64_t GetTargetTimespan() const {
		return m_llTargetTimespan;
	}
	int64_t GetBestRecvTime() const {
		return m_llTimeBestReceived;
	}
	int64_t GetScriptCheckThreads() const {
		return m_unScriptCheckThreads;
	}
	unsigned int GetViewCacheSize() const {
		return m_llViewCacheSize;
	}
	int GetTxCacheHeight() const {
		return m_nTxCacheHeight;
	}
	int GetIntervalPos() const {
		return m_nIntervalPos;
	}
	int GetLogMaxSize() const {
		return m_nLogmaxsize;
	}
	int GetMaxDay() const {
		return GetIntervalPos() * 30;
	}
	void SetImporting(bool flag)const {
		m_bImporting = flag;
	}
	void SetReIndex(bool flag)const {
		m_bReindex = flag;
	}
	void SetBenchMark(bool flag)const {
		m_bBenchmark = flag;
	}
	void SetTxIndex(bool flag)const {
		m_bTxIndex = flag;
	}
	void SetBestRecvTime(int64_t nTime)const {
		m_llTimeBestReceived = nTime;
	}
	void SetScriptCheckThreads(int64_t nNum)const {
		m_unScriptCheckThreads = nNum;
	}
	void SetViewCacheSize(unsigned int nSize)const {
		m_llViewCacheSize = nSize;
	}
	void SetTxCacheHeight(int nHeight)const {
		m_nTxCacheHeight = nHeight;
	}
	bool GetOutPutLog() const {
		return m_bOutPut;
	}
	bool GetAddressToTxFlag() const {
		return m_bAddressToTx;
	}

 public:
	enum emNetWork {
		EM_MAIN,            //!< MAIN
		EM_TESTNET,         //!< TESTNET
		EM_REGTEST,         //!< REGTEST
		EM_MAX_NETWORK_TYPES//!< MAX_NETWORK_TYPES
	};

	enum emBase58Type {
		EM_PUBKEY_ADDRESS, //!< PUBKEY_ADDRESS
		EM_SCRIPT_ADDRESS, //!< SCRIPT_ADDRESS
		EM_SECRET_KEY,     //!< SECRET_KEY
		EM_EXT_PUBLIC_KEY, //!< EXT_PUBLIC_KEY
		EM_EXT_SECRET_KEY, //!< EXT_SECRET_KEY
		EM_ACC_ADDRESS,    //!< ACC_ADDRESS
		EM_MAX_BASE58_TYPES//!< MAX_BASE58_TYPES
	};

	const uint256& HashGenesisBlock() const {
		return m_cHashGenesisBlock;
	}
	const MessageStartChars& MessageStart() const {
		return m_pchMessageStart;
	}
	const vector<unsigned char>& AlertKey() const {
		return m_vchAlertPubKey;
	}
	int GetDefaultPort() const {
		return m_nDefaultPort;
	}
	const arith_uint256 ProofOfWorkLimit()  {
		return m_bnProofOfStakeLimit;
	}
	int SubsidyHalvingInterval() const {
		return m_nSubsidyHalvingInterval;
	}
	virtual int64_t GetMaxFee() const {
		return 1000000000;
	}
	virtual const CBlock& GenesisBlock() const = 0;
	virtual bool RequireRPCPassword() const {
		return true;
	}
	const string& DataDir() const {
		return m_strDataDir;
	}
	virtual emNetWork NetworkID() const = 0;
	const vector<CDNSSeedData>& DNSSeeds() const {
		return m_vSeeds;
	}
	const vector<unsigned char> &Base58Prefix(emBase58Type type) const {
		return m_vchBase58Prefixes[type];
	}
	virtual const vector<CAddress>& FixedSeeds() const = 0;
	virtual bool IsInFixedSeeds(CAddress &addr) = 0;
	int RPCPort() const {
		return m_nRPCPort;
	}
	int GetUIPort() const {
		return m_nUIPort;
	}
	const string& GetPublicKey() const {
		return m_strPublicKey;
	}
	/******************************paras**************************************/
	static bool IntialParams(int argc, const char* const argv[]);
	static int64_t GetArg(const string& strArg, int64_t nDefault);
	static string GetArg(const string& strArg, const string& strDefault);
	static bool GetBoolArg(const string& strArg, bool fDefault);
	static bool SoftSetArg(const string& strArg, const string& strValue);
	static bool SoftSetBoolArg(const string& strArg, bool fValue);
	static bool IsArgCount(const string& strArg);
	static bool SoftSetArgCover(const string& strArg, const string& strValue);
	static void EraseArg(const string& strArgKey);

	virtual ~CBaseParams() {
	}

	bool CreateGenesisRewardTx(vector<std::shared_ptr<CBaseTransaction> > &vRewardTx, const vector<string> &vInitPubKey);
	static void ParseParameters(int argc, const char* const argv[]);

	static const vector<string> &GetMultiArgs(const string& strArg);
	static  int GetArgsSize();
	static  int GetMultiArgsSize();
	static  map<string, string> GetMapArgs() {
		return m_mapArgs;
	}
	static  map<string, vector<string> > GetMapMultiArgs() {
		return m_mapMultiArgs;
	}
	static void SetMapArgs(const map<string, string> &mapArgs) {
		m_mapArgs = mapArgs;
	}
	static void SetMultiMapArgs(const map<string, vector<string> >&mapMultiArgs) {
		m_mapMultiArgs = mapMultiArgs;
	}

 protected:
	static map<string, string> m_mapArgs;
	static map<string, vector<string> > m_mapMultiArgs;

 protected:
	CBaseParams() ;

	uint256 m_cHashGenesisBlock;
	MessageStartChars m_pchMessageStart;
	// Raw pub key bytes for the broadcast alert signing key.
	vector<unsigned char> m_vchAlertPubKey;
	int m_nDefaultPort;
	int m_nRPCPort;
	int m_nUIPort;
	string m_strPublicKey;
	arith_uint256 m_bnProofOfStakeLimit;
	int m_nSubsidyHalvingInterval;
	string m_strDataDir;
	vector<CDNSSeedData> m_vSeeds;
	vector<unsigned char> m_vchBase58Prefixes[EM_MAX_BASE58_TYPES];

protected:
	mutable bool m_bDebugAll;
	mutable bool m_bDebug;
	mutable bool m_bPrintToConsole;
	mutable bool m_bPrintToToFile;
	mutable bool m_bLogTimestamps;
	mutable bool m_bLogPrintFileLine;
	mutable bool m_bServer;
	mutable bool m_bImporting;
	mutable bool m_bReindex;
	mutable bool m_bBenchmark;
	mutable bool m_bTxIndex;
	mutable int64_t m_llTimeBestReceived;
	mutable int64_t m_llpaytxfee;
	int64_t m_llTargetSpacing;   								//用于限制一个块产生的时间
	int64_t m_llTargetTimespan;
	mutable unsigned int m_unScriptCheckThreads;
	mutable int64_t m_llViewCacheSize;
	mutable int m_nTxCacheHeight;
	mutable int m_nIntervalPos; 								//用于限制矿工 挖矿的 块间隔
	int m_nLogmaxsize; 											// byte  用于限制日志文件的最大长度
	bool m_bOutPut;    											//是否保存合约脚本操作账户日志
	bool m_bAddressToTx;										//是否保存地址与交易的对应
};

extern CBaseParams &SysCfg();

/**
 * Return the currently selected parameters. This won't change after app startup
 * outside of the unit tests.
 */
//CBaseParams &Params();

/** Sets the params returned by Params() to those for the given network. */
//void SelectParams(CBaseParams::Network network);

/**
 * Looks for -regtest or -testnet and then calls SelectParams as appropriate.
 * Returns false if an invalid combination is given.
 */
//bool SelectParamsFromCommandLine();

inline bool TestNet() {
	// Note: it's deliberate that this returns "false" for regression test mode.
	return SysCfg().NetworkID() == CBaseParams::EM_TESTNET;
}

inline bool RegTest() {
	return SysCfg().NetworkID() == CBaseParams::EM_REGTEST;
}

//write for test code
extern const CBaseParams &SysParamsMain();

//write for test code
extern const CBaseParams &SysParamsTest();

//write for test code
extern const CBaseParams &SysParamsReg();

extern string g_strInitPubKey[];

#endif
