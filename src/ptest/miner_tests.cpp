// Copyright (c) 2011-2014 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "miner.h"
#include "uint256.h"
#include "util.h"
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "json/json_spirit_writer_template.h"
#include "./rpc/rpcclient.h"
using namespace std;
using namespace boost;
using namespace json_spirit;

extern Object CallRPC(const string& strMethod, const Array& params);
extern void SHA256Transform(void* pstate, void* pinput, const void* pinit);

int CommandLineRPC_GetValue(int nArgc, const char *szArgv[], Value &value) {
	string strPrint;
	int nRet = 0;
	try {
		// Skip switches
		while (nArgc > 1 && IsSwitchChar(szArgv[1][0])) {
			nArgc--;
			szArgv++;
		}

		// Method
		if (nArgc < 2) {
			throw runtime_error("too few parameters");
		}
		string strMethod = szArgv[1];

		// Parameters default to strings
		std::vector<std::string> strParams(&szArgv[2], &szArgv[nArgc]);
		Array params = RPCConvertValues(strMethod, strParams);

		// Execute
		Object reply = CallRPC(strMethod, params);

		// Parse reply
		const Value& result = find_value(reply, "result");
		const Value& error = find_value(reply, "error");

		if (error.type() != null_type) {
			// Error
			strPrint = "error: " + write_string(error, false);
			int code = find_value(error.get_obj(), "code").get_int();
			nRet = abs(code);
		} else {
			value = result;
			// Result
			if (result.type() == null_type) {
				strPrint = "";
			} else if (result.type() == str_type) {
				strPrint = result.get_str();
			} else {
				strPrint = write_string(result, true);
			}
		}
	} catch (boost::thread_interrupted) {
		throw;
	} catch (std::exception& e) {
		strPrint = string("error: ") + e.what();
		nRet = abs(-1);
	} catch (...) {
		PrintExceptionContinue(NULL, "CommandLineRPC()");
		throw;
	}

	if (strPrint != "") {
//      fprintf((nRet == 0 ? stdout : stderr), "%s\n", strPrint.c_str());
	}

	return nRet;
}

const static int g_snFrozenHeight = 100;

const static int g_snMatureHeight = 100;

struct ST_ACC_STATE {
	int64_t lldUnmatureMoney;
	int64_t lldFreeMoney;
	int64_t lldFrozenMoney;
	ST_ACC_STATE() {
		lldUnmatureMoney = 0;
		lldFreeMoney = 0;
		lldFrozenMoney = 0;
	}
	ST_ACC_STATE(int64_t a, int64_t b, int64_t c) {
		lldUnmatureMoney = a;
		lldFreeMoney = b;
		lldFrozenMoney = c;
	}

	bool operator==(ST_ACC_STATE &a) {
		if (lldUnmatureMoney == a.lldUnmatureMoney && lldFreeMoney == a.lldFreeMoney
				&& lldFrozenMoney == a.lldFrozenMoney) {
			return true;
		}
		return false;
	}

	bool SumEqual(ST_ACC_STATE &a) {
		int64_t l = lldUnmatureMoney + lldFreeMoney + lldFrozenMoney;
		int64_t r = a.lldUnmatureMoney + a.lldFreeMoney + a.lldFrozenMoney;

		return l == r;
	}

	ST_ACC_STATE& operator+=(ST_ACC_STATE &a) {
		lldUnmatureMoney += a.lldUnmatureMoney;
		lldFreeMoney += a.lldFreeMoney;
		lldFrozenMoney += a.lldFrozenMoney;
		return *this;
	}
};

struct ST_ACC_OPER_LOG {
	std::map<int, ST_ACC_STATE> mapAccState;
	ST_ACC_OPER_LOG() {
		mapAccState.clear();
	}
	bool Add(int &nHeight, ST_ACC_STATE &accstate) {
		mapAccState[nHeight] = accstate;
		return true;
	}

	void MergeAcc(int nHeight) {
		for (auto &item : mapAccState) {
			if (nHeight > item.first + g_snFrozenHeight) {
				item.second.lldFreeMoney += item.second.lldFrozenMoney;
				item.second.lldFrozenMoney = 0.0;
			}

			if (nHeight > item.first + g_snMatureHeight) {
				item.second.lldFreeMoney += item.second.lldUnmatureMoney;
				item.second.lldUnmatureMoney = 0.0;
			}
		}
	}
};

