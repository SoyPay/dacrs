/*
 * CycleP2PBet_test.h
 *
 *  Created on: 2015年1月15日
 *      Author: spark.huang
 */

#ifndef CYCLEP2PBET_TEST_H_
#define CYCLEP2PBET_TEST_H_

#include "../test/systestbase.h"
#include "CycleTestBase.h"

#pragma pack(push)
#pragma pack(1)

typedef struct {
	unsigned char type;
	unsigned char noperateType;
	uint64_t money;
	unsigned short hight;
	unsigned char dhash[32];IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			READWRITE(noperateType);
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
	unsigned char noperateType;
	uint64_t money;
	unsigned char data;
	unsigned char txhash[32];		//发起对赌的哈希，也是对赌数据的关键字
	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			READWRITE(noperateType);
			READWRITE(money);
			READWRITE(data);
			for(int i = 0; i < 32; i++)
			{
				READWRITE(txhash[i]);
			}
	)
} ACCEPT_DATA;

typedef struct {
	unsigned char type;
	unsigned char noperateType;
	unsigned char txhash[32];		//发起对赌的哈希，也是对赌数据的关键字
	unsigned char dhash[33];IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			READWRITE(noperateType);
			for(int i = 0; i < 32; i++)
			{
				READWRITE(txhash[i]);
			}
			for(int ii = 0; ii < 33; ii++)
			{
				READWRITE(dhash[ii]);
			}
	)
} OPEN_DATA;

#pragma pack(pop)

#define ADDR_A    "mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v"   // 0-6
#define VADDR_A   "[\"mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v\"]"
#define ADDR_B    "mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7"
#define VADDR_B   "[\"mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7\"]"  //0-7


class CTestBetTx:public CycleTestBase,public SysTestBase
{
	bool RegScript(void);
	bool ASendP2PBet(void);
	bool BAcceptP2PBet(void);
	bool AOpenP2PBet(void);
public:
	CTestBetTx();
	virtual TEST_STATE run();
	~CTestBetTx();
	uint64_t GetRandomBetAmount() {
		srand(time(NULL));
		int r = (rand() % 1000000) + 500000;
		return r;
	}

	bool GetRandomData(unsigned char *buf, int num)
	{
		RAND_bytes(buf, num);
		return true;
	}
	int GetBetData()
	{
		unsigned char buf;
		RAND_bytes(&buf, 1);
		int num = buf;

		if(num>0&&num<=6)
			return num;
		num = num%6 +1;
		return num;
	}
	unsigned char GetRanOpType(){
		unsigned char cType;
		RAND_bytes(&cType, sizeof(cType));
		unsigned char  gussnum = cType % 2;
		//cout<<"type:"<<(int)gussnum<<endl;
		return gussnum;
	}
private:
	unsigned char nSdata[33];
	int mCurStep;
	string strRegScriptHash;
	string strAsendHash;
	string strBacceptHash;
	string strAopenHash;
	string scriptid;
	uint64_t betamount;
};

#endif /* CYCLEP2PBET_TEST_H_ */
