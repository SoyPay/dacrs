
#include "CycleSesureTrade_tests.h"

CTestSesureTrade::CTestSesureTrade() {

	mCurStep = 0;
	strStep1RegHash ="";
//	GenerateOneBlock();
}

TEST_STATE CTestSesureTrade::run() {
//	cout << "CTestSesureTrade::run  step: " << mCurStep << endl;
//	return  ++mCurStep > 5 ? end_state :next_state;
	bool bRes = false;
	switch (mCurStep) {
	case 0:
		Step1RegisterScript();
		break;
	case 1:
		Step1ModifyAuthor();
		break;
	case 2:
		Step1SendContract();
		break;
	case 3:
		Step2ModifyAuthor();
		break;
	case 4:
		Step2SendContract();
		break;
	case 5:
		Step3ModifyAuthor();
		break;
	case 6:
		Step3SendContract();
		break;
	case 7:
		Step4ModifyAuthor();
		break;
	case 8:
		Step4SendContract();
		break;
	case 9:
		bRes = CheckLastSendTx();
		break;
	default:
		assert(0);
		break;
	}

//	if (9 == mCurStep && bRes) {
//		return end_state;
//	}
	return this_state;
}

bool CSesureTradeHelp::ModifyAuthor(unsigned char nUserData, const string& strSignAddr, string& strTxHash) {
	vector<unsigned char> vUserDefine;
	string strHash1, strHash2;
	vUserDefine.push_back(nUserData);
	CNetAuthorizate author(10000, vUserDefine, 100000, 1000000, 1000000);
	Value valueRes = SysTestBase::ModifyAuthor(strSignAddr, strRegScriptID, 0, 10000, author);

	if (!GetHashFromCreatedTx(valueRes, strTxHash)) {
		return false;
	}

	return true;
}

bool CTestSesureTrade::Step1RegisterScript() {
	Value valueRes = RegisterScriptTx(BUYER_ADDR, "SecuredTrade.bin", 0, 100000);
	if (GetHashFromCreatedTx(valueRes, strStep1RegHash)) {
		mCurStep++;
		return true;
	}
	return false;
}
bool CTestSesureTrade::Step1ModifyAuthor() {
	if(VerifyTxInBlock(strStep1RegHash))
	{
		if (!GetTxConfirmedRegID(strStep1RegHash, strRegScriptID) ) {
			return false;
		}

		if(ModifyAuthor(1, BUYER_ADDR,strStep1ModifyHash))
		{
			mCurStep++;
			return true;
		}
	}
	return false;
}

bool CTestSesureTrade::Step1SendContract() {
	if (VerifyTxInBlock(strStep1ModifyHash)) {
		FIRST_TRADE_CONTRACT firstConstract;
		PacketFirstContract(BUYER_ID, SELLER_ID, ARBIT_ID, 200, 100000, 100000, 100000, 100000,&firstConstract);
     	string strData = PutDataIntoString((char*) &firstConstract, sizeof(firstConstract));
     	strData.assign((char*) &firstConstract, (char*) &firstConstract +  sizeof(firstConstract));

		auto valueRes = CreateContractTxEx(strRegScriptID, VADDR_BUYER, strData, 0, 100000);
		if (GetHashFromCreatedTx(valueRes, strStep1SendHash)) {
			mCurStep++;
			return true;
		}
	}
	return false;
}

bool CTestSesureTrade::Step2ModifyAuthor() {
	if(VerifyTxInBlock(strStep1SendHash))
	{
		if(ModifyAuthor(2, SELLER_ADDR,strStep2ModifyHash))
		{
			mCurStep++;
			return true;
		}
	}
	return false;
}

bool CTestSesureTrade::Step2SendContract() {
	if (VerifyTxInBlock(strStep2ModifyHash)) {
		string strReversFirstTxHash = GetReverseHash(strStep1SendHash);
		NEXT_TRADE_CONTRACT secondContract;
		PacketNextContract(2, (unsigned char*) strReversFirstTxHash.c_str(), &secondContract);
		string strData = PutDataIntoString((char*) &secondContract, sizeof(secondContract));

		Value valueRes = CreateContractTxEx(strRegScriptID, VADDR_SELLER, strData, 0, 100000);
		if (GetHashFromCreatedTx(valueRes, strStep2SendHash)) {
			mCurStep++;
			return true;
		}
	}

	return false;
}

