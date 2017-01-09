#include "./rpc/rpcserver.h"
#include "./rpc/rpcclient.h"
#include "util.h"
#include "core.h"
#include "chainparams.h"
#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include "tx.h"
using namespace std;
using namespace boost;
using namespace json_spirit;
#include "../test/systestbase.h"

map<string, string> g_mapDesAddress[] = {
        boost::assign::map_list_of
        ("000000000900",	"dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem")
        ("000000000500",	"dsGb9GyDGYnnHdjSvRfYbj9ox2zPbtgtpo")
        ("000000000300",	"dejEcGCsBkwsZaiUH1hgMapjbJqPdPNV9U")
        ("000000000800",	"e21rEzVwkPFQYfgxcg7xLp7DKeYrW4Fpoz")
        ("000000000700",	"dd936HZcwj9dQkefHPqZpxzUuKZZ2QEsbN")
        ("000000000400",	"dkoEsWuW3aoKaGduFbmjVDbvhmjxFnSbyL")
        ("000000000100",	"dggsWmQ7jH46dgtA5dEZ9bhFSAK1LASALw")
        ("000000000600",	"doym966kgNUKr2M9P7CmjJeZdddqvoU5RZ")
        ("000000000200",	"dtKsuK9HUvLLHtBQL8Psk5fUnTLTFC83GS"),

        boost::assign::map_list_of
        ("010000003300",	"mm8f5877wY4u2WhhX2JtGWPTPKwLbGJi37")
        ("010000002d00",	"n31DG5wjP1GcKyVMupGBvjvweNkM75MPuR")
        ("010000002a00",	"mkeict2uyvmb4Gjx3qXh6vReoTw1A2gkLZ")
        ("010000002600",	"mqwRHqjQZcJBqJP46E256Z2VuqqRAkZkKH")
        ("010000004300",	"mvV1fW4NMv9MGoRwHw583TDi8gnqqjQovZ")
        ("010000003500",	"mzGkgfAkWtkQ4AP2Ut4yiAzCNx6EDzWjW8")
        ("010000002500",	"n3wo9Ts6AUmHdGM1PixpRnLRFrG5G8a5QA")
        ("010000004600",	"mxrmM6qNswgZHmp1u2HTu2soncQLkud7tF")
        ("010000004500",	"mmBBV47uFguukjceTXkPsB3izndht2YXx7"),

        boost::assign::map_list_of
        ("010000003200",	"n3eyjajBMwXiK56ohkzvA2Xu53W9E6jj8K")
        ("010000002900",	"muDs5TAdk6n8rSyLdq6HTzBkJT2XxPF1wP")
        ("010000003700",	"mxZqVtfao3A6dbwymKtn6oE4GacXoJNsac")
        ("010000004000",	"mspw67fn4KGwUrG9oo9mvLJQAPrTYwxQ6w")
        ("010000004700",	"mvNmNnB98GDSeYqg2jH2gSU557XEivs3N5")
        ("010000003a00",	"mjEGztB67nfscqSg5ryUGtzyTGwwEASZeQ")
        ("010000004100",	"mmCvt8WZzF27VGBMWWVkED3vsDRdpnigGV")
        ("010000003100",	"miUVkZNCDaKLTveqT3uWcy8kkpkA94gNvS")
        ("010000004200",	"mxx4MohV2ZfifQiZnmU4yVUVf2QUVM2grx"),

		boost::assign::map_list_of
        ("010000003f00",	"mjuZWVqVQ2cmFoB8pJRj7XWVCPkeoiWJAq")
        ("010000002b00",	"n2tTaaF8xoWWYvaxSDkfQP5GeEcCCsjq1t")
        ("010000002700",	"mw1XUknDsVtb68BUJNj25rKAikYG8qELHJ")
        ("010000002f00",	"mgE3hASaGCRPxJdZruAsydr2ygQz2UBWZM")
        ("010000003400",	"n1vNXyu2GNypJGdZYxzCBCeQFVt1Fd42Qn")
        ("010000003d00",	"n4Cti65cSeufvfxStKUozHNGX3fQSHsDe5")
        ("010000003c00",	"mnZUhyb83ZTQWc9TXXFfhjJEu65q4cFj4S")
        ("010000004800",	"muGiULSeqi2FQ2ypzU7aP8Uu1SWC5kRBki")
        ("010000003900",	"mrMFs4kk8sqZ7iE8DquqPLL8udyGNDUZ8T"),

        boost::assign::map_list_of
        ("010000003800",	"mogX7FTZ9Yuu6gYscKaEf2oxroeRuNDi76")
        ("010000002e00",	"mgs1mDsaXuj16aJ5YMHqLx7xsQ88snsZmB")
        ("010000003b00",	"mzUKrawp7a7LNB7D7kKzKEpgAStsAAHz18")
        ("010000002800",	"miNou7awKXUPN9wbzVP32zTXcWvPsZBpYg")
        ("010000003e00",	"mnnd1QQx2dM5yfp1j8Vp7Dcq7BhiS6bNEQ")
        ("010000004400",	"muS2Nxtva88d45uN6up7WeHszi3oWAcadK")
        ("010000003600",	"mw8yB7Pp7GYiDHhLQT2GNsLc439rfJ3Fai")
        ("010000002c00",	"miRVDrwxtJJh4XnZFnYR6YbdqpAuirVDzZ")
        ("010000003000",	"mvqUh3LR4R7cDWfw4AW7mRUSxfZbvonQ8v")};


