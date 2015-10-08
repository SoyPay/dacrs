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


/*
 * 测试地址及余额
 * */
#if 0
{
    "Address" : "dcmCbKbAfKrofNz35MSFupxrx4Uwn3vgjL",
    "KeyID" : "000f047f75144705b0c7f4eb30d205cd66f4599a",
    "RegID" : "1826-1437",
    "PublicKey" : "02bbb24c80a808cb6eb13de90c1dca99196bfce02bcf32812b7c4357a368877c68",
    "MinerPKey" : "02d546cc51b22621b093f08c679102b9b8ca3f1a07ea1d751de3f67c10670e635b",
    "Balance" : 129999825142,
    "CoinDays" : 0,
    "UpdateHeight" : 45433,
    "CurCoinDays" : 1457,
    "postion" : "inblock"
}
{
    "Address" : "dcmWdcfxEjRXUHk8LpygtgDHTpixoo3kbd",
    "KeyID" : "001e1384420cad01b0cd364ef064852b1bf3fd96",
    "RegID" : "1826-1285",
    "PublicKey" : "03d765b0f2bae7f6f61350b17bce5e57445cc286cada56d9c61987db5cbd533c43",
    "MinerPKey" : "025cae56b5faf1042f2d6610cde892f0cb1178282fb7b345b78611ccee4feab128",
    "Balance" : 128999717658,
    "CoinDays" : 0,
    "UpdateHeight" : 46859,
    "CurCoinDays" : 169,
    "postion" : "inblock"
}
{
    "Address" : "dcnGLkGud6c5bZJSUghzxvCqV45SJEwRcH",
    "KeyID" : "00429013e06bbcdc0529dd5b1117ddf4630544ad",
    "RegID" : "1826-1081",
    "PublicKey" : "02b509a4240ae08118ff2336981301cb2baf6207faf86aa1731a9ce8443e72f7f0",
    "MinerPKey" : "0394b395e1ef08f9c6e71eb2ecd70fe511f7ec0c0fe5a96c139fd4589b8f8a671c",
    "Balance" : 126999676405,
    "CoinDays" : 0,
    "UpdateHeight" : 45130,
    "CurCoinDays" : 1691,
    "postion" : "inblock"
}
#endif

//!<开发者账户，接收担保押金
//string regAddr="dk2NNjraSvquD9b4SQbysVRQeFikA55HLi";   //RegID = "0-20"

#define ADDR_SEND_A        "dcmCbKbAfKrofNz35MSFupxrx4Uwn3vgjL"  //挂单者A  220700009d05 买家
// 1826-1437

#define ADDR_ACCEPT_B      "dcmWdcfxEjRXUHk8LpygtgDHTpixoo3kbd"  //接单者B  220700000505 卖家
// 1826-1285

#define ADDR_ARBITRATION_C "dcnGLkGud6c5bZJSUghzxvCqV45SJEwRcH"  //仲裁者C   220700003904
// 1826-1081

#define ID_strAppRegId  "47172-1"    //脚本应用ID 待填
//#define HASH_sendhash     "7de1faafc2c9f14be5294f5f2b1082eaf92c7d66da5d42be1016e0988143318d"  //挂单交易hash 待填
static const unsigned char HASH_sendhash[] ={
//		0x77,0xf2,0xce,0xaa,0xcc,0xc5,0x49,0xd9,
//		0x9d,0x6c,0xad,0x1e,0x20,0x4b,0x7c,0xed,
//		0x0f,0x00,0x50,0x77,0xce,0xc8,0xb3,0x4d,
//		0x6c,0xe3,0x7f,0x40,0x4d,0xa4,0x3b,0x6e

//		0x4f,0xe5,0xcd,0xc0,0xc4,0x23,0x1e,0x5f,
//		0xa5,0x27,0x22,0x88,0x94,0xfe,0x45,0x47,
//		0x29,0xf0,0xbd,0x5a,0x54,0x98,0xe2,0x72,
//		0x10,0x81,0xc6,0xd9,0x17,0x94,0x5c,0x1a

		0x5f,0xc8,0xd8,0xd3,0xe7,0x4f,0xf5,0xc7,
		0xa4,0xdf,0x66,0x82,0x5c,0x04,0x58,0xf7,
		0x74,0x9e,0x5c,0x63,0xac,0x62,0xd0,0x22,
		0x6d,0x92,0xb9,0xa9,0x8f,0x43,0x71,0x95
};
static const unsigned char HASH_accepthash[] ={//接单交易hash 待填
		0x4f,0xe5,0xcd,0xc0,0xc4,0x23,0x1e,0x5f,
		0xa5,0x27,0x22,0x88,0x94,0xfe,0x45,0x47,
		0x29,0xf0,0xbd,0x5a,0x54,0x98,0xe2,0x72,
		0x10,0x81,0xc6,0xd9,0x17,0x94,0x5c,0x1a
};


