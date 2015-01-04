#ifndef CYCLESESURETRADE_H_
#define CYCLESESURETRADE_H_

#include "../test/SysTestBase.h"
#include "CycleTestBase.h"

#define BUYER	"01"
#define SELLER	"02"
#define ARBIT	"03"

#define BUYER_ADDR 		"mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc"
#define SELLER_ADDR 	"mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y"
#define ARBIT_ADDR		"moZJZgsGFC4qvwRdjzS7Bj3fHrtpUfEVEE"

#define BUYER_ID 		"000000000200"
#define SELLER_ID 		"000000000300"
#define ARBIT_ID		"000000000400"

#define VADDR_BUYER   	"[\"mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc\"]"
#define VADDR_SELLER   	"[\"mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y\"]"
#define VADDR_ARBIT   	"[\"moZJZgsGFC4qvwRdjzS7Bj3fHrtpUfEVEE\"]"

#define MAX_ACCOUNT_LEN 20
#define ACCOUNT_ID_SIZE 6
#define MAX_ARBITRATOR 3
#define HASH_SIZE		32

#pragma pack(1)
typedef struct tag_INT64 {
	unsigned char data[8];
} Int64;

typedef struct tagACCOUNT_ID
{
	char accounid[MAX_ACCOUNT_LEN];
}ACCOUNT_ID;

typedef struct  {
	unsigned char nType;
	unsigned char nArbitratorCount;
	ACCOUNT_ID 	buyer;
	ACCOUNT_ID seller;
	ACCOUNT_ID arbitrator[MAX_ARBITRATOR];
	long nHeight;
	Int64 nFineMoney;
	Int64 nPayMoney;
	Int64 nFee;
	Int64 ndeposit;
}FIRST_TRADE_CONTRACT;

typedef struct {
	unsigned char nType;
	unsigned char hash[HASH_SIZE];
} NEXT_TRADE_CONTRACT;

typedef struct {
	unsigned char nType;
	unsigned char hash[HASH_SIZE];
	Int64 nMinus;
}ARBIT_RES_CONTRACT;
#pragma popup()

class CSesureTradeHelp :public SysTestBase{
public:
	bool ModifyAuthor(unsigned char nUserData,const string& strSignAddr,string& strTxHash);
	string GetReverseHash(const string& strTxHash);

	string PutDataIntoString(char* pData, int nDateLen);
	bool VerifyTxInBlock(const string& strTxHash,bool bTryForever = false);
	//bool GetScriptID(const string& strTxHash, string& strScriptID);

	void PacketFirstContract(const char*pBuyID, const char* pSellID, const char* pArID, int nHeight, int nFine,
			int nPay, int nFee, int ndeposit, FIRST_TRADE_CONTRACT* pContract);
	void PacketNextContract(unsigned char nStep, unsigned char* pHash, NEXT_TRADE_CONTRACT* pNextContract);
	void PacketLastContract(unsigned char* pHash, int nFine, ARBIT_RES_CONTRACT* pLastContract);

protected:
	string strRegScriptID;
};

class CTestSesureTrade:	public CycleTestBase,
						//public SysTestBase,
						public CSesureTradeHelp
{
    bool Step1RegisterScript();
	bool Step1ModifyAuthor();
	bool Step1SendContract();
	bool Step2ModifyAuthor();
	bool Step2SendContract();
	bool Step3ModifyAuthor();
	bool Step3SendContract();
	bool Step4ModifyAuthor();
	bool Step4SendContract();
	bool CheckLastSendTx();

public:
	CTestSesureTrade();
	virtual TEST_STATE run();
	~CTestSesureTrade();

private:
	int mCurStep;
	string strStep1RegHash;
	string strStep1ModifyHash;
	string strStep1SendHash;
	string strStep2ModifyHash;
	string strStep2SendHash;
	string strStep3ModifyHash;
	string strStep3SendHash;
	string strStep4ModifyHash;
	string strStep4SendHash;
};

#endif
