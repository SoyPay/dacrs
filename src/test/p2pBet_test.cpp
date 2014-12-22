/*
 * p2pBet_test.cpp
 *
 *  Created on: 2014年12月11日
 *      Author: spark.huang
 */
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

using namespace std;
using namespace boost;

enum OperAccountType {
	ADDFREE = 1, //!< ADD_FREE
	MINUSFREE,   //!< MINUS_FREE
	ADDSELF,     //!< ADD_SELF
	MINUSSELF,   //!< MINUS_SELF
	ADDFREEZD,   //!< ADD_FREEZD
	MINUSFREEZD, //!< MINUS_FREEZD
	NULLOPERTYPE,//!< NULL_OPERTYPE
};

//enum OPERATETYPE {
//	SEND,
//	ACCEPT,
//	OPEN
//};
struct AccINFO {
	int64_t RewardFund;		//!< reward money
	int64_t FreedomFund;		//!< freedom money
	int64_t Freeze;			//!< freezed money
	int64_t SelfFreeze;		//!< self-freeze money
	AccINFO() {
		RewardFund = 0;
		FreedomFund = 0;
		Freeze = 0;
		SelfFreeze = 0;
	}
	AccINFO(int64_t ref, int64_t frf, int64_t frez, int64_t self) {
		RewardFund = ref;
		FreedomFund = frf;
		Freeze = frez;
		SelfFreeze = self;
	}

	bool SetInfo(int64_t ref, int64_t frf, int64_t frez, int64_t self) {
			RewardFund = ref;
			FreedomFund = frf;
			Freeze = frez;
			SelfFreeze = self;
			return true;
		}

	bool operator==(AccINFO &a) {
		if (RewardFund == a.RewardFund && FreedomFund == a.FreedomFund && Freeze == a.Freeze
				&& SelfFreeze == a.SelfFreeze) {
			return true;
		}
		return false;
	}

//	bool SumEqual(AccINFO &a)
//	{
//		int64_t l = dUnmatureMoney+dFreeMoney+dFrozenMoney;
//		int64_t r = a.dUnmatureMoney+a.dFreeMoney+a.dFrozenMoney;
//
//		return l==r;
//	}

	AccINFO& operator+=(AccINFO &a) {
		RewardFund += a.RewardFund;
		FreedomFund += a.FreedomFund;
		Freeze += a.Freeze;
		SelfFreeze += a.SelfFreeze;
		return *this;
	}
};

struct AccINFOOperLog {
	std::map<int, AccINFO> mapAccState;
	AccINFOOperLog() {
		mapAccState.clear();
	}
	bool Add(int &nHeight, AccINFO &accstate) {
		mapAccState[nHeight] = accstate;
		return true;
	}

//	void MergeAcc(int nHeight)
//	{
//		for(auto &item:mapAccState)
//		{
//			if(nHeight > item.first+nFrozenHeight)
//			{
//				item.second.dFreeMoney += item.second.dFrozenMoney;
//				item.second.dFrozenMoney = 0.0;
//			}
//
//			if(nHeight > item.first+nMatureHeight)
//			{
//				item.second.dFreeMoney += item.second.dUnmatureMoney;
//				item.second.dUnmatureMoney = 0.0;
//			}
//		}
//	}
};

class Cp2pBetTest: public SysTestBase {
public:
	std::map<string, AccINFO> mapAccState;
	std::map<string, AccINFOOperLog> mapAccOperLog;
	int operatetimes;

	string scriptid;
	string sendtxhash;
	uint64_t betamount;
	unsigned char sdata[5];
	unsigned char rdata[5];

	Cp2pBetTest() {
		mapAccState.clear();
		mapAccOperLog.clear();
		operatetimes = 0;
		scriptid.clear();
		sendtxhash.clear();
		betamount = 0;
		memset(sdata, 0, sizeof(sdata));
		memset(rdata, 0, sizeof(rdata));
	}

	bool clear()
	{
		mapAccState.clear();
		mapAccOperLog.clear();
		operatetimes = 0;
		scriptid.clear();
		sendtxhash.clear();
		betamount = 0;
		memset(sdata, 0, sizeof(sdata));
		memset(rdata, 0, sizeof(rdata));
		return true;
	}

	~Cp2pBetTest() {
//		StopServer();
	}

	uint64_t GetRandomBetAmount() {
		srand(time(NULL));
		int r = (rand() % 1000000) + 500000;
		return r;
	}

