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

void CreateRegScriptdbTx()
{
	int argc = 7;
	char *argv[7] =
			{ "rpctest", "registerscripttx", "5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG","0",
					"D:\\cppwork\\vmsdk\\testrollbackDB\\Debug\\Exe\\testrollback.bin",
					"1000000", "2" };
	CommandLineRPC(argc, argv);
}
void CreateContactTx(int param)
{
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\"]");
	char buffer[3] = {0};
	sprintf(buffer,"%02x",param);
	vInputParams.push_back(buffer);
	vInputParams.push_back("1000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	TestCallRPC("createcontracttx", vInputParams, strReturn);
	cout<<strReturn<<endl;
	return ;
}
void disblock1()
{
	int argc = 3;
	char *argv[3] = { "rpctest", "disconnectblock", "1" };
//	sprintf(argv[2], "%d", number);
	CommandLineRPC(argc, argv);
}
BOOST_AUTO_TEST_SUITE(test_rollback)

BOOST_AUTO_TEST_CASE(db_fun)
{
	CreateRegScriptdbTx();
	GenerateMiner();
	cout << "1" << endl;
	CreateContactTx(1);    //新增脚本数据
	GenerateMiner();
	cout << "2" << endl;
	CreateContactTx(2);;   //修改脚本数据
	GenerateMiner();

	cout << "3" << endl;
	CreateContactTx(3);    //删除脚本数据
	GenerateMiner();

	cout << "4" << endl;
	disblock1();           //删除1个block
	cout << "41" << endl;
	CreateContactTx(4);    //check删除的脚本是否恢复
	GenerateMiner();

	cout << "5" << endl;
	disblock1();
	disblock1();
	CreateContactTx(5);    //check修改的脚本数据是否恢复
	GenerateMiner();

	cout << "6" << endl;
	disblock1();
	disblock1();
	CreateContactTx(6);   //check新增的脚本数据是否恢复
	GenerateMiner();

}
BOOST_AUTO_TEST_SUITE_END()
