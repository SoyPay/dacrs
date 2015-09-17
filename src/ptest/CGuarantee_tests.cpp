/*
 * CAnony_tests.cpp
 *
 *  Created on: 2015-04-24
 *      Author: frank
 */

#include "CGuarantee_tests.h"
#include "CycleTestManger.h"
#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_reader.h"
using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;


#define ADDR_SEND_A        "dyjC8fuSoVGpepRGi8F2SridVX4VjykLG4"  //挂单者A
#define ADDR_ACCEPT_B      "dfLo3CHErzWPrxRtthMiitcoGkrR6DskiF"  //接单者B
#define ADDR_ARBITRATION_C "dwpwbNeGEP9bZzFfNnn4gK96LKNA83R93D"  //仲裁者C

#define ID_strAppRegId  "47048-1"    //脚本应用ID 待填
#define HASH_sendhash     "7de1faafc2c9f14be5294f5f2b1082eaf92c7d66da5d42be1016e0988143318d"  //挂单交易hash 待填
#define HASH_accepthash   "da30cc269660c9f120d6a6e5364493ef7d9aaa1a6cc54a434c9c10840dfefc2f"  //接单交易hash 待填

CGuaranteeTest::CGuaranteeTest():nNum(0), nStep(0), strTxHash(""), strAppRegId(ID_strAppRegId) {

}

TEST_STATE CGuaranteeTest::Run(){

	cout<<"CGuaranteeTest run start"<<endl;
#if 0
    // 注册ipo脚本
	RegistScript();

	/// 等待ipo脚本被确认到block中
	while(true)
	{
		if(WaitComfirmed(strTxHash, strAppRegId)) {
					break;
				}
	}
#endif

#if 0
	/// 给每个地址转一定的金额
	int64_t money = COIN;
	size_t t_num = sizeof(arrayData) / sizeof(arrayData[0]);
	BOOST_CHECK(t_num <= max_user);         //防止越界
	for(int i=0;i <t_num;i++)
	{
		string des =strprintf("%s", userarray[i].address);
		basetest.CreateNormalTx(des,money);
	}

	 cout<<"end mempool"<<endl;
	while(true)
	{
		if(basetest.IsMemoryPoolEmpty())
			break;
		MilliSleep(100);
	}
#endif

	Register();
//	UnRegister();
//	SendStartTrade();
//	SendCancelTrade();
//	AcceptTrade();
//	BuyerConfirm();
//	Arbitration();
//	RunFinalResult();
	cout<<"CGuaranteeTest run end"<<endl;
	return end_state;
}


bool CGuaranteeTest::RegistScript(){

	const char* pKey[] = { "cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU",
			"cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU"};
	int nCount = sizeof(pKey) / sizeof(char*);
	basetest.ImportWalletKey(pKey, nCount);

	string strFileName("guarantee.bin");
	int nFee = basetest.GetRandomFee();
	int nCurHight;
	basetest.GetBlockHeight(nCurHight);
	string regAddr="dk2NNjraSvquD9b4SQbysVRQeFikA55HLi";

	//reg anony app
	Value regscript = basetest.RegisterAppTx(regAddr, strFileName, nCurHight, nFee+1*COIN);//20
	if(basetest.GetHashFromCreatedTx(regscript, strTxHash)){
		return true;
	}
	return false;
}

bool CGuaranteeTest::Register()
{
   cout<<"Register start"<<endl;

	TX_REGISTER_CONTRACT senddata;
	memset(&senddata,0,sizeof(senddata));
	senddata.type = TX_REGISTER;
	senddata.arbiterMoneyX = 0.1 * COIN;
	senddata.overtimeMoneyYmax = 1 * COIN;
	senddata.configMoneyZ = 0.01 * COIN;
	senddata.overtimeheightT = 1 * 1440;
	senddata.ucLen = strlen("联系电话:13418656754");
    strcpy(senddata.comment,"联系电话:13418656754");

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
    cout<<"Register data:"<<sendcontract<<endl;
	Value  retValue= basetest.CreateContractTx(strAppRegId,ADDR_ARBITRATION_C,sendcontract,0,0,nTempSend);

	if (basetest.GetHashFromCreatedTx(retValue, strTxHash)) {
		nStep++;
	    cout<<"Register success end"<<endl;
		return true;
	}
	cout<<"Register fail end"<<endl;
	return false;
}