	uint64_t GetRandomBetfee() {
			srand(time(NULL));
			int r = (rand() % 1000000) + 1000000;
			return r;
		}

//注意，我这里将自由金额和超过30天的金额算在一起，如不注意滥用，后果自负
	bool GetAccInfo(const std::string &addr, AccINFO &accstate) {
		//CommanRpc
		char temp[64] = { 0 };
		strncpy(temp, addr.c_str(), sizeof(temp) - 1);

		char *argv[] = { "rpctest", "getaccountinfo", temp };
		int argc = sizeof(argv) / sizeof(char*);
		Value value;

		if (CommandLineRPC_GetValue(argc, argv, value)) {
			Object obj = value.get_obj();
			{
				Array tmp = find_value(obj, "RewardFund").get_array();
				for (auto &fund : tmp) {
					accstate.RewardFund += find_value(fund.get_obj(), "value").get_int64();
				}
			}
			{
				Array tmp = find_value(obj, "FreedomFund").get_array();
				for (auto &fund : tmp) {
					accstate.FreedomFund += find_value(fund.get_obj(), "value").get_int64();
				}
			}
			{
				accstate.FreedomFund += find_value(obj, "FreeValues").get_int64();
			}
			{
				Array tmp = find_value(obj, "Freeze").get_array();
				for (auto &fund : tmp) {
					accstate.Freeze += find_value(fund.get_obj(), "value").get_int64();
				}
			}
			{
				Array tmp = find_value(obj, "SelfFreeze").get_array();
				for (auto &fund : tmp) {
					accstate.SelfFreeze += find_value(fund.get_obj(), "value").get_int64();
				}
			}

//			LogPrint("spark", "\r\n addr:%s"
//					"\r\n accstate.RewardFund:%I"
//					"\r\n accstate.FreedomFund:%I"
//					"\r\n accstate.Freeze:%I"
//					"\r\n accstate.SelfFreeze:%I"
//					"\r\n",
//					addr,
//					accstate.RewardFund, accstate.FreedomFund,
//					accstate.Freeze, accstate.SelfFreeze
//					);
			return true;
		}
		return false;
	}

	bool CheckAccState(const string &addr, AccINFO &lastState) {
		AccINFO initState = mapAccState[addr];
		AccINFOOperLog operlog = mapAccOperLog[addr];

		for (auto & item : operlog.mapAccState) {
			initState += item.second;
		}

		bool b = false;

		LogPrint("spark", "\r\n ************CheckAccState:"
				"\r\n addr:%s"
				"\r\n lastState.RewardFund:%I"
				"\r\n lastState.FreedomFund:%I"
				"\r\n lastState.Freeze:%I"
				"\r\n lastState.SelfFreeze:%I"
				"\r\n", addr, lastState.RewardFund, lastState.FreedomFund, lastState.Freeze, lastState.SelfFreeze);

		LogPrint("spark", "\r\n ************CheckAccState:"
				"\r\n addr:%s"
				"\r\n initState.RewardFund:%I"
				"\r\n initState.FreedomFund:%I"
				"\r\n initState.Freeze:%I"
				"\r\n initState.SelfFreeze:%I"
				"\r\n", addr, initState.RewardFund, initState.FreedomFund, initState.Freeze, initState.SelfFreeze);

		b = (initState == lastState);

		return b;
	}

public:
	bool GetHashFromCreatedTx(const Value& valueRes, string& strHash) {
		if (valueRes.type() == null_type) {
			return false;
		}

		const Value& result = find_value(valueRes.get_obj(), "hash");
		if (result.type() == null_type) {
			return false;
		}

		strHash = result.get_str();
		return true;
	}

	int GetBlockHeight() {
		return (int) chainActive.Height();
	}

	bool SetAddrGenerteBlock(char *addr) {
		char *argv[] = { "rpctest", "generateblock", addr };
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		if (CommandLineRPC_GetValue(argc, argv, value)) {
			return true;
		}
		return false;
	}

private:
	boost::thread* pThreadShutdown;
};

#pragma pack(1)
typedef struct {
	unsigned char type;
	uint64_t money;
	int hight;
	unsigned char dhash[32];IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			READWRITE(money);
			READWRITE(hight);
			for(int i = 0; i < 32; i++)
			{
				READWRITE(dhash[i]);
			}
	)
} SEND_DATA;

