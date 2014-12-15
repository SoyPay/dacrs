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

//enum OPERATETYPE {
//	SEND,
//	ACCEPT,
//	OPEN
//};
struct AccINFO{
	int64_t RewardFund;		//!< reward money
	int64_t FreedomFund;		//!< freedom money
	int64_t Freeze;			//!< freezed money
	int64_t SelfFreeze;		//!< self-freeze money
	AccINFO()
	{
		RewardFund = 0;
		FreedomFund = 0;
		Freeze = 0;
		SelfFreeze = 0;
	}
	AccINFO(int64_t ref, int64_t frf, int64_t frez, int64_t self)
	{
		RewardFund = ref;
		FreedomFund = frf;
		Freeze = frez;
		SelfFreeze = self;
	}

	bool operator==(AccINFO &a)
	{
		if(RewardFund == a.RewardFund &&
		FreedomFund == a.FreedomFund &&
		Freeze == a.Freeze &&
		SelfFreeze == a.SelfFreeze)
		{
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

	AccINFO& operator+=(AccINFO &a)
	{
		RewardFund += a.RewardFund;
		FreedomFund += a.FreedomFund;
		Freeze += a.Freeze;
		SelfFreeze += a.SelfFreeze;
		return *this;
	}
};

struct AccINFOOperLog{
	std::map<int, AccINFO> mapAccState;
	AccINFOOperLog()
	{
		mapAccState.clear();
	}
	bool Add(int &nHeight, AccINFO &accstate)
	{
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

class Cp2pBetTest:public SysTestBase
{
public:
	std::map<string,AccINFO> mapAccState;
	std::map<string,AccINFOOperLog> mapAccOperLog;

	Cp2pBetTest() {
		mapAccState.clear();
		mapAccOperLog.clear();
	}

	~Cp2pBetTest(){
//		StopServer();
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

	bool CheckAccState(const string &addr, AccINFO &lastState)
	{
		AccINFO initState = mapAccState[addr];
		AccINFOOperLog operlog = mapAccOperLog[addr];

		for(auto & item:operlog.mapAccState)
		{
			initState += item.second;
		}


		bool b = false;

		LogPrint("spark", "\r\n ************CheckAccState:"
							"\r\n addr:%s"
							"\r\n lastState.RewardFund:%I"
							"\r\n lastState.FreedomFund:%I"
							"\r\n lastState.Freeze:%I"
							"\r\n lastState.SelfFreeze:%I"
							"\r\n",
							addr,
							lastState.RewardFund, lastState.FreedomFund,
							lastState.Freeze, lastState.SelfFreeze
							);

		LogPrint("spark", "\r\n ************CheckAccState:"
							"\r\n addr:%s"
							"\r\n initState.RewardFund:%I"
							"\r\n initState.FreedomFund:%I"
							"\r\n initState.Freeze:%I"
							"\r\n initState.SelfFreeze:%I"
							"\r\n",
							addr,
							initState.RewardFund, initState.FreedomFund,
							initState.Freeze, initState.SelfFreeze
							);

		b = (initState == lastState);

		return b;
	}

public:
	bool GetHashFromCreatedTx(const Value& valueRes,string& strHash)
	{
		if (valueRes.type() == null_type) {
			return false;
		}

		const Value& result = find_value(valueRes.get_obj(), "hash");
		if (result.type() == null_type){
			return false;
		}

		strHash = result.get_str();
		return true;
	}

	int GetBlockHeight()
	{
		return  (int)chainActive.Height();
	}

	bool SetAddrGenerteBlock(char *addr)
	{
		char *argv[] = { "rpctest", "generateblock", addr};
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
	unsigned char dhash[32];
	IMPLEMENT_SERIALIZE
	(
		READWRITE(type);
		READWRITE(money);
		READWRITE(hight);
		for(int i = 0; i < 32; i++)
		{
			READWRITE(dhash[i]);
		}
	)
}SEND_DATA;

typedef struct {
	unsigned char type;
	uint64_t money;
	unsigned char targetkey[32];//发起对赌的哈希，也是对赌数据的关键字
	unsigned char dhash[32];
	IMPLEMENT_SERIALIZE
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
}ACCEPT_DATA;

typedef struct {
	unsigned char type;
	unsigned char targetkey[32];//发起对赌的哈希，也是对赌数据的关键字
	unsigned char dhash[5];
	IMPLEMENT_SERIALIZE
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
}OPEN_DATA;


typedef struct {
	string scriptid;
	string sendtxhash;
	unsigned char sdata[5];
	unsigned char rdata[5];
}RUN_CTX;


RUN_CTX gRunCtxData;

#define MINERADDR "mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc"
#define ADDR_REG  "mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA"
#define ADDR_A    "mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5"
#define VADDR_A   "[\"mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5\"]"
#define ADDR_B    "mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y"
#define VADDR_B   "[\"mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y\"]"

Cp2pBetTest gTestValue;

BOOST_AUTO_TEST_SUITE(p2pBetTest)

//注册对赌脚本
BOOST_AUTO_TEST_CASE(regscript)
{
	string strFileName("p2pbet.bin");
	int nFee = 100000;
	//注册对赌脚本
	Value valueRes = gTestValue.RegisterScriptTx(ADDR_REG, strFileName , gTestValue.GetBlockHeight(), nFee);
	string regTxHash;
	BOOST_CHECK(gTestValue.GetHashFromCreatedTx(valueRes, regTxHash));

	do
	{
		BOOST_CHECK(gTestValue.SetAddrGenerteBlock(MINERADDR));
	}
	while(!pwalletMain->UnConfirmTx.empty());

}

////挖矿产生一个BLOCk
//BOOST_FIXTURE_TEST_CASE(mining, Cp2pBetTest)
//{
//	BOOST_CHECK(GenerateOneBlock());
//}

//A基于脚本发起对赌包
BOOST_AUTO_TEST_CASE(Asendbet)
{
	unsigned char sdata[5] = {0, 1, 2, 3, 4};
	memcpy(gRunCtxData.sdata, sdata, sizeof(sdata));
	BOOST_CHECK(gTestValue.GetOneScriptId(gRunCtxData.scriptid));
	printf("\r\n***************the script id:%s \r\n", gRunCtxData.scriptid.c_str());
	SEND_DATA senddata;
	senddata.type = 1;
	senddata.hight = 50;
	senddata.money = 500000;
	memcpy(senddata.dhash, Hash(sdata, sdata + sizeof(sdata)).begin(), sizeof(senddata.dhash));

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t sendfee = 1000000;
	Value vsend = gTestValue.PCreateContractTx(gRunCtxData.scriptid, VADDR_A, sendcontract, gTestValue.GetBlockHeight(), sendfee);
	BOOST_CHECK(gTestValue.GetHashFromCreatedTx(vsend, gRunCtxData.sendtxhash));
	{
		AccINFO initStateA;
		BOOST_REQUIRE(gTestValue.GetAccInfo(ADDR_A, initStateA));
		gTestValue.mapAccState[ADDR_A] = initStateA;

		AccINFO initStateB;
		BOOST_REQUIRE(gTestValue.GetAccInfo(ADDR_B, initStateB));
		gTestValue.mapAccState[ADDR_B] = initStateB;
	}

	{
		int temphight = gTestValue.GetBlockHeight();
		AccINFOOperLog &operlog = gTestValue.mapAccOperLog[ADDR_A];
		AccINFO acc(0, -500000 - sendfee, 500000, 0);
		operlog.Add(temphight, acc);
	}

	do
	{
		BOOST_CHECK(gTestValue.SetAddrGenerteBlock(MINERADDR));
	}
	while(!pwalletMain->UnConfirmTx.empty());

	{
		AccINFO lastStateA;
		BOOST_REQUIRE(gTestValue.GetAccInfo(ADDR_A,lastStateA));
		BOOST_REQUIRE(gTestValue.CheckAccState(ADDR_A,lastStateA));

		AccINFO lastStateB;
		BOOST_REQUIRE(gTestValue.GetAccInfo(ADDR_B,lastStateB));
		BOOST_REQUIRE(gTestValue.CheckAccState(ADDR_B,lastStateB));
	}
}

//B基于脚本发起接受A的对赌
BOOST_AUTO_TEST_CASE(acceptbet)
{
	unsigned char rdata[5] = {0, 1, 2, 3, 5};
	memcpy(gRunCtxData.rdata, rdata, sizeof(rdata));
	ACCEPT_DATA acceptdata;
	acceptdata.type = 2;
	acceptdata.money = 500000;
	memcpy(acceptdata.targetkey, uint256(gRunCtxData.sendtxhash).begin(), sizeof(acceptdata.targetkey));
	memcpy(acceptdata.dhash, Hash(rdata, rdata + sizeof(rdata)).begin(), sizeof(acceptdata.dhash));
	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << acceptdata;
	string acceptcontract = HexStr(scriptData);
	uint64_t acceptfee = 1000000;
	Value vaccept = gTestValue.PCreateContractTx(gRunCtxData.scriptid, VADDR_B, acceptcontract, gTestValue.GetBlockHeight(), acceptfee);
	string acceptTxHash;
	BOOST_CHECK(gTestValue.GetHashFromCreatedTx(vaccept, acceptTxHash));

	{
		int temphight = gTestValue.GetBlockHeight();
		AccINFOOperLog &operlog = gTestValue.mapAccOperLog[ADDR_B];
		AccINFO acc(0, -500000-acceptfee, 500000, 0);
		operlog.Add(temphight, acc);
	}

	do
	{
		BOOST_CHECK(gTestValue.SetAddrGenerteBlock(MINERADDR));
	}
	while(!pwalletMain->UnConfirmTx.empty());

	{
		AccINFO lastStateA;
		BOOST_REQUIRE(gTestValue.GetAccInfo(ADDR_A,lastStateA));
		BOOST_REQUIRE(gTestValue.CheckAccState(ADDR_A,lastStateA));

		AccINFO lastStateB;
		BOOST_REQUIRE(gTestValue.GetAccInfo(ADDR_B,lastStateB));
		BOOST_REQUIRE(gTestValue.CheckAccState(ADDR_B,lastStateB));
	}
}

//A发数据开奖
BOOST_AUTO_TEST_CASE(Aopenbet)
{
	OPEN_DATA openA;
	openA.type = 3;
	memcpy(openA.targetkey, uint256(gRunCtxData.sendtxhash).begin(), sizeof(openA.targetkey));
	memcpy(openA.dhash, gRunCtxData.sdata, sizeof(gRunCtxData.sdata));
	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << openA;
	string openAcontract = HexStr(scriptData);
	uint64_t openfee = 1000000;
	Value vopenA = gTestValue.PCreateContractTx(gRunCtxData.scriptid, VADDR_A, openAcontract, gTestValue.GetBlockHeight(), openfee);
	string openATxHash;
	BOOST_CHECK(gTestValue.GetHashFromCreatedTx(vopenA, openATxHash));

	{
		int temphight = gTestValue.GetBlockHeight();
		AccINFOOperLog &operlogA = gTestValue.mapAccOperLog[ADDR_A];
		AccINFOOperLog &operlogB = gTestValue.mapAccOperLog[ADDR_B];
		AccINFO accA(0, -openfee, 500000, 0);
		AccINFO accB(0, 0, -500000, 0);
		operlogA.Add(temphight, accA);
		operlogB.Add(temphight, accB);
	}

	do
	{
		BOOST_CHECK(gTestValue.SetAddrGenerteBlock(MINERADDR));
	}
	while(!pwalletMain->UnConfirmTx.empty());

	{
		AccINFO lastStateA;
		BOOST_REQUIRE(gTestValue.GetAccInfo(ADDR_A,lastStateA));
		BOOST_REQUIRE(gTestValue.CheckAccState(ADDR_A,lastStateA));

		AccINFO lastStateB;
		BOOST_REQUIRE(gTestValue.GetAccInfo(ADDR_B,lastStateB));
		BOOST_REQUIRE(gTestValue.CheckAccState(ADDR_B,lastStateB));
	}
}

//B发数据开奖
BOOST_AUTO_TEST_CASE(Bopenbet)
{
	OPEN_DATA openB;
	openB.type = 3;
	memcpy(openB.targetkey, uint256(gRunCtxData.sendtxhash).begin(), sizeof(openB.targetkey));
	memcpy(openB.dhash, gRunCtxData.rdata, sizeof(gRunCtxData.rdata));
	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << openB;
	string openBcontract = HexStr(scriptData);
	uint64_t openfee = 1000000;
	Value vopenB = gTestValue.PCreateContractTx(gRunCtxData.scriptid, VADDR_B, openBcontract, gTestValue.GetBlockHeight(), openfee);
	string openBTxHash;
	BOOST_CHECK(gTestValue.GetHashFromCreatedTx(vopenB, openBTxHash));

	{
		int temphight = gTestValue.GetBlockHeight();
		AccINFOOperLog &operlogA = gTestValue.mapAccOperLog[ADDR_A];
		AccINFOOperLog &operlogB = gTestValue.mapAccOperLog[ADDR_B];
		AccINFO accA;
		AccINFO accB;
		if(0)//B win
		{
			AccINFO tmpA(0, 0, -1000000, 0);
			AccINFO tmpB(0, 1000000-openfee, 0, 0);
			accA += tmpA;
			accB += tmpB;
		}
		else
		{
			AccINFO tmpA1(0, 1000000, -1000000, 0);
			AccINFO tmpB1(0, -openfee, 0, 0);
			accA += tmpA1;
			accB += tmpB1;
		}
		operlogA.Add(temphight, accA);
		operlogB.Add(temphight, accB);
	}

	do
	{
		BOOST_CHECK(gTestValue.SetAddrGenerteBlock(MINERADDR));
	}
	while(!pwalletMain->UnConfirmTx.empty());

	{
		AccINFO lastStateA;
		BOOST_REQUIRE(gTestValue.GetAccInfo(ADDR_A,lastStateA));
		BOOST_REQUIRE(gTestValue.CheckAccState(ADDR_A,lastStateA));

		AccINFO lastStateB;
		BOOST_REQUIRE(gTestValue.GetAccInfo(ADDR_B,lastStateB));
		BOOST_REQUIRE(gTestValue.CheckAccState(ADDR_B,lastStateB));
	}
}

#if 0
BOOST_FIXTURE_TEST_CASE(NormalTest,Cp2pBetTest)
{
	string strFileName("p2pbet.bin");
//	string strAddrA("[\"mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA\"]");
//	string strAddrB("[\"mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5\"]");
	int nTimeOutHeight = 5;
	int nFee = 100000;

	//注册对赌脚本
	Value valueRes = RegisterScriptTx("mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA", strFileName , nTimeOutHeight, nFee);
	string regTxHash;
	BOOST_CHECK(GetHashFromCreatedTx(valueRes, regTxHash));

	//挖矿
//	uint64_t nOldMoney = GetFreeMoney(strAddr1);
//	uint64_t nOldBlockHeight = GetBlockHeight();

	BOOST_CHECK(GenerateOneBlock());
	int nNewBlockHeight = GetBlockHeight();

//	BOOST_CHECK(IsAllTxInBlock());

	//A基于脚本发起对赌包
	unsigned char sdata[5] = {0, 1, 2, 3, 4};
	string scriptid;
	BOOST_CHECK(GetOneScriptId(scriptid));

	SEND_DATA senddata;
	senddata.type = 0;
	senddata.hight = nTimeOutHeight;
	senddata.money = 500000;
//	senddata.dhash = Hash(sdata, sdata + sizeof(sdata));
	memcpy(senddata.dhash, Hash(sdata, sdata + sizeof(sdata)).begin(), sizeof(senddata.dhash));

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);

	Value vsend = CreateContractTx(scriptid, "[\"mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA\"]", sendcontract, nNewBlockHeight);
	string sendTxHash;
	BOOST_CHECK(GetHashFromCreatedTx(vsend, sendTxHash));

	//挖矿
	BOOST_CHECK(GenerateOneBlock());

	//另外一个账户B接受对赌
	unsigned char rdata[5] = {0, 1, 2, 3, 5};
	ACCEPT_DATA acceptdata;
	acceptdata.type = 1;
	acceptdata.money = 500000;
	memcpy(acceptdata.targetkey, uint256(sendTxHash).begin(), sizeof(acceptdata.targetkey));
	memcpy(acceptdata.dhash, Hash(rdata, rdata + sizeof(rdata)).begin(), sizeof(acceptdata.dhash));
	scriptData.clear();
	scriptData << acceptdata;
	string acceptcontract = HexStr(scriptData);

	Value vaccept = CreateContractTx(scriptid, "[\"mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5\"]", acceptcontract, nNewBlockHeight);
	string acceptTxHash;
	BOOST_CHECK(GetHashFromCreatedTx(vaccept, acceptTxHash));

	//挖矿
	//	uint64_t nOldMoney = GetFreeMoney(strAddr1);
	//	uint64_t nOldBlockHeight = GetBlockHeight();
	BOOST_CHECK(GenerateOneBlock());

	//情况一：A揭赌，B不揭赌，判断结果

	//回滚一个block

	//情况二：B揭赌，A不揭赌，判断结果

	//回滚一个block

	//情况三：A揭赌，B也揭赌，判断结果
	{
		//A揭赌合约交易
		OPEN_DATA openA;
		openA.type = 2;
//		openA.targetkey = uint256(sendTxHash);
		memcpy(openA.targetkey, uint256(sendTxHash).begin(), sizeof(openA.targetkey));
		memcpy(openA.dhash, sdata, sizeof(sdata));
		scriptData.clear();
		scriptData << openA;
		string openAcontract = HexStr(scriptData);

		Value vopenA = CreateContractTx(scriptid, "[\"mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA\"]", openAcontract, nNewBlockHeight);
		string openATxHash;
		BOOST_CHECK(GetHashFromCreatedTx(vopenA, openATxHash));

		//挖矿
		BOOST_CHECK(GenerateOneBlock());

		//B揭赌合约交易
		OPEN_DATA openB;
		openB.type = 2;
//		openB.targetkey = uint256(sendTxHash);
		memcpy(openB.targetkey, uint256(sendTxHash).begin(), sizeof(openB.targetkey));
		memcpy(openB.dhash, rdata, sizeof(rdata));
		scriptData.clear();
		scriptData << openB;
		string openBcontract = HexStr(scriptData);

		Value vopenB = CreateContractTx(scriptid, "[\"mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5\"]", openBcontract, nNewBlockHeight);
		string openBTxHash;
		BOOST_CHECK(GetHashFromCreatedTx(vopenB, openBTxHash));

		//挖矿
		BOOST_CHECK(GenerateOneBlock());

	}

	//回滚一个block

	//情况四：A不揭赌，B也不揭赌

}
#endif
BOOST_FIXTURE_TEST_CASE(AbnormalTest,Cp2pBetTest)
{

}

BOOST_AUTO_TEST_SUITE_END()

