/*
 * lashou_tests.h
 *
 *  Created on: 2016年12月29日
 *      Author: ranger.shi
 */

#ifndef DACRS_PTEST_LASHOU_TESTS_H_
#define DACRS_PTEST_LASHOU_TESTS_H_

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

typedef struct {
	unsigned char uchType;  	//!<交易类型
	uint32_t  uWatchDay;		//!观察期天数
	uint32_t  uMinBalance;		//!余额最低限值
	char arrchSuperAcc[35];		//!超级用户
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(uWatchDay);
			READWRITE(uMinBalance);
			for (int i = 0; i < 35; i++) {
				READWRITE(arrchSuperAcc[i]);
			}
	)
}CONFIG_ST;  //!<注册配置信息

typedef struct {
	unsigned char uchType;            	//!<交易类型
	char  arrchModityAcc[35];			//!<交易金额

	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			for (int i = 0; i < 35; i++) {
				READWRITE(arrchModityAcc[i]);
			}
	)
}MODIFIED_ST;//!<修改帐户信息


typedef struct {
	unsigned char uchType;    	//!<交易类型
	uint32_t  uRegMoney;		//!注册金额
	char arrchUserID[35];		//!用户ID
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(uRegMoney);
			for (int i = 0; i < 35; i++) {
				READWRITE(arrchUserID[i]);
			}
	)
}REGISTER_ST;  //!<注册用户信息


typedef struct {
	unsigned char uchType;    	//!<交易类型
	uint32_t  uMoney;			//!充值金额
	char arrchUserID[35];		//!用户ID
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(uMoney);
			for (int i = 0; i < 35; i++) {
				READWRITE(arrchUserID[i]);
			}
	)
}RECHARGE_ST;  //!<充值，提现


typedef struct {
	unsigned char uchType;    //!<交易类型
	char arrchUserID[34];//!用户ID
	char ApplyHash[35];//!申请的HASH
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			for(int i = 0; i < 34; i++)
			{
				READWRITE(arrchUserID[i]);
			}

			for(int i = 0; i < 35; i++)
			{
				READWRITE(ApplyHash[i]);
			}

	)
}APPLY_ST;  //!<理赔申请

typedef struct {
	unsigned char uchType;    		//!<交易类型
	uint32_t  uMoney;				//!充值金额
	char arrchUserID[34];			//!用户ID
	uint32_t  uNumber;				//!个数
	char arrchApplyHash[34*3+1];	//!申请的HASH
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(uMoney);
			for (int i = 0; i < 34; i++) {
				READWRITE(arrchUserID[i]);
			}
			READWRITE(uNumber);
			for (int i = 0; i < (34*3+1); i++) {
				READWRITE(arrchApplyHash[i]);
			}

	)
}CLAIMS_ST;  //!<理赔操作

#define IMPORT_DATA_NNNN	90

typedef struct {
	unsigned char uchType; 			//!<交易类型
	uint32_t  uNumber;				//!充值金额
	struct {						//!<插入数据结构
		char arrchUserID[34];		//!用户ID
		uint32_t uImportMoney;		//!注册金额
		uint32_t uImportHight;		//!注册高度
	} ImportDataSt[IMPORT_DATA_NNNN];
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(uNumber);
			for (int i = 0; i < IMPORT_DATA_NNNN; i++) {
				for (int j = 0; j < 34; j++) {
					READWRITE(ImportDataSt[i].arrchUserID[j]);
				}
				READWRITE(ImportDataSt[i].uImportMoney);
				READWRITE(ImportDataSt[i].uImportHight);
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
	unsigned char uchType;           	//!<交易类型
	uint64_t ullMaxMoneyByTime;       	//!<每次限额
	uint64_t ullMaxMoneyByDay;        	//!<每日限额
	char  arrchAddress[35];             //!<备注说明 字符串以\0结束，长度不足后补0
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(ullMaxMoneyByTime);
			READWRITE(ullMaxMoneyByDay);
			for (int i = 0; i < 35; i++) {
				READWRITE(arrchAddress[i]);
			}
	)
}COMPANY_CONFIG;  //!<注册企业配置信息

typedef struct {
	unsigned char uchType;            //!<交易类型
	uint64_t ullMaxMoneyByTime;       //!<每次限额
	uint64_t ullMaxMoneyByDay;        //!<每日限额
	// char  address[35];             //!<备注说明 字符串以\0结束，长度不足后补0
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(ullMaxMoneyByTime);
			READWRITE(ullMaxMoneyByDay);
	// for(int i = 0; i < 220; i++)
	// {
	// 		READWRITE(address[i]);
	// }
	)
}COMPANY_CONFIG_MODIFY;  //!<修改企业配置信息

typedef struct {
	unsigned char uchType;            		//!<交易类型
	// uint64_t moneyM;                   	//!<交易金额

	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
//			READWRITE(moneyM);
	)
}COMPANY_RECHARGE;                  		//!<企业批发币

typedef struct {
	unsigned char uchType;            		//!<交易类型
	char  arrchAddress[35];             	//!<备注说明 字符串以\0结束，长度不足后补0
	uint64_t ullMoneyM;               		//!<交易金额

	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			for (int i = 0; i < 35; i++) {
				READWRITE(arrchAddress[i]);
			}
			READWRITE(ullMoneyM);
	)
} COMPANY_WITHDRAW;

class CLashouTest : public CycleTestBase {
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

 private:
	int m_nNum;
	int m_nStep;
	string m_strTxHash;
	string m_strAppRegId;               		//注册应用后的Id
};

#endif

#endif /* DACRS_PTEST_LASHOU_TESTS_H_ */
