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

extern Object CallRPC(const string& strMethod, const Array& params);
extern int TestCallRPC(std::string strMethod, const std::vector<std::string> &vParams, std::string &strRet);
extern void GetAccountInfo(char *address);
extern void GenerateMiner();
extern time_t sleepTime;
extern int64_t llTime;
int GetRandomFee() {
		srand(time(NULL));
		int r = (rand() % 1000000) + 1000000;
		return r;
	}

	uint64_t GetPayMoney() {
		uint64_t r = 0;
		while(true)
		{
			srand(time(NULL));
			r = (rand() % 1000002) + 100000000;
			if(r%2 == 0 && r != 0)
				break;
		}

		return r;
	}
BOOST_AUTO_TEST_SUITE(test_app)
BOOST_AUTO_TEST_CASE(sendtx){
//	int64_t runTime = GetTime()+llTime;
//	vector<string> param;
//	bool flag = true;
//	string buyaddr,selleraddr;
//	GetAddress(buyaddr,selleraddr);
//	string nfee;
//	nfee = strprintf("%d",GetRandomFee());
//	string darkhash = CreateScript("D:\\bitcoin\\data\\darksecure.bin",buyaddr,nfee);
//	BOOST_CHECK(Parsejson(darkhash) != "");
//
//	string anonyhahs = CreateScript("D:\\bitcoin\\data\\anony.bin",buyaddr,nfee);
//	BOOST_CHECK(Parsejson(anonyhahs) != "");
//
//	GenerateMiner();
////	cout<<"regscript"<<endl;
////	cout<<darkhash<<endl;
////	cout<<anonyhahs<<endl;
//	string darkscriptkid,anonyscriptid;
//	darkhash = Parsejson(darkhash);
//	anonyhahs = Parsejson(anonyhahs);
////	cout<<darkhash<<endl;
////	cout<<anonyhahs<<endl;
//	while(true)
//	{
//		darkscriptkid = GetScript(darkhash);
//		anonyscriptid = GetScript(anonyhahs);
//		if(darkscriptkid != "" && anonyscriptid != "")
//			break;
//	}
//	while(GetTime()<runTime) {
//		SendDarkTx(darkscriptkid);
//		cout<<"Send Darck end"<<endl;
//		Sleep(sleepTime);
//		SendanonyTx(anonyscriptid);
//		cout<<"Send SendanonyTx end"<<endl;
//		Sleep(sleepTime);
//	}
}
//BOOST_AUTO_TEST_CASE(test_dark){
//	string path = "D:\\bitcoin\\data\\darksecure.bin";
//	BOOST_CHECK_MESSAGE(boost::filesystem::exists(path),path + " not exitst");
//	CreateScript((char*)path.c_str());
//	SetBlockGenerte("mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7");
//	string temp = CreateDarkTx();
//	SetBlockGenerte("n4muwAThwzWvuLUh74nL3KYwujhihke1Kb");
//	string temp1 = GetAccountInfo1("010000000100");
//	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),150);
////	GetAccountInfo("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
//	temp1 = GetAccountInfo1("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
//	BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999999999900);
//	temp1 = GetAccountInfo1("mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y");
//	BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999998999950);
//	CreateSecondDarkTx(temp);
//	SetBlockGenerte("mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7");
//	GetAccountInfo1("010000000100");
//	GetAccountInfo1("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
//	temp1 = GetAccountInfo1("mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y");
//	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),150);
//}
//
//BOOST_AUTO_TEST_CASE(test_anony){
//
//	string path = "D:\\bitcoin\\data\\anony.bin";
//	BOOST_CHECK_MESSAGE(boost::filesystem::exists(path),path + " not exitst");
//	CreateScript((char*)path.c_str());
//	SetBlockGenerte("mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7");
////	cout<<"1"<<endl;
//	Createanony("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
//	SetBlockGenerte("msdDQ1SXNmknrLuTDivmJiavu5J9VyX9fV");
//	string temp1 = GetAccountInfo1("010000000100");
//	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),100);
//	temp1 = GetAccountInfo1("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
//	BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999998999900);
//	temp1 = GetAccountInfo1("n4muwAThwzWvuLUh74nL3KYwujhihke1Kb");
////	cout<<temp1<<endl;
////	cout<<"2"<<endl;
//	Createanony("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
//	SetBlockGenerte("mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v");
//	temp1 = GetAccountInfo1("010000000100");
//	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),200);
//	temp1 = GetAccountInfo1("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
//	BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999997999800);
//	temp1 = GetAccountInfo1("n4muwAThwzWvuLUh74nL3KYwujhihke1Kb");
////	cout<<temp1<<endl;
////	cout<<"3"<<endl;
//	Createanony("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
//	SetBlockGenerte("mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc");
//	temp1 = GetAccountInfo1("010000000100");
//	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),100);
//	temp1 = GetAccountInfo1("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
//	BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999996999700);
//	temp1 = GetAccountInfo1("mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y");
//	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),100);
//	temp1 = GetAccountInfo1("n4muwAThwzWvuLUh74nL3KYwujhihke1Kb");
////	cout<<temp1<<endl;
//	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),100);
//}
BOOST_AUTO_TEST_SUITE_END()
