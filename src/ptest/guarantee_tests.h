/*
 * CBlackHalo_tests.h
 *
 *  Created on: 2015-04-24
 *      Author: frank.shi
 */

#ifndef GUARANTEE_TESTS_H_
#define GUARANTEE_TESTS_H_

#include "cycle_test_base.h"
#include "../test/systestbase.h"
#include "../rpc/rpcclient.h"
#include "tx.h"

using namespace std;
using namespace boost;
using namespace json_spirit;




#define	TX_REGISTER   		0x01   //注册仲裁账户
#define TX_MODIFYREGISTER  	0x02 // 修改仲裁者注册信息
#define TX_ARBIT_ON     	0x03 //仲裁开启
#define TX_ARBIT_OFF    	0x04 //仲裁暂停
#define	TX_UNREGISTER 		0x05 //注销仲裁账户
#define	TX_SEND  			0x06 //挂单
#define	TX_CANCEL  			0x07 //取消挂单
#define	TX_ACCEPT  			0x08 //接单
#define TX_DELIVERY 		0x09//发货
#define	TX_BUYERCONFIRM  	0x0a //买家确认收货
#define	TX_ARBITRATION  	0x0b //申请仲裁
#define	TX_FINALRESULT  	0x0c //裁决结果






#define	SEND_TYPE_BUY   0x00   //!<挂单 买
#define	SEND_TYPE_SELL  0x01  //!<挂单 卖


typedef struct {
	unsigned char uchSysType;               //0xff
	unsigned char uchType;            // 0x01 提?现?  02 充?值μ  03 提?现?一?定¨的?金e额?
	unsigned char uchTypeAddr;            // 0x01 regid 0x02 base58
	uint64_t     ullMoney;

	IMPLEMENT_SERIALIZE
	(
		READWRITE(uchSysType);
		READWRITE(uchType);
		READWRITE(uchTypeAddr);
		READWRITE(ullMoney);
	)
} STAPPACC_MONEY;

typedef struct {
	unsigned char uchSysType;               //0xff
	unsigned char uchType;            // 0x01 提?现?  02 充?值μ  03 提?现?一?定¨的?金e额?
	unsigned char uchTypeAddr;            // 0x01 regid 0x02 base58
//	uint64_t     money;

	IMPLEMENT_SERIALIZE
	(
		READWRITE(uchSysType);
		READWRITE(uchType);
		READWRITE(uchTypeAddr);
//		READWRITE(money);
	)
} ST_APPACC;

enum emGETDAWEL{
	EM_TX_REGID = 0x01,
	EM_TX_BASE58 = 0x02,
};



typedef struct {
	unsigned char uchType;            //!<交易类型
	uint64_t nArbiterMoneyX;             //!<仲裁费用X
	uint64_t nOvertimeMoneyYmax;  //!<超时未判决的最大赔偿费用Y
	uint64_t nConfigMoneyZ;              //!<无争议裁决费用Z
	unsigned int  nOvertimeheightT;  //!<判决期限时间T
	char  arrchComment[220];             //!<备注说明 字符串以\0结束，长度不足后补0
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(nArbiterMoneyX);
			READWRITE(nOvertimeMoneyYmax);
			READWRITE(nConfigMoneyZ);
			READWRITE(nOvertimeheightT);
			for (int i = 0; i < 220; i++) {
				READWRITE(arrchComment[i]);
			}
	)

}STTX_REGISTER_CONTRACT;  //!<注册仲裁账户

typedef struct {
	unsigned char uchType;            //!<交易类型
	unsigned char uchSendType;         //!<挂单类型:0 买  1卖
	char arrchArbitationID[6];        //!<仲裁者ID（采用6字节的账户ID）
	uint64_t nMoneyM;                   //!<交易金额
	unsigned int nHeight;           //!<每个交易环节的超时高度

	char arrchGoods[20];               //!<商品信息  字符串以\0结束，长度不足后补0
	char  arrchComment[200];             //!<备注说明 字符串以\0结束，长度不足后补0
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(uchSendType);
			for(int i = 0; i < 6; i++) {
				READWRITE(arrchArbitationID[i]);
			}
			READWRITE(nMoneyM);
			READWRITE(nHeight);
			for(int i = 0; i < 20; i++) {
				READWRITE(arrchGoods[i]);
			}
			for(int i = 0; i < 200; i++) {
				READWRITE(arrchComment[i]);
			}
	)
}STTX_SNED_CONTRACT;                  //!<挂单

typedef struct {
	unsigned char uchType;            //!<交易类型
	unsigned char arruchTxhash[32];       //!<挂单的交易hash
	unsigned int nHeight;          //!<每个交易环节的超时高度
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			for(int i = 0; i < 32; i++) {
				READWRITE(arruchTxhash[i]);
			}
			READWRITE(nHeight);
	)
} STTX_CONTRACT;

typedef struct {
	unsigned char uchType;            //!<交易类型
	unsigned char arruchTxhash[32];       //!<挂单的交易hash
	unsigned int nHeight;          //!<每个交易环节的超时高度
	char  arrchArbitationID[6];       //!<仲裁者ID（采用6字节的账户ID）
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			for(int i = 0; i < 32; i++){
				READWRITE(arruchTxhash[i]);
			}
			READWRITE(nHeight);
			for(int i = 0; i < 6; i++){
				READWRITE(arrchArbitationID[i]);
			}
	)
} STTX_Arbitration;  //!<申请仲裁

typedef struct {
	unsigned char uchType;            //!<交易类型
	unsigned char arruchArbitHash[32];      //!<申请仲裁的交易hash
	unsigned int nOvertimeheightT;//!<判决期限时间T
	char 	arrchWinner[6];      	//!<赢家ID（采用6字节的账户ID）
	uint64_t nWinnerMoney;            //!<最终获得的金额
	char  arrchLoser[6];       //!<输家ID（采用6字节的账户ID）
	uint64_t nLoserMoney;            //!<最终获得的金额
	IMPLEMENT_SERIALIZE
	(
		READWRITE(uchType);
		for(int i = 0; i < 32; i++) {
			READWRITE(arruchArbitHash[i]);
		}
		READWRITE(nOvertimeheightT);
		for(int i = 0; i < 6; i++) {
			READWRITE(arrchWinner[i]);
		}
		READWRITE(nWinnerMoney);
		for(int i = 0; i < 6; i++) {
			READWRITE(arrchLoser[i]);
		}
		READWRITE(nLoserMoney);
	)
}ST_TX_FINALRESULT_CONTRACT;        //!<最终裁决


class CGuaranteeTest: public CycleTestBase {

 public:
	CGuaranteeTest();
	~CGuaranteeTest(){};
	virtual emTEST_STATE Run() ;
	bool RegistScript();

	bool Recharge(void);
	bool Withdraw(void);
	bool WithdrawSomemoney(void);

	bool Register(unsigned char uchType);
	bool ArbitONOrOFF(unsigned char uchType);
	bool UnRegister(void);
	bool SendStartTrade(void);
	bool SendCancelTrade(void);
	bool AcceptTrade(void);
	bool DeliveryTrade(void);
	bool BuyerConfirm(void);
    bool Arbitration(void);
    bool RunFinalResult(void);
 private:
	int m_nNum;
	int m_nStep;
	string m_strTxHash;
	string m_strAppRegId;//注册应用后的Id
};

#endif /* CANONY_TESTS_H */
