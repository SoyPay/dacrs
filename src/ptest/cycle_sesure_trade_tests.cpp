
#include "cycle_sesure_trade_tests.h"

CTestSesureTrade::CTestSesureTrade() {
	m_nCurStep = 0;
	m_strStep1RegHash = "";
}

emTEST_STATE CTestSesureTrade::Run() {
	switch (m_nCurStep) {
	case 0: {
		Step1RegisterScript();
		break;
	}
	case 1: {
		Step1ModifyAuthor();
		break;
	}
	case 2: {
		Step1SendContract();
		break;
	}
	case 3: {
		Step2ModifyAuthor();
		break;
	}
	case 4: {
		Step2SendContract();
		break;
	}
	case 5: {
		Step3ModifyAuthor();
		break;
	}
	case 6: {
		Step3SendContract();
		break;
	}
	case 7: {
		Step4ModifyAuthor();
		break;
	}
	case 8: {
		Step4SendContract();
		break;
	}
	case 9: {
		CheckLastSendTx();
		break;
	}
	default: {
		assert(0);
		break;
	}
	}

	return EM_THES_STATE;
}

bool CTestSesureTrade::Step1RegisterScript() {
	const char* pkKey[] = { "cSu84vACzZkWqnP2LUdJQLX3M1PYYXo2gEDDCEKLWNWfM7B4zLiP", // addr:  dtKsuK9HUvLLHtBQL8Psk5fUnTLTFC83GS
			"cSVY69D9aUo4MugzUG9rM14DtV21cBAbZUVXmgAC2RpJwtZRUbsM", // addr:  dejEcGCsBkwsZaiUH1hgMapjbJqPdPNV9U
			"cTCcDyQvX6ucP9NEjhyHfTixamKQHQkFiSyfupm4CGZZYV7YYnf8", // addr:  dkoEsWuW3aoKaGduFbmjVDbvhmjxFnSbyL
			};
	int nCount = sizeof(pkKey) / sizeof(char*);
	m_cBasetest.ImportWalletKey(pkKey, nCount);

	Value valueRes = RegisterAppTx(BUYER_ADDR, "SecuredTrade.bin", 0, 100000);
	if (GetHashFromCreatedTx(valueRes, m_strStep1RegHash)) {
		m_nCurStep++;
		return true;
	}
	return false;
}

bool CTestSesureTrade::Step1ModifyAuthor() {
	if (VerifyTxInBlock(m_strStep1RegHash)) {
		if (!GetTxConfirmedRegID(m_strStep1RegHash, m_strRegScriptID)) {
			return false;
		}
		m_nCurStep++;
		return true;
	}
	return false;
}

bool CTestSesureTrade::Step1SendContract() {
	if (VerifyTxInBlock(m_strStep1ModifyHash)) {
		ST_FIRST_TRADE_CONTRACT tFirstConstract;
		PacketFirstContract(BUYER_ID, SELLER_ID, ARBIT_ID, 200, 100000, 100000, 100000, 100000, &tFirstConstract);
		string strData = PutDataIntoString((char*) &tFirstConstract, sizeof(tFirstConstract));
		strData.assign((char*) &tFirstConstract, (char*) &tFirstConstract + sizeof(tFirstConstract));

		auto valueRes = CreateContractTx(m_strRegScriptID, VADDR_BUYER, strData, 0, 100000);
		if (GetHashFromCreatedTx(valueRes, m_strStep1SendHash)) {
			m_nCurStep++;
			return true;
		}
	}
	return false;
}

bool CTestSesureTrade::Step2ModifyAuthor() {
	if (VerifyTxInBlock(m_strStep1SendHash)) {
		m_nCurStep++;
		return true;
	}
	return false;
}

bool CTestSesureTrade::Step2SendContract() {
	if (VerifyTxInBlock(m_strStep2ModifyHash)) {
		string strReversFirstTxHash = GetReverseHash(m_strStep1SendHash);
		ST_NEXT_TRADE_CONTRACT tSecondContract;
		PacketNextContract(2, (unsigned char*) strReversFirstTxHash.c_str(), &tSecondContract);
		string strData = PutDataIntoString((char*) &tSecondContract, sizeof(tSecondContract));

		Value valueRes = CreateContractTx(m_strRegScriptID, VADDR_SELLER, strData, 0, 100000);
		if (GetHashFromCreatedTx(valueRes, m_strStep2SendHash)) {
			m_nCurStep++;
			return true;
		}
	}

	return false;
}

bool CTestSesureTrade::Step3ModifyAuthor() {
	if (VerifyTxInBlock(m_strStep2SendHash)) {
		m_nCurStep++;
		return true;
	}
	return false;
}