class CMinerTest{
public:
	std::map<string,ST_ACC_STATE> mapAccState;
	std::map<string,ST_ACC_OPER_LOG> m_mapAccOperLog;
	int nCurHeight;
	int64_t llCurMoney;
	int64_t llCurFee;
private:
	int GetRandomFee() {
		srand(time(NULL));
		int r = (rand() % 1000000) + 1000000;
		return r;
	}
	int GetRandomMoney() {
		srand(time(NULL));
		int r = (rand() % 1000) + 1000;
		return r;
	}
public:

	bool GetOneAddr(std::string &strAddr, const char *pStrMinMoney, const char *bpBoolReg) {
		//CommanRpc
		const char *pszArgv[] = { "rpctest", "getoneaddr", pStrMinMoney, bpBoolReg };
		int nArgc = sizeof(pszArgv) / sizeof(char*);

		Value value;
		int nRet = CommandLineRPC_GetValue(nArgc, pszArgv, value);
		if (!nRet) {
			strAddr = value.get_str();
			LogPrint("test_miners", "GetOneAddr:%s\r\n", strAddr.c_str());
			return true;
		}
		return false;
	}

	bool GetOneScriptId(std::string &regscriptid) {
		//CommanRpc
		const char *pszArgv[] = { "rpctest", "listscriptregid" };
		int nArgc = sizeof(pszArgv) / sizeof(char*);

		Value value;
		int nRet = CommandLineRPC_GetValue(nArgc, pszArgv, value);
		if (!nRet) {
			Object &Oid = value.get_obj();
			regscriptid = Oid[0].value_.get_str();
			LogPrint("test_miners", "GetOneAddr:%s\r\n", regscriptid.c_str());
			return true;
		}
		return false;
	}

	bool GetNewAddr(std::string &strAddr) {
		//CommanRpc
		const char *pszArgv[] = { "rpctest", "getnewaddress" };
		int nArgc = sizeof(pszArgv) / sizeof(char*);

		Value value;
		int nRet = CommandLineRPC_GetValue(nArgc, pszArgv, value);
		if (!nRet) {
			strAddr = value.get_str();
			LogPrint("test_miners", "GetNewAddr:%s\r\n", strAddr.c_str());
			return true;
		}
		return false;
	}

	bool GetAccState(const std::string &strAddr, ST_ACC_STATE &accstate) {
		//CommanRpc
		char arrchTemp[64] = { 0 };
		strncpy(arrchTemp, strAddr.c_str(), sizeof(arrchTemp) - 1);

		const char *pszArgv[] = { "rpctest", "getaddramount", arrchTemp };
		int nArgc = sizeof(pszArgv) / sizeof(char*);
		Value value;
		int nRet = CommandLineRPC_GetValue(nArgc, pszArgv, value);
		if (!nRet) {
			Object obj = value.get_obj();
			double dfree = find_value(obj, "free amount").get_real();
			double dmature = find_value(obj, "mature amount").get_real();
			double dfrozen = find_value(obj, "frozen amount").get_real();
			accstate.lldFreeMoney = roundint64(dfree * COIN);
			accstate.lldUnmatureMoney = roundint64(dmature * COIN);
			accstate.lldFrozenMoney = roundint64(dfrozen * COIN);
			LogPrint("test_miners", "strAddr:%s GetAccState FreeMoney:%0.8lf matureMoney:%0.8lf FrozenMoney:%0.8lf\r\n",
					strAddr.c_str(), dfree, dmature, dfrozen);
			return true;
		}
		return false;
	}

	bool GetBlockHeight(int &nHeight) {
		nHeight = 0;
		const char *pszArgv[] = { "rpctest", "getinfo", };
		int nArgc = sizeof(pszArgv) / sizeof(char*);

		Value value;
		int nRet = CommandLineRPC_GetValue(nArgc, pszArgv, value);
		if (!nRet) {
			Object obj = value.get_obj();

			nHeight = find_value(obj, "blocks").get_int();
			LogPrint("test_miners", "GetBlockHeight:%d\r\n", nHeight);
			return true;
		}
		return false;
	}

