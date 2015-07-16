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


#define max_user 100
//const static IPO_USER userarray[max_user]=
//{
//		{"ddMuEBkAwhcb5K5QJ83MqQHrgHRn4EbRdh",10000,200,22},
//		{"duKfNyq6zsuy2CMGMFVrQLuGi95UC7w6DV",10000,200,22},
//		{"djhuAYvWsfFyjF42qDqTSm88nkfZbDW1BZ",10000,200,22}                   /// 这个没有注册的账户必须先打点钱
//};

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

	if(boost::unit_test::framework::master_test_suite().argc != 1){
		BOOST_ERROR("请传入参数ipo.txt路径");
	}
	string strCurDir = boost::unit_test::framework::master_test_suite().argv[1];


	//// 读json语句
	IPO_COIN ipouserarray[max_2ipouser];
	int addrcount = 0;
    ifstream file;
   // string strCurDir ="/home/share/bess/dacrs_test/ipo.txt";
	file.open(strCurDir, ios::in | ios::ate);
	if (!file.is_open())
		throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot ipo.txt file");

	file.seekg(0, file.beg);
	if (file.good()){
		Value reply;
		json_spirit::read(file,reply);
		const Array & keyarry = reply.get_array();
		for(auto const &keyItem :keyarry)
		{
			string addr = find_value(keyItem.get_obj(), "addr").get_str();
			memcpy((char*)ipouserarray[addrcount].address,(char*)addr.c_str(),sizeof(ipouserarray[addrcount].address));
			int64_t balance = find_value(keyItem.get_obj(), "balance").get_int64();
			ipouserarray[addrcount].money = balance;
			addrcount++;
			if(addrcount == (max_2ipouser -1))
				break;
		}
	}
	file.close();


	//////// 创建转账交易并且保存转账交易的hash
	int64_t money = COIN;
	std::map<std::string,std::string> maptx;
	for(int i=0;i <(addrcount+1);i++)
	{
		string des =strprintf("%s", ipouserarray[i].address);
		Value ret = basetest.CreateNormalTx(des,money);
		string hash = "";
		if(basetest.GetHashFromCreatedTx(ret, hash)){
				maptx[des]= hash;
			}else{
				cout<<des.c_str()<<endl;
			}
	}

	//////// 确保每个转账交易被确认在block中才退出
	while(maptx.size() != 0)
	{
		std::map<std::string,std::string>::iterator it = maptx.begin();
		for(;it != maptx.end();){
			string addr = it->first;
			string hash = it->second;
			string regindex = "";
			if(basetest.GetTxConfirmedRegID(hash,regindex)){
				it = maptx.erase(it);
			}else{
				it++;
			}
		}

		MilliSleep(100);
	}
}
BOOST_AUTO_TEST_SUITE_END()

