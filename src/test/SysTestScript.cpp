#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "init.h"
#include "miner.h"
#include "uint256.h"
#include "util.h"
#include <boost/test/unit_test.hpp>
#include "rpcclient.h"
#include "tx.h"
#include "wallet.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "./VmScript/VmScript.h"
#include "rpcserver.h"
#include "noui.h"
#include "ui_interface.h"
#include "SysTestBase.h"
#include <boost/algorithm/string/predicate.hpp>
#include "json/json_spirit_writer_template.h"
#include "rpcclient.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_reader.h"
#include "json/json_spirit_writer.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_stream_reader.h"

using namespace std;
using namespace boost;
std::string TxHash("");

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
class CSysScriptTest:public SysTestBase
{
public:
	CSysScriptTest() {
		StartServer();
	}

	~CSysScriptTest(){
		StopServer();
	}
private:
	void StartServer() {

	}

	void StopServer() {
	}
public:
	uint64_t GetValue(Value val,string compare)
	{
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
	bool GetHashFromCreatedTx(const Value& valueRes,string& strHash)
	{
		if (valueRes.type() == null_type) {
			return false;
		}

		const Value& result = find_value(valueRes.get_obj(), "hash");
		const Value& result1 = find_value(valueRes.get_obj(), "RawTx");

		if (result.type() == null_type && result1.type() == null_type){
			return false;
		}
		if (result.type() != null_type){
			strHash = result.get_str();
			}
		else if(result1.type() != null_type)
		{
			strHash = result1.get_str();
		}

		return true;
	}

	void CheckSdk()
	{
		int nHeight = 0;
		string param ="01";
		Value resut =CreateContractTx1("010000000100", "[\"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb\"]", param,10);
		BOOST_CHECK(GetHashFromCreatedTx(resut,TxHash));
		BOOST_CHECK(GenerateOneBlock());
		uint256 hash(TxHash.c_str());
		param ="02";
		param += HexStr(hash);
		string temp;
		resut =CreateContractTx1("010000000100", "[\"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb\"]", param,10);
		BOOST_CHECK(GetHashFromCreatedTx(resut,temp));
		BOOST_CHECK(GenerateOneBlock());

		param ="03";
		resut =CreateContractTx1("010000000100", "[\"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb\"]", param,10);
		BOOST_CHECK(GetHashFromCreatedTx(resut,temp));
		BOOST_CHECK(GenerateOneBlock());

		param ="05";
		param += HexStr(hash);

		resut =CreateContractTx1("010000000100", "[\"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb\"]", param,10);
		BOOST_CHECK(GetHashFromCreatedTx(resut,temp));
		BOOST_CHECK(GenerateOneBlock());
	}

	void CreateRegScript(char*strAddr,char*sourceCode)
	{
		int nFee = 10000000;
		string strTxHash;
		string strFileName(sourceCode);
		Value valueRes = RegisterScriptTx(strAddr,strFileName , 100, nFee);
		BOOST_CHECK(GetHashFromCreatedTx(valueRes,strTxHash));
		BOOST_CHECK(GenerateOneBlock());
	}
	void CreateContactTx(int param)
	{
		char buffer[3] = {0};
		sprintf(buffer,"%02x",param);
		string temp;
		Value resut =CreateContractTx1("010000000100", "[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\"]", buffer,10);
		BOOST_CHECK(GetHashFromCreatedTx(resut,temp));
		BOOST_CHECK(GenerateOneBlock());
		return ;
	}
	void disblock1()
	{
		int argc = 3;
		char *argv[3] = { "rpctest", "disconnectblock", "1" };
	//	sprintf(argv[2], "%d", number);
		CommandLineRPC(argc, argv);
	}

	void CheckRollBack()
	{
		CreateContactTx(6);    //新增脚本数据
		//cout<<6<<endl;
		CreateContactTx(7);;   //修改脚本数据
		//cout<<7<<endl;
		CreateContactTx(8);    //删除脚本数据
	//	cout<<8<<endl;
//		disblock1();           //删除1个block
//		mempool.mapTx.clear();
//		CreateContactTx(9);    //check删除的脚本是否恢复
//	//	cout<<9<<endl;
//		disblock1();
//		disblock1();
//		mempool.mapTx.clear();
//		CreateContactTx(10);    //check修改的脚本数据是否恢复
////		cout<<10<<endl;
//		disblock1();
//		disblock1();
//		mempool.mapTx.clear();
//		CreateContactTx(11);   //check新增的脚本数据是否恢复
	}
	bool CheckScriptid(Value val,string scriptid)
	{
		if (val.type() != obj_type)
		{
			return false;
		}
		const Value& value = val.get_obj();

		json_spirit::Value::Object obj= value.get_obj();
		for(int i = 0; i < obj.size(); ++i)
		{
			const json_spirit::Pair& pair = obj[i];
			const std::string& str_name = pair.name_;
			const json_spirit::Value& val_val = pair.value_;
			if(str_name =="PublicKey")
			{
				if(val_val.get_str() != "")
				{
					return false;
				}
			}
			else if(str_name =="KeyID")
			{
				CRegID regId(scriptid);
				CKeyID keyId = Hash160(regId.GetVec6());
				string key = HexStr(keyId.begin(), keyId.end()).c_str();
				if(val_val.get_str() != key)
				{
					return false;
				}
			}

		}
		return true;
	}
	bool CreateScriptAndCheck()
	{
		CreateRegScript("mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA","soypay_test.bin");

		CreateRegScript("mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA","scripttest.bin");

		string strRegID = "010000000100";

		Value temp1 = GetAccountInfo(strRegID);
		if(!CheckScriptid(temp1,strRegID))
		{
			return false;
		}

		strRegID = "020000000100";
		temp1 = GetAccountInfo(strRegID);
		if(!CheckScriptid(temp1,strRegID))
		{
			return false;
		}

	}

	void CreateOperateSelfScriptTx(int param)
	{
		string accountid = "010000000100";
		string temp = "";
		temp += tinyformat::format("%02x%s",param,accountid);
		Value resut =CreateContractTx1("010000000100", "[\"mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5\"]", temp,10);
		BOOST_CHECK(GetHashFromCreatedTx(resut,temp));
		return ;
	}

	Value CreateOperateOtherScriptTx(int param)
	{
		string accountid = "010000000100";
		string temp = "";
		temp += tinyformat::format("%02x%s",param,accountid);
		Value resut =CreateContractTx1("020000000100", "[\"mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5\"]", temp,10);
		return resut;
	}
	bool SetBlockGenerte(char *addr)
	{
		char *argv[] = { "rpctest", "generateblock", addr };
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		if (CommandLineRPC_GetValue(argc, argv, value)) {
			return true;
		}
		return false;
	}
	void CheckScriptAccount()
	{
		BOOST_CHECK_EQUAL(CreateScriptAndCheck(),true);

		CreateOperateSelfScriptTx(13);
		BOOST_CHECK(SetBlockGenerte("mjSwCwMsvtKczMfta1tvr78z2FTsZA1JKw"));
		Value temp1 = GetAccountInfo("010000000100");
		BOOST_CHECK_EQUAL(GetValue(temp1,"value"),10000);
		temp1 = GetAccountInfo("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
		BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999998990000);

		CreateOperateSelfScriptTx(14);
		BOOST_CHECK(SetBlockGenerte("msdDQ1SXNmknrLuTDivmJiavu5J9VyX9fV"));
		temp1 = GetAccountInfo("010000000100");
		BOOST_CHECK_EQUAL(GetValue(temp1,"value"),0);
		temp1 = GetAccountInfo("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
		BOOST_CHECK_EQUAL(GetValue(temp1,"value"),10000);

		string temp;
		Value ret=CreateOperateOtherScriptTx(1);
		BOOST_CHECK(SetBlockGenerte("mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v"));
		BOOST_CHECK(GetHashFromCreatedTx(ret,temp));
		temp1 = GetAccountInfo("010000000100");
		BOOST_CHECK_EQUAL(GetValue(temp1,"value"),10000);
		temp1 = GetAccountInfo("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
		BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999996990000);

		ret = CreateOperateOtherScriptTx(2);
		BOOST_CHECK(GenerateOneBlock());
		BOOST_CHECK(!GetHashFromCreatedTx(ret,temp));
	}

	string CreateDarkTx()
	{
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
		Value resut =CreateContractTx1("010000000100", "[\"mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5\",\"mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y\"]", temp,10);
		string strReturn;
		BOOST_CHECK(GetHashFromCreatedTx(resut,strReturn));
		resut =SignSecureTx(strReturn);
		BOOST_CHECK(GetHashFromCreatedTx(resut,strReturn));
		return strReturn;
	}
	void CreateSecondDarkTx(string hash)
	{

		uint256 hash1(hash.c_str());
		string param ="02";
		param += HexStr(hash1);

		Value resut =CreateContractTx1("010000000100", "[\"mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5\"]", param,10);
		string strReturn;
		BOOST_CHECK(GetHashFromCreatedTx(resut,strReturn));
		return ;
	}
	void Createanony(string addr)
	{
		string temp1 = "[";
		temp1+="\""+addr+"\""+"]";

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
		CDataStream scriptData(SER_DISK, CLIENT_VERSION);
		scriptData << contact;
		scriptData << info;
		scriptData << info1;
		temp = HexStr(scriptData);

		Value resut =CreateContractTx1("010000000100", temp1, temp,10);
		string strReturn;
		BOOST_CHECK(GetHashFromCreatedTx(resut,strReturn));
		return ;
	}
	void CheckDark()
	{
		string strTxHash;
		Value valueRes = RegisterScriptTx("mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA","darksecure.bin" , 100, 10000000);
		BOOST_CHECK(GetHashFromCreatedTx(valueRes,strTxHash));
		SetBlockGenerte("mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7");

		string temp = CreateDarkTx();
		SetBlockGenerte("n4muwAThwzWvuLUh74nL3KYwujhihke1Kb");
		Value temp1 = GetAccountInfo("010000000100");
		BOOST_CHECK_EQUAL(GetValue(temp1,"value"),150);
		temp1 = GetAccountInfo("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
		BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999999999900);
		temp1 = GetAccountInfo("mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y");
		BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999998999950);

		CreateSecondDarkTx(temp);
		SetBlockGenerte("mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7");
		GetAccountInfo("010000000100");
		GetAccountInfo("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
		temp1 = GetAccountInfo("mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y");
		BOOST_CHECK_EQUAL(GetValue(temp1,"value"),150);
	}

	void CheckAnony()
	{

		CreateRegScript("mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA","anony.bin");
		SetBlockGenerte("mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7");

		Createanony("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
		SetBlockGenerte("msdDQ1SXNmknrLuTDivmJiavu5J9VyX9fV");
		Value temp1 = GetAccountInfo("010000000100");
		BOOST_CHECK_EQUAL(GetValue(temp1,"value"),100);
		 temp1 = GetAccountInfo("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
		BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999998999900);
		 temp1 = GetAccountInfo("n4muwAThwzWvuLUh74nL3KYwujhihke1Kb");

		Createanony("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
		SetBlockGenerte("mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v");
		 temp1 = GetAccountInfo("010000000100");
		BOOST_CHECK_EQUAL(GetValue(temp1,"value"),200);
		 temp1 = GetAccountInfo("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
		BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999997999800);
		 temp1 = GetAccountInfo("n4muwAThwzWvuLUh74nL3KYwujhihke1Kb");

		Createanony("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
		SetBlockGenerte("mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc");
		 temp1 = GetAccountInfo("010000000100");
		BOOST_CHECK_EQUAL(GetValue(temp1,"value"),100);
		 temp1 = GetAccountInfo("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
		BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999996999700);
		 temp1 = GetAccountInfo("mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y");
		BOOST_CHECK_EQUAL(GetValue(temp1,"value"),100);
		 temp1 = GetAccountInfo("n4muwAThwzWvuLUh74nL3KYwujhihke1Kb");

		BOOST_CHECK_EQUAL(GetValue(temp1,"value"),100);
	}
};


BOOST_FIXTURE_TEST_SUITE(sysScript_test,CSysScriptTest)

BOOST_FIXTURE_TEST_CASE(script_test,CSysScriptTest)
{
//	//// some debug
//	ResetEnv();
//	BOOST_CHECK(0==chainActive.Height());
//	CreateRegScript("mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA","soypay_test.bin");
//	CheckSdk();


	ResetEnv();
	BOOST_CHECK(0==chainActive.Height());
	CreateRegScript("mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA","soypay_test.bin");
	CheckRollBack();
//
//	ResetEnv();
//	BOOST_CHECK(0==chainActive.Height());
//	CheckScriptAccount();

}
BOOST_FIXTURE_TEST_CASE(darksecure,CSysScriptTest)
{

	ResetEnv();
	CheckDark();
}
BOOST_FIXTURE_TEST_CASE(Anony,CSysScriptTest)
{

	ResetEnv();
	CheckAnony();
}
BOOST_AUTO_TEST_SUITE_END()

