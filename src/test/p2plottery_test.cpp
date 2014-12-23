/*
 * p2plottery_test.cpp
 *
 *  Created on: 2014年12月16日
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

typedef struct {
	unsigned char num[15];
	unsigned char numlen;
	uint64_t amonut;
}SELECT;

class Cp2pLottoTest: public SysTestBase {
public:
	std::map<string, AccINFO> mapAccState;
	std::map<string, AccINFOOperLog> mapAccOperLog;
	int operatetimes;

	string scriptid;
	string scriptaddr;
	int reglottohight;
	int lottoterm;

	std::map<string, SELECT> mapSelect;

	Cp2pLottoTest() {
		mapAccState.clear();
		mapAccOperLog.clear();
		operatetimes = 0;
		scriptid.clear();
	}

	bool clear()
	{
		mapAccState.clear();
		mapAccOperLog.clear();
		operatetimes = 0;
		scriptid.clear();
		return true;
	}

	~Cp2pLottoTest() {
//		StopServer();
	}

	uint64_t GetRandomAmount() {
		srand(time(NULL));
		int r = (rand() % 500000) + 200000;
		return r;
	}

	uint64_t GetRandomfee() {
			srand(time(NULL));
			int r = (rand() % 1000000) + 1000000;
			return r;
		}

	bool GetScriptAddrByID(const std::string &scriptid, string &addr)
	{
		char temp[64] = { 0 };
		strncpy(temp, scriptid.c_str(), sizeof(temp) - 1);

		char *argv[] = { "rpctest", "getaccountinfo", temp };
		int argc = sizeof(argv) / sizeof(char*);
		Value value;

		if (CommandLineRPC_GetValue(argc, argv, value)) {
			Object obj = value.get_obj();
			{
				addr = find_value(obj, "Address").get_str();
			}
			return true;
		}
		return false;
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

		b = (initState == lastState);

		if(b == false)
		{
			LogPrint("spark", "\r\n");
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
			LogPrint("spark", "\r\n");
		}

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

//enum OPERATETYPE {
//	REG,
//	ORDER,
//	OPEN
//};

#pragma pack(1)
typedef struct {
	unsigned char type;
	uint256 hash;
	unsigned char num[15];
	unsigned char numlen;
	IMPLEMENT_SERIALIZE
	(
		READWRITE(type);
		READWRITE(hash);
		for(int i = 0; i < 15; i++)
		{
			READWRITE(num[i]);
		}
		READWRITE(numlen);
	)
}TEST_DATA;

typedef struct {
	unsigned char type;
	uint64_t money;
	int hight;
	IMPLEMENT_SERIALIZE
	(
		READWRITE(type);
		READWRITE(money);
		READWRITE(hight);
	)
}REG_DATA;


typedef struct {
	unsigned char type;
	unsigned char num[15];
	unsigned char numlen;
	uint64_t money;
	IMPLEMENT_SERIALIZE
	(
		READWRITE(type);
		for(int i = 0; i < 15; i++)
		{
			READWRITE(num[i]);
		}
		READWRITE(numlen);
		READWRITE(money);
	)
}ORDER_DATA;


typedef struct {
	unsigned char type;
	IMPLEMENT_SERIALIZE
	(
		READWRITE(type);
	)
}OPEN_DATA;


#define MINERADDR       "mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA"
#define ADDR_REGSCRIPT  "mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5"

#define ADDR_REGLOTTO   "mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y"

#define REWARD_NO1 3000
#define REWARD_NO2 17
#define REWARD_NO3 2

string OrderAddrs[] = {
		"n4muwAThwzWvuLUh74nL3KYwujhihke1Kb",
		"mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7",
		"moZJZgsGFC4qvwRdjzS7Bj3fHrtpUfEVEE",
		"mjSwCwMsvtKczMfta1tvr78z2FTsZA1JKw",
		"msdDQ1SXNmknrLuTDivmJiavu5J9VyX9fV",
		"mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v",
		"mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc"
};

static string AddrToVaddr(const string &addr)
{
	string tmp = "[\"" + addr + "\"]";
	return tmp;
}

Cp2pLottoTest gLottoTestData;

typedef struct
{
 int m;
 int n;
 int result;
}MAP_M6;

const static MAP_M6 ms6map[] = {
		{6,  6, 1},
		{7,  6, 7},
		{8,  6, 28},
		{9,  6, 84},
		{10, 6, 210},
		{11, 6, 462},
		{12, 6, 924},
		{13, 6, 1716},
		{14, 6, 3003},
		{15, 6, 5005},
		{5,  5, 1},
		{6,  5, 6},
		{7,  5, 21},
		{8,  5, 56},
		{9,  5, 126},
		{10, 5, 252},
		{11, 5, 462},
		{12, 5, 792},
		{13, 5, 1287},
		{14, 5, 2002},
		{15, 5, 3003},
		{4,  4, 1},
		{5,  4, 5},
		{6,  4, 15},
		{7,  4, 35},
		{8,  4, 70},
		{9,  4, 126},
		{10, 4, 210},
		{11, 4, 330},
		{12, 4, 495},
		{13, 4, 715},
		{14, 4, 1001},
		{15, 4, 1365},
		{3,  3, 1},
		{4,  3, 4},
		{5,  3, 10},
		{6,  3, 20},
		{7,  3, 35},
		{8,  3, 56},
		{9,  3, 84},
		{10, 3, 120},
		{11, 3, 165},
		{12, 3, 220},
		{13, 3, 286},
		{14, 3, 364},
		{15, 3, 455},
		{2,  2, 1},
		{3,  2, 3},
		{4,  2, 6},
		{5,  2, 10},
		{6,  2, 15},
		{7,  2, 21},
		{8,  2, 28},
		{9,  2, 36},
		{10, 2, 45},
		{11, 2, 55},
		{12, 2, 66},
		{13, 2, 78},
		{14, 2, 91},
		{15, 2, 105},
		{1,  1, 1},
		{2,  1, 2},
		{3,  1, 3},
		{4,  1, 4},
		{5,  1, 5},
		{6,  1, 6},
		{7,  1, 7},
		{8,  1, 8},
		{9,  1, 9},
		{10, 1, 10},
		{11, 1, 11},
		{12, 1, 12},
		{13, 1, 13},
		{14, 1, 14},
		{15, 1, 15},
};

int MselectN(int m, int n) {
	if(m < n)
	{
		return 0;
	}
	for(auto tmp : ms6map)
	{
		if(tmp.m == m && tmp.n == n)
		{
			return tmp.result;
		}
	}

	return 0;
}



BOOST_AUTO_TEST_SUITE(p2pLottoTest)

static bool RegScript(void) {
	string strFileName("lottery.bin");
	int nFee = gLottoTestData.GetRandomfee();
	//注册对赌脚本
	Value valueRes = gLottoTestData.RegisterScriptTx(ADDR_REGSCRIPT, strFileName, gLottoTestData.GetBlockHeight(), nFee);
	string regTxHash;
	BOOST_CHECK(gLottoTestData.GetHashFromCreatedTx(valueRes, regTxHash));

	do {
		BOOST_CHECK(gLottoTestData.SetAddrGenerteBlock(MINERADDR));
	} while (!pwalletMain->UnConfirmTx.empty());

	assert(pScriptDBTip->Flush());

	return true;
}

static bool GetRandomByteOrderData(unsigned char *buf, unsigned char &num)
{
	int len;
	do
	{
		len = GetRandInt(15);
	}while(len < 6);

	std::vector<unsigned char> vch;
	unsigned char randch;
	do
	{
		RAND_bytes(&randch, sizeof(randch));
		if (randch < 15 && std::find(vch.begin(), vch.end(), randch) == vch.end()) {
			vch.push_back(randch);
		}
	}while(vch.size() < len);

	memcpy(buf, &vch[0], len);
	num = len;
//	printf("\r\n");
//	for(int ii = 0; ii < num; ii++)
//	{
//		printf("%x ", buf[ii]);
//	}
//	printf("\r\n");
	return true;
}

static bool GetAddrInitInfo(const string &addr)
{
	AccINFO tmp;
	BOOST_REQUIRE(gLottoTestData.GetAccInfo(addr, tmp));
	gLottoTestData.mapAccState[addr] = tmp;

	return true;
}

static bool OperateAccount(const string &addr, const OperAccountType &operate, const uint64_t &amount, const uint64_t &fee)
{
	AccINFOOperLog &operlog = gLottoTestData.mapAccOperLog[addr];

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

	operlog.Add(gLottoTestData.operatetimes, acc);
	gLottoTestData.operatetimes++;

	return true;
}

static bool CheckAccountInfo(void)
{
	for(auto tmp : gLottoTestData.mapAccState)
	{
		AccINFO tmpaccount;
		BOOST_CHECK(gLottoTestData.GetAccInfo(tmp.first, tmpaccount));
		BOOST_CHECK(gLottoTestData.CheckAccState(tmp.first, tmpaccount));
	}
	return true;
}

static bool InitAccountInfo(const string &addr)
{
	AccINFO tmp;
	BOOST_REQUIRE(gLottoTestData.GetAccInfo(addr, tmp));
	gLottoTestData.mapAccState[addr] = tmp;

	return true;
}

static bool InitAllAccountInfo()
{
	InitAccountInfo(ADDR_REGLOTTO);
	for(auto tmp : OrderAddrs)
	{
		InitAccountInfo(tmp);
	}
	return true;
}

static bool RegLotto(int shight) {
	BOOST_CHECK(gLottoTestData.GetOneScriptId(gLottoTestData.scriptid));
//	BOOST_CHECK(gLottoTestData.GetScriptAddrByID(gLottoTestData.scriptid, gLottoTestData.scriptaddr));

	REG_DATA regdata;
	regdata.type = 0;
	regdata.hight = shight;
	regdata.money = 5000000000;

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << regdata;
	string regcontract = HexStr(scriptData);
	uint64_t sendfee = gLottoTestData.GetRandomfee();
	Value vsend = gLottoTestData.PCreateContractTx(gLottoTestData.scriptid, AddrToVaddr(ADDR_REGLOTTO), regcontract, gLottoTestData.GetBlockHeight(),
			sendfee);
	string txhash;
	BOOST_CHECK(gLottoTestData.GetHashFromCreatedTx(vsend, txhash));

//	GetAddrInitInfo(gLottoTestData.scriptaddr);


//	OperateAccount(gLottoTestData.scriptaddr, ADDFREE, regdata.money, 0);

	do {
		BOOST_CHECK(gLottoTestData.SetAddrGenerteBlock(MINERADDR));
	} while (!pwalletMain->UnConfirmTx.empty());

	gLottoTestData.reglottohight = chainActive.Height();
//	LogPrint("spark", "\r\ngLottoTestData.reglottohight = %d\r\n", gLottoTestData.reglottohight);

	OperateAccount(ADDR_REGLOTTO, MINUSFREE, regdata.money, sendfee);
	OperateAccount(ADDR_REGLOTTO, ADDFREEZD, regdata.money, 0);

	CheckAccountInfo();

	return true;
}

static bool OrderLotto(const string &addr)
{
	ORDER_DATA orderdata;

	orderdata.type = 1;
//	orderdata.money = gLottoTestData.GetRandomAmount();
	orderdata.money = 100000;
	GetRandomByteOrderData(orderdata.num, orderdata.numlen);

	gLottoTestData.mapSelect[addr].numlen = orderdata.numlen;
	memcpy(gLottoTestData.mapSelect[addr].num, orderdata.num, sizeof(orderdata.num));
	gLottoTestData.mapSelect[addr].amonut = orderdata.money;

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << orderdata;
	string regcontract = HexStr(scriptData);
//	LogPrint("spark", "\r\norder contract data:%s\r\n", regcontract);
	uint64_t orderfee = gLottoTestData.GetRandomfee();
	Value vsend = gLottoTestData.PCreateContractTx(gLottoTestData.scriptid, AddrToVaddr(addr), regcontract, gLottoTestData.GetBlockHeight(),
			orderfee);
	string txhash;
	BOOST_CHECK(gLottoTestData.GetHashFromCreatedTx(vsend, txhash));


//	OperateAccount(gLottoTestData.scriptaddr, ADDFREE, orderdata.money, 0);

	do {
		BOOST_CHECK(gLottoTestData.SetAddrGenerteBlock(MINERADDR));
	} while (!pwalletMain->UnConfirmTx.empty());

	OperateAccount(addr, MINUSFREE, orderdata.money*MselectN(orderdata.numlen, 6), orderfee);
	OperateAccount(ADDR_REGLOTTO, ADDFREEZD, orderdata.money*MselectN(orderdata.numlen, 6), 0);

	CheckAccountInfo();

	return true;
}

static uint64_t GetOpenReward()
{
	return 2000000;
}

typedef struct
{
	int64_t top1;
	int64_t top2;
	int64_t top3;
}REWARD_RESULT;

static bool GetLuckyNum(const uint256 &phash, vector<unsigned char> &luckynum)
{
	unsigned char luckysize = 0, ii = 0;
	unsigned char hash[32] = {0};

	memcpy(hash, phash.begin(), 32);

//	printf("\r\nhash buf: ");
//	for(int kk = 0; kk < 32; kk++)
//	{
//		printf("%x: ", hash[kk]);
//	}
//	printf("\r\n");

	luckynum.clear();

	do
	{
		do
		{
			if (hash[ii] < 15 && std::find(luckynum.begin(), luckynum.end(), hash[ii]) == luckynum.end()) {
				luckynum.push_back(hash[ii]);
			}
			ii++;
		}
		while (luckynum.size() < 6 && ii < 32);

		if(ii >= 32 && luckynum.size() < 6)
		{
			luckynum.clear();
			ii = 0;
			uint256 tmp = Hash(hash, hash + sizeof(hash));
//			LogPrint("spark", "\r\nhash again:%s\r\n", HexStr(tmp.begin(), tmp.end()));
			memcpy(hash, tmp.begin(), 32);
		}
	} while (luckynum.size() < 6);

	return true;
}

static REWARD_RESULT DrawLottery(const uint256 &phash, const unsigned char *pdata, unsigned char datalen)
{
	REWARD_RESULT result;
	vector<unsigned char> luckynum;
	GetLuckyNum(phash, luckynum);
	int selsize = 0;
//	LogPrint("spark", "\r\nphash:%s\r\n", HexStr(phash.begin(), phash.end()));
//	LogPrint("spark", "\r\nluckynum:%s\r\n", HexStr(luckynum));

	for(int ii = 0; ii < datalen; ii++)
	{
		if (std::find(luckynum.begin(), luckynum.end(), pdata[ii]) != luckynum.end()) {
			selsize++;
		}
	}

//	LogPrint("spark", "\r\nselsize:%d\r\n", selsize);

	result.top1 = MselectN(selsize, 6) * MselectN(datalen - selsize, 0);
	result.top2 = MselectN(selsize, 5) * MselectN(datalen - selsize, 1);
	result.top3 = MselectN(selsize, 4) * MselectN(datalen - selsize, 2);

	return result;
}

static bool OpenLotto(const string &addr, int openhight)
{
	OPEN_DATA opendata;

	opendata.type = 2;

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << opendata;
	string regcontract = HexStr(scriptData);
	uint64_t openfee = gLottoTestData.GetRandomfee();
	Value vsend = gLottoTestData.PCreateContractTx(gLottoTestData.scriptid, AddrToVaddr(addr), regcontract, gLottoTestData.GetBlockHeight(),
			openfee);
	string txhash;
	BOOST_CHECK(gLottoTestData.GetHashFromCreatedTx(vsend, txhash));

	OperateAccount(addr, MINUSFREE, 0, openfee);
//	OperateAccount(addr, ADDFREE, openfee + GetOpenReward(), 0);

	CBlockIndex* pblockindex = chainActive[openhight];
	{
	//更新投注者账户
		for(auto tmp : gLottoTestData.mapSelect)
		{
//			int total = MselectN(tmp.second.numlen, 6);
			uint64_t price = tmp.second.amonut;
			REWARD_RESULT tmpslt = DrawLottery(pblockindex->GetBlockHash(), tmp.second.num, tmp.second.numlen);
			LogPrint("spark", "\r\nrslt addr:%s\r\n", tmp.first);
			{
//				LogPrint("spark", "\r\ntotal tickets:%d\r\n", total);
				LogPrint("spark", "\r\nprice:%I\r\n", price);
				LogPrint("spark", "\r\ntmpslt.top1:%I\r\n", tmpslt.top1);
				LogPrint("spark", "\r\ntmpslt.top2:%I\r\n", tmpslt.top2);
				LogPrint("spark", "\r\ntmpslt.top3:%I\r\n", tmpslt.top3);
			}

			uint64_t rsltamount = (tmpslt.top1*REWARD_NO1 + tmpslt.top2*REWARD_NO2 + tmpslt.top3*REWARD_NO3)*price;
			LogPrint("spark", "\r\nrsltamount:%I\r\n", rsltamount);
			OperateAccount(tmp.first, ADDFREE, rsltamount, 0);
			OperateAccount(ADDR_REGLOTTO, MINUSFREEZD, rsltamount, 0);
		}
	}

	do {
		BOOST_CHECK(gLottoTestData.SetAddrGenerteBlock(MINERADDR));
	} while (!pwalletMain->UnConfirmTx.empty());

	CheckAccountInfo();

	return true;
}

BOOST_AUTO_TEST_CASE(normal0)
{
	gLottoTestData.ResetEnv();
	InitAllAccountInfo();
	RegScript();
	RegLotto(100);

//	OrderLotto("n4muwAThwzWvuLUh74nL3KYwujhihke1Kb");
	for(auto tmp : OrderAddrs)
	{
		OrderLotto(tmp);
	}

	do {
		BOOST_CHECK(gLottoTestData.SetAddrGenerteBlock(MINERADDR));
	} while (chainActive.Height() < gLottoTestData.reglottohight + 10 + 5);

//	LogPrint("spark", "\r\nopen hight = %d\r\n", gLottoTestData.reglottohight + 10);

	OpenLotto(ADDR_REGSCRIPT, gLottoTestData.reglottohight + 10);
}

#if 0
BOOST_AUTO_TEST_CASE(testhash)
{
	RegScript();

	RegLotto(100);

	unsigned char buf[] = {0, 1, 2, 3, 4, 5, 6};

	uint256 hash = Hash(buf, buf + sizeof(buf));

	TEST_DATA test;
	test.type = 5;
	test.hash = hash;
	GetRandomByteOrderData(test.num, test.numlen);

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << test;
	string regcontract = HexStr(scriptData);
//	LogPrint("spark", "\r\n:%s\r\n", regcontract);
	uint64_t orderfee = gLottoTestData.GetRandomfee();
	Value vsend = gLottoTestData.PCreateContractTx(gLottoTestData.scriptid, AddrToVaddr(ADDR_REGSCRIPT), regcontract, gLottoTestData.GetBlockHeight(),
			orderfee);
	string txhash;
	BOOST_CHECK(gLottoTestData.GetHashFromCreatedTx(vsend, txhash));


	REWARD_RESULT rslt = DrawLottery(hash, test.num, test.numlen);

//	LogPrint("spark", "\r\nhash:%s\r\n", HexStr(hash.begin(), hash.end()));

	LogPrint("spark",
			"\r\n rslt.top1:%I"
			"\r\n rslt.top2:%I"
			"\r\n rslt.top3:%I"
			"\r\n",
			rslt.top1, rslt.top2,rslt.top3
			);
}
#endif

BOOST_AUTO_TEST_SUITE_END()



