bool CTestSesureTrade::Step3ModifyAuthor() {
	if(VerifyTxInBlock(strStep2SendHash))
		{
			if(ModifyAuthor(1, ARBIT_ADDR,strStep3ModifyHash))
			{
				mCurStep++;
				return true;
			}
		}
		return false;
}

bool CTestSesureTrade::Step3SendContract() {
	if (VerifyTxInBlock(strStep3ModifyHash)) {
		string strReversFirstTxHash = GetReverseHash(strStep2SendHash);
		NEXT_TRADE_CONTRACT thirdContract;
		PacketNextContract(3, (unsigned char*) strReversFirstTxHash.c_str(), &thirdContract);
		string strData = PutDataIntoString((char*) &thirdContract, sizeof(thirdContract));

		Value valueRes = CreateContractTxEx(strRegScriptID, VADDR_ARBIT, strData, 0, 100000);
		if (GetHashFromCreatedTx(valueRes, strStep3SendHash)) {
			mCurStep++;
			return true;
		}
	}

	return false;
}

bool CTestSesureTrade::Step4ModifyAuthor() {
	if (VerifyTxInBlock(strStep3SendHash)) {
		if (ModifyAuthor(3, ARBIT_ADDR, strStep4ModifyHash)) {
			mCurStep++;
			return true;
		}
	}
	return false;
}

bool CTestSesureTrade::Step4SendContract() {
	if (VerifyTxInBlock(strStep4ModifyHash)) {
		string strReversFirstTxHash = GetReverseHash(strStep3SendHash);
		ARBIT_RES_CONTRACT arContract;
		PacketLastContract((unsigned char*) strReversFirstTxHash.c_str(), 100000, &arContract);
		string strData = PutDataIntoString((char*) &arContract, sizeof(arContract));

		Value valueRes = CreateContractTxEx(strRegScriptID, VADDR_ARBIT, strData, 0, 100000);
		if (GetHashFromCreatedTx(valueRes, strStep4SendHash)) {
			mCurStep++;
			return true;
		}
	}

	return false;
}

bool CTestSesureTrade::CheckLastSendTx() {
	if (VerifyTxInBlock(strStep4SendHash)) {
		mCurStep = 0;
		return true;
	}
	return false;
}

void CSesureTradeHelp::PacketFirstContract(const char* pBuyID, const char* pSellID, const char* pArID, int nHeight,
		int nFine, int nPay, int nFee, int ndeposit, FIRST_TRADE_CONTRACT* pContract) {

		BOOST_CHECK(pContract);
		memset(pContract,0,sizeof(FIRST_TRADE_CONTRACT));
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

bool CSesureTradeHelp::VerifyTxInBlock(const string& strTxHash, bool bTryForever) {
	string strScriptID;
	do {
		if (GetTxConfirmedRegID(strTxHash, strScriptID)) {
			if (!strScriptID.empty())
				return true;
		}
	} while (bTryForever);

	return false;
}

string CSesureTradeHelp::PutDataIntoString(char* pData, int nDateLen) {
	string strData;
	strData.assign(pData, pData + nDateLen);
	return strData;
}

string CSesureTradeHelp::GetReverseHash(const string& strTxHash) {
	vector<unsigned char> vHash = ParseHex(strTxHash);
	reverse(vHash.begin(), vHash.end());
	string strHash;
	strHash.assign(vHash.begin(), vHash.end());
	return strHash;
}

void CSesureTradeHelp::PacketNextContract(unsigned char nStep, unsigned char* pHash, NEXT_TRADE_CONTRACT* pNextContract) {
	memset(pNextContract, 0, sizeof(NEXT_TRADE_CONTRACT));
	pNextContract->nType = nStep;
	memcpy(pNextContract->hash, pHash, HASH_SIZE);
}

void CSesureTradeHelp::PacketLastContract(unsigned char* pHash, int nFine, ARBIT_RES_CONTRACT* pLastContract) {
	memset(pLastContract, 0, sizeof(ARBIT_RES_CONTRACT));
	pLastContract->nType = 4;
	memcpy(pLastContract->hash, pHash, HASH_SIZE);
	memcpy(&pLastContract->nMinus, (const char*) &nFine, sizeof(int));
}



