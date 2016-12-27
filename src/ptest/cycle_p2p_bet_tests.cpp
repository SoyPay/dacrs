/*
 * CycleP2PBet_test.cpp
 *
 *  Created on: 2015年1月15日
 *      Author: spark.huang
 */

#include "cycle_p2p_bet_tests.h"

enum emOPERATE{
	EM_OP_SYSTEMACC = 0x00,
	EM_OP_APPACC = 0x01,
	EM_OP_NULL = 0x02,
};
#define OUT_HEIGHT 20
CTestBetTx::CTestBetTx() {
	memset(m_arrnchSdata, 0, sizeof(m_arrnchSdata));
	m_nCurStep = 0;
	m_strRegScriptHash = "";
	m_strScriptid = "";
	m_strRegScriptHash = "";
	m_strAsendHash = "";
	m_strBacceptHash = "";
	m_strAopenHash = "";
	m_llBetamount = 0;
}

CTestBetTx::~CTestBetTx() {
}

emTEST_STATE CTestBetTx::Run() {
	switch (m_nCurStep) {
		case 0:{
			RegScript();
			break;
		}
		case 1:{
			WaiteRegScript();
			break;
		}
		case 2:{
			ASendP2PBet();
			break;
		}
		case 3:{
			WaitASendP2PBet();
			break;
		}
		case 4:{
			BAcceptP2PBet();
			break;
		}
		case 5:{
			WaitBAcceptP2PBet();
			break;
		}
		case 6:{
			AOpenP2PBet();
			break;
		}
		case 7:{
			WaitAOpenP2PBet();
			break;
		}
		default:{
			assert(0);
			break;
		}
	}
	return EM_NEXT_STATE;
}


bool CTestBetTx::RegScript(void) {

	const char* pkKey[] = {
			"cPqVgscsWpPgkLHZP3pKJVSU5ZTCCvVhkd5cmXVWVydXdMTtBGj7",
			"cU1dxQgvyKt8yEqqkKiNLK9jfyW498RKi8y2evqzjtLXrLD4fBMs", };
	int nCount = sizeof(pkKey) / sizeof(char*);
	m_cBasetest.ImportWalletKey(pkKey, nCount);

	string strFileName("p2pbet.bin");
	int nCurHight;
	GetBlockHeight(nCurHight);
	//注册对赌脚本
	Value valueRes = RegisterAppTx(ADDR_A, strFileName, nCurHight, 200000000);
	//BOOST_CHECK(GetHashFromCreatedTx(valueRes, m_strRegScriptHash));
	if(GetHashFromCreatedTx(valueRes, m_strRegScriptHash)){
		m_nCurStep++;
		return true;
	}
//	BOOST_CHECK(GenerateOneBlock());
	return true;
}

bool CTestBetTx::WaiteRegScript(void){
	if (m_cBasetest.GetTxConfirmedRegID(m_strRegScriptHash, m_strScriptid)) {
			m_nCurStep++;
			return true;
	}
	return true;
}

bool CTestBetTx::ASendP2PBet() {

	if(m_strScriptid == "") {
		return false;
	}
	unsigned char arruchRandData[32];
	GetRandomData(arruchRandData, sizeof(arruchRandData));
	int nNum = GetBetData();
//		cout<<"win:"<<num<<endl;
	memcpy(m_arrnchSdata,arruchRandData,sizeof(arruchRandData));
	memcpy(&m_arrnchSdata[32],&nNum,sizeof(char));
	ST_SEND_DATA tSendData;

	tSendData.uchNoperateType = GetRanOpType();
	tSendData.uchType = 1;
	tSendData.usHight = OUT_HEIGHT;
	tSendData.llMoney = GetRandomBetAmount();
	m_llBetamount = tSendData.llMoney;
	memcpy(tSendData.arruchDhash, Hash(m_arrnchSdata, m_arrnchSdata + sizeof(m_arrnchSdata)).begin(), sizeof(tSendData.arruchDhash));
//		vector<unsigned char>temp;
//		temp.assign(m_arrnchSdata,m_arrnchSdata + sizeof(m_arrnchSdata));
//		vector<unsigned char>temp2;
//		temp2.assign(senddata.dhash,senddata.dhash + sizeof(senddata.dhash));

	CDataStream cScriptData(SER_DISK, CLIENT_VERSION);
	cScriptData << tSendData;
	string strSendContract = HexStr(cScriptData);
	//uint64_t sendfee = GetRandomBetfee();

	uint64_t llTempSend = 0;
	if ((int)tSendData.uchNoperateType == 0x01) {
		llTempSend = 0;
	}else{
		llTempSend = m_llBetamount;
	}
	Value  sendret= CreateContractTx(m_strScriptid,ADDR_A,strSendContract,0,0,llTempSend);

	if (GetHashFromCreatedTx(sendret, m_strAsendHash)) {
		m_nCurStep++;
		return true;
	}
	return true;
}

