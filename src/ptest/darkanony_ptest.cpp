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
void CreateScript(char * vmpath)
{
	int argc = 7;
	char *argv[7] =
			{ "rpctest", "registerscripttx", "mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA","0",
					vmpath,
					"1000000", "2" };
	CommandLineRPC(argc, argv);
}
#pragma pack(1)
typedef struct  {
	unsigned char dnType;					//!<类型
	char buyer[6];						//!<买家ID（采用6字节的账户ID）
	char seller[6];						//!<卖家ID（采用6字节的账户ID）
	int nHeight;							//!<超时绝对高度
	uint64_t nPayMoney;						//!<买家向卖家支付的金额
	IMPLEMENT_SERIALIZE
	(
			READWRITE(dnType);
			for(int i = 0;i < 6;i++)
			READWRITE(buyer[i]);
			for(int i = 0;i < 6;i++)
			READWRITE(seller[i]);
			READWRITE(nHeight);
			READWRITE(nPayMoney);
	)
} FIRST_CONTRACT;

typedef struct {
	unsigned char dnType;				//!<交易类型
	char hash[32];		//!<上一个交易包的哈希
	IMPLEMENT_SERIALIZE
	(
			READWRITE(nType);
	for(int i = 0;i < 32;i++)
			READWRITE(hash[i]);
	)
} NEXT_CONTRACT;
string Parsejson(string str)
{
	json_spirit::Value val;
	json_spirit::read(str, val);
//	json_spirit::json ::
	if (val.type() != obj_type)
	{
		return "";
	}
	json_spirit::Value::Object obj=  val.get_obj();
	string ret;
	for(int i = 0; i < obj.size(); ++i)
	{
		const json_spirit::Pair& pair = obj[i];
		const std::string& str_name = pair.name_;
		const json_spirit::Value& val_val = pair.value_;
		if(str_name =="RawTx")
		{
			if(val_val.get_str() != "")
			{
				ret = val_val.get_str();
			}
		}else if(str_name =="hash")
		{
			ret = val_val.get_str();
		}

	}
	return ret;
}
string CreateDarkTx()
{
	string accountid = "010000000100";
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5\",\"mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y\"]");

	FIRST_CONTRACT contact;
	contact.dnType = 0x01;
	contact.nHeight = 10;
	contact.nPayMoney = 100;
	CRegID regIdb("0-5");
	CRegID regIds("0-3");
	memcpy(contact.buyer,&regIdb.GetVec6().at(0),sizeof(contact.buyer));
	memcpy(contact.seller,&regIds.GetVec6().at(0),sizeof(contact.seller));

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << contact;
	string temp = HexStr(scriptData);
//	cout<<"first:"<<temp<<endl;
	vInputParams.push_back(temp);
	vInputParams.push_back("1000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	if(TestCallRPC("createcontracttx", vInputParams, strReturn)){
			strReturn= Parsejson(strReturn);
	//		cout<<strReturn<<endl;
			vInputParams.clear();
			vInputParams.push_back(strReturn);
			if (TestCallRPC("signcontracttx", vInputParams, strReturn) > 0) {
				strReturn= Parsejson(strReturn);
			}
		}
//	cout <<strReturn << endl;
	return strReturn;
}
void CreateSecondDarkTx(string hash)
{
	string accountid = "010000000100";
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5\"]");

	NEXT_CONTRACT contact;
	contact.dnType = 0x02;
	uint256 tx(hash.c_str());
//	cout<<"hash:"<<hash<<endl;
	//contact.hash = tx;
	memcpy(contact.hash,tx.begin(),sizeof(contact.hash));
	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << contact;
	string temp = HexStr(scriptData);
	uint256 hash1(hash.c_str());
	string param ="02";
	param += HexStr(hash1);
//	cout<<"second:"<<param<<endl;
	vInputParams.push_back(param);
	vInputParams.push_back("1000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	TestCallRPC("createcontracttx", vInputParams, strReturn);
//	cout<<strReturn<<endl;
	return ;
}

typedef struct  {
	char 	Sender[6];						//!<转账人ID（采用6字节的账户ID）
	int nHeight;							//!<超时绝对高度
	uint64_t nPayMoney;						//!<转账的人支付的金额
	unsigned short len;                     //!<接受钱账户信息长度
	IMPLEMENT_SERIALIZE
	(
			for(int i = 0;i < 6;i++)
			READWRITE(Sender[i]);
			READWRITE(nHeight);
			READWRITE(nPayMoney);
			READWRITE(len);
	)
}CONTRACT_ANONY;

typedef struct  {
	char 	account[6];						//!<接受钱的ID（采用6字节的账户ID）
	uint64_t nReciMoney;						    //!<	收到钱的金额
	IMPLEMENT_SERIALIZE
	(
			for(int i = 0;i < 6;i++)
			READWRITE(account[i]);
			READWRITE(nReciMoney);

	)
} ACCOUNT_INFO;

