#include <stdlib.h>
#include "util.h"
#include "rpcclient.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
using namespace std;
using namespace boost;

#define BUYER	"01"
#define SELLER	"02"
#define ARBIT	"03"
extern void GetAccountInfo(char *address);
extern void GenerateMiner();
//
///**
// *
// * @param pAddress 账号签名地址
// * @param bRigsterScript	true为注册脚本，false为修改签名账户的权限
// * @param strUserData	用户自定义数据
// */
//void CreateScriptTx(char* pAddress,bool bRigsterScript,const string& strUserData)
// {
//	int argc = 13;
//	char szSript[MAX_PATH] = {0};
//	char szType[1] = {0};
//
//	if (bRigsterScript) {
//		strcpy(szSript,"D:\\bitcoin\\data\\testrollback.bin");
//		szType[0] = '1';
//	}else{
//		strcpy(szSript,"010000000100");
//		szType[0] = '0';
//	}
//
//	string message = szSript;
//	message += " not exitst";
//	BOOST_CHECK_MESSAGE(boost::filesystem::exists(szSript), message);
//
//	char *argv[13] = { "rpctest", "registerscripttx",pAddress, szType, szSript, "100000", "100", "this is description",
//			"10000", "0", "0", "0" ,(char*)strUserData.c_str()};
//	CommandLineRPC(argc, argv);
//}
//
//void CreateContactTx(int param)
//{
//	int argc = 8;
//	std::vector<std::string> vInputParams;
//	vInputParams.clear();
//	vInputParams.push_back("010000000100");
//	vInputParams.push_back(
//			"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\"]");
//	char buffer[3] = {0};
//	sprintf(buffer,"%02x",param);
//	vInputParams.push_back(buffer);
//	vInputParams.push_back("1000000");
//	vInputParams.push_back("10");
//	std::string strReturn("");
//	TestCallRPC("createcontracttx", vInputParams, strReturn);
//	cout<<strReturn<<endl;
//	return ;
//}

BOOST_AUTO_TEST_SUITE(sesuretrade_test)
BOOST_AUTO_TEST_CASE(sesuretrade)
{
//	CreateScriptTx("000000000900",true,BUYER);
//	GenerateMiner();
}
BOOST_AUTO_TEST_SUITE_END()
