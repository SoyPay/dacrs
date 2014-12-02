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
using namespace std;
using namespace boost;
using namespace json_spirit;

extern Object CallRPC(const string& strMethod, const Array& params);
extern int TestCallRPC(std::string strMethod, const std::vector<std::string> &vParams, std::string &strRet);
extern void GetAccountInfo(char *address);
extern void GenerateMiner();

void CreateScriptTx1()
{
	int argc = 7;
	char *argv[7] =
			{ "rpctest", "registerscripttx", "mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA","0",
					"D:\\cppwork\\vmsdk\\testscriptid\\Debug\\Exe\\testscriptid.bin",
					"1000000", "2" };
	CommandLineRPC(argc, argv);
}
void CreateScriptTx2()
{
	int argc = 7;
	char *argv[7] =
			{ "rpctest", "registerscripttx", "mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA","0",
					"D:\\cppwork\\vmsdk\\sendsriptid\\Debug\\Exe\\scripttest.bin",
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
	cout<<strReturn<<endl;
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
	cout<<strReturn<<endl;
	return ;
}

void CreateOperateOtherScriptTx(int param)
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
	cout<<strReturn<<endl;
	return ;
}
BOOST_AUTO_TEST_SUITE(test_script)

BOOST_AUTO_TEST_CASE(script_account){
	CreateScriptAndCheck();
	cout<<"1"<<endl;
	CreateOperateSelfScriptTx(1);
	GenerateMiner();
	GetAccountInfo("010000000100");
	GetAccountInfo("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
	cout<<"2"<<endl;
	CreateOperateSelfScriptTx(2);
	GenerateMiner();
	GetAccountInfo("010000000100");
	GetAccountInfo("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
	cout<<"3"<<endl;
	CreateOperateOtherScriptTx(1);
	GenerateMiner();
	GetAccountInfo("010000000100");
	GetAccountInfo("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");
	cout<<"4"<<endl;
	CreateOperateOtherScriptTx(2);
	GenerateMiner();
	GetAccountInfo("010000000100");
	GetAccountInfo("mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5");

}
BOOST_AUTO_TEST_SUITE_END()
