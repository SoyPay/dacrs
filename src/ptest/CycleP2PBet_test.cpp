/*
 * CycleP2PBet_test.cpp
 *
 *  Created on: 2015年1月15日
 *      Author: spark.huang
 */

#include "CycleP2PBet_test.h"
enum OPERATE{
	OP_SYSTEMACC = 0x00,
	OP_APPACC = 0x01,
	OP_NULL = 0x02,
};
#define OUT_HEIGHT 20
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
	default:
		assert(0);
		break;
	}
	return next_state;
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
	BOOST_CHECK(GetHashFromCreatedTx(valueRes, strRegScriptHash));
	BOOST_CHECK(GenerateOneBlock());
	if (basetest.GetTxConfirmedRegID(strRegScriptHash, scriptid)) {
			mCurStep++;
			return true;
	}

	return true;
}

bool CTestBetTx::ASendP2PBet() {

	if(scriptid == "")
		return false;

		unsigned char randdata[32];
		GetRandomData(randdata, sizeof(randdata));
		int num = GetBetData();
		cout<<"win:"<<num<<endl;
		memcpy(nSdata,randdata,sizeof(randdata));
		memcpy(&nSdata[32],&num,sizeof(char));
		SEND_DATA senddata;

		senddata.noperateType = GetRanOpType();
		senddata.type = 1;
		senddata.hight = OUT_HEIGHT;
		senddata.money = GetRandomBetAmount();
		betamount = senddata.money;
		memcpy(senddata.dhash, Hash(nSdata, nSdata + sizeof(nSdata)).begin(), sizeof(senddata.dhash));
		vector<unsigned char>temp;
		temp.assign(nSdata,nSdata + sizeof(nSdata));
		vector<unsigned char>temp2;
		temp2.assign(senddata.dhash,senddata.dhash + sizeof(senddata.dhash));

		CDataStream scriptData(SER_DISK, CLIENT_VERSION);
		scriptData << senddata;
		string sendcontract = HexStr(scriptData);
		uint64_t sendfee = GetRandomBetfee();
		Value  sendret= CreateContractTx(scriptid,ADDR_A,sendcontract,0,0,betamount);
		if((int)senddata.noperateType == 0x01){
			strAsendHash = "";
			GetHashFromCreatedTx(sendret, strAsendHash);
			if(strAsendHash == "") {
				return true;
			}
			else{
				BOOST_CHECK(GenerateOneBlock());
			}
		}else{
			BOOST_CHECK(GetHashFromCreatedTx(sendret, strAsendHash));
			BOOST_CHECK(GenerateOneBlock());
		}

		string index = "";
		if (basetest.GetTxConfirmedRegID(strAsendHash, index)) {
				mCurStep++;
				return true;
		}
		return true;
}

bool CTestBetTx::BAcceptP2PBet(void) {
		unsigned char cType;
		RAND_bytes(&cType, sizeof(cType));
		unsigned char  gussnum = cType % 2;
		cout <<"guess:"<<(int)gussnum<<endl;
		ACCEPT_DATA acceptdata;
		acceptdata.type = 2;
		acceptdata.noperateType = GetRanOpType();
		acceptdata.money = betamount;
		acceptdata.data=gussnum;
		memcpy(acceptdata.txhash, uint256(strAsendHash).begin(), sizeof(acceptdata.txhash));

		CDataStream scriptData(SER_DISK, CLIENT_VERSION);
		scriptData << acceptdata;

		string acceptcontract = HexStr(scriptData);
		int nCurHight;
		GetBlockHeight(nCurHight);
		Value vaccept = CreateContractTx(scriptid, ADDR_B, acceptcontract, nCurHight, 0,betamount);
		string hash = "";
		if(acceptdata.noperateType == 0x01){

			GetHashFromCreatedTx(vaccept, hash);
			if(hash == "") {
				return true;
			}
			else{
				BOOST_CHECK(GenerateOneBlock());
			}
		}else{
			BOOST_CHECK(GetHashFromCreatedTx(vaccept, hash));
			BOOST_CHECK(GenerateOneBlock());
		}
		BOOST_CHECK(GetHashFromCreatedTx(vaccept, strBacceptHash));
		BOOST_CHECK(GenerateOneBlock());
		string index = "";
		if (basetest.GetTxConfirmedRegID(strBacceptHash, index)) {
				mCurStep++;
				return true;
		}
		return true;
}

bool CTestBetTx::AOpenP2PBet(void) {

		OPEN_DATA openA;
		openA.noperateType = GetRanOpType();
		openA.type = 3;
		memcpy(openA.txhash, uint256(strAsendHash).begin(), sizeof(openA.txhash));
		memcpy(openA.dhash, nSdata, sizeof(nSdata));
		CDataStream scriptData(SER_DISK, CLIENT_VERSION);
		scriptData << openA;
		string openAcontract = HexStr(scriptData);

		int nCurHight;
		GetBlockHeight(nCurHight);
		Value vopenA = CreateContractTx(scriptid, ADDR_A, openAcontract, nCurHight, 0);
		BOOST_CHECK(GetHashFromCreatedTx(vopenA, strAopenHash));
		BOOST_CHECK(GenerateOneBlock());
		string index = "";
		if (basetest.GetTxConfirmedRegID(strAopenHash, index)) {
				mCurStep = 1;
				return true;
		}
		return true;
}

