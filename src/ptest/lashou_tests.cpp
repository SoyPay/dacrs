/*
 * lasho_tests.cpp
 *
 *  Created on: 2016年12月29日
 *      Author: ranger.shi
 */


#include "lashou_tests.h"
#include "cycle_test_manger.h"
#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_reader.h"
using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;
#if 1

#define ADDR_USER_TESTA        "p9FHhzevXpzmVp2kH1XTQRPSn8hwJkExy2"  //测试用户A
#define ADDR_USER_TESTB        "pRHobNPmeiahWQDV1RzB6GbDfwakWR5iaz"  //测试用户B
#define ADDR_USER_TESTC        "CCCCCzevXpzmVp2kH1XTQRPSn8hwJkCCCC"  //测试用户C

//#define ADDR_USER_TESTN        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAaa"  //测试用户C
#define ADDR_USER_TESTN        "QbAfAAAAAAAAAABBAAAAAAAAAAbbccff"  //测试用户C

#define ADDR_CLAIMS_TEST        "AaAAAAAAAAAAAAAAAAAAAAAAAAAAbbc000pEHtvX7yVtF4vwEdZYADjcpCUd3QLx9xYFpGdHUEPjmkQCeNfgnx1LrdknmWfTew7ggE"  //理赔帐户
//#define ADDR_CLAIMS_TEST      "AaAAAAAAAAAAAAAAAAAAAAAAAAAAbbc002AaAAAAAAAAAAAAAAAAAAAAAAAAAAbbc003AaAAAAAAAAAAAAAAAAAAAAAAAAAAbbc004"  //理赔帐户

#define ADDR_APPLY_HASH        "DknmMz7Qkkri9GRqHGxU3uCdqDZTeGBajK"  //理赔HASH


#define ADDR_USER_A        "dqidLeyaDiYovYTJCkGKutXJF8cygP56qg"  //用户A
// 1826-1437  129692871376

#define ADDR_USER_B      "dtEpiAShCyhLNNr9X2PNYcGEB27PdG4bg1"  //用户B

// 1826-1285  128999717658

#define ADDR_SUPER "dqidLeyaDiYovYTJCkGKutXJF8cygP56qg"  //超级用户
// 1826-1081  126691618233



#define ID_strAppRegId  "258-1"    //121006-1 139429-1
CLashouTest::CLashouTest():m_nNum(0), m_nStep(0), m_strTxHash(""), m_strAppRegId(ID_strAppRegId) {

}

emTEST_STATE CLashouTest::Run(){

#if 0
// 注册ipo脚本
	if(!RegistScript()){
		cout<<"CLashouTest RegistScript err"<<endl;
		return end_state;
	}

	/// 等待ipo脚本被确认到block中
	while(true)
	{
		if(WaitComfirmed(strTxHash, strAppRegId)) {
					break;
				}
	}

#else
//	Config();
//	Modify();
	Register();
//	Recharge();
//	Withdraw();
//	ApplyForClaims();
//	ClaimsOperate();
//	ImportDate();
//	CodeTest();
//	for(uint32_t jj=0;jj<10;jj++)
//	{
//		ImportDateNN();
//	}
#endif
	cout<<"CLashouTest run end"<<endl;
	return EM_END_STATE;
}


bool CLashouTest::CodeTest(void)
{
	int DataTest=12345;
	int DT[20]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	cout << "CodeTest start" << endl;

	const char *argv[] = {"progname", "-datadir=/home/share/cgp/popcoin_test"};
	int argc = sizeof(argv) / sizeof(char*);
	CBaseParams::IntialParams(argc, argv);
	SysCfg().InitalConfig();


//	LogPrint("net", "Cgp Code test!\n");

//	cout << SysCfg().ToString() + "datadir:"+GetDataDir().string()<< endl;

	ERRORMSG(" ERRORMSG TEST ONLY STRING!\n");//只输出字符
	ERRORMSG(" ERRORMSG TEST Param1:%s,Param1:%d\n","StringOutput",DataTest);//输出两个参数

	LogPrint("CGP", " LogPrint TEST ONLY STRING!\n");//只输出字符
	LogPrint("CGP", " LogPrint TEST Param1:%s,Param1:%d\n","StringOutput",DataTest);//输出两个参数



	LogPrint("INFO", " LogPrint TEST %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s\n",DT[0],DT[1],DT[2],DT[3],DT[4],DT[5],DT[6],DT[7],DT[8],DT[9],DT[10],DT[11],DT[12],DT[13],DT[14],"End");
	LogPrint("INFO", " LogPrint TEST %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s\n",DT[0],DT[1],DT[2],DT[3],DT[4],DT[5],DT[6],DT[7],DT[8],DT[9],DT[10],DT[11],DT[12],DT[13],DT[14],"End");



	cout << "CodeTest success end"<<endl;
	return false;
}


