#ifndef DACRS_PTEST_CYCLE_SESURE_TRADE_TESTS_H_
#define DACRS_PTEST_CYCLE_SESURE_TRADE_TESTS_H_

#include "../test/systestbase.h"
#include "create_tx_tests.h"

#define BUYER	"01"
#define SELLER	"02"
#define ARBIT	"03"

#define BUYER_ADDR 		"dtKsuK9HUvLLHtBQL8Psk5fUnTLTFC83GS"
#define SELLER_ADDR 	"dejEcGCsBkwsZaiUH1hgMapjbJqPdPNV9U"
#define ARBIT_ADDR		"dkoEsWuW3aoKaGduFbmjVDbvhmjxFnSbyL"

#define BUYER_ID 		"000000000200"
#define SELLER_ID 		"000000000300"
#define ARBIT_ID		"000000000400"

#define VADDR_BUYER   	"[\"dtKsuK9HUvLLHtBQL8Psk5fUnTLTFC83GS\"]"
#define VADDR_SELLER   	"[\"dejEcGCsBkwsZaiUH1hgMapjbJqPdPNV9U\"]"
#define VADDR_ARBIT   	"[\"dkoEsWuW3aoKaGduFbmjVDbvhmjxFnSbyL\"]"

#define MAX_ACCOUNT_LEN 20
#define ACCOUNT_ID_SIZE 6
#define MAX_ARBITRATOR 3
#define HASH_SIZE		32

#pragma pack(push)
#pragma pack(1)
typedef struct {
	unsigned char arruchData[8];
} ST_INT64;

typedef struct {
	char arrchAccounid[MAX_ACCOUNT_LEN];
} ST_ACCOUNT_ID;

typedef struct {
	unsigned char uchType;
	unsigned char uchArbitratorCount;
	ST_ACCOUNT_ID tBuyer;
	ST_ACCOUNT_ID tSeller;
	ST_ACCOUNT_ID arrtArbitrator[MAX_ARBITRATOR];
	long lHeight;
	ST_INT64 tFineMoney;
	ST_INT64 tPayMoney;
	ST_INT64 tFee;
	ST_INT64 tDeposit;
} ST_FIRST_TRADE_CONTRACT;

typedef struct {
	unsigned char uchType;
	unsigned char arruchHash[HASH_SIZE];
} ST_NEXT_TRADE_CONTRACT;

typedef struct {
	unsigned char uchType;
	unsigned char arruchHash[HASH_SIZE];
	ST_INT64 tMinus;
} ST_ARBIT_RES_CONTRACT;
#pragma pack(pop)

class CSesureTradeHelp: public SysTestBase {
 public:
	string GetReverseHash(const string& strTxHash);

	string PutDataIntoString(char* pData, int nDateLen);
	bool VerifyTxInBlock(const string& strTxHash, bool bTryForever = false);
	//bool GetScriptID(const string& strTxHash, string& strScriptID);

	void PacketFirstContract(const char*pBuyID, const char* pSellID, const char* pArID, int nHeight, int nFine,
			int nPay, int nFee, int ndeposit, ST_FIRST_TRADE_CONTRACT* pContract);
	void PacketNextContract(unsigned char uchStep, unsigned char* pHash, ST_NEXT_TRADE_CONTRACT* pNextContract);
	void PacketLastContract(unsigned char* pHash, int nFine, ST_ARBIT_RES_CONTRACT* pLastContract);

 protected:
	string m_strRegScriptID;
};

class CTestSesureTrade: public CycleTestBase, public CSesureTradeHelp {
 public:
	CTestSesureTrade();
	virtual emTEST_STATE Run();
	~CTestSesureTrade();

 private:
	int m_nCurStep;
	string m_strStep1RegHash;
	string m_strStep1ModifyHash;
	string m_strStep1SendHash;
	string m_strStep2ModifyHash;
	string m_strStep2SendHash;
	string m_strStep3ModifyHash;
	string m_strStep3SendHash;
	string m_strStep4ModifyHash;
	string m_strStep4SendHash;

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
};

#endif
