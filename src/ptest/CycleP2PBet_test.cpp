/*
 * CycleP2PBet_test.cpp
 *
 *  Created on: 2015年1月15日
 *      Author: spark.huang
 */

#include "CycleP2PBet_test.h"

CTestBetTx::CTestBetTx() {
	memset(nSdata, 0, sizeof(nSdata));
	mCurStep = 0;
	strRegScriptHash = "";
	scriptid = "";
	strRegScriptHash = "";
	strAsendHash = "";
	strBacceptHash = "";
	strAopenHash = "";
	betamount = 0;
}

CTestBetTx::~CTestBetTx() {
}

TEST_STATE CTestBetTx::run() {
	switch (mCurStep) {
	case 0:
		RegScript();
		break;
	case 1:
		ASendP2PBet();
		break;
	case 2:
		BAcceptP2PBet();
		break;
	case 3:
		AOpenP2PBet();
		break;
	case 4:
		CheckLastTx();
		break;
	default:
		assert(0);
		break;
	}
	return this_state;
}

bool CTestBetTx::VerifyTxInBlock(const string& strTxHash, bool bTryForever) {
	string strScriptID;
	do {
		if (GetTxConfirmedRegID(strTxHash, strScriptID)) {
			if (!strScriptID.empty())
				return true;
		}
	} while (bTryForever);

	return false;
}

bool CTestBetTx::RegScript(void) {

	const char* pKey[] = { "cPqVgscsWpPgkLHZP3pKJVSU5ZTCCvVhkd5cmXVWVydXdMTtBGj7",
			"cU1dxQgvyKt8yEqqkKiNLK9jfyW498RKi8y2evqzjtLXrLD4fBMs", };
	int nCount = sizeof(pKey) / sizeof(char*);
	basetest.ImportWalletKey(pKey, nCount);

	string strFileName("p2pbet.bin");
	int nFee = GetRandomBetfee();
	int nCurHight;
	GetBlockHeight(nCurHight);
	//注册对赌脚本
	Value valueRes = RegisterScriptTx(ADDR_A, strFileName, nCurHight, nFee);
	GetHashFromCreatedTx(valueRes, strRegScriptHash);
	mCurStep++;
	return true;
}

bool CTestBetTx::ASendP2PBet() {

	if(VerifyTxInBlock(strRegScriptHash))
	{
		if (!GetTxConfirmedRegID(strRegScriptHash, scriptid) ) {
					return false;
				}
		int nCurHight;
		GetBlockHeight(nCurHight);

		GetRandomBetData(nSdata, sizeof(nSdata));
		SEND_DATA senddata;
		senddata.type = 1;
		senddata.hight = nCurHight + 100;
		senddata.money = GetRandomBetAmount();
		betamount = senddata.money;
		memcpy(senddata.dhash, Hash(nSdata, nSdata + sizeof(nSdata)).begin(), sizeof(senddata.dhash));

		CDataStream scriptData(SER_DISK, CLIENT_VERSION);
		scriptData << senddata;
		string sendcontract = HexStr(scriptData);
		uint64_t sendfee = GetRandomBetfee();
		Value vsend = PCreateContractTx(scriptid, VADDR_A, sendcontract, nCurHight,
				sendfee);
		GetHashFromCreatedTx(vsend, strAsendHash);
		mCurStep++;
		return true;
	}
}

bool CTestBetTx::BAcceptP2PBet(void) {
	if (VerifyTxInBlock(strAsendHash)) {
		unsigned char rdata[5];
		GetRandomBetData(rdata, sizeof(rdata));
		ACCEPT_DATA acceptdata;
		acceptdata.type = 2;
		acceptdata.money = betamount;
		memcpy(acceptdata.targetkey, uint256(strAsendHash).begin(), sizeof(acceptdata.targetkey));
		memcpy(acceptdata.data, rdata, sizeof(rdata));
		CDataStream scriptData(SER_DISK, CLIENT_VERSION);
		scriptData << acceptdata;
		string acceptcontract = HexStr(scriptData);
		uint64_t acceptfee = GetRandomBetfee();
		int nCurHight;
		GetBlockHeight(nCurHight);
		Value vaccept = PCreateContractTx(scriptid, VADDR_B, acceptcontract, nCurHight, acceptfee);
		GetHashFromCreatedTx(vaccept, strBacceptHash);
		mCurStep++;
		return true;
	}
}

bool CTestBetTx::AOpenP2PBet(void) {
	if (VerifyTxInBlock(strBacceptHash)) {
		OPEN_DATA openA;
		openA.type = 3;
		memcpy(openA.targetkey, uint256(strAsendHash).begin(), sizeof(openA.targetkey));
		memcpy(openA.dhash, nSdata, sizeof(nSdata));
		CDataStream scriptData(SER_DISK, CLIENT_VERSION);
		scriptData << openA;
		string openAcontract = HexStr(scriptData);
		uint64_t openfee = GetRandomBetfee();
		int nCurHight;
		GetBlockHeight(nCurHight);
		Value vopenA = PCreateContractTx(scriptid, VADDR_A, openAcontract, nCurHight, openfee);
		GetHashFromCreatedTx(vopenA, strAopenHash);
		mCurStep++;
		return true;
	}
}
