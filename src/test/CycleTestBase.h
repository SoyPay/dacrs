/*
 * CycleTestBase.h
 *
 *  Created on: 2014Äê12ÔÂ30ÈÕ
 *      Author: ranger.shi
 */

#ifndef CYCLETESTBASE_H_
#define CYCLETESTBASE_H_

#include "SysTestBase.h"

enum TEST_STATE{
	this_state,
	next_state,
	end_state,
};
class CycleTestBase {
protected:
	SysTestBase basetest;
public:
	CycleTestBase() {};
	virtual TEST_STATE run() {return end_state;};
	~CycleTestBase() {} ;
};

class CTestSesureTrade:public CycleTestBase
{
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
}FIRST_CONTRACT;

typedef struct {
	unsigned char nType;
	unsigned char hash[HASH_SIZE];
} NEXT_CONTRACT;

typedef struct {
	unsigned char nType;
	unsigned char hash[HASH_SIZE];
	Int64 nMinus;
}ARBIT_RES_CONTRACT;
#pragma popup()


	string BUYER ;
	string SELLER ;
	string ARBIT ;

	string BUYER_ADDR;
	string SELLER_ADDR;
	string ARBIT_ADDR ;

	string BUYER_ID ;
	string SELLER_ID ;
	string ARBIT_ID ;

	string VADDR_BUYER ;
	string VADDR_SELLER ;
	string VADDR_ARBIT ;

	int mCurStep;
	string strStep0Hash;
	string strStep1Hash;
	string Scriptregid;
	string strStep2Hash;
	string strStep3Hash;


	bool ModifyAuthor(unsigned char nUserData,const string& strSignAddr,const string&regid,string& strTxHash) {
		vector<unsigned char> vUserDefine;
		string strHash1, strHash2;
		vUserDefine.push_back(nUserData);
		CNetAuthorizate author(10000, vUserDefine, 100000, 1000000, 1000000);
		Value valueRes = basetest.ModifyAuthor(strSignAddr, regid, 0, 10000, author);

		if (!basetest.GetHashFromCreatedTx(valueRes,strTxHash)) {
			return false;
		}

		return true;
	}

	void PacketFirstContract(const char*pBuyID,const char* pSellID,const char* pArID,
		int nHeight,int nFine,int nPay,int nFee,int ndeposit,FIRST_CONTRACT* pContract);



    bool Step0RegisterScript();
    bool Step1ModifyAuthor();
    bool Step2SendContract();
    bool Step3ModifyAuthor();
public:
	CTestSesureTrade();
	virtual TEST_STATE run();
	~CTestSesureTrade();
};



#endif /* CYCLETESTBASE_H_ */