bool CLashouTest::RegistScript(){
/*
	const char* pKey[] = { "cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU",
			"cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU"};
	int nCount = sizeof(pKey) / sizeof(char*);
	m_cBasetest.ImportWalletKey(pKey, nCount);
*/

	//string strFileName("kaola.bin");
//	string strFileName("kaola.lua");
	string strFileName("lashou.lua");
	int nFee = /*m_cBasetest.GetRandomFee()*/0.1 *COIN;
	int nCurHight;
	m_cBasetest.GetBlockHeight(nCurHight);
//	string regAddr="pGdHUEPjmkQCeNfgnx1LrdknmWfTew7ggE";
	string regAddr="pHoFvGYVtyLm5Zb3TwNPsjxqXy6ncs4A7m";
	//reg anony app
	Value regscript = m_cBasetest.RegisterAppTx(regAddr, strFileName, nCurHight, nFee + 1 *COIN);// + 20 *COIN
	if(m_cBasetest.GetHashFromCreatedTx(regscript, m_strTxHash)){

		return true;
	}
	return false;
}

//STEP:1
//配置信息，注册 、修改
bool CLashouTest::Config(void)
{
	cout<<"Config start"<<endl;

	CONFIG_ST senddata;

	memset(&senddata,0,sizeof(senddata));

	senddata.uchType = TX_CONFIG;
	senddata.uWatchDay = 1;//!观察期天数
	senddata.uMinBalance = 9;//!余额最低限值,元
	strcpy(senddata.arrchSuperAcc,ADDR_USER_A);//!超级用户


	CDataStream scriptData(SER_DISK, g_sClientVersion);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;//20 * COIN;

	cout<<"Config data:"<<sendcontract<<endl;
	cout<<"Config strAppRegId:"<<m_strAppRegId<<endl;
	Value  retValue= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_SUPER,sendcontract,0,0.1 * COIN,nTempSend);
	if (m_cBasetest.GetHashFromCreatedTx(retValue, m_strTxHash)) {
		m_nStep++;
	}else{
		cout<<"Config err end"<<endl;
		return false;
	}

#if 1
	/// 等待交易被确认到block中
	while(true)
	{
		if(WaitComfirmed(m_strTxHash, m_strAppRegId)) {
					break;
				}
	}
#endif
	cout<<"Config success end"<<endl;
	return false;
}

//STEP:2
//修改帐户
bool CLashouTest::Modify(void)
{
   cout<<"Modify acc start"<<endl;

   MODIFIED_ST senddata;

	memset(&senddata,0,sizeof(senddata));
	senddata.uchType = TX_MODIFIED;
	strcpy(senddata.arrchModityAcc,ADDR_USER_A);//!普通管理员用户

	CDataStream scriptData(SER_DISK, g_sClientVersion);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;

	cout<<"Modify data:"<<sendcontract<<endl;
	cout<<"Modify strAppRegId:"<<m_strAppRegId<<endl;
	Value  retValue= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_SUPER,sendcontract,0,0.001 * COIN,nTempSend);
	if (m_cBasetest.GetHashFromCreatedTx(retValue, m_strTxHash)) {
		m_nStep++;
	}else{
		cout<<"Modify err end"<<endl;
		return false;
	}