	bool CreateNormalTx(const std::string &srcAddr, const std::string &desAddr, const int nHeight) {
		//CommanRpc
		char arrchSrc[64] = { 0 };
		strncpy(arrchSrc, srcAddr.c_str(), sizeof(arrchSrc) - 1);

		char arrchDest[64] = { 0 };
		strncpy(arrchDest, desAddr.c_str(), sizeof(arrchDest) - 1);

		char arrchMoney[64] = { 0 };
		int nMoney = GetRandomMoney();
		sprintf(arrchMoney, "%d00000000", nMoney);
		llCurMoney = nMoney * COIN;

		char arrchFee[64] = { 0 };
		int nFee = GetRandomFee();
		sprintf(arrchFee, "%d", nFee);
		llCurFee = nFee;

		char arrchHeight[16] = { 0 };
		sprintf(arrchHeight, "%d", nHeight);

		const char *pchArgv[] = { "rpctest", "createnormaltx", arrchSrc, arrchDest, arrchMoney, arrchFee, arrchHeight };
		int nArgc = sizeof(pchArgv) / sizeof(char*);

		Value value;
		int nRet = CommandLineRPC_GetValue(nArgc, pchArgv, value);
		if (!nRet) {
			LogPrint("test_miners", "CreateNormalTx:%s\r\n", value.get_str().c_str());
			return true;
		}
		return false;
	}

	bool CreateFreezeTx(const std::string &strAddr, const int nHeight) {
		//CommanRpc
		char arrchAddr[64] = { 0 };
		strncpy(arrchAddr, strAddr.c_str(), sizeof(arrchAddr) - 1);

		char arrchMoney[64] = { 0 };
		int nMoney = GetRandomMoney();
		sprintf(arrchMoney, "%d00000000", nMoney);
		llCurMoney = nMoney * COIN;

		char arrchFee[64] = { 0 };
		int nFee = GetRandomFee();
		sprintf(arrchFee, "%d", nFee);
		llCurFee = nFee;

		char arrchHeight[16] = { 0 };
		sprintf(arrchHeight, "%d", nHeight);

		char arrchFreeheight[16] = { 0 };
		sprintf(arrchFreeheight, "%d", nHeight + 100);

		const char *argv[] = { "rpctest", "createfreezetx", arrchAddr, arrchMoney, arrchFee, arrchHeight,
				arrchFreeheight };
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		int nRet = CommandLineRPC_GetValue(argc, argv, value);
		if (!nRet) {
			LogPrint("test_miners", "CreateFreezeTx:%s\r\n", value.get_str().c_str());
			return true;
		}
		return false;
	}

	bool registaccounttx(const std::string &strAddr,const int nHeight)
	{
		//CommanRpc
		char arrchAddr[64] = { 0 };
		strncpy(arrchAddr, strAddr.c_str(), sizeof(arrchAddr)-1);

		char arrchFee[64] = { 0 };
		int nFee = GetRandomFee();
		sprintf(arrchFee, "%d", nFee);
		llCurFee = nFee;

		char arrchHeight[16] = {0};
		sprintf(arrchHeight,"%d",nHeight);


		const char *argv[] = { "rpctest", "registaccounttx", arrchAddr, arrchFee, arrchHeight};
		int argc = sizeof(argv)/sizeof(char*);

		Value value;
		int nRet = CommandLineRPC_GetValue(argc, argv, value);
		if (!nRet)
		{
			LogPrint("test_miners","RegisterSecureTx:%s\r\n",value.get_str().c_str());
			return true;
		}
		return false;
	}

	bool CreateContractTx(const std::string &strScriptid, const std::string &strAddrs, const std::string &strContract, const int nHeight)
	{
//		char cscriptid[1024] = { 0 };
//		vector<char> te(strScriptid.begin(),strScriptid.end());
//		&te[0]
//		strncpy(cscriptid, strScriptid.c_str(), sizeof(strScriptid)-1);

//		char caddr[1024] = { 0 };
//		strncpy(caddr, strAddrs.c_str(), sizeof(strAddrs)-1);

//		char ccontract[128*1024] = { 0 };
//		strncpy(ccontract, strContract.c_str(), sizeof(strContract)-1);

		char arrchFee[64] = { 0 };
		int nFee = GetRandomFee();
		sprintf(arrchFee, "%d", nFee);
		llCurFee = nFee;

		char arrchHeight[16] = {0};
		sprintf(arrchHeight,"%d",nHeight);

		 const char *argv[] = { "rpctest", "createcontracttx", (char *)(strScriptid.c_str()), (char *)(strAddrs.c_str()), (char *)(strContract.c_str()), arrchFee, arrchHeight};
		int argc = sizeof(argv)/sizeof(char*);

		Value value;
		int nRet = CommandLineRPC_GetValue(argc, argv, value);
		if (!nRet)
		{
			LogPrint("test_miners","createcontracttx:%s\r\n",value.get_str().c_str());
			return true;
		}
		return false;
	}