//!<仲裁者C的配置信息
#define ARBITER_arbiterMoneyX      (2 * 1000000)      //!<仲裁费用X
#define ARBITER_overtimeMoneyYmax  (1 * 100000000) //!<超时未判决的最大赔偿费用Y
#define ARBITER_configMoneyZ       (1 * 1000000)       //!<无争议裁决费用Z
#define ARBITER_overtimeheightT    (1 * 1440)  //!<判决期限时间T

//!<挂单者的配置信息
#define SEND_moneyM     (2 * 1000000)    //!<交易金额
#define SEND_height     (2 * 1440)       //!<每个交易环节的超时高度

#define  ARBITER_winnerMoney  (1 * 1000000) //!<裁决后，赢家分配金额

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
//	size_t t_num = sizeof(arrayData) / sizeof(arrayData[0]);
//	BOOST_CHECK(t_num <= max_user);         //防止越界
//	for(int i=0;i <t_num;i++)
	{
		string des =strprintf("%s", ADDR_ARBITRATION_C);//userarray[i].address);
		basetest.CreateNormalTx(des,money);

//		des =strprintf("%s", ADDR_ACCEPT_B);//userarray[i].address);
//		basetest.CreateNormalTx(des,money);
//
//		des =strprintf("%s", ADDR_SEND_A);//userarray[i].address);
//		basetest.CreateNormalTx(des,money);
	}

	 cout<<"end mempool"<<endl;
	while(true)
	{
		if(basetest.IsMemoryPoolEmpty())
			break;
		MilliSleep(100);
	}
#endif

//	Recharge();
//	Withdraw();

	Register(TX_REGISTER);
//	Register(TX_MODIFYREGISTER);
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
	Value regscript = basetest.RegisterAppTx(regAddr, strFileName, nCurHight, nFee + 1 *COIN);// + 20 *COIN
	if(basetest.GetHashFromCreatedTx(regscript, strTxHash)){
		return true;
	}
	return false;
}

bool CGuaranteeTest::Recharge()
{
   cout<<"Recharge start"<<endl;

   APPACC senddata;
	memset(&senddata,0,sizeof(senddata));
	senddata.systype = 0xff;
	senddata.type = 0x02;

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 1 * COIN;
    cout<<"Recharge data:"<<sendcontract<<endl;
	Value  retValue= basetest.CreateContractTx(strAppRegId,ADDR_ARBITRATION_C,sendcontract,0,0,nTempSend);
//   ADDR_SEND_A   //	Id = "1826-1437";
//   ADDR_ACCEPT_B   //	Id = "1826-1285";
//	ADDR_ARBITRATION_C  Id = "1826-1081"
	if (basetest.GetHashFromCreatedTx(retValue, strTxHash)) {
		nStep++;
	    cout<<"Recharge success end"<<endl;
		return true;
	}
	cout<<"Recharge fail end"<<endl;
	return false;
}

bool CGuaranteeTest::Withdraw()
{
   cout<<"Withdraw start"<<endl;

   APPACC senddata;
	memset(&senddata,0,sizeof(senddata));
	senddata.systype = 0xff;
	senddata.type = 0x03;
	senddata.typeaddr = TX_BASE58;
	senddata.money = 1 * COIN;

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
    cout<<"Withdraw data:"<<sendcontract<<endl;
	Value  retValue= basetest.CreateContractTx(strAppRegId,ADDR_SEND_A,sendcontract,0,0,nTempSend);

	if (basetest.GetHashFromCreatedTx(retValue, strTxHash)) {
		nStep++;
	    cout<<"Withdraw success end"<<endl;
		return true;
	}
	cout<<"Withdraw fail end"<<endl;
	return false;
}