#if 1
	/// 等待交易被确认到block中
	while(true)
	{
		if(WaitComfirmed(m_strTxHash, m_strAppRegId)) {
					break;
				}
	}
#endif
	cout<<"Modify success end"<<endl;
	return false;
}

//STEP:3
//注册用户信息
bool CLashouTest::Register(void)
{
	cout<<"Register start"<<endl;

	REGISTER_ST senddata;

	memset(&senddata,0,sizeof(senddata));

	senddata.uchType = TX_REGISTER;
	senddata.uRegMoney = 10;//!注册金额
	strcpy(senddata.arrchUserID,ADDR_USER_A);//!用户A
//	strcpy(senddata.UserID,ADDR_USER_B);//!用户B
//	strcpy(senddata.UserID,ADDR_SUPER);//!用户B

//	strcpy(senddata.UserID,ADDR_USER_TESTC);//!用户B

	CDataStream scriptData(SER_DISK, g_sClientVersion);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 0;// 20 * COIN

	cout<<"Register data:"<<sendcontract<<endl;
	cout<<"Register strAppRegId:"<<m_strAppRegId<<endl;
	Value  retValue= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_SUPER,sendcontract,0,0.01 * COIN,nTempSend);
	if (m_cBasetest.GetHashFromCreatedTx(retValue, m_strTxHash)) {
		m_nStep++;
	}else{
		cout<<"Register err end"<<endl;
		return false;
	}

#if 1
	/// 等待交易被确认到block中
	while(true)
	{
		if(WaitComfirmed(m_strTxHash, m_strAppRegId)) {
					break;
				}
	}
#endif
	cout<<"Register success end"<<endl;
	return false;
}

//STEP:4
//充值
bool CLashouTest::Recharge()
{
   cout<<"Recharge start"<<endl;

   RECHARGE_ST senddata;

	memset(&senddata,0,sizeof(senddata));
	senddata.uchType = TX_RECHARGE;
	senddata.uMoney=12;
	strcpy(senddata.arrchUserID,ADDR_USER_A);//!用户ID

////测试修改时间
//	senddata.type = TX_MODIFIED_TIME;
//	senddata.Money=0x8912B046;
//	strcpy(senddata.UserID,ADDR_USER_TESTA);//!用户ID

//	strcpy(senddata.ImportDataSt[0].UserID,ADDR_USER_TESTA);//!用户ID
//	senddata.ImportDataSt[0].ImportMoney=456;
//	senddata.ImportDataSt[0].ImportHight=0x5812B046;//改为时间了0X46b01258差1天

	CDataStream scriptData(SER_DISK, g_sClientVersion);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 1 * COIN;

    cout<<"Recharge data:"<<sendcontract<<endl;
	Value  retValue= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_SUPER,sendcontract,0,0.001 * COIN,nTempSend);
	if (m_cBasetest.GetHashFromCreatedTx(retValue, m_strTxHash)) {
		m_nStep++;

	}else{
	    cout<<"Recharge err end"<<endl;
	    return false;
    }

#if 1
	/// 等待交易被确认到block中
	while(true)
	{
		if(WaitComfirmed(m_strTxHash, m_strAppRegId)) {
				break;
			}
	}
#endif
	 cout<<"Recharge success end"<<endl;
	return false;
}

//STEP:5
//提现
bool CLashouTest::Withdraw()
{
   cout<<"Recharge start"<<endl;

   RECHARGE_ST senddata;

	memset(&senddata,0,sizeof(senddata));
	senddata.uchType = TX_WITHDRAW;
	senddata.uMoney=57;
	strcpy(senddata.arrchUserID,ADDR_USER_A);//!用户ID

	CDataStream scriptData(SER_DISK, g_sClientVersion);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 1 * COIN;

    cout<<"Recharge data:"<<sendcontract<<endl;
	Value  retValue= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_SUPER,sendcontract,0,0.01 * COIN,nTempSend);
	if (m_cBasetest.GetHashFromCreatedTx(retValue, m_strTxHash)) {
		m_nStep++;

	}else{
	    cout<<"Recharge err end"<<endl;
	    return false;
    }