	bool RegisterAppTx(const std::string &strAddr, const std::string &strScript, const int nHeight)
	{
		//CommanRpc
		char arrchAddr[64] = { 0 };
		strncpy(arrchAddr, strAddr.c_str(), sizeof(arrchAddr)-1);

		char arrchCsript[128*1024] = {0};
		strncpy(arrchCsript, strScript.c_str(), sizeof(arrchCsript)-1);

		char arrchFee[64] = { 0 };
		int nFee = GetRandomFee();
		sprintf(arrchFee, "%d", nFee);
		llCurFee = nFee;

		char arrchHeight[16] = {0};
		sprintf(arrchHeight,"%d",nHeight);


		const char *argv[] = { "rpctest", "registerapptx", arrchAddr, arrchCsript, arrchFee, arrchHeight};
		int argc = sizeof(argv)/sizeof(char*);

		Value value;
		int nRet = CommandLineRPC_GetValue(argc, argv, value);
		if (!nRet)
		{
			LogPrint("test_miners","RegisterSecureTx:%s\r\n",value.get_str().c_str());
			return true;
		}
		return false;
	}

	bool CreateSecureTx(const string &strScriptid,const vector<string> &vstrObaddrs,
			const vector<string> &strAddrs,const string&strContract,const int nHeight)
	{
		//CommanRpc
		char arrchCscriptid[64] = { 0 };
		strncpy(arrchCscriptid, strScriptid.c_str(), sizeof(arrchCscriptid)-1);

		char arrchCobstr[512] = {0};
		{
			Array array;
			array.clear();
			for (const auto &str : vstrObaddrs)
			{
				array.push_back(str);
			}
			string arraystr = write_string(Value(array),false);
			strncpy(arrchCobstr, arraystr.c_str(), sizeof(arrchCobstr)-1);
		}
		char arrchAddrstr[512] = {0};
		{
			Array array;
			array.clear();
			for (const auto &str : strAddrs) {
				array.push_back(str);
			}
			string strArraystr = write_string(Value(array), false);
			strncpy(arrchAddrstr, strArraystr.c_str(), sizeof(arrchAddrstr) - 1);
		}

		char arrchContract[10*1024] = { 0 };
		strncpy(arrchContract, strContract.c_str(), sizeof(arrchContract)-1);

		char arrchHeight[16] = {0};
		sprintf(arrchHeight,"%d",nHeight);


		const char *argv[] = { "rpctest", "createsecuretx", arrchCscriptid,arrchCobstr,arrchAddrstr,arrchContract,"1000000",arrchHeight};
		int argc = sizeof(argv)/sizeof(char*);

		Value value;
		int nRet = CommandLineRPC_GetValue(argc, argv, value);
		if (!nRet)
		{
			LogPrint("test_miners","CreateSecureTx:%s\r\n",value.get_str().c_str());
			return true;
		}
		return false;
	}

	bool SignSecureTx(const string &securetx)
	{
		//CommanRpc
		char arrchCsecuretx[10*1024] = { 0 };
		strncpy(arrchCsecuretx, securetx.c_str(), sizeof(arrchCsecuretx)-1);


		const char *argv[] = { "rpctest", "signsecuretx", arrchCsecuretx};
		int argc = sizeof(argv)/sizeof(char*);

		Value value;
		int nRet = CommandLineRPC_GetValue(argc, argv, value);
		if (!nRet)
		{
			LogPrint("test_miners","SignSecureTx:%s\r\n",value.get_str().c_str());
			return true;
		}
		return false;
	}

	bool IsAllTxInBlock()
	{
		const char *argv[] = { "rpctest", "listunconfirmedtx" };
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		int nRet = CommandLineRPC_GetValue(argc, argv, value);
		if (!nRet) {
			Array array = value.get_array();
			if(array.size() == 0)
			return true;
		}
		return false;
	}