bool CGuaranteeTest::Register(unsigned char type)
{
	TX_REGISTER_CONTRACT senddata;
	memset(&senddata,0,sizeof(senddata));
    if(type == TX_REGISTER)
    {
    	senddata.type = TX_REGISTER;
    	 cout<<"TX_REGISTER start"<<endl;
    }else if(type == TX_MODIFYREGISTER)
    {
    	senddata.type = TX_MODIFYREGISTER;
    	 cout<<"TX_MODIFYREGISTER start"<<endl;
    }
    else{
    	cout<<"Register input err"<<endl;
    	return false;
    }
	senddata.arbiterMoneyX = ARBITER_arbiterMoneyX;
	senddata.overtimeMoneyYmax = ARBITER_overtimeMoneyYmax;
	senddata.configMoneyZ = ARBITER_configMoneyZ;//0x8234567812345678
	senddata.overtimeheightT = ARBITER_overtimeheightT;
    strcpy(senddata.comment,"联系电话:13418656754");

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
    cout<<"Register data:"<<sendcontract<<endl;
    cout<<"Register strAppRegId:"<<strAppRegId.c_str()<<endl;
	Value  retValue= basetest.CreateContractTx(strAppRegId,ADDR_ARBITRATION_C,sendcontract,0,0,nTempSend);

	if (basetest.GetHashFromCreatedTx(retValue, strTxHash)) {
		nStep++;
	    cout<<"Register success"<<endl;
	}else
	{
	    cout<<"Register fail"<<endl;
	    return false;
	}
#if 0
	/// 等待交易被确认到block中
	while(true)
	{
		if(WaitComfirmed(strTxHash, strAppRegId)) {
					break;
				}
	}
#endif
	cout<<"Register success stop"<<endl;
	return true;
}

bool CGuaranteeTest::UnRegister()
{
   cout<<"UnRegister start"<<endl;

   unsigned char senddata = TX_UNREGISTER;

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
	cout<<"UnRegister data:"<<sendcontract.c_str()<<endl;
	Value  retValue= basetest.CreateContractTx(strAppRegId,ADDR_ARBITRATION_C,sendcontract,0,0,nTempSend);

	if (basetest.GetHashFromCreatedTx(retValue, strTxHash)) {
		nStep++;
		cout<<"UnRegister success end"<<endl;
		return true;
	}
#if 0
	/// 等待交易被确认到block中
	while(true)
	{
		if(WaitComfirmed(strTxHash, strAppRegId)) {
					break;
				}
	}
#endif

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
//	senddata.sendType = SEND_TYPE_SELL;   //待修改 挂单类型

//	string arbitationID = "47046-1";
	unsigned int height = 1826;   //待填   仲裁者ID arbiterAddr_C
	unsigned short index = 1081;

	memcpy(&senddata.arbitationID[0],&height,4);
    memcpy(&senddata.arbitationID[4],&index,2);
	senddata.moneyM = SEND_moneyM;
	senddata.height = SEND_height;
	strcpy(senddata.goods,"小米3手机");
	strcpy(senddata.comment,"1个币买.联系电话:13418656754");

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
	cout<<"SendStartTrade data:"<<sendcontract.c_str()<<endl;
	Value sendret;
    if(senddata.sendType == SEND_TYPE_BUY)
    {
    	sendret= basetest.CreateContractTx(strAppRegId,ADDR_SEND_A,sendcontract,0,0,nTempSend); // 待填写
    }else{
    	sendret= basetest.CreateContractTx(strAppRegId,ADDR_ACCEPT_B,sendcontract,0,0,nTempSend); // 待填写
    }

	if (basetest.GetHashFromCreatedTx(sendret, strTxHash)) {
		nStep++;
		cout<<"SendStartTrade success end"<<endl;
#if 0
		/// 等待交易被确认到block中
		while(true)
		{
			if(WaitComfirmed(strTxHash, strAppRegId)) {
						break;
					}
		}
#endif
		return true;
	}
	cout<<"SendStartTrade fail end"<<endl;
	return true;
}

bool CGuaranteeTest::SendCancelTrade()
{
   cout<<"SendCancelTrade start"<<endl;

	TX_CONTRACT senddata;
	memset(&senddata,0,sizeof(senddata));
	senddata.type = TX_CANCEL;
//	memcpy(senddata.txhash, uint256S(HASH_sendhash).begin(), sizeof(senddata.txhash)); //待填交易HASH
	memcpy(senddata.txhash,HASH_sendhash, sizeof(senddata.txhash)); //待填交易HASH

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
	cout<<"SendCancelTrade data:"<<sendcontract.c_str()<<endl;
	Value  sendret= basetest.CreateContractTx(strAppRegId,ADDR_SEND_A,sendcontract,0,0,nTempSend); // 取消挂买单
//    Value  sendret= basetest.CreateContractTx(strAppRegId,ADDR_ACCEPT_B,sendcontract,0,0,nTempSend); //取消挂卖单
	if (basetest.GetHashFromCreatedTx(sendret, strTxHash)) {
		nStep++;
		cout<<"SendCancelTrade success end"<<endl;
		return true;
	}
	cout<<"SendCancelTrade fail end"<<endl;
	return true;
}

