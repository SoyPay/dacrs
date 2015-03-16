#include "rpcserver.h"
#include "rpcclient.h"
#include "util.h"
#include "chainparams.h"
#include "tx.h"
#include "../json/json_spirit_value.h"
#include <boost/test/unit_test.hpp>
using namespace json_spirit;

string srcAddress="mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5";
extern int GetRandomFee();
extern int CommandLineRPC_GetValue(int argc, const char *argv[], Value &value);

int GetRandomMoney() {
	srand(time(NULL));
	int r = (rand() % 100) + 100;
	return r;
}

int GetRandTxType() {
	unsigned char cType;
	RAND_bytes(&cType, sizeof(cType));
	//srand(time(NULL));
	int iIndex = cType % 4;
	return iIndex + 1;
}

bool CreateCommonTx(string srcAddr, string desAddr) {
	char fee[64] = {0};
	int nfee = GetRandomFee();
	sprintf(fee, "%d", nfee);
	char money[64] = {0};
	int nmoney = GetRandomMoney();
	sprintf(money, "%d00000000", nmoney);
	if ("" == desAddr) {
		const char *argv[] = { "rpctest", "getnewaddress" };
		int argc = sizeof(argv) / sizeof(char *);
		Value value;
		if (0 != CommandLineRPC_GetValue(argc, argv, value)) {
			return false;
		}
		const Value& retNewAddr = find_value(value.get_obj(), "addr");
		if (retNewAddr.type() == null_type) {
			return false;
		}
		desAddr = retNewAddr.get_str();
	}
	const char *argv1[] = { "rpctest", "sendtoaddresswithfee", srcAddr.c_str(), desAddr.c_str(), money, fee};
	int argc = sizeof(argv1) / sizeof(char*);
	Value value;
	if (0 != CommandLineRPC_GetValue(argc, argv1, value)) {
		return false;
	}
	const Value& result = find_value(value.get_obj(), "hash");
	if(result == null_type) {
		return false;
	}
	string txHash = result.get_str();
	cout << "CreateCommonTx: " <<txHash << " succeed!" << endl;
	return true;
}
/**
 * 创建注册账户交易
 * @param addr
 * @return
 */
bool CreateRegAcctTx() {
	//获取一个新的地址
	const char *argv[] = {"rpctest", "getnewaddress"};
	int argc = sizeof(argv) /sizeof(char *);
	Value value;
	if(0 != CommandLineRPC_GetValue(argc, argv, value))
	{
		return false;
	}
	const Value& retNewAddr = find_value(value.get_obj(), "addr");
	if(retNewAddr.type() == null_type) {
		return false;
	}
	string newAddress = retNewAddr.get_str();
	//向新产生地址发送一笔钱
	if(!CreateCommonTx(srcAddress, newAddress))
		return false;

	char fee[64] = {0};
	int nfee = GetRandomFee();
	sprintf(fee, "%d", nfee);

	//新产生地址注册账户
	const char *argvReg[] = {"rpctest", "registaccounttx", newAddress.c_str(), fee, "false"};
	int argcReg = sizeof(argvReg) / sizeof(char *);
	if(0 != CommandLineRPC_GetValue(argcReg, argvReg, value))
	{
		return false;
	}
	const Value& result = find_value(value.get_obj(), "hash");
	if(result.type() == null_type) {
		return false;
	}
	string txHash = result.get_str();
	cout << "CreateRegAcctTx: " <<txHash << " succeed!" << endl;
	return true;
}

/**
 * 创建主动冻结交易
 * @return
 */
bool CreateFreezeTx() {
	char money[64] = {0};
	int frozenmoney = GetRandomMoney();
	sprintf(money, "%d00000000", frozenmoney);
	char fee[64] = {0};
	int nfee = GetRandomFee();
	sprintf(fee, "%d", nfee);
	int freeHeight = rand() % 90 + 10;
	char free[64] = {0};
	sprintf(free, "%d", freeHeight);
	const char *argv[] = {"rpctest", "createfreezetx", srcAddress.c_str(), money, fee, "0", free};
	int argc = sizeof(argv) / sizeof(char *);
	Value value;
	if (0 != CommandLineRPC_GetValue(argc, argv, value)) {
		return false;
	}
	const Value& result = find_value(value.get_obj(), "hash");
	if(result.type() == null_type) {
		return false;
	}
	string txHash = result.get_str();
	cout << "CreateFreezeTx: " <<txHash << " succeed!" << endl;
	return true;
}

bool CreateRegistScriptTx() {
	char fee[64] = {0};
	int nfee = GetRandomFee();
	sprintf(fee, "%d", nfee);
	string scriptPath = SysCfg().GetDefaultTestDataPath() + "test.bin";

	const char *argv[] = {"rpctest", "registerscripttx", srcAddress.c_str(), "0", scriptPath.c_str(), fee, "0"};
	int argc = sizeof(argv) / sizeof(char *);
	Value value;
	if (0 != CommandLineRPC_GetValue(argc, argv, value)) {
		return false;
	}
	const Value& result = find_value(value.get_obj(), "hash");
	if(result.type() == null_type) {
		return false;
	}
	string txHash = result.get_str();
	cout << "createRegScriptTx: " <<txHash << " succeed!" << endl;
	return true;
}

BOOST_AUTO_TEST_SUITE(create_tx_tests)
BOOST_AUTO_TEST_CASE(tests)
{
	while(true) {
		int nTxType = GetRandTxType();
		switch(nTxType) {
		case 1:
			{
				BOOST_CHECK(CreateRegAcctTx());
			}
			break;
		case 2:
			{
				BOOST_CHECK(CreateCommonTx(srcAddress, ""));
			}
			break;
		case 3:
			{
				BOOST_CHECK(CreateFreezeTx());
			}
			break;
		case 4:
			{
				BOOST_CHECK(CreateRegistScriptTx());
			}
			break;
		default:
			assert(0);
		}
		MilliSleep(500);
	}
}
BOOST_AUTO_TEST_CASE(test1) {
	CAccount account;
	try {
			boost::filesystem::path workpath = GetDataDir() / "accountdata.txt";
			FILE* fp = fopen(workpath.string().c_str(), "rb");
			fseek(fp, 0, SEEK_SET);
			CAutoFile fileout = CAutoFile(fp, SER_DISK, CLIENT_VERSION);
			if (!fileout)
				return;
			vector<char> vCh;
			fileout >> vCh;
			CDataStream ds1(vCh, SER_DISK, CLIENT_VERSION);
			ds1 >> account;
			fclose(fp);
		} catch (std::exception &e) {
			LogPrint("INFO", "save account error:%s\n", e.what());
		}
	cout << account.ToString() << endl;
	cout << "size:" << account.GetSerializeSize(SER_DISK,CLIENT_VERSION) << endl;
	int64_t llTime(0);
	for(int i=0; i<10000; ++i) {
		CAccount accountTemp;
		llTime = GetTimeMillis();
		accountTemp = account;
		cout << "assigned account elapse time: " << GetTimeMillis() - llTime << "ms" << endl;
	}
}
BOOST_AUTO_TEST_SUITE_END()