void Createanony(string addr)
{
	string accountid = "010000000100";
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	string temp1 = "[";
	temp1+="\""+addr+"\""+"]";
//	cout<<temp1<<endl;
	vInputParams.push_back(temp1);

	string temp = "";
	CONTRACT_ANONY contact;
	contact.nHeight = 10;
	contact.nPayMoney = 100;


	CRegID regIdb("0-5");
	memcpy(contact.Sender,&regIdb.GetVec6().at(0),sizeof(contact.Sender));

	ACCOUNT_INFO info;
	info.nReciMoney = 50;
	CRegID regId1("0-3");
	memcpy(info.account,&regId1.GetVec6().at(0),sizeof(info.account));

	ACCOUNT_INFO info1;
	info1.nReciMoney = 50;
	CRegID regId2("0-8");
	memcpy(info1.account,&regId2.GetVec6().at(0),sizeof(info1.account));

	contact.len = ::GetSerializeSize(info, SER_DISK, CLIENT_VERSION)*2;
//	cout<<contact.len<<endl;
	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << contact;
	scriptData << info;
	scriptData << info1;
	temp = HexStr(scriptData);
//	cout<<"cotx:"<<temp<<endl;
	vInputParams.push_back(temp);
	vInputParams.push_back("1000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	TestCallRPC("createcontracttx", vInputParams, strReturn);
//	cout<<strReturn<<endl;
	return ;
}
uint64_t GetValue(string str,string compare)
{
	json_spirit::Value val;
	json_spirit::read(str, val);
//	json_spirit::json ::
	if (val.type() != obj_type)
	{
		return 0;
	}
	json_spirit::Value::Object obj=  val.get_obj();
	string ret;
	for(int i = 0; i < obj.size(); ++i)
	{
		const json_spirit::Pair& pair = obj[i];
		const std::string& str_name = pair.name_;
		const json_spirit::Value& val_val = pair.value_;

		if(str_name.compare(compare) == 0)
		{
			return val_val.get_int64();
		}

		if(compare == "value")
		{
			if(str_name =="FreedomFund")
				{
					json_spirit::Value::Array narray = val_val.get_array();
					if(narray.size() == 0)
						return false;
					json_spirit::Value::Object obj1 = narray[0].get_obj();
					for(int j = 0; j < obj1.size(); ++j)
					{
						const json_spirit::Pair& pair = obj1[j];
						const std::string& str_name = pair.name_;
						const json_spirit::Value& val_val = pair.value_;
						if(str_name == "value")
						{
							return val_val.get_int64();
						}
					}

				}
		}



	}
	return 0;
}
string GetAccountInfo1(string address) {
	//cout << "Get Address " << address << "INFO" << endl;
//	int argc = 3;
//	char *argv[3] = { "rpctest", "getaccountinfo", address };
//	CommandLineRPC(argc, argv);
	std::vector<std::string> vInputParams;
	vInputParams.push_back(address);
	std::string strReturn("");
	TestCallRPC("getaccountinfo", vInputParams, strReturn);
	return strReturn;

}
void SetBlockGenerte(string address)
{
	std::vector<std::string> vInputParams;
	vInputParams.push_back(address);
	string strReturn;
	TestCallRPC("generateblock", vInputParams, strReturn);
}
BOOST_AUTO_TEST_SUITE(test_app)

BOOST_AUTO_TEST_CASE(test_dark){
	string path = "D:\\bitcoin\\data\\darksecure.bin";
	BOOST_CHECK_MESSAGE(boost::filesystem::exists(path),path + " not exitst");
	CreateScript((char*)path.c_str());
	SetBlockGenerte("mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7");
	string temp = CreateDarkTx();
	SetBlockGenerte("n4muwAThwzWvuLUh74nL3KYwujhihke1Kb");
	string temp1 = GetAccountInfo1("010000000100");
	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),150);
//	GetAccountInfo("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
	temp1 = GetAccountInfo1("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
	BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999999999900);
	temp1 = GetAccountInfo1("mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y");
	BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999998999950);
	CreateSecondDarkTx(temp);
	SetBlockGenerte("mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7");
	GetAccountInfo1("010000000100");
	GetAccountInfo1("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
	temp1 = GetAccountInfo1("mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y");
	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),150);
}

BOOST_AUTO_TEST_CASE(test_anony){

	string path = "D:\\bitcoin\\data\\anony.bin";
	BOOST_CHECK_MESSAGE(boost::filesystem::exists(path),path + " not exitst");
	CreateScript((char*)path.c_str());
	SetBlockGenerte("mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7");
//	cout<<"1"<<endl;
	Createanony("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
	SetBlockGenerte("msdDQ1SXNmknrLuTDivmJiavu5J9VyX9fV");
	string temp1 = GetAccountInfo1("010000000100");
	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),100);
	temp1 = GetAccountInfo1("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
	BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999998999900);
	temp1 = GetAccountInfo1("n4muwAThwzWvuLUh74nL3KYwujhihke1Kb");
//	cout<<temp1<<endl;
//	cout<<"2"<<endl;
	Createanony("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
	SetBlockGenerte("mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v");
	temp1 = GetAccountInfo1("010000000100");
	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),200);
	temp1 = GetAccountInfo1("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
	BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999997999800);
	temp1 = GetAccountInfo1("n4muwAThwzWvuLUh74nL3KYwujhihke1Kb");
//	cout<<temp1<<endl;
//	cout<<"3"<<endl;
	Createanony("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
	SetBlockGenerte("mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc");
	temp1 = GetAccountInfo1("010000000100");
	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),100);
	temp1 = GetAccountInfo1("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
	BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999996999700);
	temp1 = GetAccountInfo1("mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y");
	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),100);
	temp1 = GetAccountInfo1("n4muwAThwzWvuLUh74nL3KYwujhihke1Kb");
//	cout<<temp1<<endl;
	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),100);
}
BOOST_AUTO_TEST_SUITE_END()