int64_t g_llSendValues[] = { 1000000000, 2000000000, 3000000000, 4000000000, 5000000000, 6000000000, 7000000000,
		8000000000, 9000000000, 10000000000 };

void SubmitBlock(vector<string> &vstrParam) {
	if (1 != vstrParam.size()) {
		return;
	}
	vstrParam.insert(vstrParam.begin(), "submitblock");
	vstrParam.insert(vstrParam.begin(), "rpctest");

	char *pszArgv[vstrParam.size()];
	for (size_t i = 0; i < vstrParam.size(); ++i) {
		pszArgv[i] = const_cast<char *>(vstrParam[i].c_str());
		++i;
	}
	CommandLineRPC(vstrParam.size(), pszArgv);
}

bool readblock(const string &filePath) {
	CBlock cBlock;
	FILE* pFile = fopen(filePath.c_str(), "rb");
	if (!pFile) {
		return false;
	}
	fseek(pFile, 8, SEEK_SET); // skip msgheader/size

	CAutoFile cFileIn = CAutoFile(pFile, SER_DISK, g_sClientVersion);
	if (!cFileIn) {
		return false;
	}
	while (!feof(pFile)) {
		cFileIn >> cBlock;
		CDataStream cDs(SER_DISK, g_sClientVersion);
		cDs << cBlock;
		vector<string> param;
		param.push_back(HexStr(cDs));
		SubmitBlock(param);
	}
	return true;
}

class CMiningTest {
 public:
	//初始化运行环境，导入Block信息
	CMiningTest() {
	}

	~CMiningTest() {
	};

};

class CSendItem : public SysTestBase {
 public:
	CSendItem() {
	};

	CSendItem(const string &strRegId, const string &strDesAddr, const int64_t &llSendValue) {
		m_strRegId = strRegId;
		m_strAddress = strDesAddr;
		m_llSendValue = llSendValue;
	}

	void GetContranctData(vector<unsigned char> &vuchContranct ) {
		//vector<unsigned char> temp = ParseHex(m_strRegId);
		CRegID cRegID(m_strRegId);
		vuchContranct.insert(vuchContranct.end(), cRegID.GetVec6().begin(), cRegID.GetVec6().end());
		//temp.clear();
		CDataStream cDataStream(SER_DISK, g_sClientVersion);
		cDataStream << m_llSendValue;
		vector<unsigned char> temp(cDataStream.begin(), cDataStream.end());
		//string strSendValue = HexStr(temp);
		vuchContranct.insert(vuchContranct.end(), temp.begin(), temp.end());
	}

