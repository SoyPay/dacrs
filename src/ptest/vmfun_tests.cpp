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
void CreateRegScriptTx1() {
	cout <<"CreateRegScriptTx1" << endl;
	int argc = 6;
	char *argv[6] =
			{ "rpctest", "registerscripttx", "5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG",
					"fd6208020053000000000022220000000000000000222202081ee4900f2f784179018002f0a3d8fcd9fa7a007b0f900062783079018015e493a3ad82ae838a828b83f0a3aa82ab838d828e83d8e9d9e712069b12085e75d0007581bf750c00750d0f020017357a51506343315970464d7477786948373837705358616e5545436f4773785571334b5a69654a7856470001000000e0faa3e0fba3e0fca3e0fd22cac0e0e6f0a308dafad0e0fa22cac0e0e0a3c582ccc582c583cdc583f0a3c582ccc582c583cdc583dae6d0e0ca22250c10af08f50c400c150d8008f50c4002150dd2af2200c0d0250c10af08f50c500c050d8008f50c5002050dd2afd0d022250cf58210af08f50c400c150d8008f50c4002150dd2af850d8322c582250cc582c583350d10af07f50d85820c8007f50d85820cd2afc58322250cf582e4350df58322250cfae4350dfb22250cfce4350dfd22c8250cc8c9350dc922ca250ccacb350dcb22cc250ccccd350dcd22250cc582c0e0e50d34ffc583c0e0e50cc3958224f910af0885830d85820c800885830d85820cd2afcef0a3e520f0a37808e608f0a3defaeff0a3e58124fbf8e608f0a3e608f0a30808e608f0a3e608f0a315811581d0e0fed0e0f815811581e8c0e0eec0e022850d83850c82e0a3fee0a3f5207808e0a3f608dffae0a3ffe0a3c0e0e0a3c0e0e0a3c0e0e0a3c0e010af0885820c85830d800885820c85830dd2afd083d0822274028000c0e0f4041200fcd0e012009d2274028000ccc0e0edc0e0e50cc39cccad0d50011d10af068c0c8d0d80068c0c8d0dd2af1200aad0e0fdd0e0fc2274f812016ae990fbfef01200087f010201d0c082c083ea90fbfef0eba3f0120012020359ea2cf8eb3df9c3e89ae99b4007c3e89ce99d5004d2f08002c2f0a2f02274f512016a74fc1200cb850c82850d83ecf0a3edf08a088b0990f7fee0fea3e0ff850c82850d83e02ef8a3e03ff9e82402f85001097402120135e8f0a3e9f0850c82850d83e02402fca3e03400fdeefaeffb12027240127c027d00850c82850d8312035e1202725005790012024e750a02750b00780a12021074041201477afe7bf71207b774021200e174fe2efe74f73fff780a1202107402120147ee2402fae43ffb1207b774021200e1850c82850d83120221ac08ad09ee2404fae43ffb1207b774021200e1020364d083d08222e0faa3e0fb2274041200e17f040201d0740102036674f512016a74ff1200cbe91203b31202107c007d007afe7bf71207f474021200e17c017d00aa0cab0d12028feefceffdaa08ab0912028f7a097b0012026080bb850c82850d83f08a088b09ecfeedff750a00750b02780a2274f512016aeafeebff8c088d09750a00750b02780a1202107c007d007afe7bf71207f474021200e17c207d00eefaeffb12028f7c027d00740b12013f12028f7a0a7b00120260740b120135c082c083120550d083d0821205494004e8497003c3801790f7fe1202217c007df8aa08ab091207b774021200e1d302036974f512016aeafeebff8c088d09750a00750b02780a1202107c007d007afe7bf71207f474021200e17c207d00eefaeffb12028f7c027d00740b12013f12028f7a0b7b00120260740b120135c082c083120550d083d0821205494004e8497003c3801790f7fe1202217c007df8aa08ab091207b774021200e1d302036974f512016aeafeebff8c088d09750a00750b02780a1202107c007d007afe7bf71207f474021200e1eefaeffb1208421205be12028f7a0c7b00120260740b120135c082c083120550d083d082120549401890f7fee064417002a3e0600f90f7fee064217002a3e06003c3801790f7fe1202217c007df8aa08ab091207b774021200e1d3020369c3e098a3e0992290f7fee0f8a3e0f92274f512016aeafeebff8c088d09750a00750b02780a1202107c007d007afe7bf71207f474021200e1eefaeffb1208421205be12028f7a0d7b0012026090f7fee064087002a3e06003c3801790f7fe1202217c007df8aa08ab091207b774021200e1d30203698b0bea2401fce4350bfdeefaeffb2274041200e17f020201d074f712016a74fc1200cb1206421202107c007d007afe7bf71207f474021200e17c047d00aa0cab0d12028f7a107b00120260740f120135c082c083120550d083d0821205494004e8497003c3801b90f7fe1202217c007df8740f12013512035e1207b774021200e1d3808b850c82850d83eaf0a3ebf0a3ecf0a3edf075080075090278082274f712016a8a088b09ecfeedff7a197b00120260120550c3ee98ef994004e8497003c3801790f7fe1202217c007df8aa08ab091207b774021200e1d30205d27582b87583fd120117900f2f7c287d0212016174201200aa900f4f740812014774201200aa7c20fd740812013f12065c79017c207d00740812013f1203737508207509007808120210782a790212014f880889097808120210900f2b1200911205d774041200e179017c207d007a287b021201581203737508007509027808120210742a120147740a12013f1203cb74021200e179017c007d02742812013f1203737808120210742a120147740a12013f12044774021200e179017c007d02742812013f1203737508417509007808120210742a1201477a007b0f1204c374021200e179017c007d02742812013f120373ac0cad0d7a007b0f12055979017c087d00aa0cab0d120373790112024e7a007b007582487583021201172274f812016aeaf8ebf97408120135e0fea3e0801f8c828d83e088828983f0a3a882a9838c828d83a3ac82ad83ee24ff1eef34ffffee4f70dc7f010201d0c082c083850c82850d83e0f8a3e0f9e84960128a828b83ecf0a3e824ff18e934fff94870f2d083d0822274f812016a74fe1200cbeafeebff850c82850d83eef0a3eff0aa0cab0d790112086180eac082c0838a828b838001a3e070fce582c39afae5839bfbd083d082220200142200",
					"1000000", "2" };
	CommandLineRPC(argc, argv);
}
bool CreateTx()
{
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\"]");
	vInputParams.push_back("0b434b430046003531303030303030300000000000000000000000");
	vInputParams.push_back("100000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	if (TestCallRPC("createcontracttx", vInputParams, strReturn) > 0) {
		vInputParams.clear();
		cout << "create secure tx succeed1:"<<strReturn<< endl;
		TxHash = strReturn;
	}
	return false;
}
void ListRegScript1() {
	//cout << "listRegScript" << endl;
	int argc = 2;
	char *argv[2] = { "rpctest", "listregscript" };
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
	vInputParams.push_back("100000000");
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
	vInputParams.push_back("02");
	vInputParams.push_back("100000000");
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

BOOST_AUTO_TEST_CASE(Gloal_fun)
{
	//cout << "=====================init account info ========================" << endl;
	GetAccountState1();
	CreateRegScriptTx1();
	GenerateMiner();
	cout << "=====================create tx 1========================" << endl;
	ListRegScript1();
	CreateTx();
	GenerateMiner();
//	GetAccountState1();
}

BOOST_AUTO_TEST_CASE(test_fun)
{
	GetAccountState1();
	CreateRegScriptTx1();
	GenerateMiner();
	cout << "=====================create tx 1========================" << endl;
	ListRegScript1();
	CreateFirstTx();
	GenerateMiner(10);

	CreateSecondTx();
//	GetAccountState1();
}
BOOST_AUTO_TEST_SUITE_END()
