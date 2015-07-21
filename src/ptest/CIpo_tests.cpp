/*
 * CAnony_tests.cpp
 *
 *  Created on: 2015-04-24
 *      Author: frank
 */

#include "CIpo_tests.h"
#include "CycleTestManger.h"
#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_reader.h"
using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;

typedef struct user{
	unsigned char address[35];
	int64_t money;
	int64_t freemoney;
	int64_t freeMothmoney;
	user()
	{
		memset(address,0,35);
		money = 0;
		freemoney = 0;
		freeMothmoney = 0;
	}
	IMPLEMENT_SERIALIZE
	(
			for(int i = 0;i < 35;i++)
			READWRITE(address[i]);
			READWRITE(money);
			READWRITE(freemoney);
			READWRITE(freeMothmoney);
	)
}IPO_USER;

std::string ipo_data2[][2] = {
	{"1000000000","e1pzvqWNDezm3DNqoTZgVEWZ4avBgWe2c5"},
	{"1000000000","e1dEuSNrgiURqL61qEUKqtbrh4SS54ow39"},
	{"1000000000","dmwLekTvkdmjKKNsUJNmUGXq4KzUQvWNhM"},
	{"1000000000","du39jqpNvbxaUDjFbWNqqydGjtLtZiqByq"},
	{"1000000000","dhqJ5QXyRwmCSbw1Gj9jCQwSg2xPcrKZQr"},
	{"1000000000","doSsredaG4Levipfn5uGTVvmy9PeHMtGke"},
	{"1000000000","dkhz6w3AvD1K1Yxef6LaRFFCx6HcuCC2tj"},
	{"1000000000","duFcCMuXtELJVYzbqRXZ2RCbN3vBmytFe9"},
	{"1000000000", "dnyUwCZT1nhmQH3fa8GZig8LRmkBYT48m4"},
	{"1000000000","dePxY7knqTU21tKXYF5gLBJnmNMb8e9H2r"},
	{"1000000000", "dtB5EDtydo4Yvmaon5HpwjkBYfsV2tbTN2"},
	{"1000000000","dfuqbuXmsNWrY5cayKMb4qVKn8EZNMWfrf"},
	{"1000000000","dzVc4TdXJBdXLF9yTsEg7zVEKYTmQ16u3n"},
	{"1000000000","drdxMCY7mFyCrKw58VpwGadVC4TiL85jhG"},
	{"1000000000","df9ryp7TPchvyzKGAi3Nj87KYeUQfnHahc"},
	{"1000000000", "dq4PhbcFTqijXKc8SF5FpZR7vMTeXHASXG"},
	{"1000000000","dwygx8bexsat8Pqs8T17pc99khPvMLMJjj"},
	{"1000000000", "dk2NNjraSvquD9b4SQbysVRQeFikA55HLi"},
	{"1000000000","do22zke78bz3F49MAGhQk8jFUmXqBfpbip"},
	{"1000000000", "dg7gANKTD6sCnpopPcbiU51m15WQmihiua"},
	{"1000000000", "ds31UhuB3a5KmhNcLBFrwtJpvSpKchHsmm"},
	{"1000000000", "djZU3bKwggaAJYuwJdkfDZjV7y5to22x3z"},
	{"1000000000", "duqKKByNNGSEzfJTJjP3hv8SEAgBikRLyD"},
	{"1000000000", "dkqLckzJaTqgT1KJ5hVBRG8ZB1TBPanb49"},
	{"1000000000", "djW171pyq1e9odLB1pPRJEJEDohRk6LyEn"},
	{"1000000000", "dkTwTAHEPNPowTvna88ZKoffJH3eR2vzQz"},
	{"1000000000", "de3qRpoP7jogfPa1WRpFXNESJjNWoMgHHu"}
};

#define max_user 100


static IPO_USER userarray[max_user];
CIpoTest::CIpoTest():nNum(0), nStep(0), strTxHash(""), strAppRegId("") {

}

TEST_STATE CIpoTest::Run(){

//	int addrcount = 0;
//    ifstream file;
//    string strCurDir ="/home/share/bess/dacrs_test/ipo.txt";
//	file.open(strCurDir, ios::in | ios::ate);
//	if (!file.is_open())
//		throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open wallet dump file");
//
//	file.seekg(0, file.beg);
//	if (file.good()){
//		Value reply;
//		json_spirit::read(file,reply);
//		const Array & keyarry = reply.get_array();
//		for(auto const &keyItem :keyarry)
//		{
//			string addr = find_value(keyItem.get_obj(), "addr").get_str();
//			memcpy((char*)userarray[addrcount].address,(char*)addr.c_str(),sizeof(userarray[addrcount].address));
//			userarray[addrcount].money  = find_value(keyItem.get_obj(), "money").get_int64();
//			userarray[addrcount].freemoney = find_value(keyItem.get_obj(), "freemoney").get_int64();
//			userarray[addrcount].freeMothmoney = find_value(keyItem.get_obj(), "freeMothmoney").get_int64();
//			addrcount++;
//			if(addrcount == (max_user -1))
//				break;
//		}
//	}
//	file.close();


	for (int i = 0; i < max_user; i++) {
		string newaddr;
		BOOST_CHECK(basetest.GetNewAddr(newaddr, true));
		memcpy((char*)userarray[i].address,(char*)newaddr.c_str(),sizeof(userarray[i].address));
		userarray[i].money = 10000;
		userarray[i].freemoney = 200;
		userarray[i].freeMothmoney = 22;
	}

    // 注册ipo脚本
	RegistScript();

	/// 等待ipo脚本被确认到block中
	while(true)
	{
		if(WaitComfirmed(strTxHash, strAppRegId)) {
					break;
				}
	}
	/// 给每个地址转一定的金额
	int64_t money = COIN;
	for(int i=0;i <max_user;i++)
	{
		string des =strprintf("%s", userarray[i].address);
		basetest.CreateNormalTx(des,money);
	}

	 cout<<"end mempool"<<endl;
	while(true)
	{
		if(basetest.IsMemoryPoolEmpty())
			break;
		MilliSleep(100);
	}

   cout<<"SendIpoTx start"<<endl;
	SendIpoTx();
	 cout<<"SendIpoTx end"<<endl;
}