typedef struct {
	unsigned char type;
	uint64_t money;
	unsigned char targetkey[32];		//发起对赌的哈希，也是对赌数据的关键字
	unsigned char dhash[32];IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			READWRITE(money);
			for(int i = 0; i < 32; i++)
			{
				READWRITE(targetkey[i]);
			}
			for(int ii = 0; ii < 32; ii++)
			{
				READWRITE(dhash[ii]);
			}
	)
} ACCEPT_DATA;

typedef struct {
	unsigned char type;
	unsigned char targetkey[32];		//发起对赌的哈希，也是对赌数据的关键字
	unsigned char dhash[5];IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			for(int i = 0; i < 32; i++)
			{
				READWRITE(targetkey[i]);
			}
			for(int ii = 0; ii < 5; ii++)
			{
				READWRITE(dhash[ii]);
			}
	)
} OPEN_DATA;

#define MINERADDR "mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc"
#define ADDR_REG  "mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA"
#define ADDR_A    "mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5"
#define VADDR_A   "[\"mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5\"]"
#define ADDR_B    "mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y"
#define VADDR_B   "[\"mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y\"]"

static Cp2pBetTest gTestValue;

BOOST_AUTO_TEST_SUITE(p2pBetTest)

static bool RegScript(void) {
	string strFileName("p2pbet.bin");
	int nFee = gTestValue.GetRandomBetfee();
	//注册对赌脚本
	Value valueRes = gTestValue.RegisterScriptTx(ADDR_REG, strFileName, gTestValue.GetBlockHeight(), nFee);
	string regTxHash;
	BOOST_CHECK(gTestValue.GetHashFromCreatedTx(valueRes, regTxHash));

	do {
		BOOST_CHECK(gTestValue.SetAddrGenerteBlock(MINERADDR));
	} while (!pwalletMain->UnConfirmTx.empty());

	assert(pScriptDBTip->Flush());

	return true;
}

static bool GetRandomBetData(unsigned char *buf, int num)
{
	RAND_bytes(buf, num);
	return true;
}

static bool GetABInitInfo(void)
{
	AccINFO initStateA;
	BOOST_REQUIRE(gTestValue.GetAccInfo(ADDR_A, initStateA));
	gTestValue.mapAccState[ADDR_A] = initStateA;

	AccINFO initStateB;
	BOOST_REQUIRE(gTestValue.GetAccInfo(ADDR_B, initStateB));
	gTestValue.mapAccState[ADDR_B] = initStateB;
	return true;
}

static bool OperateAccount(const string &addr, const OperAccountType &operate, const uint64_t &amount, const uint64_t &fee)
{
	AccINFOOperLog &operlog = gTestValue.mapAccOperLog[addr];

	AccINFO acc;
	switch (operate) {
	case ADDFREE:
		acc.SetInfo(0, amount-fee, 0, 0);
		break;
	case MINUSFREE:
		acc.SetInfo(0, -amount-fee, 0, 0);
		break;
	case ADDFREEZD:
		acc.SetInfo(0, -fee, amount, 0);
		break;
	case MINUSFREEZD:
		acc.SetInfo(0, -fee, -amount, 0);
		break;
	case ADDSELF:
		acc.SetInfo(0, -fee, 0, amount);
		break;
	case MINUSSELF:
		acc.SetInfo(0, -fee, 0, -amount);
		break;
		defualt: break;
	}

	operlog.Add(gTestValue.operatetimes, acc);
	gTestValue.operatetimes++;

	return true;
}

static bool CheckAccountInfo(void)
{
	for(auto tmp : gTestValue.mapAccState)
	{
		AccINFO tmpaccount;
		BOOST_CHECK(gTestValue.GetAccInfo(tmp.first, tmpaccount));
		BOOST_CHECK(gTestValue.CheckAccState(tmp.first, tmpaccount));
	}
	return true;
}