bool CGuaranteeTest::UnRegister()
{
   cout<<"UnRegister start"<<endl;

   unsigned char senddata = TX_UNREGISTER;

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
	cout<<"UnRegister data:"<<sendcontract.c_str()<<endl;;
	Value  retValue= basetest.CreateContractTx(strAppRegId,ADDR_ARBITRATION_C,sendcontract,0,0,nTempSend);

//	if (basetest.GetHashFromCreatedTx(retValue, strTxHash)) {
//		nStep++;
//		cout<<"UnRegister success end"<<endl;
//		return true;
//	}
	cout<<"UnRegister fail end"<<endl;
	return false;
}

bool CGuaranteeTest::SendStartTrade()
{
	cout<<"SendStartTrade start"<<endl;

	TX_SNED_CONTRACT senddata;
	memset(&senddata,0,sizeof(senddata));
	senddata.type = TX_SEND;
	senddata.sendType = SEND_TYPE_BUY;   //待修改 挂单类型
//	string arbitationID = "47046-1";
	unsigned int height = 47046;   //待填   仲裁者ID arbiterAddr_C
	unsigned short index = 1;
    memcpy(&senddata.arbitationID[0],&height,4);
    memcpy(&senddata.arbitationID[4],&index,2);
	senddata.moneyM = 1 * COIN;
	senddata.height = 2 * 1440;
	senddata.ucLen = strlen("用1个币买一部手机.联系电话:13418656754");
	strcpy(senddata.comment,"用1个币买一部手机.联系电话:13418656754");

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
	cout<<"SendStartTrade data:"<<sendcontract.c_str()<<endl;
//	Value  sendret= basetest.CreateContractTx(strAppRegId,ADDR_SEND_A,sendcontract,0,0,nTempSend); // 待填写
//
//	if (basetest.GetHashFromCreatedTx(sendret, strTxHash)) {
//		nStep++;
//		cout<<"SendStartTrade success end"<<endl;
//		return true;
//	}
	cout<<"SendStartTrade fail end"<<endl;
	return true;
}

bool CGuaranteeTest::SendCancelTrade()
{
   cout<<"SendCancelTrade start"<<endl;

	TX_CONTRACT senddata;
	memset(&senddata,0,sizeof(senddata));
	senddata.type = TX_CANCEL;
	memcpy(senddata.txhash, uint256S(HASH_sendhash).begin(), sizeof(senddata.txhash)); //待填交易HASH

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
	cout<<"SendCancelTrade data:"<<sendcontract.c_str()<<endl;
//	Value  sendret= basetest.CreateContractTx(strAppRegId,ADDR_SEND_A,sendcontract,0,0,nTempSend); // 待填写
//
//	if (basetest.GetHashFromCreatedTx(sendret, strTxHash)) {
//		nStep++;
//		cout<<"SendCancelTrade success end"<<endl;
//		return true;
//	}
	cout<<"SendCancelTrade fail end"<<endl;
	return true;
}

