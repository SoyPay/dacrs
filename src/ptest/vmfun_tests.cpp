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
#include "VmScript/VmScriptRun.h"
#include "VmScript/CVir8051.h"
#include "VmScript/TestMcu.h"
#include "json/json_spirit_writer_template.h"
#include "rpcclient.h"
using namespace std;
using namespace boost;
using namespace json_spirit;

extern Object CallRPC(const string& strMethod, const Array& params);
extern int TestCallRPC(std::string strMethod, const std::vector<std::string> &vParams, std::string &strRet);
extern void GetAccountInfo(char *address);
extern void GenerateMiner();
	//
//	string strPrint;
//	int nRet;
//	Array params = RPCConvertValues(strMethod, vParams);
//
//	Object reply = CallRPC(strMethod, params);
//
//	// Parse reply
//	const Value& result = find_value(reply, "result");
//	const Value& error = find_value(reply, "error");
//
//	if (error.type() != null_type) {
//		// Error
//		strPrint = "error: " + write_string(error, false);
//		int code = find_value(error.get_obj(), "code").get_int();
//		nRet = abs(code);
//	} else {
//		// Result
//		if (result.type() == null_type)
//			strPrint = "";
//		else if (result.type() == str_type)
//			strPrint = result.get_str();
//		else
//			strPrint = write_string(result, true);
//	}
//	strRet = strPrint;
//	BOOST_MESSAGE(strPrint);
//	//cout << strPrint << endl;
//	return nRet;
//}
//static void GetAccountInfo(char *address) {
//	int argc = 3;
//	char *argv[3] = { "rpctest", "getaccountinfo", address };
//	CommandLineRPC(argc, argv);
//
//}
//static void GenerateMiner() {
//	int argc = 3;
//	char *argv[3] = { "rpctest", "setgenerate", "true" };
//	CommandLineRPC(argc, argv);
//}


std::string TxHash("");
void GenerateMiner(int count) {
	//cout <<"Generate miner" << endl;
	int argc = 3;
	char buffer[10] = {0};
	sprintf(buffer,"%d",count);
	char *argv[4] = { "rpctest", "setgenerate", "true", buffer};
	CommandLineRPC(argc, argv);
}
void GetAccountState1() {
	GetAccountInfo("5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG");
}
void ListRegScript1() {
	//cout << "listRegScript" << endl;
	int argc = 2;
	char *argv[2] = { "rpctest", "listregscript" };
	CommandLineRPC(argc, argv);
}
void CreateRegScriptTx2() {
	cout <<"CreateRegScriptTx1" << endl;
	int argc = 7;
	char *argv[7] =
			{ "rpctest", "registerscripttx", "5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG","0",
					"D:\\VmSdk\\sdk\\Debug\\Exe\\sdk.bin",
					"1000000", "2" };
	CommandLineRPC(argc, argv);
}
void CreateFirstTx()
{
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\"]");
	vInputParams.push_back("01");
	vInputParams.push_back("1000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	if (TestCallRPC("createcontracttx", vInputParams, strReturn) > 0) {
		vInputParams.clear();
		cout << "create secure tx succeed1:"<<strReturn<< endl;
		TxHash = strReturn;
	}
	return ;
}
void CreateSecondTx()
{
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\"]");
	uint256 hash(TxHash.c_str());
	string param ="02";
	param += HexStr(hash);
	vInputParams.push_back(param);
	vInputParams.push_back("1000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	if (TestCallRPC("createcontracttx", vInputParams, strReturn) > 0) {
		vInputParams.clear();
		cout << "create secure tx succeed1:"<<strReturn<< endl;
	//	TxHash = strReturn;
	}
	return ;
}

void CreateThirdTx()
{
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\"]");
	vInputParams.push_back("03");
	vInputParams.push_back("1000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	if (TestCallRPC("createcontracttx", vInputParams, strReturn) > 0) {
		vInputParams.clear();
		cout << "create secure tx succeed1:"<<strReturn<< endl;
	//	TxHash = strReturn;
	}
	return ;
}
void CreateForthTx()
{
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\"]");
	vInputParams.push_back("04");
	vInputParams.push_back("1000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	if (TestCallRPC("createcontracttx", vInputParams, strReturn) > 0) {
		vInputParams.clear();
		cout << "create secure tx succeed1:"<<strReturn<< endl;
	//	TxHash = strReturn;
	}
	return ;
}

void CreateRegScriptTx3() {
	cout <<"CreateRegScriptTx1" << endl;
	int argc = 7;
	char *argv[7] =
			{ "rpctest", "registerscripttx", "5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG","1",
					"010000000100",
					"1000000", "2" };
	CommandLineRPC(argc, argv);
}
void CreateFiveTx()
{
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\"]");
	uint256 hash(TxHash.c_str());
	string param ="05";
	param += HexStr(hash);
	vInputParams.push_back(param);
	vInputParams.push_back("1000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	if (TestCallRPC("createcontracttx", vInputParams, strReturn) > 0) {
		vInputParams.clear();
		cout << "create secure tx succeed1:"<<strReturn<< endl;
		TxHash = strReturn;
	}
	return ;
}
BOOST_AUTO_TEST_SUITE(VM_fun)
BOOST_AUTO_TEST_CASE(test_fun)
{
	GetAccountState1();
	CreateRegScriptTx2();
	GenerateMiner();
//	cout << "start first" << endl;
//	ListRegScript1();
	CreateFirstTx();
	GenerateMiner();
	//cout << "strat second:" << endl;
	CreateSecondTx();
	GenerateMiner();

//	cout << "strat third:" << endl;
	CreateThirdTx();
	GenerateMiner();

//	cout << "script reg:" << endl;
	CreateRegScriptTx3();
	GenerateMiner();

//	cout << "five reg:" << endl;
	CreateFiveTx();
	GenerateMiner();

//	cout << "strat forth:" << endl;
//	CreateForthTx();
//	GenerateMiner(1);
}
BOOST_AUTO_TEST_SUITE_END()