#if 1
	/// 等待交易被确认到block中
	while(true)
	{
		if(WaitComfirmed(m_strTxHash, m_strAppRegId)) {
				break;
			}
	}
#endif
	 cout<<"Recharge success end"<<endl;
	return false;
}

//STEP:6
//申请理赔
bool CLashouTest::ApplyForClaims()
{
   cout<<"ApplyForClaims start"<<endl;

   APPLY_ST senddata;

	memset(&senddata,0,sizeof(senddata));
	senddata.uchType = TX_CLAIM_APPLY;

	strcpy(senddata.arrchUserID,ADDR_USER_TESTB);//!用户ID
	strcpy(senddata.ApplyHash,ADDR_APPLY_HASH);//!理赔HASH

	CDataStream scriptData(SER_DISK, g_sClientVersion);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 1 * COIN;

    cout<<"ApplyForClaims data:"<<sendcontract<<endl;
	Value  retValue= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_SUPER,sendcontract,0,0.1 * COIN,nTempSend);
	if (m_cBasetest.GetHashFromCreatedTx(retValue, m_strTxHash)) {
		m_nStep++;

	}else{
	    cout<<"ApplyForClaims err end"<<endl;
	    return false;
    }

	#if 1
	/// 等待交易被确认到block中
	while(true)
	{
		if(WaitComfirmed(m_strTxHash, m_strAppRegId))
		{
			break;
		}
	}
	#endif
	cout<<"ApplyForClaims success end"<<endl;
	return false;
}


//STEP:7
//理赔操作
bool CLashouTest::ClaimsOperate()
{
   cout<<"ClaimsOperate start"<<endl;

   CLAIMS_ST senddata;

	memset(&senddata,0,sizeof(senddata));
	senddata.uchType = TX_CLAIM_OPERATE;
	senddata.uMoney =7;//!理赔金额
	strcpy(senddata.arrchUserID,ADDR_USER_TESTB);//!获理赔用户ID
	senddata.uNumber =3;//!个数
	strcpy(senddata.arrchApplyHash,ADDR_CLAIMS_TEST);//!申请的HASH

	CDataStream scriptData(SER_DISK, g_sClientVersion);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 1 * COIN;

    cout<<"ClaimsOperate data:"<<sendcontract<<endl;
	Value  retValue= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_SUPER,sendcontract,0,0.001 * COIN,nTempSend);
	if (m_cBasetest.GetHashFromCreatedTx(retValue, m_strTxHash)) {
		m_nStep++;

	}else{
	    cout<<"ClaimsOperate err end"<<endl;
	    return false;
    }

	#if 1
	/// 等待交易被确认到block中
	while(true)
	{
		if(WaitComfirmed(m_strTxHash, m_strAppRegId))
		{
			break;
		}
	}
	#endif
	cout<<"ApplyForClaims success end"<<endl;
	return false;
}



//STEP:8
//插入数据
bool CLashouTest::ImportDate()
{
   cout<<"ApplyForClaims start"<<endl;

	IMPORT_ST senddata;

	memset(&senddata,0,sizeof(senddata));
	senddata.uchType = TX_IMPORT_DATA;
	senddata.uNumber =3;//!个数

	strcpy(senddata.ImportDataSt[0].arrchUserID,ADDR_USER_B);//!用户ID
	senddata.ImportDataSt[0].uImportMoney=135;
	senddata.ImportDataSt[0].uImportHight=0x5812B046;//改为时间了

	strcpy(senddata.ImportDataSt[1].arrchUserID,ADDR_USER_TESTA);//!用户ID
	senddata.ImportDataSt[1].uImportMoney=520;
	senddata.ImportDataSt[1].uImportHight=0x5812B046;//改为时间了0X46b01258差1天

	strcpy(senddata.ImportDataSt[2].arrchUserID,ADDR_USER_TESTB);//!用户ID
	senddata.ImportDataSt[2].uImportMoney=135;
	senddata.ImportDataSt[2].uImportHight=0x5812B046;//改为时间了

	CDataStream scriptData(SER_DISK, g_sClientVersion);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 1 * COIN;

    cout<<"ApplyForClaims data:"<<sendcontract<<endl;
	Value  retValue= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_SUPER,sendcontract,0,0.1 * COIN,nTempSend);
	if (m_cBasetest.GetHashFromCreatedTx(retValue, m_strTxHash)) {
		m_nStep++;

	}else{
	    cout<<"ApplyForClaims err end"<<endl;
	    return false;
    }

	#if 1
	/// 等待交易被确认到block中
	while(true)
	{
		if(WaitComfirmed(m_strTxHash, m_strAppRegId))
		{
			break;
		}
	}
	#endif
	cout<<"ApplyForClaims success end"<<endl;
	return false;
}




