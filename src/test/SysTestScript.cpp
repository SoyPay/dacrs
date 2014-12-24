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
#include "tx.h"
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
		const Value& result3 = find_value(valueRes.get_obj(), "script");
		if (result.type() == null_type && result1.type() == null_type && result3.type() == null_type){
			return false;
		}
		if (result.type() != null_type){
			strHash = result.get_str();
			}
		else if(result1.type() != null_type)
		{
			strHash = result1.get_str();
		}else if(result3.type() != null_type)
		{
			strHash = result3.get_str();
		}

		return true;
	}

	void CheckSdk()
	{
		int nHeight = 0;
		string param ="01";
		Value resut =CreateContractTx1("010000000100", "[\"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb\"]", param,10);
		BOOST_CHECK(GetHashFromCreatedTx(resut,TxHash));
		LogPrint("vm", "create new contract tx:hash=%s\n", TxHash);
		LogPrint("INFO", "create new contract tx:hash=%s\n", TxHash);
		BOOST_CHECK(GenerateOneBlock());
		uint256 hash(TxHash.c_str());
		param ="02";
		param += HexStr(hash);
		string temp;
		resut =CreateContractTx1("010000000100", "[\"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb\"]", param,10);
		BOOST_CHECK(GetHashFromCreatedTx(resut,temp));
		LogPrint("vm", "create new contract tx:hash=%s\n", temp);
		LogPrint("INFO", "create new contract tx:hash=%s\n", temp);
		BOOST_CHECK(GenerateOneBlock());

		param ="03";
		resut =CreateContractTx1("010000000100", "[\"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb\"]", param,10);
		BOOST_CHECK(GetHashFromCreatedTx(resut,temp));
		LogPrint("vm", "create new contract tx:hash=%s\n", temp);
		LogPrint("INFO", "create new contract tx:hash=%s\n", temp);
		BOOST_CHECK(GenerateOneBlock());

		param ="05";
		param += HexStr(hash);

		resut =CreateContractTx1("010000000100", "[\"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb\"]", param,10);
		BOOST_CHECK(GetHashFromCreatedTx(resut,temp));
		LogPrint("vm", "create new contract tx:hash=%s\n", temp);
		LogPrint("INFO", "create new contract tx:hash=%s\n", temp);
		BOOST_CHECK(GenerateOneBlock());
	}

	string CreateRegScript(char*strAddr,char*sourceCode)
	{
		int nFee = 10000000;
		string strTxHash;
		string strFileName(sourceCode);
		Value valueRes = RegisterScriptTx(strAddr,strFileName , 100, nFee);
		BOOST_CHECK(GetHashFromCreatedTx(valueRes,strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		return strTxHash;
	}
	string CreateContactTx(int param)
	{
		char buffer[3] = {0};
		sprintf(buffer,"%02x",param);
		string temp;
		Value resut =CreateContractTx1("010000000100", "[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\"]", buffer,10);
		BOOST_CHECK(GetHashFromCreatedTx(resut,temp));
		LogPrint("vm", "create new contract tx:hash=%s\n", temp);
		LogPrint("INFO", "create new contract tx:hash=%s\n", temp);
		BOOST_CHECK(GenerateOneBlock());
		return temp;
	}
	void disblock1()
	{
		int argc = 3;
		char *argv[3] = { "rpctest", "disconnectblock", "1" };
	//	sprintf(argv[2], "%d", number);
		Value dummy;
		CommandLineRPC_GetValue(argc, argv,dummy);
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
		LogPrint("vm", "create new contract tx:hash=%s\n", temp);
		LogPrint("INFO", "create new contract tx:hash=%s\n", temp);
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
		LogPrint("vm", "create new contract tx:hash=%s\n", strReturn);
		LogPrint("INFO", "create new contract tx:hash=%s\n", strReturn);
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
		LogPrint("vm", "create new contract tx:hash=%s\n", strReturn);
		LogPrint("INFO", "create new contract tx:hash=%s\n", strReturn);
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
		LogPrint("vm", "create new contract tx:hash=%s\n", strReturn);
		LogPrint("INFO", "create new contract tx:hash=%s\n", strReturn);
		return ;
	}

	void GetScriptDataSize() {
		char *param[] = { "rpctest",
					"getscriptdbsize",
					"010000000100"};
//		CommandLineRPC(3, param);
		Value dummy;
		CommandLineRPC_GetValue(3, param,dummy);
	}

	void ListScriptData() {
		char *param[] = {
				"rpctest",
				"getscriptdata",
				"010000000100",
				"100",
				"1"};
//		CommandLineRPC(5, param);
		Value dummy;
		CommandLineRPC_GetValue(5, param,dummy);

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
	bool CheckScriptDB(int nheigh,string srcipt,int height,string hash,int flag)
	{
		int tipH = chainActive.Height();
		int outHeight =tipH <nheigh?nheigh: (tipH-nheigh) +5 +height;
		int maxKey = tipH <nheigh?200:((tipH- nheigh+1)*200);
		int Size =  tipH <nheigh?1000:(5-(tipH- nheigh+1))*200;
		string  hash2 = "hash";
		if(flag)
		{
			maxKey = maxKey>600?600:maxKey;
			if(maxKey > 600)
			outHeight = 11;

		}
		CRegID regid(srcipt);
		if (regid.IsEmpty() == true) {
			return false;
		}

		CScriptDBViewCache contractScriptTemp(*pScriptDBTip, true);
		if (!contractScriptTemp.HaveScript(regid)) {
			return false;
		}
		int dbsize;
		contractScriptTemp.GetScriptDataCount(regid, dbsize);
		if(Size <=0)
		{
			BOOST_CHECK(0==dbsize);
			return true;
		}
		BOOST_CHECK(Size==dbsize);

		int curtiph = chainActive.Height();
		vector<unsigned char> value;
		vector<unsigned char> vScriptKey;
		int nHeight = 0;
		set<CScriptDBOperLog> setOperLog;
		if (!contractScriptTemp.GetScriptData(curtiph,regid, 0, vScriptKey, value, nHeight,setOperLog)) {
			return false;
		}
		uint256 hash1(value);
		string pvalue(value.begin(),value.end());
		if(flag)
		BOOST_CHECK(hash==hash1.GetHex()|| pvalue == hash2);
		else{
			BOOST_CHECK(hash==hash1.GetHex());
		}
		BOOST_CHECK(nHeight>=outHeight);
		unsigned short key = 0;
		memcpy(&key,  &vScriptKey.at(0), sizeof(key));
		BOOST_CHECK(key>=(maxKey - 200));

		int count = dbsize - 1;
		while (count--) {
			if (!contractScriptTemp.GetScriptData(curtiph, regid, 1, vScriptKey, value, nHeight, setOperLog)) {
				return false;
			}
			uint256 hash3(value);
			string pvalue(value.begin(), value.end());
			if (flag)
				BOOST_CHECK(hash == hash3.GetHex() || pvalue == hash2);
			else {
				BOOST_CHECK(hash == hash1.GetHex());
			}
			BOOST_CHECK(nHeight >= outHeight);
			unsigned short key = 0;
			memcpy(&key, &vScriptKey.at(0), sizeof(key));
			BOOST_CHECK(key >= (maxKey - 200));
		}
		return true;
	}

	void CreateTx(string pcontact,string addr)
	{
		string temp ="[";
		temp += "\""+addr+"\""+"]";
		Value resut =CreateContractTx1("010000000100", temp, pcontact,10);
		string strReturn;
		BOOST_CHECK(GetHashFromCreatedTx(resut,strReturn));
		BOOST_CHECK(GenerateOneBlock());
		return ;
	}
	bool GetScriptData(string srcipt,vector<unsigned char> key)
	{
		CRegID regid(srcipt);
			if (regid.IsEmpty() == true) {
				return false;
			}
			CScriptDBViewCache contractScriptTemp(*pScriptDBTip, true);
			if (!contractScriptTemp.HaveScript(regid)) {
				return false;
			}
			vector<unsigned char> value;
			int nHeight = 0;
			int tipH = chainActive.Height();
			CScriptDBOperLog operLog;
			if (!contractScriptTemp.GetScriptData(tipH,regid,key, value, nHeight,operLog)) {
				return false;
			}
			return true;
	}
	int GetScriptSize(string srcipt)
	{
			CRegID regid(srcipt);
			if (regid.IsEmpty() == true) {
				return 0;
			}

			if (!pScriptDBTip->HaveScript(regid)) {
				return 0;
			}
			int dbsize;
			pScriptDBTip->GetScriptDataCount(regid, dbsize);
			return dbsize;
	}
	string CreatWriteTx(string &hash)
	{
		string shash = CreateRegScript("mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA","soypay_test.bin");
		Value ret =GetScriptID(shash);
		string scriptid;
		BOOST_CHECK(GetHashFromCreatedTx(ret,scriptid));
		//// first tx
		string phash = CreateContactTx(15);
		int  height = chainActive.Height();

		BOOST_CHECK(CheckScriptDB((height+5),scriptid,height,phash,false));

		hash = phash;
		return scriptid;
	}
	void testdb()
	{
		string phash = "";
		string scriptid =  CreatWriteTx(phash);
		int height = chainActive.Height();
		int circle = 4;
		while(circle--)
		{
			BOOST_CHECK(GenerateOneBlock());
		}

		int count = GetScriptSize(scriptid);
		while(count > 1)
		{
			//// second tx
				uint256 hash(phash.c_str());
				int param =16;
				string temp = "";
				temp += tinyformat::format("%02x%s%02x",param,HexStr(hash),height);
			//	cout<<"cont:"<<temp<<endl;
				CreateTx(temp,"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb");

				vector<unsigned char> key;
				char *key1="2_error";
				key.insert(key.begin(),key1, key1 + strlen(key1) +1);
				BOOST_CHECK(!GetScriptData(scriptid,key));

				CheckScriptDB((height+5),scriptid,height,phash,false);
				count = GetScriptSize(scriptid);
		}

		while(true)
		{
			disblock1();
			CheckScriptDB((height+5),scriptid,height,phash,false);
			count = GetScriptSize(scriptid);
			if(count == 1000)
				break;
		}

	}

	void testdeletmodifydb()
	{
		string  writetxhash= "";
		string scriptid =  CreatWriteTx(writetxhash);
		int height = chainActive.Height();

		///// 修改删除包
		int param =17;
		string temp = "";
		temp += tinyformat::format("%02x%02x",param,11);
		CreateTx(temp,"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb");
		vector<unsigned char> key;
		char *key1="3_error";
		key.insert(key.begin(),key1, key1 + strlen(key1) +1);
		BOOST_CHECK(!GetScriptData(scriptid,key));
		CheckScriptDB((height+5),scriptid,height,writetxhash,true);
		int modHeight = chainActive.Height();

	//	cout<<"end:"<<endl;
		//// 遍历
		int count = GetScriptSize(scriptid);
        while(count > 1)
        {
    		int param =18;
    		string temp = "";
    		temp += tinyformat::format("%02x",param);
    		CreateTx(temp,"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb");
  //  		cout<<"cont:"<<endl;
   // 		cout<<chainActive.Height()<<endl;
        	CheckScriptDB((height+5),scriptid,height,writetxhash,true);
        	count = GetScriptSize(scriptid);
        }

    	count = GetScriptSize(scriptid);
      /// 回滚
		while(true)
		{
			disblock1();
			count = GetScriptSize(scriptid);
			if(chainActive.Height() > modHeight){
			CheckScriptDB((height+5),scriptid,height,writetxhash,true);
			}else{
				CheckScriptDB((height+5),scriptid,height,writetxhash,false);
			}
			if(count == 1000)
				break;
		}
	}
	void TestMinner()
	{
		for(int i=0;i < 100;i++)
		{
			string newaddr;
			srand(time(NULL));
			int r = (rand() % 2);
			string strflag = "false";
			if(r == 1)
			{
				strflag ="true";
			}
			BOOST_CHECK(GetNewAddr(newaddr,r));

			char fee[64] = { 0 };
			int nfee = GetRandomFee();
			sprintf(fee, "%d", nfee);

			char *argv[] = { "rpctest", "sendtoaddress", (char*)newaddr.c_str(),fee};
			int argc = sizeof(argv) / sizeof(char*);

			Value value;
			BOOST_CHECK(CommandLineRPC_GetValue(argc, argv, value));
			string hash = "";
			BOOST_CHECK(GetHashFromCreatedTx(value,hash));
			BOOST_CHECK(GenerateOneBlock());
			char *argv1[] = { "rpctest", "registeraccounttx", (char*)newaddr.c_str(),fee,(char*)strflag.c_str()};
			int argc1 = sizeof(argv1) / sizeof(char*);
			Value value1;
			BOOST_CHECK(CommandLineRPC_GetValue(argc1, argv1, value1));
			BOOST_CHECK(GetHashFromCreatedTx(value1,hash));
			BOOST_CHECK(GenerateOneBlock());
			for(int j=0; j<100 ;++j)
			cout<<'\b';
			cout << "TestMinner progress: "<<  (int)(((i+1)/(float)100) * 100) << "%";
		}

		BOOST_CHECK(DisConnectBlock(chainActive.Height()-1));
		BOOST_CHECK(GenerateOneBlock());
		BOOST_CHECK(GenerateOneBlock());
	}
};


BOOST_FIXTURE_TEST_SUITE(sysScript_test,CSysScriptTest)

BOOST_FIXTURE_TEST_CASE(script_test,CSysScriptTest)
{
//	//// some debug
	ResetEnv();
	BOOST_CHECK(0==chainActive.Height());
	CreateRegScript("mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA","soypay_test.bin");
	CheckSdk();

	ResetEnv();
	BOOST_CHECK(0==chainActive.Height());
	CreateRegScript("mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA","soypay_test.bin");
	CheckRollBack();

	ResetEnv();
	BOOST_CHECK(0==chainActive.Height());
	CheckScriptAccount();

	ResetEnv();
	BOOST_CHECK(0==chainActive.Height());
	testdb();

	ResetEnv();
	BOOST_CHECK(0==chainActive.Height());
	 testdeletmodifydb();
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
BOOST_FIXTURE_TEST_CASE(minier,CSysScriptTest)
{

	ResetEnv();
	TestMinner();
}
BOOST_AUTO_TEST_SUITE_END()

