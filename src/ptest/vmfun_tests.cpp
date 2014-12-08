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
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_reader.h"
#include "json/json_spirit_writer.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_stream_reader.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
using namespace std;
using namespace boost;
using namespace json_spirit;
using namespace std;
using namespace boost;
using namespace json_spirit;

extern Object CallRPC(const string& strMethod, const Array& params);
extern int TestCallRPC(std::string strMethod, const std::vector<std::string> &vParams, std::string &strRet);
extern void GetAccountInfo(char *address);
extern void GenerateMiner();
extern string Parsejson(string str);
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
	GetAccountInfo("mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA");
}
void ListRegScript1() {
	//cout << "listRegScript" << endl;
	int argc = 2;
	char *argv[2] = { "rpctest", "listregscript" };
	CommandLineRPC(argc, argv);
}
void CreateRegScriptTx2() {
	int argc = 7;
	char* path = "D:\\bitcoin\\data\\sdk.bin";
	string message = path;
	message += "not exitst";
	BOOST_CHECK_MESSAGE(boost::filesystem::exists(path),message);
	char *argv[7] =
			{ "rpctest", "registerscripttx", "mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y","0",
					path,
					"1000000", "2" };
	CommandLineRPC(argc, argv);
}
string CreateFirstTx()
{
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb\"]");
	vInputParams.push_back("01");
	vInputParams.push_back("1000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	if (TestCallRPC("createcontracttx", vInputParams, strReturn) > 0) {
		vInputParams.clear();
	//	cout << "create secure tx succeed1:"<<strReturn<< endl;
		TxHash = Parsejson(strReturn);
	}
	return strReturn;
}
string CreateSecondTx()
{
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb\"]");
	uint256 hash(TxHash.c_str());
	string param ="02";
	param += HexStr(hash);
	vInputParams.push_back(param);
	vInputParams.push_back("1000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	if (TestCallRPC("createcontracttx", vInputParams, strReturn) > 0) {
		vInputParams.clear();
	//	cout << "create secure tx succeed1:"<<strReturn<< endl;
	//	TxHash = strReturn;
	}
	return strReturn;
}

string CreateThirdTx()
{
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb\"]");
	vInputParams.push_back("03");
	vInputParams.push_back("1000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	if (TestCallRPC("createcontracttx", vInputParams, strReturn) > 0) {
		vInputParams.clear();
	//	cout << "create secure tx succeed1:"<<strReturn<< endl;
	//	TxHash = strReturn;
	}
	return strReturn;
}
string CreateForthTx()
{
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb\"]");
	vInputParams.push_back("04");
	vInputParams.push_back("1000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	if (TestCallRPC("createcontracttx", vInputParams, strReturn) > 0) {
		vInputParams.clear();
	//	cout << "create secure tx succeed1:"<<strReturn<< endl;
	//	TxHash = strReturn;
	}
	return strReturn;
}

void CreateRegScriptTx3() {
//	cout <<"CreateRegScriptTx1" << endl;
	int argc = 7;
	char *argv[7] =
			{ "rpctest", "registerscripttx", "n4muwAThwzWvuLUh74nL3KYwujhihke1Kb","1",
					"010000000100",
					"1000000", "2" };
	CommandLineRPC(argc, argv);
}
string CreateFiveTx()
{
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb\"]");
	uint256 hash(TxHash.c_str());
	string param ="05";
	param += HexStr(hash);
	vInputParams.push_back(param);
	vInputParams.push_back("1000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	if (TestCallRPC("createcontracttx", vInputParams, strReturn) > 0) {
		vInputParams.clear();
	//	cout << "create secure tx succeed1:"<<strReturn<< endl;
		TxHash = Parsejson(strReturn);
	}
	return strReturn;
}
BOOST_AUTO_TEST_SUITE(VM_fun)
BOOST_AUTO_TEST_CASE(test_fun)
{
	//GetAccountState1();
	CreateRegScriptTx2();
	GenerateMiner();
//	cout << "start first" << endl;
//	ListRegScript1();
	string temp=CreateFirstTx();
	BOOST_CHECK_EQUAL(Parsejson(temp) != "",true);
	GenerateMiner();
	//cout << "strat second:" << endl;
	temp=CreateSecondTx();
	BOOST_CHECK_EQUAL(Parsejson(temp) != "",true);
	GenerateMiner();

//	cout << "strat third:" << endl;
	temp=CreateThirdTx();
	BOOST_CHECK_EQUAL(Parsejson(temp) != "",true);
	GenerateMiner();

//	cout << "script reg:" << endl;
	CreateRegScriptTx3();
	GenerateMiner();

//	cout << "five reg:" << endl;
	temp=CreateFiveTx();
	BOOST_CHECK_EQUAL(Parsejson(temp) != "",true);
	GenerateMiner();

//	cout << "strat forth:" << endl;
//	CreateForthTx();
//	GenerateMiner(1);
}
BOOST_AUTO_TEST_SUITE_END()
