/*
 * CycleP2PBet_test.h
 *
 *  Created on: 2015年1月15日
 *      Author: spark.huang
 */

#ifndef CYCLEP2PBET_TEST_H_
#define CYCLEP2PBET_TEST_H_

#include "../test/SysTestBase.h"
#include "CycleTestBase.h"

#pragma pack(push)
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
	unsigned char data[5];IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			READWRITE(money);
			for(int i = 0; i < 32; i++)
			{
				READWRITE(targetkey[i]);
			}
			for(int ii = 0; ii < 5; ii++)
			{
				READWRITE(data[ii]);
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

#pragma pack(pop)

#define ADDR_A    "mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v"
#define VADDR_A   "[\"mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v\"]"
#define ADDR_B    "mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7"
#define VADDR_B   "[\"mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7\"]"


class CTestBetTx:public CycleTestBase,public SysTestBase
{
	bool RegScript(void);
	bool ASendP2PBet(void);
	bool BAcceptP2PBet(void);
	bool AOpenP2PBet(void);
	bool CheckLastTx(void) {
		if (VerifyTxInBlock(strAopenHash)) {
			mCurStep = 1;
			return true;
		}
		return false;
	}
public:
	CTestBetTx();
	virtual TEST_STATE run();
	~CTestBetTx();
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

	bool GetRandomBetData(unsigned char *buf, int num)
	{
		RAND_bytes(buf, num);
		return true;
	}

	bool VerifyTxInBlock(const string& strTxHash,bool bTryForever = false);
private:
	unsigned char nSdata[5];
	int mCurStep;
	string strRegScriptHash;
	string strAsendHash;
	string strBacceptHash;
	string strAopenHash;
	string scriptid;
	uint64_t betamount;
};

#endif /* CYCLEP2PBET_TEST_H_ */
