/*
 * CycleTestBase.cpp
 *
 *  Created on: 2014Äê12ÔÂ30ÈÕ
 *      Author: ranger.shi
 */

#include "CycleTestBase.h"




CTestSesureTrade::CTestSesureTrade() {

	mCurStep = 0;
	strStep0Hash ="";


	BUYER = "01";
	SELLER = "02";
	ARBIT = "03";

	BUYER_ADDR = "mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc";
	SELLER_ADDR = "mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y";
	ARBIT_ADDR = "moZJZgsGFC4qvwRdjzS7Bj3fHrtpUfEVEE";

	BUYER_ID = "000000000200";
	SELLER_ID = "000000000300";
	ARBIT_ID = "000000000400";

	VADDR_BUYER = "[\"mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc\"]";
	VADDR_SELLER = "[\"mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y\"]";
	VADDR_ARBIT = "[\"moZJZgsGFC4qvwRdjzS7Bj3fHrtpUfEVEE\"]";


}

TEST_STATE CTestSesureTrade::run() {
//	cout << "hellow word ! step " << mCurStep << endl;
//	return  ++mCurStep > 5 ? end_state :next_state;
	switch (mCurStep) {
		case 0:
			Step0RegisterScript();
			break;
		case 1:
			Step1ModifyAuthor();
				break;
		case 2:
			Step2SendContract();
				break;
		case 3:
			Step3ModifyAuthor();
				break;

		default:
			assert(0);
			break;
	}
	 return end_state;
}

bool CTestSesureTrade::Step0RegisterScript() {
	Value valueRes = basetest.RegisterScriptTx(BUYER_ADDR, "SecuredTrade.bin", 0, 100000);
	if(basetest.GetHashFromCreatedTx(valueRes, strStep0Hash))
		{
			mCurStep++;
			return true;
		}
	return false;
}
bool CTestSesureTrade::Step1ModifyAuthor() {
	string regid;
	if(basetest.GetTxConfirmedRegID(strStep0Hash,regid))
	{
		if(ModifyAuthor(1, BUYER_ADDR,regid,strStep1Hash))
		{
			Scriptregid = regid;
			mCurStep++;
			return true;
		}
	}
	return false;
}
bool CTestSesureTrade::Step3ModifyAuthor() {
	string regid;
	if(basetest.GetTxConfirmedRegID(strStep2Hash,regid))
	{
		if(ModifyAuthor(1, BUYER_ADDR,regid,strStep3Hash))
		{
			mCurStep++;
			return true;
		}
	}
	return false;
}
bool CTestSesureTrade::Step2SendContract() {
	string regid;
	if (basetest.GetTxConfirmedRegID(strStep0Hash, regid)) {
		FIRST_CONTRACT firstConstract;
		PacketFirstContract(BUYER_ID.c_str(), SELLER_ID.c_str(), ARBIT_ID.c_str(), 200, 100000, 100000, 100000, 100000,&firstConstract);
     	string strData ;//= PutDataIntoString((char*) &firstConstract, sizeof(firstConstract));
     	strData.assign((char*) &firstConstract, (char*) &firstConstract +  sizeof(firstConstract));
		auto valueRes = basetest.CreateContractTxEx(Scriptregid, VADDR_BUYER, strData, 0, 100000);
		if (basetest.GetHashFromCreatedTx(valueRes, strStep2Hash)) {
			mCurStep++;
			return true;
		}
	}
	return false;
}

void CTestSesureTrade::PacketFirstContract(const char* pBuyID, const char* pSellID, const char* pArID, int nHeight,
		int nFine, int nPay, int nFee, int ndeposit, FIRST_CONTRACT* pContract) {

		BOOST_CHECK(pContract);
		memset(pContract,0,sizeof(FIRST_CONTRACT));
		pContract->nType = 1;
		pContract->nArbitratorCount = 1;
		pContract->nHeight = nHeight;

		unsigned char nSize = sizeof(int);
		vector<unsigned char> v = ParseHex(pBuyID);
		memcpy(pContract->buyer.accounid,&v[0],ACCOUNT_ID_SIZE);

		v = ParseHex(pSellID);
		memcpy(pContract->seller.accounid,&v[0],ACCOUNT_ID_SIZE);

		v = ParseHex(pArID);
		memcpy(pContract->arbitrator[0].accounid,&v[0],ACCOUNT_ID_SIZE);

		memcpy(&pContract->nFineMoney,(const char*)&nFine,nSize);//100
		memcpy(&pContract->nPayMoney,(const char*)&nPay,nSize);//80
		memcpy(&pContract->ndeposit,(const char*)&ndeposit,nSize);//20
		memcpy(&pContract->nFee,(const char*)&nFee,nSize);//10

}

CTestSesureTrade::~CTestSesureTrade() {
}
