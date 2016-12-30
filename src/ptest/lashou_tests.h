/*
 * lashou_tests.h
 *
 *  Created on: 2016年12月29日
 *      Author: ranger.shi
 */

#ifndef LASHOU_TESTS_H_
#define LASHOU_TESTS_H_

#include "cycle_test_base.h"
#include "../test/systestbase.h"
#include "../rpc/rpcclient.h"
#include "tx.h"

using namespace std;
using namespace boost;
using namespace json_spirit;

#if 1

#define TX_CONFIG   	 0x01//--配置信息【用来配置参数和存储一些全局变量】
#define TX_MODIFIED 	 0X02//--修改配置信息
#define TX_REGISTER 	 0X03//--注册信息
#define TX_RECHARGE 	 0x04//--充值
#define TX_WITHDRAW 	 0x05//--理赔提现
#define TX_CLAIM_APPLY 	 0X06//--理赔申请信息
#define TX_CLAIM_OPERATE 0X07//--理赔操作
#define TX_IMPORT_DATA   0X08//--导入用户数据
#define TX_MODIFIED_TIME 0X09//--修改加入时间

typedef struct
{
	unsigned char type;    //!<交易类型
	uint32_t  WatchDay;//!观察期天数
	uint32_t  MinBalance;//!余额最低限值
	char SuperAcc[35];//!超级用户
	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			READWRITE(WatchDay);
			READWRITE(MinBalance);
			for(int i = 0; i < 35; i++)
			{
				READWRITE(SuperAcc[i]);
			}
	)
}CONFIG_ST;  //!<注册配置信息

//
typedef struct {
	unsigned char type;            //!<交易类型
	char  ModityAcc[35];//!<交易金额

	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			for(int i = 0; i < 35; i++)
			{
				READWRITE(ModityAcc[i]);
			}
	)
}MODIFIED_ST;//!<修改帐户信息


typedef struct
{
	unsigned char type;    //!<交易类型
	uint32_t  RegMoney;//!注册金额
	char UserID[35];//!用户ID
	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			READWRITE(RegMoney);
			for(int i = 0; i < 35; i++)
			{
				READWRITE(UserID[i]);
			}
	)
}REGISTER_ST;  //!<注册用户信息


typedef struct
{
	unsigned char type;    //!<交易类型
	uint32_t  Money;//!充值金额
	char UserID[35];//!用户ID
	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			READWRITE(Money);
			for(int i = 0; i < 35; i++)
			{
				READWRITE(UserID[i]);
			}
	)
}RECHARGE_ST;  //!<充值，提现


typedef struct
{
	unsigned char type;    //!<交易类型
	char UserID[34];//!用户ID
	char ApplyHash[35];//!申请的HASH
	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			for(int i = 0; i < 34; i++)
			{
				READWRITE(UserID[i]);
			}

			for(int i = 0; i < 35; i++)
			{
				READWRITE(ApplyHash[i]);
			}

	)
}APPLY_ST;  //!<理赔申请



typedef struct
{
	unsigned char type;    //!<交易类型
	uint32_t  Money;//!充值金额
	char UserID[34];//!用户ID
	uint32_t  Number;//!个数
	char ApplyHash[34*3+1];//!申请的HASH
	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			READWRITE(Money);
			for(int i = 0; i < 34; i++)
			{
				READWRITE(UserID[i]);
			}
			READWRITE(Number);
			for(int i = 0; i < (34*3+1); i++)
			{
				READWRITE(ApplyHash[i]);
			}

	)
}CLAIMS_ST;  //!<理赔操作

#define IMPORT_DATA_NNNN	90
typedef struct
{
	unsigned char type; //!<交易类型
	uint32_t  Number;	//!充值金额
	struct
	{//!<插入数据结构
		char UserID[34];//!用户ID
		uint32_t ImportMoney;//!注册金额
		uint32_t ImportHight;//!注册高度
	} ImportDataSt[IMPORT_DATA_NNNN];
	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			READWRITE(Number);
			for(int i = 0; i < IMPORT_DATA_NNNN; i++)
			{
				for(int j = 0; j < 34; j++)
				{
					READWRITE(ImportDataSt[i].UserID[j]);
				}
				READWRITE(ImportDataSt[i].ImportMoney);
				READWRITE(ImportDataSt[i].ImportHight);
			}
	)
}IMPORT_ST;  //!<插入操作




//======================================================================
//======================================================================
//======================================================================

enum GETDAWEL{
	TX_REGID = 0x01,
	TX_BASE58 = 0x02,
};

typedef struct {
	unsigned char type;            //!<交易类型
	uint64_t maxMoneyByTime;       //!<每次限额
	uint64_t maxMoneyByDay;        //!<每日限额
	char  address[35];             //!<备注说明 字符串以\0结束，长度不足后补0
	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			READWRITE(maxMoneyByTime);
			READWRITE(maxMoneyByDay);
			for(int i = 0; i < 35; i++)
			{
				READWRITE(address[i]);
			}
	)
}COMPANY_CONFIG;  //!<注册企业配置信息
typedef struct {
	unsigned char type;            //!<交易类型
	uint64_t maxMoneyByTime;       //!<每次限额
	uint64_t maxMoneyByDay;        //!<每日限额
//	char  address[35];             //!<备注说明 字符串以\0结束，长度不足后补0
	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			READWRITE(maxMoneyByTime);
			READWRITE(maxMoneyByDay);
//			for(int i = 0; i < 220; i++)
//			{
//				READWRITE(address[i]);
//			}
	)
}COMPANY_CONFIG_MODIFY;  //!<修改企业配置信息

typedef struct {
	unsigned char type;            //!<交易类型
//	uint64_t moneyM;                   //!<交易金额

	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
//			READWRITE(moneyM);
	)
}COMPANY_RECHARGE;                  //!<企业批发币


typedef struct {
	unsigned char type;            //!<交易类型
	char  address[35];             //!<备注说明 字符串以\0结束，长度不足后补0
	uint64_t moneyM;               //!<交易金额

	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			for(int i = 0; i < 35; i++)
			{
				READWRITE(address[i]);
			}
			READWRITE(moneyM);
	)
} COMPANY_WITHDRAW;


class CLashouTest: public CycleTestBase {
	int nNum;
	int nStep;
	string strTxHash;
	string strAppRegId;//注册应用后的Id
public:
	CLashouTest();
	~CLashouTest(){};
	virtual emTEST_STATE Run() ;
	bool RegistScript();

	bool Config(void);
	bool Modify(void);
	bool Register(void);
	bool Recharge(void);
	bool Withdraw(void);
	bool ApplyForClaims(void);
	bool ClaimsOperate(void);
	bool ImportDate(void);
	bool ImportDateNN(void);
	bool CodeTest(void);
};
#endif





#endif /* LASHOU_TESTS_H_ */
