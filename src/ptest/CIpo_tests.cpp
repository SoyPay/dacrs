/*
 * CAnony_tests.cpp
 *
 *  Created on: 2015-04-24
 *      Author: frank
 */

#include "CIpo_tests.h"
#include "CycleTestManger.h"

typedef struct {
	unsigned char address[35];
	int64_t money;
	int64_t freemoney;
	int64_t freeMothmoney;
	IMPLEMENT_SERIALIZE
	(
			for(int i = 0;i < 35;i++)
			READWRITE(address[i]);
			READWRITE(money);
			READWRITE(freemoney);
			READWRITE(freeMothmoney);
	)
}IPO_USER;

//#define max_user 3

#define max_user 100
//const static IPO_USER userarray[max_user]=
//{
//		{"ddMuEBkAwhcb5K5QJ83MqQHrgHRn4EbRdh",10000,200,22},
//		{"duKfNyq6zsuy2CMGMFVrQLuGi95UC7w6DV",10000,200,22},
//		{"djhuAYvWsfFyjF42qDqTSm88nkfZbDW1BZ",10000,200,22}                   /// 这个没有注册的账户必须先打点钱
//};

 static IPO_USER userarray[max_user];
CIpoTest::CIpoTest():nNum(0), nStep(0), strTxHash(""), strAppRegId("") {

}

TEST_STATE CIpoTest::Run(){
//	switch(nStep){
//	case 0:
//	{
//		if(RegistScript())
//			nStep = 2;
//		break;
//	}
//	case 1:
//		CreateIpoTx();
//		break;
//	case 2:
//		if(WaitComfirmed(strTxHash, strAppRegId)) {
//			nStep = 1;
//		}
//		break;
//	default:
//		nStep = 1;
//		break;
//	}
//	return next_state;


	for (int i = 0; i < max_user; i++) {
		string newaddr;
		BOOST_CHECK(basetest.GetNewAddr(newaddr, true));
		memcpy((char*)userarray[i].address,(char*)newaddr.c_str(),sizeof(userarray[i].address));
		userarray[i].money = 10000;
		userarray[i].freemoney = 200;
		userarray[i].freeMothmoney = 22;
	}


	RegistScript();

	while(true)
	{
		if(WaitComfirmed(strTxHash, strAppRegId)) {
					break;
				}
	}
	int64_t money = 10;
	for(int i=0;i <max_user;i++)
	{
		string des =strprintf("%s", userarray[i].address);
		basetest.CreateNormalTx(des,money);
	}

	while(true)
	{
		if(basetest.IsMemoryPoolEmpty())
			break;
		sleep(100);
	}


	SendIpoTx();
}

bool CIpoTest::RegistScript(){

	const char* pKey[] = { "cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU",
			"dk2NNjraSvquD9b4SQbysVRQeFikA55HLi"};
	int nCount = sizeof(pKey) / sizeof(char*);
	basetest.ImportWalletKey(pKey, nCount);

	string strFileName("IpoApp.bin");
	int nFee = basetest.GetRandomFee();
	int nCurHight;
	basetest.GetBlockHeight(nCurHight);
	string regAddr="dk2NNjraSvquD9b4SQbysVRQeFikA55HLi";

	//reg anony app
	Value regscript = basetest.RegisterAppTx(regAddr, strFileName, nCurHight, nFee+20*COIN);
	if(basetest.GetHashFromCreatedTx(regscript, strTxHash)){
		return true;
	}
	return false;
}

bool CIpoTest::CreateIpoTx(string contact,int64_t llSendTotal){
	int pre =0xff;
	int type = 2;
	string buffer =strprintf("%02x%02x", pre,type);

	buffer += contact;

	Value  retValue = basetest.CreateContractTx(strAppRegId, SEND_A, buffer, 0, 10*COIN, llSendTotal);
	if(basetest.GetHashFromCreatedTx(retValue, strTxHash)){
			return true;
	}
	return false;
}
bool CIpoTest::SendIpoTx()
{
	for(int i =0;i <max_user;i++)
	{
		CDataStream scriptData(SER_DISK, CLIENT_VERSION);
		scriptData << userarray[i];
		string sendcontract = HexStr(scriptData);
		CreateIpoTx(sendcontract,userarray[i].money);
	}
	return true;
}
BOOST_FIXTURE_TEST_SUITE(CreateIpoTxTest,CIpoTest)

BOOST_FIXTURE_TEST_CASE(Test,CIpoTest)
{
	Run();
}

BOOST_AUTO_TEST_SUITE_END()
