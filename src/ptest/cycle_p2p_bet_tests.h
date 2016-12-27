/*
 * CycleP2PBet_test.h
 *
 *  Created on: 2015年1月15日
 *      Author: spark.huang
 */

#ifndef CYCLE_P2P_BET_TEST_H_
#define CYCLE_P2P_BET_TEST_H_

#include "../test/systestbase.h"
#include "cycle_test_base.h"

#pragma pack(push)
#pragma pack(1)

typedef struct {
	unsigned char uchType;  /*TX_SENDBET = 0x01,TX_ACCEPTBET = 0x02,TX_OPENBET = 0x03*/
	unsigned char uchNoperateType;   //竞猜类型值0 -1
	uint64_t llMoney;
	unsigned short usHight;  //超时高度 20
	unsigned char arruchDhash[32];
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(uchNoperateType);
			READWRITE(llMoney);
			READWRITE(usHight);
			for(int i = 0; i < 32; i++) {
				READWRITE(arruchDhash[i]);
			}
	)
} ST_SEND_DATA;

typedef struct {
	unsigned char uchType;
	unsigned char uchNoperateType;
	uint64_t llMoney;
	unsigned char uchData;
	unsigned char uchTxhash[32];		//发起对赌的哈希，也是对赌数据的关键字
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(uchNoperateType);
			READWRITE(llMoney);
			READWRITE(uchData);
			for(int i = 0; i < 32; i++)	{
				READWRITE(uchTxhash[i]);
			}
	)
} ST_ACCEPT_DATA;

typedef struct {
	unsigned char uchType;
	unsigned char uchNoperateType;
	unsigned char arruchTxhash[32];		//发起对赌的哈希，也是对赌数据的关键字
	unsigned char arruchDhash[33];
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(uchNoperateType);
			for(int i = 0; i < 32; i++)	{
				READWRITE(arruchTxhash[i]);
			}
			for(int ii = 0; ii < 33; ii++) {
				READWRITE(arruchDhash[ii]);
			}
	)
} ST_OPEN_DATA;

#pragma pack(pop)

#define ADDR_A    "doym966kgNUKr2M9P7CmjJeZdddqvoU5RZ"   // 0-6
#define VADDR_A   "[\"doym966kgNUKr2M9P7CmjJeZdddqvoU5RZ\"]"
#define ADDR_B    "dd936HZcwj9dQkefHPqZpxzUuKZZ2QEsbN"
#define VADDR_B   "[\"dd936HZcwj9dQkefHPqZpxzUuKZZ2QEsbN\"]"  //0-7


class CTestBetTx:public CycleTestBase,public SysTestBase
{
 public:
	CTestBetTx();
	virtual emTEST_STATE Run();
	~CTestBetTx();
	uint64_t GetRandomBetAmount() {
		srand(time(NULL));
		int nr = (rand() % 1000000) + 500000;
		return nr;
	}

	bool GetRandomData(unsigned char *pBuf, int nNum) {
		RAND_bytes(pBuf, nNum);
		return true;
	}

	int GetBetData() {
		unsigned char uchBuf;
		RAND_bytes(&uchBuf, 1);
		int nNum = uchBuf;
		if(nNum>0&&nNum<=6) {
			return nNum;
		}
		nNum = nNum%6 +1;
		return nNum;
	}

	unsigned char GetRanOpType() {
		unsigned char uchType;
		RAND_bytes(&uchType, sizeof(uchType));
		unsigned char  uchGussnum = uchType % 2;
		return uchGussnum;
	}

 private:
	unsigned char m_arrnchSdata[33];  //前32字节的随机数,后1个字节的1-6之间的数
	int m_nCurStep;
	string m_strRegScriptHash;
	string m_strAsendHash;
	string m_strBacceptHash;
	string m_strAopenHash;
	string m_strScriptid;
	uint64_t m_llBetamount;

	bool RegScript(void);
	bool WaiteRegScript(void);
	bool ASendP2PBet(void);
	bool WaitASendP2PBet(void);
	bool BAcceptP2PBet(void);
	bool WaitBAcceptP2PBet(void);
	bool AOpenP2PBet(void);
	bool WaitAOpenP2PBet(void);

};

#endif /* CYCLEP2PBET_TEST_H_ */