bool CTestSesureTrade::Step3SendContract() {
	if (VerifyTxInBlock(m_strStep3ModifyHash)) {
		string strReversFirstTxHash = GetReverseHash(m_strStep2SendHash);
		ST_NEXT_TRADE_CONTRACT tThirdContract;
		PacketNextContract(3, (unsigned char*) strReversFirstTxHash.c_str(), &tThirdContract);
		string strData = PutDataIntoString((char*) &tThirdContract, sizeof(tThirdContract));

		Value valueRes = CreateContractTx(m_strRegScriptID, VADDR_ARBIT, strData, 0, 100000);
		if (GetHashFromCreatedTx(valueRes, m_strStep3SendHash)) {
			m_nCurStep++;
			return true;
		}
	}

	return false;
}

bool CTestSesureTrade::Step4ModifyAuthor() {
	if (VerifyTxInBlock(m_strStep3SendHash)) {
		m_nCurStep++;
		return true;
	}
	return false;
}

bool CTestSesureTrade::Step4SendContract() {
	if (VerifyTxInBlock(m_strStep4ModifyHash)) {
		string strReversFirstTxHash = GetReverseHash(m_strStep3SendHash);
		ST_ARBIT_RES_CONTRACT tArContract;
		PacketLastContract((unsigned char*) strReversFirstTxHash.c_str(), 100000, &tArContract);
		string strData = PutDataIntoString((char*) &tArContract, sizeof(tArContract));

		Value valueRes = CreateContractTx(m_strRegScriptID, VADDR_ARBIT, strData, 0, 100000);
		if (GetHashFromCreatedTx(valueRes, m_strStep4SendHash)) {
			m_nCurStep++;
			return true;
		}
	}

	return false;
}

bool CTestSesureTrade::CheckLastSendTx() {
	if (VerifyTxInBlock(m_strStep4SendHash)) {
		m_nCurStep = 0;
		return true;
	}
	return false;
}

void CSesureTradeHelp::PacketFirstContract(const char* pBuyID, const char* pSellID, const char* pArID, int nHeight,
		int nFine, int nPay, int nFee, int ndeposit, ST_FIRST_TRADE_CONTRACT* pContract) {
	BOOST_CHECK(pContract);
	memset(pContract, 0, sizeof(ST_FIRST_TRADE_CONTRACT));
	pContract->uchType = 1;
	pContract->uchArbitratorCount = 1;
	pContract->lHeight = nHeight;

	unsigned char uchSize = sizeof(int);
	vector<unsigned char> vuchBuyID = ParseHex(pBuyID);
	memcpy(pContract->tBuyer.arrchAccounid, &vuchBuyID[0], ACCOUNT_ID_SIZE);

	vuchBuyID = ParseHex(pSellID);
	memcpy(pContract->tSeller.arrchAccounid, &vuchBuyID[0], ACCOUNT_ID_SIZE);

	vuchBuyID = ParseHex(pArID);
	memcpy(pContract->arrtArbitrator[0].arrchAccounid, &vuchBuyID[0], ACCOUNT_ID_SIZE);

	memcpy(&pContract->tFineMoney, (const char*) &nFine, uchSize); //100
	memcpy(&pContract->tPayMoney, (const char*) &nPay, uchSize); //80
	memcpy(&pContract->tDeposit, (const char*) &ndeposit, uchSize); //20
	memcpy(&pContract->tFee, (const char*) &nFee, uchSize); //10

}

CTestSesureTrade::~CTestSesureTrade() {
}

bool CSesureTradeHelp::VerifyTxInBlock(const string& strTxHash, bool bTryForever) {
	string strScriptID;
	do {
		if (GetTxConfirmedRegID(strTxHash, strScriptID)) {
			if (!strScriptID.empty()) {
				return true;
			}
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
	vector<unsigned char> vuchHash = ParseHex(strTxHash);
	reverse(vuchHash.begin(), vuchHash.end());
	string strHash;
	strHash.assign(vuchHash.begin(), vuchHash.end());
	return strHash;
}

void CSesureTradeHelp::PacketNextContract(unsigned char uchStep, unsigned char* pHash,
		ST_NEXT_TRADE_CONTRACT* pNextContract) {
	memset(pNextContract, 0, sizeof(ST_NEXT_TRADE_CONTRACT));
	pNextContract->uchType = uchStep;
	memcpy(pNextContract->arruchHash, pHash, HASH_SIZE);
}

void CSesureTradeHelp::PacketLastContract(unsigned char* pHash, int nFine, ST_ARBIT_RES_CONTRACT* pLastContract) {
	memset(pLastContract, 0, sizeof(ST_ARBIT_RES_CONTRACT));
	pLastContract->uchType = 4;
	memcpy(pLastContract->arruchHash, pHash, HASH_SIZE);
	memcpy(&pLastContract->tMinus, (const char*) &nFine, sizeof(int));
}



