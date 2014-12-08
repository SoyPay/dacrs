#include "rpcserver.h"
#include "rpcclient.h"
#include "util.h"
#include "core.h"
#include "chainparams.h"
#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include "tx.h"
using namespace std;
using namespace boost;
using namespace json_spirit;

map<string, string> mapDesAddress[] = {
        boost::assign::map_list_of
        ("000000000900",	"mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA")
        ("000000000500",	"mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5")
        ("000000000300",	"mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y")
        ("000000000800",	"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb")
        ("000000000700",	"mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7")
        ("000000000400",	"moZJZgsGFC4qvwRdjzS7Bj3fHrtpUfEVEE")
        ("000000000100",	"mjSwCwMsvtKczMfta1tvr78z2FTsZA1JKw")
        ("000000000600",	"mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v")
        ("000000000200",	"mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc"),

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


int64_t sendValues[] = {1000000000, 2000000000, 3000000000, 4000000000, 5000000000, 6000000000, 7000000000, 8000000000, 9000000000, 10000000000};


void SubmitBlock(vector<string> &param) {
	if(1 != param.size())
			return;
	param.insert(param.begin(), "submitblock");
	param.insert(param.begin(), "rpctest");

	char *argv[param.size()];
	int i=0;
	for(auto & item : param) {
		argv[i] = const_cast<char *>(param[i].c_str());
		++i;
	}
	CommandLineRPC(param.size(), argv);
}

bool readblock(const string &filePath)
{
	CBlock block;
    FILE* fp = fopen(filePath.c_str(), "rb");
    if (!fp) return false;

    fseek(fp, 8, SEEK_SET); // skip msgheader/size

    CAutoFile filein = CAutoFile(fp, SER_DISK, CLIENT_VERSION);
    if (!filein) return false;
    while(!feof(fp)) {
    	filein >> block;
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		ds << block;
    	vector<string> param;
    	param.push_back(HexStr(ds));
    	SubmitBlock(param);
    }
    return true;
}

class CMiningTest {
public:
	//初始化运行环境，导入Block信息
	CMiningTest() {

	}
	~CMiningTest() {};

};


class CSendItem{
private:
	string m_strRegId;
	string m_strAddress;
	int64_t m_llSendValue;
public:
	CSendItem(){
	};
	CSendItem(const string &strRegId, const string &strDesAddr, const int64_t &llSendValue)
	{
		m_strRegId = strRegId;
		m_strAddress = strDesAddr;
		m_llSendValue = llSendValue;
	}
	void GetContranctData(vector<unsigned char> &vContranct ) {
		//vector<unsigned char> temp = ParseHex(m_strRegId);
		CRegID reg(m_strRegId);
		vContranct.insert(vContranct.end(), reg.GetVec6().begin(), reg.GetVec6().end());
		//temp.clear();
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		ds << m_llSendValue;
		vector<unsigned char> temp(ds.begin(), ds.end());
		//string strSendValue = HexStr(temp);
		vContranct.insert(vContranct.end(), temp.begin(), temp.end());
	}
	//nIndex 取值范围1~5，表示1~5个客户端
	static CSendItem GetRandomSendItem(int nIndex) {
		int randAddr = std::rand() % 9;
		int randSendValue = std::rand() % 10;
		map<string, string>::iterator iterAddr = mapDesAddress[nIndex - 1].begin();
		map<string, string>::iterator iterLast = mapDesAddress[nIndex - 1].end();
		--iterLast;
		do {
			iterAddr++;
		} while (--randAddr > 0 && iterAddr != iterLast);
		return CSendItem(iterAddr->first, iterAddr->second, sendValues[randSendValue]);
	}
	string GetRegID() {
		return m_strRegId;
	}

	string GetAddress() {
		return m_strAddress;
	}

	string GetSendValue() {
		char cSendValue[12] = {0};
		sprintf(&cSendValue[0], "%lld", m_llSendValue);
		string strSendValue(cSendValue);
		cout << "GetSendValue:" << strSendValue << endl;
		return strSendValue;
	}


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
void CreateNormalTx(vector<string> &param) {
	if(3 != param.size())
		return;
	param.insert(param.begin(), "sendtoaddress");
	param.insert(param.begin(), "rpctest");
	char *argv[param.size()];
	int i=0;
	for(auto & item : param) {
		argv[i] = const_cast<char *>(param[i].c_str());
		++i;
	}
	CommandLineRPC(param.size(), argv);
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
void CreateContractTx(vector<string> &param) {
	if(5 != param.size())
		return;
	param.insert(param.begin(), "createcontracttx");
	param.insert(param.begin(), "rpctest");
	char *argv[param.size()];
	int i=0;
	for(auto & item : param) {
		argv[i] = const_cast<char *>(param[i].c_str());
		++i;
	}
	CommandLineRPC(param.size(), argv);
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
void CreateRegScriptTx(vector<string> &param) {
	if(5 > param.size())
		return;
	param.insert(param.begin(), "registerscripttx");
	param.insert(param.begin(), "rpctest");

	char *argv[param.size()];
	int i=0;
	for(auto & item : param) {
		argv[i] = const_cast<char *>(param[i].c_str());
		++i;
	}
	CommandLineRPC(param.size(), argv);
}

time_t string2time(const char * str,const char * formatStr)
{
  struct tm tm1;
  int year,mon,mday,hour,min,sec;
  if( -1 == sscanf(str,formatStr,&year,&mon,&mday,&hour,&min,&sec)) return -1;
  tm1.tm_year=year-1900;
  tm1.tm_mon=mon-1;
  tm1.tm_mday=mday;
  tm1.tm_hour=hour;
  tm1.tm_min=min;
  tm1.tm_sec=sec;
  return mktime(&tm1);
}
BOOST_FIXTURE_TEST_SUITE(auto_mining_test, CSendItem)
BOOST_AUTO_TEST_CASE(regscript) {
	//注册脚本交易
	vector<string> param;
	param.push_back("mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA");
	param.push_back("0");
	param.push_back("D:\\cppwork\\vmsdk\\testUint\\Debug\\Exe\\test.bin");
	param.push_back("100000000");
	param.push_back("0");
	param.push_back("test");
	int64_t curTime = GetTime();
	char charTime[20] = {0};
	sprintf(charTime, "%ld", curTime);
	param.push_back(charTime);
	param.push_back("1000000000000");
	param.push_back("100000000000000");
	param.push_back("100000000000000");
	param.push_back("userdefine");
	CreateRegScriptTx(param);
}
BOOST_FIXTURE_TEST_CASE(test1, CSendItem)
{
	char *argv[] = {"progname", "-datadir=D:\\bitcoin\\1"};
	int argc = sizeof(argv) / sizeof(char*);
	CBaseParams::IntialParams(argc, argv);
//	time_t t1 = string2time("2014-12-01 17:30:00","%d-%d-%d %d:%d:%d");
	int64_t runTime = GetTime()+10*10*60;   //测试十分钟
	vector<string> param;
	while(GetTime()<runTime) {
		//创建客户端1->客户端2的普通交易
		CSendItem sendItem = CSendItem::GetRandomSendItem(1);
		CSendItem recItem = CSendItem::GetRandomSendItem(2);
		param.clear();
		param.push_back(sendItem.GetAddress());      	//源地址
		param.push_back(recItem.GetAddress());    	    //目的地址
		param.push_back(recItem.GetSendValue());	    //转账金额
		CreateNormalTx(param);                          //创建普通交易
		Sleep(1000);

		//创建客户端1->客户端2的合约交易
		CSendItem sendItem1 = CSendItem::GetRandomSendItem(1);
		CSendItem recItem1 = CSendItem::GetRandomSendItem(2);
		CSendItem recItem2 = CSendItem::GetRandomSendItem(2);
		CSendItem recItem3 = CSendItem::GetRandomSendItem(2);
		param.clear();
		param.push_back("020000000100");                     //脚本ID
		param.push_back("[\""+sendItem1.GetAddress()+"\"]"); //交易发起地址
		vector<unsigned char> vContranct;
		vContranct.clear();
		vector<unsigned char> vTemp;
		vTemp.clear();
		recItem1.GetContranctData(vTemp);
		vContranct.insert(vContranct.end(), vTemp.begin(), vTemp.end());
		vTemp.clear();
		recItem2.GetContranctData(vTemp);
		vContranct.insert(vContranct.end(), vTemp.begin(), vTemp.end());
		vTemp.clear();
		recItem3.GetContranctData(vTemp);
		vContranct.insert(vContranct.end(), vTemp.begin(), vTemp.end());
		param.push_back(HexStr(vContranct));			//合约内容
		cout << HexStr(vContranct) << endl;
		param.push_back("100000000");					//手续费
		param.push_back("0");                           //有效高度
		CreateContractTx(param);                        //创建合约交易
		Sleep(1000);
	}
}
BOOST_AUTO_TEST_CASE(test2)
{
	char *argv[] = {"progname", "-datadir=D:\\bitcoin\\2"};
	int argc = sizeof(argv) / sizeof(char*);
	CBaseParams::IntialParams(argc, argv);
	int64_t runTime = GetTime()+10*10*60;   //测试十分钟
	vector<string> param;
	while(GetTime()<runTime) {
		//创建客户端2->客户端3的普通交易
		CSendItem sendItem = CSendItem::GetRandomSendItem(2);
		CSendItem recItem = CSendItem::GetRandomSendItem(3);
		param.clear();
		param.push_back(sendItem.GetAddress());      	//源地址
		param.push_back(recItem.GetAddress());    	    //目的地址
		param.push_back(recItem.GetSendValue());	    //转账金额
		CreateNormalTx(param);                          //创建普通交易
		Sleep(1000);

		//创建客户端2->客户端3的合约交易
		CSendItem sendItem1 = CSendItem::GetRandomSendItem(2);
		CSendItem recItem1 = CSendItem::GetRandomSendItem(3);
		CSendItem recItem2 = CSendItem::GetRandomSendItem(3);
		CSendItem recItem3 = CSendItem::GetRandomSendItem(3);
		param.clear();
		param.push_back("020000000100");                     //脚本ID
		param.push_back("[\""+sendItem1.GetAddress()+"\"]"); //交易发起地址
		vector<unsigned char> vContranct;
		vContranct.clear();
		vector<unsigned char> vTemp;
		vTemp.clear();
		recItem1.GetContranctData(vTemp);
		vContranct.insert(vContranct.end(), vTemp.begin(), vTemp.end());
		vTemp.clear();
		recItem2.GetContranctData(vTemp);
		vContranct.insert(vContranct.end(), vTemp.begin(), vTemp.end());
		recItem3.GetContranctData(vTemp);
		vContranct.insert(vContranct.end(), vTemp.begin(), vTemp.end());
		param.push_back(HexStr(vContranct));			//合约内容
		param.push_back("100000000");					//手续费
		param.push_back("0");                           //有效高度
		CreateContractTx(param);                        //创建合约交易
		Sleep(1000);
	}

}
BOOST_AUTO_TEST_CASE(test3)
{
	char *argv[] = {"progname", "-datadir=D:\\bitcoin\\3"};
	int argc = sizeof(argv) / sizeof(char*);
	CBaseParams::IntialParams(argc, argv);
	int64_t runTime = GetTime()+10*10*60;   //测试十分钟
	vector<string> param;
	while(GetTime()<runTime) {
		//创建客户端3->客户端4的普通交易
		CSendItem sendItem = CSendItem::GetRandomSendItem(3);
		CSendItem recItem = CSendItem::GetRandomSendItem(4);
		param.clear();
		param.push_back(sendItem.GetAddress());      	//源地址
		param.push_back(recItem.GetAddress());    	    //目的地址
		param.push_back(recItem.GetSendValue());	    //转账金额
		CreateNormalTx(param);                          //创建普通交易
		Sleep(1000);

		//创建客户端3->客户端4的合约交易
		CSendItem sendItem1 = CSendItem::GetRandomSendItem(3);
		CSendItem recItem1 = CSendItem::GetRandomSendItem(4);
		CSendItem recItem2 = CSendItem::GetRandomSendItem(4);
		CSendItem recItem3 = CSendItem::GetRandomSendItem(4);
		param.clear();
		param.push_back("020000000100");                     //脚本ID
		param.push_back("[\""+sendItem1.GetAddress()+"\"]"); //交易发起地址
		vector<unsigned char> vContranct;
		vContranct.clear();
		vector<unsigned char> vTemp;
		vTemp.clear();
		recItem1.GetContranctData(vTemp);
		vContranct.insert(vContranct.end(), vTemp.begin(), vTemp.end());
		vTemp.clear();
		recItem2.GetContranctData(vTemp);
		vContranct.insert(vContranct.end(), vTemp.begin(), vTemp.end());
		recItem3.GetContranctData(vTemp);
		vContranct.insert(vContranct.end(), vTemp.begin(), vTemp.end());
		param.push_back(HexStr(vContranct));			//合约内容
		param.push_back("100000000");					//手续费
		param.push_back("0");                           //有效高度
		CreateContractTx(param);                        //创建合约交易
		Sleep(1000);
	}
}

BOOST_AUTO_TEST_CASE(test4)
{

	char *argv[] = {"progname", "-datadir=D:\\bitcoin\\4"};
	int argc = sizeof(argv) / sizeof(char*);
	CBaseParams::IntialParams(argc, argv);
	int64_t runTime = GetTime()+10*10*60;   //测试十分钟
	vector<string> param;
	while(GetTime()<runTime) {
		//创建客户端4->客户端5的普通交易
		CSendItem sendItem = CSendItem::GetRandomSendItem(4);
		CSendItem recItem = CSendItem::GetRandomSendItem(5);
		param.clear();
		param.push_back(sendItem.GetAddress());      	//源地址
		param.push_back(recItem.GetAddress());    	    //目的地址
		param.push_back(recItem.GetSendValue());	    //转账金额
		CreateNormalTx(param);                          //创建普通交易
		Sleep(1000);

		//创建客户端4->客户端5的合约交易
		CSendItem sendItem1 = CSendItem::GetRandomSendItem(4);
		CSendItem recItem1 = CSendItem::GetRandomSendItem(5);
		CSendItem recItem2 = CSendItem::GetRandomSendItem(5);
		CSendItem recItem3 = CSendItem::GetRandomSendItem(5);
		param.clear();
		param.push_back("020000000100");                     //脚本ID
		param.push_back("[\""+sendItem1.GetAddress()+"\"]"); //交易发起地址
		vector<unsigned char> vContranct;
		vContranct.clear();
		vector<unsigned char> vTemp;
		vTemp.clear();
		recItem1.GetContranctData(vTemp);
		vContranct.insert(vContranct.end(), vTemp.begin(), vTemp.end());
		vTemp.clear();
		recItem2.GetContranctData(vTemp);
		vContranct.insert(vContranct.end(), vTemp.begin(), vTemp.end());
		recItem3.GetContranctData(vTemp);
		vContranct.insert(vContranct.end(), vTemp.begin(), vTemp.end());
		param.push_back(HexStr(vContranct));			//合约内容
		param.push_back("100000000");					//手续费
		param.push_back("0");                           //有效高度
		CreateContractTx(param);                        //创建合约交易
		Sleep(1000);
	}
}
BOOST_AUTO_TEST_CASE(test5)
{

	char *argv[] = {"progname", "-datadir=D:\\bitcoin\\5"};
	int argc = sizeof(argv) / sizeof(char*);
	CBaseParams::IntialParams(argc, argv);

	int64_t runTime = GetTime()+10*10*60;   //测试十分钟
	vector<string> param;
	while(GetTime()<runTime) {
		//创建客户端5->客户端1的普通交易
		CSendItem sendItem = CSendItem::GetRandomSendItem(5);
		CSendItem recItem = CSendItem::GetRandomSendItem(1);
		param.clear();
		param.push_back(sendItem.GetAddress());      	//源地址
		param.push_back(recItem.GetAddress());    	    //目的地址
		param.push_back(recItem.GetSendValue());	    //转账金额
		CreateNormalTx(param);                          //创建普通交易
		Sleep(1000);

		//创建客户端5->客户端1的合约交易
		CSendItem sendItem1 = CSendItem::GetRandomSendItem(5);
		CSendItem recItem1 = CSendItem::GetRandomSendItem(1);
		CSendItem recItem2 = CSendItem::GetRandomSendItem(1);
		CSendItem recItem3 = CSendItem::GetRandomSendItem(1);
		param.clear();
		param.push_back("020000000100");                     //脚本ID
		param.push_back("[\""+sendItem1.GetAddress()+"\"]"); //交易发起地址
		vector<unsigned char> vContranct;
		vContranct.clear();
		vector<unsigned char> vTemp;
		vTemp.clear();
		recItem1.GetContranctData(vTemp);
		vContranct.insert(vContranct.end(), vTemp.begin(), vTemp.end());
		vTemp.clear();
		recItem2.GetContranctData(vTemp);
		vContranct.insert(vContranct.end(), vTemp.begin(), vTemp.end());
		recItem3.GetContranctData(vTemp);
		vContranct.insert(vContranct.end(), vTemp.begin(), vTemp.end());
		param.push_back(HexStr(vContranct));			//合约内容
		param.push_back("100000000");					//手续费
		param.push_back("0");                           //有效高度
		CreateContractTx(param);                        //创建合约交易
		Sleep(1000);
	}

}

BOOST_AUTO_TEST_SUITE_END()