	bool GetBlockHash(const int nHeight,std::string &strBlockhash)
	{
		char arrchHeight[16] = {0};
		sprintf(arrchHeight,"%d",nHeight);

		const char *argv[] = {"rpctest", "getblockhash",arrchHeight};
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		int nRet = CommandLineRPC_GetValue(argc, argv, value);
		if (!nRet)
		{
			strBlockhash = value.get_str();
			LogPrint("test_miners","GetBlockHash:%s\r\n",strBlockhash.c_str());
			return true;
		}
		return false;
	}

	bool GetBlockMinerAddr(const std::string &strBlockhash,std::string &strAddr)
	{
		char cblockhash[80] = {0};
		strncpy(cblockhash,strBlockhash.c_str(),sizeof(cblockhash)-1);

		const char *argv[] = {"rpctest", "getblock",cblockhash};
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		int nRet = CommandLineRPC_GetValue(argc, argv, value);
		if (!nRet)
		{
			Array txs = find_value(value.get_obj(),"tx").get_array();
			strAddr = find_value(txs[0].get_obj(),"strAddr").get_str();
			LogPrint("test_miners","GetBlockMinerAddr:%s\r\n",strAddr.c_str());
			return true;
		}
		return false;
	}

	bool GenerateOneBlock()
	{
		const char *argv[] = {"rpctest", "setgenerate","true"};
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		int nRet = CommandLineRPC_GetValue(argc, argv, value);
		if (!nRet) {
			return true;
		}
		return false;
	}

public:
	CMinerTest() {
		mapAccState.clear();
		m_mapAccOperLog.clear();
		nCurHeight = 0;
	}
	bool CheckAccState(const string &strAddr, ST_ACC_STATE &tLastState, bool bAccurate = false) {
		ST_ACC_STATE tInitState = mapAccState[strAddr];
		ST_ACC_OPER_LOG operlog = m_mapAccOperLog[strAddr];
		operlog.MergeAcc(nCurHeight);

		for (auto & item : operlog.mapAccState) {
			tInitState += item.second;
		}

		bool b = false;
		if (bAccurate) {
			b = (tInitState == tLastState);
		} else {
			b = tLastState.SumEqual(tInitState);
		}
		return b;
	}
};

BOOST_FIXTURE_TEST_SUITE(miner_tests,CMinerTest)

BOOST_FIXTURE_TEST_CASE(block_normaltx_and_regaccounttx,CMinerTest) {
	//printf("\r\block_normaltx test start:\r\n");
	string strSrcaddr;
	string strDestAddr;
	BOOST_REQUIRE(GetOneAddr(strSrcaddr, "1100000000000", "true"));

	ST_ACC_STATE tInitState;
	BOOST_REQUIRE(GetAccState(strSrcaddr, tInitState));
	mapAccState[strSrcaddr] = tInitState; //insert

	int nHeight = 0;
	BOOST_REQUIRE(GetBlockHeight(nHeight));
	nCurHeight = nHeight;
	uint64_t llTotalfee = 0;
	{
		BOOST_REQUIRE(GetNewAddr(strDestAddr));
		BOOST_REQUIRE(CreateNormalTx(strSrcaddr, strDestAddr, nHeight));
		llTotalfee += llCurFee;
		LogPrint("test_miners", "strSrcaddr:%s\r\ndestaddr:%s\r\nnCurFee:%I64d\r\n", strSrcaddr.c_str(),
				strDestAddr.c_str(), llCurFee);
		ST_ACC_OPER_LOG &tOperlog1 = m_mapAccOperLog[strSrcaddr];
		ST_ACC_OPER_LOG &tOperlog2 = m_mapAccOperLog[strDestAddr];
		ST_ACC_STATE tAcc1(0, -(llCurMoney + llCurFee), 0);
		ST_ACC_STATE tAcc2(0, llCurMoney, 0);
		tOperlog1.Add(nHeight, tAcc1);
		tOperlog2.Add(nHeight, tAcc2);
	}
	BOOST_REQUIRE(GenerateOneBlock());
	nHeight++;

	BOOST_REQUIRE(IsAllTxInBlock());
	string strMineraddr;
	string strBlockhash;
	BOOST_REQUIRE(GetBlockHash(nHeight, strBlockhash));
	BOOST_REQUIRE(GetBlockMinerAddr(strBlockhash, strMineraddr));

	if (strMineraddr == strSrcaddr) {
		ST_ACC_STATE tAcc(llTotalfee, 0, 0);
		m_mapAccOperLog[strMineraddr].Add(nHeight, tAcc);
	} else {
		BOOST_REQUIRE(GetAccState(strMineraddr, tInitState));
		mapAccState[strMineraddr] = tInitState; //insert
	}

	for (auto & item : m_mapAccOperLog) {
		ST_ACC_STATE tLastState;
		BOOST_REQUIRE(GetAccState(item.first, tLastState));
		BOOST_REQUIRE(CheckAccState(item.first, tLastState));
	}

	//test reg account
	{
		nCurHeight = nHeight;
		BOOST_REQUIRE(registaccounttx(strDestAddr, nHeight));
		{
			ST_ACC_OPER_LOG &operlog1 = m_mapAccOperLog[strDestAddr];
			ST_ACC_STATE acc1(0, -llCurFee, 0);
			operlog1.Add(nHeight, acc1);
		}
		BOOST_REQUIRE(GenerateOneBlock());
		nHeight++;

		BOOST_REQUIRE(IsAllTxInBlock());
		string strMineraddr;
		string strBlockhash;
		BOOST_REQUIRE(GetBlockHash(nHeight, strBlockhash));
		BOOST_REQUIRE(GetBlockMinerAddr(strBlockhash, strMineraddr));

		if (strMineraddr == strDestAddr) {
			ST_ACC_STATE tAcc(llCurFee, 0, 0);
			m_mapAccOperLog[strMineraddr].Add(nHeight, tAcc);
		} else {
			BOOST_REQUIRE(GetAccState(strMineraddr, tInitState));
			mapAccState[strMineraddr] = tInitState; //insert
		}

		for (auto & item : m_mapAccOperLog) {
			ST_ACC_STATE tLastState;
			BOOST_REQUIRE(GetAccState(item.first, tLastState));
			BOOST_REQUIRE(CheckAccState(item.first, tLastState));
		}
	}
}