static bool ASendP2PBet(int shight) {
	GetRandomBetData(gTestValue.sdata, sizeof(gTestValue.sdata));
	BOOST_CHECK(gTestValue.GetOneScriptId(gTestValue.scriptid));
//	printf("\r\n***************the script id:%s \r\n", gTestValue.scriptid.c_str());
	SEND_DATA senddata;
	senddata.type = 1;
	senddata.hight = shight;
	senddata.money = gTestValue.GetRandomBetAmount();
	gTestValue.betamount = senddata.money;
	memcpy(senddata.dhash, Hash(gTestValue.sdata, gTestValue.sdata + sizeof(gTestValue.sdata)).begin(), sizeof(senddata.dhash));

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t sendfee = gTestValue.GetRandomBetfee();
	Value vsend = gTestValue.PCreateContractTx(gTestValue.scriptid, VADDR_A, sendcontract, gTestValue.GetBlockHeight(),
			sendfee);
	BOOST_CHECK(gTestValue.GetHashFromCreatedTx(vsend, gTestValue.sendtxhash));

	GetABInitInfo();

	OperateAccount(ADDR_A, MINUSFREE, senddata.money, sendfee);
	OperateAccount(ADDR_A, ADDFREEZD, senddata.money, 0);

	do {
		BOOST_CHECK(gTestValue.SetAddrGenerteBlock(MINERADDR));
	} while (!pwalletMain->UnConfirmTx.empty());

	CheckAccountInfo();

	return true;
}

static bool BAcceptP2PBet(void) {
	GetRandomBetData(gTestValue.rdata, sizeof(gTestValue.rdata));
	ACCEPT_DATA acceptdata;
	acceptdata.type = 2;
	acceptdata.money = gTestValue.betamount;
	memcpy(acceptdata.targetkey, uint256(gTestValue.sendtxhash).begin(), sizeof(acceptdata.targetkey));
	memcpy(acceptdata.dhash, Hash(gTestValue.rdata, gTestValue.rdata + sizeof(gTestValue.rdata)).begin(), sizeof(acceptdata.dhash));
	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << acceptdata;
	string acceptcontract = HexStr(scriptData);
	uint64_t acceptfee = gTestValue.GetRandomBetfee();
	Value vaccept = gTestValue.PCreateContractTx(gTestValue.scriptid, VADDR_B, acceptcontract,
			gTestValue.GetBlockHeight(), acceptfee);
	string acceptTxHash;
	BOOST_CHECK(gTestValue.GetHashFromCreatedTx(vaccept, acceptTxHash));


	OperateAccount(ADDR_B, MINUSFREE, acceptdata.money, acceptfee);
	OperateAccount(ADDR_B, ADDFREEZD, acceptdata.money, 0);

	do {
		BOOST_CHECK(gTestValue.SetAddrGenerteBlock(MINERADDR));
	} while (!pwalletMain->UnConfirmTx.empty());

	CheckAccountInfo();
	return true;
}

static bool IsSendIDWin(void)
{
	unsigned char rslt = 0;

	for(int ii = 0; ii < 5; ii++)
	{
		LogPrint("p2p", "\r\nsdata:%x\r\n", gTestValue.sdata[ii]);
	}

	for(int kk = 0; kk < 5; kk++)
	{
		LogPrint("p2p", "\r\nrdata:%x\r\n", gTestValue.rdata[kk]);
	}

	for (auto tmp : gTestValue.sdata) {
		rslt += tmp;
	}

	for (auto tmp : gTestValue.rdata) {
		rslt += tmp;
	}
	LogPrint("p2p", "\r\nrslt:%x\r\n", rslt);
	return (rslt%2 == 1)?(true):(false);
}

static bool AOpenP2PBet(const bool isfistopen) {
	OPEN_DATA openA;
	openA.type = 3;
	memcpy(openA.targetkey, uint256(gTestValue.sendtxhash).begin(), sizeof(openA.targetkey));
	memcpy(openA.dhash, gTestValue.sdata, sizeof(gTestValue.sdata));
	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << openA;
	string openAcontract = HexStr(scriptData);
	uint64_t openfee = gTestValue.GetRandomBetfee();
	Value vopenA = gTestValue.PCreateContractTx(gTestValue.scriptid, VADDR_A, openAcontract,
			gTestValue.GetBlockHeight(), openfee);
	string openATxHash;
	BOOST_CHECK(gTestValue.GetHashFromCreatedTx(vopenA, openATxHash));

	if(!isfistopen)
	{
		if (!IsSendIDWin())		//B win
		{
			OperateAccount(ADDR_A, MINUSFREEZD, 2*gTestValue.betamount, 0);
			OperateAccount(ADDR_B, ADDFREE, 2*gTestValue.betamount, 0);
			OperateAccount(ADDR_A, MINUSFREE, 0, openfee);
		} else {
			OperateAccount(ADDR_A, MINUSFREEZD, 2*gTestValue.betamount, 0);
			OperateAccount(ADDR_A, ADDFREE, 2*gTestValue.betamount, 0);
			OperateAccount(ADDR_A, MINUSFREE, 0, openfee);
		}
	}
	else
	{
		OperateAccount(ADDR_A, ADDFREEZD, gTestValue.betamount, openfee);
		OperateAccount(ADDR_B, MINUSFREEZD, gTestValue.betamount, 0);
	}

	do {
		BOOST_CHECK(gTestValue.SetAddrGenerteBlock(MINERADDR));
	} while (!pwalletMain->UnConfirmTx.empty());

	CheckAccountInfo();
	return true;
}