bool CIpoTest::RegistScript(){

	const char* pKey[] = { "cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU",
			"cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU"};
	int nCount = sizeof(pKey) / sizeof(char*);
	basetest.ImportWalletKey(pKey, nCount);

	string strFileName("IpoApp.bin");
	int nFee = basetest.GetRandomFee();
	int nCurHight;
	basetest.GetBlockHeight(nCurHight);
	string regAddr="dk2NNjraSvquD9b4SQbysVRQeFikA55HLi";

	//reg anony app
	Value regscript = basetest.RegisterAppTx(regAddr, strFileName, nCurHight, nFee+20*COIN);
	if(basetest.GetHashFromCreatedTx(regscript, strTxHash)){
		return true;
	}
	return false;
}

bool CIpoTest::CreateIpoTx(string contact,int64_t llSendTotal){
	int pre =0xff;
	int type = 2;
	string buffer =strprintf("%02x%02x", pre,type);

	buffer += contact;

	Value  retValue = basetest.CreateContractTx(strAppRegId, SEND_A, buffer, 0, 10*COIN, llSendTotal);
	if(basetest.GetHashFromCreatedTx(retValue, strTxHash)){
			return true;
	}
	return false;
}
bool CIpoTest::SendIpoTx()
{
	for(int i =0;i <max_user;i++)
	{
		CDataStream scriptData(SER_DISK, CLIENT_VERSION);
		scriptData << userarray[i];
		string sendcontract = HexStr(scriptData);
		CreateIpoTx(sendcontract,userarray[i].money);
	}
	return true;
}
BOOST_FIXTURE_TEST_SUITE(CreateIpoTxTest,CIpoTest)

BOOST_FIXTURE_TEST_CASE(Test,CIpoTest)
{
//	while(true)
//	{
//		string newaddr;
//		BOOST_CHECK(basetest.GetNewAddr(newaddr, true));
//		cout<<"len:"<<newaddr.length()<<endl;
//		if(newaddr.length() != 34)
//		{
//			cout<<"address:"<<newaddr.c_str()<<endl;
//			break;
//		}
//	}
	Run();
}

typedef struct _IPOCON{
	unsigned char address[35];
	int64_t money;
}IPO_COIN;
#define max_2ipouser 100

BOOST_FIXTURE_TEST_CASE(get_coin,CIpoTest)
{

	// 创建转账交易并且保存转账交易的hash
	Object objRet;
	Array SucceedArray;
	Array UnSucceedArray;
	ofstream file("ipo_ret", ios::out | ios::ate);
	if (!file.is_open())
		throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open wallet dump file");

	map<string, string> mapTxHash;
	for (size_t i = 0; i < sizeof(ipo_data2) / sizeof(ipo_data2[0]); ++i) {
		string des = strprintf("%s", ipo_data2[i][1]);
		int64_t nMoney = atoi64(ipo_data2[i][0]);
		Value ret = basetest.CreateNormalTx(des, nMoney);
		string txHash;
		Object obj;
		if(basetest.GetHashFromCreatedTx(ret, txHash)) {
			mapTxHash[des]= txHash;
			obj.push_back(Pair("addr", des));
			obj.push_back(Pair("amount", nMoney));
			obj.push_back(Pair("txhash", txHash));
			SucceedArray.push_back(obj);
		} else {
			obj.push_back(Pair("addr", des));
			obj.push_back(Pair("amount", nMoney));
			UnSucceedArray.push_back(obj);
		}
	}
	objRet.push_back(Pair("succeed", SucceedArray));
	objRet.push_back(Pair("unsucceed", UnSucceedArray));
	file << json_spirit::write_string(Value(objRet), true).c_str();
	file.close();

	//确保每个转账交易被确认在block中才退出
	while(mapTxHash.size() != 0)
	{
		map<string, string>::iterator it = mapTxHash.begin();
		for(;it != mapTxHash.end();){
			string addr = it->first;
			string hash = it->second;
			string regindex = "";
			if(basetest.GetTxConfirmedRegID(hash,regindex)){
				it = mapTxHash.erase(it);
			}else{
				it++;
			}
		}
		MilliSleep(100);
	}
}
BOOST_AUTO_TEST_SUITE_END()