BOOST_FIXTURE_TEST_CASE(block_regscripttx_and_contracttx,CMinerTest) {
	string strSrcaddr;
	BOOST_REQUIRE(GetOneAddr(strSrcaddr, "1100000000000", "true"));

	ST_ACC_STATE tInitState;
	BOOST_REQUIRE(GetAccState(strSrcaddr, tInitState));
	mapAccState[strSrcaddr] = tInitState; //insert

	int nHeight = 0;
	BOOST_REQUIRE(GetBlockHeight(nHeight));
	nCurHeight = nHeight;
	//test regscripttx
	{
		string strScript =
				"fd3e0102001d000000000022220000000000000000222202011112013512013a75d0007581bf750900750a0f020017250910af08f509400c150a8008f5094002150ad2af222509c582c0e0e50a34ffc583c0e0e509c3958224f910af0885830a858209800885830a858209d2afcef0a3e520f0a37808e608f0a3defaeff0a3e58124fbf8e608f0a3e608f0a30808e608f0a3e608f0a315811581d0e0fed0e0f815811581e8c0e0eec0e022850a83850982e0a3fee0a3f5207808e0a3f608dffae0a3ffe0a3c0e0e0a3c0e0e0a3c0e0e0a3c0e010af0885820985830a800885820985830ad2afd083d0822274f8120042e990fbfef01200087f010200a8c082c083ea90fbfef0eba3f012001202010cd083d0822274f812004274fe12002ceafeebff850982850a83eef0a3eff0aa09ab0a790112013d80ea79010200e80200142200";
		BOOST_REQUIRE(RegisterAppTx(strSrcaddr, strScript, nHeight));
		ST_ACC_OPER_LOG &tOperlog1 = m_mapAccOperLog[strSrcaddr];
		ST_ACC_STATE tAcc1(0, -llCurFee, 0);
		tOperlog1.Add(nHeight, tAcc1);

		BOOST_REQUIRE(GenerateOneBlock());
		nHeight++;

		BOOST_REQUIRE(IsAllTxInBlock());
		string strMineraddr;
		string strBlockhash;
		BOOST_REQUIRE(GetBlockHash(nHeight, strBlockhash));
		BOOST_REQUIRE(GetBlockMinerAddr(strBlockhash, strMineraddr));

		if (strMineraddr == strSrcaddr) {
			ST_ACC_STATE tAcc(llCurFee, 0, 0);
			m_mapAccOperLog[strMineraddr].Add(nHeight, tAcc);
		} else {
			BOOST_REQUIRE(GetAccState(strMineraddr, tInitState));
			mapAccState[strMineraddr] = tInitState; //insert
		}

		for (auto & item : m_mapAccOperLog) {
			ST_ACC_STATE tLastState;
			BOOST_REQUIRE(GetAccState(item.first, tLastState));
			BOOST_REQUIRE(CheckAccState(item.first, tLastState));
		}
	}

	//test contracttx
	{
		CRegID scriptId(nHeight, (uint16_t) 1);

		string strConaddr;
		do {
			BOOST_REQUIRE(GetOneAddr(strConaddr, "1100000000000", "true"));
		} while (strConaddr == strSrcaddr);
		string strVconaddr = "[\"" + strConaddr + "\"] ";

		ST_ACC_STATE tInitState;
		BOOST_REQUIRE(GetAccState(strConaddr, tInitState));
		mapAccState[strConaddr] = tInitState; //insert

		string strScriptid;
		BOOST_REQUIRE(GetOneScriptId(strScriptid));

		BOOST_REQUIRE(CreateContractTx(strScriptid, strVconaddr, "010203040506070809", nHeight));

		ST_ACC_OPER_LOG &tOperlog1 = m_mapAccOperLog[strConaddr];
		ST_ACC_STATE tAcc1(0, -llCurFee, 0);
		tOperlog1.Add(nHeight, tAcc1);

		BOOST_REQUIRE(GenerateOneBlock());
		nHeight++;

		BOOST_REQUIRE(IsAllTxInBlock());
		string strMineraddr;
		string strBlockhash;
		BOOST_REQUIRE(GetBlockHash(nHeight, strBlockhash));
		BOOST_REQUIRE(GetBlockMinerAddr(strBlockhash, strMineraddr));

		if (strMineraddr == strConaddr) {
			ST_ACC_STATE tAcc(llCurFee, 0, 0);
			m_mapAccOperLog[strMineraddr].Add(nHeight, tAcc);
		} else {
			BOOST_REQUIRE(GetAccState(strMineraddr, tInitState));
			mapAccState[strMineraddr] = tInitState; //insert
		}

		for (auto & item : m_mapAccOperLog) {
			ST_ACC_STATE tLastState;
			BOOST_REQUIRE(GetAccState(item.first, tLastState));
			BOOST_REQUIRE(CheckAccState(item.first, tLastState));
		}
	}
}