	//nIndex 取值范围1~5，表示1~5个客户端
	static CSendItem GetRandomSendItem(int nIndex) {
		int nRandAddr = std::rand() % 9;
		int nRandSendValue = std::rand() % 10;
		map<string, string>::iterator iterAddr = g_mapDesAddress[nIndex - 1].begin();
		map<string, string>::iterator iterLast = g_mapDesAddress[nIndex - 1].end();
		--iterLast;
		do {
			iterAddr++;
		} while (--nRandAddr > 0 && iterAddr != iterLast);
		return CSendItem(iterAddr->first, iterAddr->second, g_llSendValues[nRandSendValue]);
	}

	string GetRegID() {
		return m_strRegId;
	}

	string GetAddress() {
		return m_strAddress;
	}

	uint64_t GetSendValue() {
		char chSendValue[12] = {0};
		sprintf(&chSendValue[0], "%ld", m_llSendValue);
		string strSendValue(chSendValue);
		cout << "GetSendValue:" << strSendValue << endl;
		return m_llSendValue;
	}

 private:
	string m_strRegId;
	string m_strAddress;
	int64_t m_llSendValue;
};

/**
 *构建普通交易
 * @param param
 * param[0]:源地址
 * param[1]:目的地址
 * param[2]:转账金额
 * param[3]:手续费
 * param[4]:有效期高度
 */
void CreateNormalTx(vector<string> &vstrParam) {
	if (3 != vstrParam.size()) {
		return;
	}
	vstrParam.insert(vstrParam.begin(), "sendtoaddress");
	vstrParam.insert(vstrParam.begin(), "rpctest");
	char *argv[vstrParam.size()];
	for (size_t i = 0; i < vstrParam.size(); ++i) {
		argv[i] = const_cast<char *>(vstrParam[i].c_str());
	}
	CommandLineRPC(vstrParam.size(), argv);
}

/**
 * 构建合约交易
 * @param param
 * param[0]:脚本注册ID
 * param[1]:账户地址列表,json的数组格式
 * param[2]:合约内容
 * param[3]:手续费
 * param[4]:有效期高度
 */
void CreateContractTx(vector<string> &vstrParam) {
	if (5 != vstrParam.size()) {
		return;
	}
	vstrParam.insert(vstrParam.begin(), "createcontracttx");
	vstrParam.insert(vstrParam.begin(), "rpctest");
	char *argv[vstrParam.size()];
	for (size_t i = 0; i < vstrParam.size(); ++i) {
		argv[i] = const_cast<char *>(vstrParam[i].c_str());
	}
	CommandLineRPC(vstrParam.size(), argv);
}

/**
 * 构建注册脚本交易
 * @param param
 * param[0]:注册脚本的账户地址
 * param[1]:注册脚本标识位，0-标识脚本内容的文件路径，1-已注册脚本ID
 * param[2]:文件路径或注册脚本ID
 * param[3]:手续费
 * param[4]:有效期高度
 * param[5]:脚本描述 （针对新注册脚本,可选）
 * param[6]:脚本授权时间 （可选）
 * param[7]:授权脚本每次从账户中扣减金额上限 （可选）
 * param[8]:授权脚本总共扣钱金额上限 （可选）
 * param[9]:授权脚本每天扣钱金额上限 （可选）
 * param[10]:用户自定义数据
 *
 */
void CreateRegScriptTx(vector<string> &vstrParam) {
	if (5 > vstrParam.size()) {
		return;
	}
	vstrParam.insert(vstrParam.begin(), "registerapptx");
	vstrParam.insert(vstrParam.begin(), "rpctest");

	char *argv[vstrParam.size()];
	for (size_t i = 0; i < vstrParam.size(); ++i) {
		argv[i] = const_cast<char *>(vstrParam[i].c_str());
	}
	CommandLineRPC(vstrParam.size(), argv);
}