bool CGuaranteeTest::AcceptTrade()
{
   cout<<"AcceptTrade start"<<endl;

	TX_CONTRACT senddata;
	memset(&senddata,0,sizeof(senddata));
	senddata.type = TX_ACCEPT;
	memcpy(senddata.txhash,HASH_sendhash, sizeof(senddata.txhash)); //待填交易HASH

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
	cout<<"AcceptTrade data:"<<sendcontract.c_str()<<endl;
	Value  sendret= basetest.CreateContractTx(strAppRegId,ADDR_ACCEPT_B,sendcontract,0,0,nTempSend);//卖家接单
//	Value  sendret= basetest.CreateContractTx(strAppRegId,ADDR_SEND_A,sendcontract,0,0,nTempSend);//买家接单

	if (basetest.GetHashFromCreatedTx(sendret, strTxHash)) {
		nStep++;
		cout<<"AcceptTrade success end"<<endl;
		return true;
	}
	cout<<"AcceptTrade fail end"<<endl;
	return true;
}
bool CGuaranteeTest::BuyerConfirm()
{
   cout<<"BuyerConfirm start"<<endl;

	TX_CONTRACT senddata;
	memset(&senddata,0,sizeof(senddata));
	senddata.type = TX_BUYERCONFIRM;
	memcpy(senddata.txhash,HASH_sendhash, sizeof(senddata.txhash)); //待填交易HASH

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
	cout<<"BuyerConfirm data:"<<sendcontract.c_str()<<endl;
	Value  sendret= basetest.CreateContractTx(strAppRegId,ADDR_SEND_A,sendcontract,0,0,nTempSend); //待填写

	if (basetest.GetHashFromCreatedTx(sendret, strTxHash)) {
		nStep++;
		cout<<"BuyerConfirm success end"<<endl;
		return true;
	}
	cout<<"BuyerConfirm fail end"<<endl;
	return true;
}


bool CGuaranteeTest::Arbitration()
{
   cout<<"Arbitration start"<<endl;

   TX_Arbitration senddata;
	memset(&senddata,0,sizeof(senddata));
	senddata.type = TX_ARBITRATION;
	memcpy(senddata.txhash,HASH_sendhash, sizeof(senddata.txhash)); //待填交易HASH
//	string arbitationID = "47046-1";
	unsigned int height = 1826;   //待填   仲裁者ID arbiterAddr_C
	unsigned short index = 1081;

	memcpy(&senddata.arbitationID[0],&height,4);
	memcpy(&senddata.arbitationID[4],&index,2);


	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
	cout<<"Arbitration data:"<<sendcontract.c_str()<<endl;
	Value  sendret= basetest.CreateContractTx(strAppRegId,ADDR_SEND_A,sendcontract,0,0,nTempSend);// 待填写 ADDR_ARBITRATION_C

	if (basetest.GetHashFromCreatedTx(sendret, strTxHash)) {
		nStep++;
		cout<<"Arbitration success end"<<endl;
		return true;
	}
	cout<<"Arbitration fail end"<<endl;
	return true;
}
bool CGuaranteeTest::RunFinalResult()
{
   cout<<"RunFinalResult start"<<endl;

	TX_FINALRESULT_CONTRACT senddata;
	memset(&senddata,0,sizeof(senddata));
	senddata.type = TX_FINALRESULT;
	memcpy(senddata.sendhash, HASH_sendhash, sizeof(senddata.sendhash)); //待填交易HASH
	memcpy(senddata.accepthash, HASH_accepthash, sizeof(senddata.accepthash)); //待填交易HASH

//	string buyId = "1826-1437";
	unsigned int height = 1826;   //待填   赢家 ID 买家 赢
	unsigned short index = 1437;  //1437

    memcpy(&senddata.winner[0],&height,4);
    memcpy(&senddata.winner[4],&index,2);
	senddata.winnerMoney = ARBITER_winnerMoney;

//	string sellerId = "1826-1285";
	height = 1826;  //待填   输家ID
	index = 1285;  //1285
    memcpy(&senddata.loser[0],&height,4);
    memcpy(&senddata.loser[4],&index,2);
	senddata.loserMoney = SEND_moneyM - ARBITER_winnerMoney;  //  交易金额M - 赢家分配的钱  待填写

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;
	cout<<"RunFinalResult data:"<<sendcontract.c_str()<<endl;
	Value  sendret= basetest.CreateContractTx(strAppRegId,ADDR_ARBITRATION_C,sendcontract,0,0,nTempSend);

	if (basetest.GetHashFromCreatedTx(sendret, strTxHash)) {
		nStep++;
		cout<<"RunFinalResult success end"<<endl;
		return true;
	}
	cout<<"RunFinalResult fail end"<<endl;
	return true;
}

BOOST_FIXTURE_TEST_SUITE(CGuaranteeTxTest,CGuaranteeTest)

BOOST_FIXTURE_TEST_CASE(Test,CGuaranteeTest)
{
	Run();
}
BOOST_AUTO_TEST_SUITE_END()