BOOST_FIXTURE_TEST_CASE(block_frozentx,CMinerTest) {
	//printf("\r\nblock_frozentx test start:\r\n");
	string strSrcaddr;
	BOOST_REQUIRE(GetOneAddr(strSrcaddr, "1100000000000", "true"));

	ST_ACC_STATE tInitState;
	BOOST_REQUIRE(GetAccState(strSrcaddr, tInitState));
	mapAccState[strSrcaddr] = tInitState; //insert

	int nHeight = 0;
	BOOST_REQUIRE(GetBlockHeight(nHeight));
	nCurHeight = nHeight;
	uint64_t llTotalfee = 0;
	{
		BOOST_REQUIRE(CreateFreezeTx(strSrcaddr, nHeight));
		llTotalfee += llCurFee;
		ST_ACC_OPER_LOG &tOperlog1 = m_mapAccOperLog[strSrcaddr];
		ST_ACC_STATE tAcc(0, -(llCurMoney + llCurFee), llCurMoney);
		tOperlog1.Add(nHeight, tAcc);
	}
	BOOST_REQUIRE(GenerateOneBlock());
	nHeight++;
	BOOST_REQUIRE(IsAllTxInBlock());
	string strMineraddr;
	string strBlockhash;
	BOOST_REQUIRE(GetBlockHash(nHeight, strBlockhash));
	BOOST_REQUIRE(GetBlockMinerAddr(strBlockhash, strMineraddr));

	if (strMineraddr == strSrcaddr) {
		ST_ACC_STATE tAcc(llTotalfee, 0, 0);
		m_mapAccOperLog[strMineraddr].Add(nHeight, tAcc);
	} else {
		BOOST_REQUIRE(GetAccState(strMineraddr, tInitState));
		mapAccState[strMineraddr] = tInitState; //insert
	}

	for (auto & item : m_mapAccOperLog) {
		ST_ACC_STATE tLastState;
		BOOST_REQUIRE(GetAccState(item.first, tLastState));
		BOOST_REQUIRE(CheckAccState(item.first, tLastState));
	}
}



BOOST_AUTO_TEST_SUITE_END()
