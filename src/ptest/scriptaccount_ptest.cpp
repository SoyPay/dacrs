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
extern string GetAccountInfo1(string address);
extern uint64_t GetValue(string str,string compare);
extern string Parsejson(string str);
extern void SetBlockGenerte(string address);
void CreateScriptTx1()
{
	int argc = 7;
	char* path = "D:\\bitcoin\\data\\testscriptid.bin";
	string message = path;
	message += "not exitst";
		BOOST_CHECK_MESSAGE(boost::filesystem::exists(path),message);
	char *argv[7] =
			{ "rpctest", "registerscripttx", "mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA","0",
					path,
					"1000000", "2" };
	CommandLineRPC(argc, argv);
}
void CreateScriptTx2()
{
	int argc = 7;
	char* path = "D:\\bitcoin\\data\\scripttest.bin";
	string message = path;
	message += "not exitst";
		BOOST_CHECK_MESSAGE(boost::filesystem::exists(path),message);
	char *argv[7] =
			{ "rpctest", "registerscripttx", "mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA","0",
					path,
					"1000000", "2" };
	CommandLineRPC(argc, argv);
}
void listregidinformation(char* regid)
{
	int argc = 7;
	char *argv[7] =
			{ "rpctest", "listregid", regid};
	CommandLineRPC(argc, argv);
}
bool CheckScriptid(string str,string scriptid)
{
	json_spirit::Value val;
	json_spirit::read(str, val);

//	json_spirit::json ::
	if (val.type() != obj_type)
	{
		return false;
	}
	if (val.type() != obj_type)
	{
		return false;
	}
	const Value& value = val.get_obj();
	if (value.type() != obj_type)
	{
		return false;
	}
	json_spirit::Value::Object obj= value.get_obj();
	for(int i = 0; i < obj.size(); ++i)
	{
		const json_spirit::Pair& pair = obj[i];
		const std::string& str_name = pair.name_;
		const json_spirit::Value& val_val = pair.value_;
		if(str_name =="publicKey:")
		{
			if(val_val.get_str() != "")
			{
				cout<<"pbulic error"<<val_val.get_str()<<endl;
			}
		}
		else if(str_name =="keyID:")
		{
			CRegID regId(scriptid);
			CKeyID keyId = Hash160(regId.GetVec6());
			string key = HexStr(keyId.begin(), keyId.end()).c_str();
			if(val_val.get_str() != key)
			{
				cout<<"key1:"<<key<<endl;
				cout<<"key2"<<val_val.get_str() <<endl;
			}
		}

	}
	return true;
}
bool CreateScriptAndCheck()
{
	CreateScriptTx1();
	GenerateMiner();

	CreateScriptTx2();
	GenerateMiner();

	string strRegID = "010000000100";
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	std::string strReturn("");
	TestCallRPC("getaccountinfo", vInputParams, strReturn);
//	cout<<strReturn<<endl;
	if(!CheckScriptid(strReturn,strRegID))
	{
		return false;
	}

	strRegID = "020000000100";
	vInputParams.clear();
	vInputParams.push_back("020000000100");
	strReturn ="";
	TestCallRPC("getaccountinfo", vInputParams, strReturn);
	if(!CheckScriptid(strReturn,strRegID))
	{
		return false;
	}

}

void CreateOperateSelfScriptTx(int param)
{
	string accountid = "010000000100";
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5\"]");

	string temp = "";
	temp += tinyformat::format("%02x%s",param,accountid);
	vInputParams.push_back(temp);
	vInputParams.push_back("1000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	TestCallRPC("createcontracttx", vInputParams, strReturn);
//	cout<<strReturn<<endl;
	return ;
}

string CreateOperateOtherScriptTx(int param)
{
	string accountid = "010000000100";
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("020000000100");
	vInputParams.push_back(
			"[\"mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5\"]");

	string temp = "";
	temp += tinyformat::format("%02x%s",param,accountid);
	vInputParams.push_back(temp);
	vInputParams.push_back("1000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	TestCallRPC("createcontracttx", vInputParams, strReturn);
//	cout<<strReturn<<endl;
	return strReturn;
}
BOOST_AUTO_TEST_SUITE(test_script)

BOOST_AUTO_TEST_CASE(script_account){
	BOOST_CHECK_EQUAL(CreateScriptAndCheck(),true);

	CreateOperateSelfScriptTx(1);
	SetBlockGenerte("mjSwCwMsvtKczMfta1tvr78z2FTsZA1JKw");
	string temp1 = GetAccountInfo1("010000000100");
	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),10000);
	temp1 = GetAccountInfo1("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
	BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999998990000);

	CreateOperateSelfScriptTx(2);
	SetBlockGenerte("msdDQ1SXNmknrLuTDivmJiavu5J9VyX9fV");
	temp1 = GetAccountInfo1("010000000100");
	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),0);
	temp1 = GetAccountInfo1("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),10000);

	CreateOperateOtherScriptTx(1);
	SetBlockGenerte("mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v");
	temp1 = GetAccountInfo1("010000000100");
	BOOST_CHECK_EQUAL(GetValue(temp1,"value"),10000);
	temp1 = GetAccountInfo1("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
	BOOST_CHECK_EQUAL(GetValue(temp1,"FreeValues"),999999996990000);

	string ret = CreateOperateOtherScriptTx(2);
	GenerateMiner();
	BOOST_CHECK_EQUAL(Parsejson(ret) == "",true);

}
BOOST_AUTO_TEST_SUITE_END()