time_t g_tSleepTime = 500;     //每隔1秒发送一个交易
int64_t g_llTime = 24*60*60;   //测试24小时

time_t string2time(const char * str, const char * formatStr) {
	struct tm tTm1;
	int year, mon, mday, hour, min, sec;
	if (-1 == sscanf(str, formatStr, &year, &mon, &mday, &hour, &min, &sec)) {
		return -1;
	}
	tTm1.tm_year = year - 1900;
	tTm1.tm_mon = mon - 1;
	tTm1.tm_mday = mday;
	tTm1.tm_hour = hour;
	tTm1.tm_min = min;
	tTm1.tm_sec = sec;
	return mktime(&tTm1);
}

BOOST_FIXTURE_TEST_SUITE(auto_mining_test, CSendItem)
BOOST_FIXTURE_TEST_CASE(regscript,CSendItem) {
	//注册脚本交易
	SysTestBase::RegisterAppTx("dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem","unit_test.bin",0);
}
BOOST_FIXTURE_TEST_CASE(test1, CSendItem)
{
	const char *argv[] = {"progname", "-datadir=D:\\bitcoin\\1"};
	int argc = sizeof(argv) / sizeof(char*);
	CBaseParams::IntialParams(argc, argv);
//	time_t t1 = string2time("2014-12-01 17:30:00","%d-%d-%d %d:%d:%d");

	Value resulut = RegisterAppTx("dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem","unit_test.bin",0);
	string strScripthash = "";
	BOOST_CHECK(GetHashFromCreatedTx(resulut,strScripthash));
	string strScriptid = "";
	BOOST_CHECK(GetTxConfirmedRegID(strScripthash,strScriptid));

	int64_t llRunTime = GetTime()+g_llTime;
	vector<string> vstrParam;
	while(GetTime()<llRunTime) {
		//创建客户端1->客户端2的普通交易
		CSendItem cSendItem = CSendItem::GetRandomSendItem(1);
		CSendItem cRecItem = CSendItem::GetRandomSendItem(2);
		CreateNormalTx(cSendItem.GetAddress(),cRecItem.GetAddress(),cRecItem.GetSendValue());                          //创建普通交易
		MilliSleep(g_tSleepTime);

		//创建客户端1->客户端2的合约交易
		CSendItem cSendItem1 = CSendItem::GetRandomSendItem(1);

		CreateContractTx(strScriptid,cSendItem1.GetAddress(),"01",0);                        //创建合约交易
		MilliSleep(g_tSleepTime);
	}
}
BOOST_AUTO_TEST_CASE(test2)
{
	const char *argv[] = {"progname", "-datadir=D:\\bitcoin\\2"};
	int argc = sizeof(argv) / sizeof(char*);
	CBaseParams::IntialParams(argc, argv);
	int64_t llRunTime = GetTime()+g_llTime;
	Value resulut = RegisterAppTx("dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem","unit_test.bin",0);
	string strScripthash = "";
	BOOST_CHECK(GetHashFromCreatedTx(resulut,strScripthash));
	string strScriptid = "";
	BOOST_CHECK(GetTxConfirmedRegID(strScripthash,strScriptid));

	while(GetTime()<llRunTime) {
		//创建客户端2->客户端3的普通交易
		CSendItem cSendItem = CSendItem::GetRandomSendItem(2);
		CSendItem cRecItem = CSendItem::GetRandomSendItem(3);
		CreateNormalTx(cSendItem.GetAddress(),cRecItem.GetAddress(),cRecItem.GetSendValue());
		MilliSleep(g_tSleepTime);

		//创建客户端2->客户端3的合约交易
		CSendItem cSendItem1 = CSendItem::GetRandomSendItem(2);
		CreateContractTx(strScriptid,cSendItem1.GetAddress(),"01",0);                        //创建合约交易
		MilliSleep(g_tSleepTime);
	}

}
BOOST_AUTO_TEST_CASE(test3)
{
	const char *argv[] = {"progname", "-datadir=D:\\bitcoin\\3"};
	int argc = sizeof(argv) / sizeof(char*);
	CBaseParams::IntialParams(argc, argv);
	int64_t llRunTime = GetTime()+g_llTime;

	Value resulut = RegisterAppTx("dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem","unit_test.bin",0);
	string strScripthash = "";
	BOOST_CHECK(GetHashFromCreatedTx(resulut,strScripthash));
	string strScriptid = "";
	BOOST_CHECK(GetTxConfirmedRegID(strScripthash,strScriptid));
	while(GetTime()<llRunTime) {
		//创建客户端3->客户端4的普通交易
		CSendItem cSendItem = CSendItem::GetRandomSendItem(3);
		CSendItem cRecItem = CSendItem::GetRandomSendItem(4);
		CreateNormalTx(cSendItem.GetAddress(),cRecItem.GetAddress(),cRecItem.GetSendValue());
		MilliSleep(g_tSleepTime);

		//创建客户端3->客户端4的合约交易
		CSendItem sendItem1 = CSendItem::GetRandomSendItem(3);
		CreateContractTx(strScriptid,sendItem1.GetAddress(),"01",0);
		MilliSleep(g_tSleepTime);
	}
}

