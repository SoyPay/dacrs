// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_CHAIN_PARAMS_H
#define DACRS_CHAIN_PARAMS_H
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
	string name, host;
	CDNSSeedData(const string &strName, const string &strHost) :
			name(strName), host(strHost) {
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
protected:
	mutable bool fDebugAll;
	mutable bool fDebug;
	mutable bool fPrintToConsole;
	mutable bool fPrintToToFile;
	mutable bool fLogTimestamps;
	mutable bool fLogPrintFileLine;
	mutable bool fServer;
	mutable bool fImporting;
	mutable bool fReindex;
	mutable bool fBenchmark;
	mutable bool fTxIndex;
	mutable int64_t nTimeBestReceived;
	mutable int64_t paytxfee;
	int64_t nTargetSpacing;   //��������һ���������ʱ��
	int64_t nTargetTimespan;
	mutable unsigned int nScriptCheckThreads;
	mutable int64_t nViewCacheSize;
	mutable int nTxCacheHeight;
	mutable int nIntervalPos; //�������ƿ� �ڿ�� ����
	int nLogmaxsize; // byte  ����������־�ļ�����󳤶�
	bool bOutPut;    //�Ƿ񱣴��Լ�ű������˻���־
	bool bAddressToTx;//�Ƿ񱣴��ַ�뽻�׵Ķ�Ӧ

public:

	std::shared_ptr<vector<string> > GetMultiArgsMap(string str) const{
		std::shared_ptr<vector<string> > temp = make_shared<vector<string> >();
		vector<string> te = m_mapMultiArgs[str];
		temp.get()->assign(te.begin(), te.end());
		return temp;
	}
	virtual bool InitalConfig() {
		fServer = GetBoolArg("-server", false);

		m_mapMultiArgs["-debug"].push_back("ERROR"); //add froce ERROR to log
		fDebug = !m_mapMultiArgs["-debug"].empty();
		if (fDebug) {
			fDebugAll = GetBoolArg("-logprintall", false);
			fPrintToConsole = GetBoolArg("-printtoconsole", false);
			fLogTimestamps = GetBoolArg("-logtimestamps", true);
			fPrintToToFile = GetBoolArg("-logprintfofile", false);
			fLogPrintFileLine = GetBoolArg("-logprintfileline", false);
		}
		int64_t nTransactionFee ;
		if(ParseMoney(GetArg("-paytxfee", ""), nTransactionFee) && nTransactionFee > 0){
		paytxfee = nTransactionFee;
		}

	   nIntervalPos = GetArg("-intervalpos", 1440);
	   nLogmaxsize = GetArg("-logmaxsize", 100) * 1024 * 1024;
	   bOutPut = GetBoolArg("-output", false);
	   bAddressToTx = GetBoolArg("-addresstotx", false);
		return true;
	}
	virtual string ToString() const {
		string te = "";

		for (auto & tep1 : m_mapMultiArgs) {
			te += strprintf("key:%s\n",tep1.first);
			vector<string> tep = tep1.second;
			for(auto const & tep3:tep)
			{
				te += strprintf("value :%s\n",tep3.c_str());
			}
		}
		te += strprintf("fDebugAll:%s\n",fDebugAll);
		te += strprintf("fDebug:%s\n",fDebug);
		te += strprintf("fPrintToConsole:%d\n",fPrintToConsole);
		te += strprintf("fPrintToToFile:%d\n",fPrintToToFile);
		te += strprintf("fLogTimestamps:%d\n",fLogTimestamps);
		te += strprintf("fLogPrintFileLine:%d\n",fLogPrintFileLine);
		te += strprintf("fServer:%d\n",fServer);

		te += strprintf("fImporting:%d\n",fImporting);
		te += strprintf("fReindex:%d\n",fReindex);
		te += strprintf("fBenchmark:%d\n",fBenchmark);
		te += strprintf("fTxIndex:%d\n",fTxIndex);
		te += strprintf("nTimeBestReceived:%d\n",nTimeBestReceived);
		te += strprintf("paytxfee:%d\n",paytxfee);
		te += strprintf("nTargetSpacing:%d\n",nTargetSpacing);
		te += strprintf("nTargetTimespan:%d\n",nTargetTimespan);
		te += strprintf("nScriptCheckThreads:%d\n",nScriptCheckThreads);
		te += strprintf("nViewCacheSize:%d\n",nViewCacheSize);
		te += strprintf("nTxCacheHeight:%d\n",nTxCacheHeight);
		te += strprintf("nIntervalPos:%d\n",nIntervalPos);
		te += strprintf("nLogmaxsize:%d\n",nLogmaxsize);

		return te;
	}
	virtual int GetBlockMaxNonce() const
	{
		return 1000;
	}
    int64_t GetTxFee()const;
    int64_t SetDeflautTxFee(int64_t fee)const;
	virtual string GetDefaultTestDataPath() const
	{
		char findchar;
		#ifdef WIN32
		findchar = '\\';
		#else
		findchar = '/';
		#endif

		string strCurDir = boost::filesystem::initial_path<boost::filesystem::path>().string();
		int index = strCurDir.find_last_of(findchar);
		int count = 3;
		while (count--) {
			index = strCurDir.find_last_of(findchar);
			strCurDir = strCurDir.substr(0, index);

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
	int getConnectTimeOut()const {
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


	int64_t GetTargetSpacing() const {
		return nTargetSpacing;
	}
	int64_t GetTargetTimespan() const {
		return nTargetTimespan;
	}

	int64_t GetBestRecvTime() const {
		return nTimeBestReceived;
	}
	int64_t GetScriptCheckThreads() const {
		return nScriptCheckThreads;
	}
	unsigned int GetViewCacheSize() const {
		return nViewCacheSize;
	}
	int GetTxCacheHeight() const {
		return nTxCacheHeight;
	}
	int GetIntervalPos() const {
		return nIntervalPos;
	}
	int GetLogMaxSize() const
	{
		return nLogmaxsize;
	}
	int GetMaxDay() const
	{
	 return GetIntervalPos() * 30;
	}

	void SetImporting(bool flag)const {
		fImporting = flag;
	}

	void SetReIndex(bool flag)const {
		fReindex = flag;
	}
	void SetBenchMark(bool flag)const {
		fBenchmark = flag;
	}
	void SetTxIndex(bool flag)const {
		fTxIndex = flag;
	}
	void SetBestRecvTime(int64_t nTime)const {
		nTimeBestReceived = nTime;
	}
	void SetScriptCheckThreads(int64_t nNum)const {
		nScriptCheckThreads = nNum;
	}
	void SetViewCacheSize(unsigned int nSize)const {
		nViewCacheSize = nSize;
	}
	void SetTxCacheHeight(int nHeight)const {
		nTxCacheHeight = nHeight;
	}
	bool GetOutPutLog() const {
		return bOutPut;
	}
	bool GetAddressToTxFlag() const {
		return bAddressToTx;
	}
public:
	/**
	 *
	 */
	enum Network {
		MAIN,            //!< MAIN
		TESTNET,         //!< TESTNET
		REGTEST,         //!< REGTEST
		MAX_NETWORK_TYPES//!< MAX_NETWORK_TYPES
	};
/**
 *
 */
	enum Base58Type {
		PUBKEY_ADDRESS, //!< PUBKEY_ADDRESS
		SCRIPT_ADDRESS, //!< SCRIPT_ADDRESS
		SECRET_KEY,     //!< SECRET_KEY
		EXT_PUBLIC_KEY, //!< EXT_PUBLIC_KEY
		EXT_SECRET_KEY, //!< EXT_SECRET_KEY
		ACC_ADDRESS,    //!< ACC_ADDRESS
		MAX_BASE58_TYPES//!< MAX_BASE58_TYPES
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
	const arith_uint256 ProofOfWorkLimit()  {
		return bnProofOfStakeLimit;
	}
	int SubsidyHalvingInterval() const {
		return nSubsidyHalvingInterval;
	}
	virtual int64_t GetMaxFee()const {return 1000000000;}
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
	virtual bool IsInFixedSeeds(CAddress &addr) = 0;
	int RPCPort() const {
		return nRPCPort;
	}
	const string& GetPublicKey() const {return publicKey;}
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
	/********************************************************************/

protected:
	CBaseParams() ;

	uint256 hashGenesisBlock;
	MessageStartChars pchMessageStart;
	// Raw pub key bytes for the broadcast alert signing key.
	vector<unsigned char> vAlertPubKey;
	int nDefaultPort;
	int nRPCPort;
	string publicKey;
	arith_uint256 bnProofOfStakeLimit;
	int nSubsidyHalvingInterval;
	string strDataDir;
	vector<CDNSSeedData> vSeeds;
	vector<unsigned char> base58Prefixes[MAX_BASE58_TYPES];
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
	return SysCfg().NetworkID() == CBaseParams::TESTNET;
}

inline bool RegTest() {
	return SysCfg().NetworkID() == CBaseParams::REGTEST;
}

//write for test code
extern const CBaseParams &SysParamsMain();

//write for test code
extern const CBaseParams &SysParamsTest();

//write for test code
extern const CBaseParams &SysParamsReg();

extern string initPubKey[];

#endif