bool CTestBetTx::WaitASendP2PBet(void){
	string strIndex = "";
	if (m_cBasetest.GetTxConfirmedRegID(m_strAsendHash, strIndex)) {
		m_nCurStep++;
		return true;
	}
	return true;
}

bool CTestBetTx::BAcceptP2PBet(void) {
	unsigned char uchType;
	RAND_bytes(&uchType, sizeof(uchType));
	unsigned char  uchGussnum = uchType % 2;
	ST_ACCEPT_DATA tAcceptData;
	tAcceptData.uchType = 2;
	tAcceptData.uchNoperateType = GetRanOpType();
	tAcceptData.llMoney = m_llBetamount;
	tAcceptData.uchData=uchGussnum;
	memcpy(tAcceptData.uchTxhash, uint256S(m_strAsendHash).begin(), sizeof(tAcceptData.uchTxhash));

	CDataStream cScriptData(SER_DISK, CLIENT_VERSION);
	cScriptData << tAcceptData;

	string strAcceptContract = HexStr(cScriptData);
	int nCurHight;
	GetBlockHeight(nCurHight);

	uint64_t llTempSend = 0;
	if((int)tAcceptData.uchNoperateType  == 0x01){
		llTempSend = 0;
	}else{
		llTempSend = m_llBetamount;
	}
	Value vaccept = CreateContractTx(m_strScriptid, ADDR_B, strAcceptContract, nCurHight, 0,llTempSend);
	string strHash = "";
//		cout<<"type"<<(int)acceptdata.noperateType<<endl;
/*		if((int)acceptdata.noperateType == 0x01){

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
	}*/

	if (GetHashFromCreatedTx(vaccept, m_strBacceptHash)) {
		m_nCurStep++;
		return true;
	}
	return true;
}

bool CTestBetTx::WaitBAcceptP2PBet(void) {
	string strIndex = "";
	if (m_cBasetest.GetTxConfirmedRegID(m_strBacceptHash, strIndex)) {
		m_nCurStep++;
		return true;
	}
	return true;
}

bool CTestBetTx::AOpenP2PBet(void) {

	ST_OPEN_DATA tOpenA;
	tOpenA.uchNoperateType = GetRanOpType();
	tOpenA.uchType = 3;
	memcpy(tOpenA.arruchTxhash, uint256S(m_strAsendHash).begin(), sizeof(tOpenA.arruchTxhash));
	memcpy(tOpenA.arruchDhash, m_arrnchSdata, sizeof(m_arrnchSdata));
	CDataStream cScriptData(SER_DISK, CLIENT_VERSION);
	cScriptData << tOpenA;
	string openAcontract = HexStr(cScriptData);

	int nCurHight;
	GetBlockHeight(nCurHight);
	Value vopenA = CreateContractTx(m_strScriptid, ADDR_A, openAcontract, nCurHight, 0);
	if (GetHashFromCreatedTx(vopenA, m_strAopenHash)) {
		m_nCurStep++;
		return true;
	}
	return true;
}

bool CTestBetTx::WaitAOpenP2PBet(void) {
	string strIndex = "";
	if (m_cBasetest.GetTxConfirmedRegID(m_strAopenHash, strIndex)) {
		m_nCurStep = 1;
		IncSentTotal();
		return true;
	}
	return true;
}