BOOST_AUTO_TEST_CASE(test4)
{

	const char *argv[] = {"progname", "-datadir=D:\\bitcoin\\4"};
	int argc = sizeof(argv) / sizeof(char*);
	CBaseParams::IntialParams(argc, argv);
	int64_t llRunTime = GetTime()+g_llTime;

	Value resulut = RegisterAppTx("dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem","unit_test.bin",0);
	string strScripthash = "";
	BOOST_CHECK(GetHashFromCreatedTx(resulut,strScripthash));
	string strScriptid = "";
	BOOST_CHECK(GetTxConfirmedRegID(strScripthash,strScriptid));
	while(GetTime()<llRunTime) {
		//创建客户端4->客户端5的普通交易
		CSendItem cSendItem = CSendItem::GetRandomSendItem(4);
		CSendItem cRecItem = CSendItem::GetRandomSendItem(5);
		CreateNormalTx(cSendItem.GetAddress(),cRecItem.GetAddress(),cRecItem.GetSendValue());                     //创建普通交易
		MilliSleep(g_tSleepTime);

		//创建客户端4->客户端5的合约交易
		CSendItem sendItem1 = CSendItem::GetRandomSendItem(4);
		CreateContractTx(strScriptid,sendItem1.GetAddress(),"01",0);
		MilliSleep(g_tSleepTime);
	}
}
BOOST_AUTO_TEST_CASE(test5)
{

	const char *argv[] = {"progname", "-datadir=D:\\bitcoin\\5"};
	int argc = sizeof(argv) / sizeof(char*);
	CBaseParams::IntialParams(argc, argv);

	int64_t llRunTime = GetTime()+g_llTime;
	Value resulut = RegisterAppTx("dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem","unit_test.bin",0);
	string strScripthash = "";
	BOOST_CHECK(GetHashFromCreatedTx(resulut,strScripthash));
	string strScriptid = "";
	BOOST_CHECK(GetTxConfirmedRegID(strScripthash,strScriptid));
	while(GetTime()<llRunTime) {
		//创建客户端5->客户端1的普通交易
		CSendItem cSendItem = CSendItem::GetRandomSendItem(5);
		CSendItem cRecItem = CSendItem::GetRandomSendItem(1);
		CreateNormalTx(cSendItem.GetAddress(),cRecItem.GetAddress(),cRecItem.GetSendValue());
		MilliSleep(g_tSleepTime);

		//创建客户端5->客户端1的合约交易
		CSendItem sendItem1 = CSendItem::GetRandomSendItem(5);
		CreateContractTx(strScriptid,sendItem1.GetAddress(),"01",0);
		MilliSleep(g_tSleepTime);
	}

}

BOOST_AUTO_TEST_SUITE_END()