static bool BOpenP2PBet(const bool isfistopen) {
	OPEN_DATA openB;
	openB.type = 3;
	memcpy(openB.targetkey, uint256(gTestValue.sendtxhash).begin(), sizeof(openB.targetkey));
	memcpy(openB.dhash, gTestValue.rdata, sizeof(gTestValue.rdata));
	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << openB;
	string openBcontract = HexStr(scriptData);
	uint64_t openfee = gTestValue.GetRandomBetfee();
	Value vopenB = gTestValue.PCreateContractTx(gTestValue.scriptid, VADDR_B, openBcontract,
			gTestValue.GetBlockHeight(), openfee);
	string openBTxHash;
	BOOST_CHECK(gTestValue.GetHashFromCreatedTx(vopenB, openBTxHash));

	if(!isfistopen)
	{
		if (!IsSendIDWin())		//B win
		{
			LogPrint("p2p", "\r\nB WIN\r\n");
			OperateAccount(ADDR_A, MINUSFREEZD, 2*gTestValue.betamount, 0);
			OperateAccount(ADDR_B, ADDFREE, 2*gTestValue.betamount, 0);
			OperateAccount(ADDR_B, MINUSFREE, 0, openfee);
		} else {
			LogPrint("p2p", "\r\nA WIN\r\n");
			OperateAccount(ADDR_A, MINUSFREEZD, 2*gTestValue.betamount, 0);
			OperateAccount(ADDR_A, ADDFREE, 2*gTestValue.betamount, 0);
			OperateAccount(ADDR_B, MINUSFREE, 0, openfee);
		}
	}
	else
	{
		OperateAccount(ADDR_B, ADDFREEZD, gTestValue.betamount, openfee);
		OperateAccount(ADDR_A, MINUSFREEZD, gTestValue.betamount, 0);
	}

	do {
		BOOST_CHECK(gTestValue.SetAddrGenerteBlock(MINERADDR));
	} while (!pwalletMain->UnConfirmTx.empty());

	CheckAccountInfo();
	return true;
}

static bool TestTimeOut(int hight)
{
	do {
			BOOST_CHECK(gTestValue.SetAddrGenerteBlock(MINERADDR));
		} while (gTestValue.GetBlockHeight() < hight);
	BOOST_CHECK(gTestValue.SetAddrGenerteBlock(MINERADDR));
	return true;
}

static bool AccountNotTimeOut(const string &addr)
{
	OperateAccount(addr, MINUSFREEZD, 2*gTestValue.betamount, 0);
	OperateAccount(addr, ADDFREE, 2*gTestValue.betamount, 0);

	CheckAccountInfo();
	return true;
}


BOOST_AUTO_TEST_CASE(normal0)
{
	gTestValue.ResetEnv();
	gTestValue.clear();
	RegScript();
	//A、B揭赌
	ASendP2PBet(15);
	BAcceptP2PBet();
	AOpenP2PBet(true);
	BOpenP2PBet(false);
}

//账户冻结金额合并尚未解决，以下代码暂时屏蔽
#if 1
BOOST_AUTO_TEST_CASE(normal1)
{
	gTestValue.ResetEnv();
	gTestValue.clear();
	RegScript();
	//A揭赌
	ASendP2PBet(15);
	BAcceptP2PBet();
	AOpenP2PBet(true);
	TestTimeOut(15);
	AccountNotTimeOut(ADDR_A);
}

BOOST_AUTO_TEST_CASE(normal2)
{
	gTestValue.ResetEnv();
	gTestValue.clear();
	RegScript();
	//B揭赌
	ASendP2PBet(15);
	BAcceptP2PBet();
	BOpenP2PBet(true);
	TestTimeOut(15);
	AccountNotTimeOut(ADDR_B);
}
#endif

BOOST_AUTO_TEST_SUITE_END()