//STEP:9
//插入数据 多数据测试：NN
bool CLashouTest::ImportDateNN()
{
static uint32_t jj=0;
jj++;
//for(uint32_t jj=0;jj<1;jj++)
{
   cout<<"BIG DATA TEST start:"<<jj<<endl;

	IMPORT_ST senddata;

	memset(&senddata,0,sizeof(senddata));
	senddata.uchType = TX_IMPORT_DATA;
	senddata.uNumber =IMPORT_DATA_NNNN;//!个数IMPORT_DATA_NNNN




	for(uint32_t ii=0;ii<senddata.uNumber;ii++)
	{
		strcpy(senddata.ImportDataSt[ii].arrchUserID,ADDR_USER_TESTN);//!用户ID
		senddata.ImportDataSt[ii].arrchUserID[0]=0x30+jj;
		senddata.ImportDataSt[ii].arrchUserID[1]=0x30+jj;
		senddata.ImportDataSt[ii].arrchUserID[34-3]=0x30+ii/100+jj;
		senddata.ImportDataSt[ii].arrchUserID[34-2]=0x30+(ii%100)/10;
		senddata.ImportDataSt[ii].arrchUserID[34-1]=0x30+ii%10;
		senddata.ImportDataSt[ii].uImportMoney=100;
		senddata.ImportDataSt[ii].uImportHight=0x58176A80;//改为时间了0X46b01258差1天
	}


	CDataStream scriptData(SER_DISK, g_sClientVersion);
	scriptData << senddata;
	string sendcontract = HexStr(scriptData);
	uint64_t nTempSend = 1 * COIN;

    cout<<"BIG DATA TEST data:"<<sendcontract<<endl;
	Value  retValue= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_SUPER,sendcontract,0,0.001 * COIN,nTempSend);
	if (m_cBasetest.GetHashFromCreatedTx(retValue, m_strTxHash)) {
		m_nStep++;

	}else{
	    cout<<"BIG DATA TEST err end"<<endl;
	    return false;
    }

	#if 1
	/// 等待交易被确认到block中
	while(true)
	{
		if(WaitComfirmed(m_strTxHash, m_strAppRegId))
		{
			break;
		}
	}
	#endif
	cout<<"BIG DATA TEST success"<<endl;
	cout<<"success:"<<jj<<endl;
}

	return false;
}





BOOST_FIXTURE_TEST_SUITE(CLashouTxTest,CLashouTest)

BOOST_FIXTURE_TEST_CASE(Test,CLashouTest)
{
	Run();
}
/*
BOOST_FIXTURE_TEST_CASE(Config,CLashouTest)
{
	cout<<"CLashouTest run start"<<endl;
	Config();
	cout<<"CLashouTest run end"<<endl;
}

BOOST_FIXTURE_TEST_CASE(Modify,CLashouTest)
{
	cout<<"CLashouTest run start"<<endl;
	Modify();
	cout<<"CLashouTest run end"<<endl;
}

BOOST_FIXTURE_TEST_CASE(Recharge,CLashouTest)
{
	cout<<"CLashouTest run start"<<endl;
	Recharge();
	cout<<"CLashouTest run end"<<endl;
}

BOOST_FIXTURE_TEST_CASE(Withdraw,CLashouTest)
{
	cout<<"CLashouTest run start"<<endl;
	Withdraw();
	cout<<"CLashouTest run end"<<endl;
}
*/
BOOST_AUTO_TEST_SUITE_END()
#endif