bool CGuaranteeTest::AcceptTrade()
{
   cout<<"AcceptTrade start"<<endl;

	TX_CONTRACT senddata;
	memset(&senddata,0,sizeof(senddata));
	senddata.type = TX_ACCEPT;
	memcpy(senddata.txhash, uint256S(HASH_sendhash).begin(), sizeof(senddata.txhash)); //待填交易HASH

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
	cout<<"AcceptTrade data:"<<sendcontract.c_str()<<endl;
//	Value  sendret= basetest.CreateContractTx(strAppRegId,ADDR_ACCEPT_B,sendcontract,0,0,nTempSend);//待填写
//
//	if (basetest.GetHashFromCreatedTx(sendret, strTxHash)) {
//		nStep++;
//		cout<<"AcceptTrade success end"<<endl;
//		return true;
//	}
	cout<<"AcceptTrade fail end"<<endl;
	return true;
}
bool CGuaranteeTest::BuyerConfirm()
{
   cout<<"BuyerConfirm start"<<endl;

	TX_CONTRACT senddata;
	memset(&senddata,0,sizeof(senddata));
	senddata.type = TX_BUYERCONFIRM;
	memcpy(senddata.txhash, uint256S(HASH_sendhash).begin(), sizeof(senddata.txhash)); //待填交易HASH

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
	cout<<"BuyerConfirm data:"<<sendcontract.c_str()<<endl;
//	Value  sendret= basetest.CreateContractTx(strAppRegId,ADDR_SEND_A,sendcontract,0,0,nTempSend); //待填写
//
//	if (basetest.GetHashFromCreatedTx(sendret, strTxHash)) {
//		nStep++;
//		cout<<"BuyerConfirm success end"<<endl;
//		return true;
//	}
	cout<<"BuyerConfirm fail end"<<endl;
	return true;
}

bool CGuaranteeTest::Arbitration()
{
   cout<<"Arbitration start"<<endl;

	TX_CONTRACT senddata;
	memset(&senddata,0,sizeof(senddata));
	senddata.type = TX_ARBITRATION;
	memcpy(senddata.txhash, uint256S(HASH_sendhash).begin(), sizeof(senddata.txhash)); //待填交易HASH

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
	cout<<"Arbitration data:"<<sendcontract.c_str()<<endl;
//	Value  sendret= basetest.CreateContractTx(strAppRegId,ADDR_SEND_A,sendcontract,0,0,nTempSend);// 待填写
//
//	if (basetest.GetHashFromCreatedTx(sendret, strTxHash)) {
//		nStep++;
//		cout<<"Arbitration success end"<<endl;
//		return true;
//	}
	cout<<"Arbitration fail end"<<endl;
	return true;
}
bool CGuaranteeTest::RunFinalResult()
{
   cout<<"RunFinalResult start"<<endl;

	TX_FINALRESULT_CONTRACT senddata;
	memset(&senddata,0,sizeof(senddata));
	senddata.type = TX_FINALRESULT;
	memcpy(senddata.sendhash, uint256S(HASH_sendhash).begin(), sizeof(senddata.sendhash)); //待填交易HASH
	memcpy(senddata.accepthash, uint256S(HASH_accepthash).begin(), sizeof(senddata.accepthash)); //待填交易HASH

	unsigned int height = 47045;   //待填   赢家 ID
	unsigned short index = 1;
    memcpy(&senddata.winner[0],&height,4);
    memcpy(&senddata.winner[4],&index,2);
	senddata.winnerMoney = 0.9 * COIN;
//	height = 47046;  //待填   输家ID
//	index = 1;
//    memcpy(&senddata.loser[0],&height,4);
//    memcpy(&senddata.loser[4],&index,2);
//	senddata.loserMoney = 0.1 * COIN;  //  交易金额M - 赢家分配的钱  待填写

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
	cout<<"RunFinalResult data:"<<sendcontract.c_str()<<endl;
//	Value  sendret= basetest.CreateContractTx(strAppRegId,ADDR_ARBITRATION_C,sendcontract,0,0,nTempSend);// 待填写
//
//	if (basetest.GetHashFromCreatedTx(sendret, strTxHash)) {
//		nStep++;
//		cout<<"RunFinalResult success end"<<endl;
//		return true;
//	}
	cout<<"RunFinalResult fail end"<<endl;
	return true;
}

BOOST_FIXTURE_TEST_SUITE(CreateGuaranteeTxTest,CGuaranteeTest)

BOOST_FIXTURE_TEST_CASE(Test,CGuaranteeTest)
{
	Run();
}
BOOST_AUTO_TEST_SUITE_END()

