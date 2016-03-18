/*
 * CBlackHalo_tests.h
 *
 *  Created on: 2015-04-24
 *      Author: frank.shi
 */

#ifndef CANONY_TESTS_H
#define CANONY_TESTS_H

#include "CycleTestBase.h"
#include "../test/systestbase.h"
#include "../rpc/rpcclient.h"
#include "tx.h"

using namespace std;
using namespace boost;
using namespace json_spirit;




#define	TX_REGISTER   0x01   //ע���ٲ��˻�
#define TX_MODIFYREGISTER  0x02 // �޸��ٲ���ע����Ϣ
#define TX_ARBIT_ON     0x03 //�ٲÿ���
#define TX_ARBIT_OFF    0x04 //�ٲ���ͣ
#define	TX_UNREGISTER  0x05 //ע���ٲ��˻�
#define	TX_SEND  0x06 //�ҵ�
#define	TX_CANCEL  0x07 //ȡ���ҵ�
#define	TX_ACCEPT  0x08 //�ӵ�
#define TX_DELIVERY 0x09//����
#define	TX_BUYERCONFIRM  0x0a //���ȷ���ջ�
#define	TX_ARBITRATION  0x0b //�����ٲ�
#define	TX_FINALRESULT  0x0c //�þ����






#define	SEND_TYPE_BUY   0x00   //!<�ҵ� ��
#define	SEND_TYPE_SELL  0x01  //!<�ҵ� ��


typedef struct {
	unsigned char systype;               //0xff
	unsigned char type;            // 0x01 �ᬨ?��?  02 ��?ֵ��  03 �ᬨ?��?һ?������?��e��?
	unsigned char typeaddr;            // 0x01 regid 0x02 base58
	uint64_t     money;

	IMPLEMENT_SERIALIZE
	(
		READWRITE(systype);
		READWRITE(type);
		READWRITE(typeaddr);
		READWRITE(money);
	)
} APPACC_money;

typedef struct {
	unsigned char systype;               //0xff
	unsigned char type;            // 0x01 �ᬨ?��?  02 ��?ֵ��  03 �ᬨ?��?һ?������?��e��?
	unsigned char typeaddr;            // 0x01 regid 0x02 base58
//	uint64_t     money;

	IMPLEMENT_SERIALIZE
	(
		READWRITE(systype);
		READWRITE(type);
		READWRITE(typeaddr);
//		READWRITE(money);
	)
} APPACC;

enum GETDAWEL{
	TX_REGID = 0x01,
	TX_BASE58 = 0x02,
};



typedef struct {
	unsigned char type;            //!<��������
	uint64_t arbiterMoneyX;             //!<�ٲ÷���X
	uint64_t overtimeMoneyYmax;  //!<��ʱδ�о�������⳥����Y
	uint64_t configMoneyZ;              //!<������þ�����Z
	unsigned int  overtimeheightT;  //!<�о�����ʱ��T
	char  comment[220];             //!<��ע˵�� �ַ�����\0���������Ȳ����0
	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			READWRITE(arbiterMoneyX);
			READWRITE(overtimeMoneyYmax);
			READWRITE(configMoneyZ);
			READWRITE(overtimeheightT);
			for(int i = 0; i < 220; i++)
			{
				READWRITE(comment[i]);
			}
	)

}TX_REGISTER_CONTRACT;  //!<ע���ٲ��˻�

typedef struct {
	unsigned char type;            //!<��������
	unsigned char sendType;         //!<�ҵ�����:0 ��  1��
	char arbitationID[6];        //!<�ٲ���ID������6�ֽڵ��˻�ID��
	uint64_t moneyM;                   //!<���׽��
	unsigned int height;           //!<ÿ�����׻��ڵĳ�ʱ�߶�

	char goods[20];               //!<��Ʒ��Ϣ  �ַ�����\0���������Ȳ����0
	char  comment[200];             //!<��ע˵�� �ַ�����\0���������Ȳ����0
	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			READWRITE(sendType);
			for(int i = 0; i < 6; i++)
			{
				READWRITE(arbitationID[i]);
			}
			READWRITE(moneyM);
			READWRITE(height);
			for(int i = 0; i < 20; i++)
			{
				READWRITE(goods[i]);
			}
			for(int i = 0; i < 200; i++)
			{
				READWRITE(comment[i]);
			}
	)
}TX_SNED_CONTRACT;                  //!<�ҵ�

typedef struct {
	unsigned char type;            //!<��������
	unsigned char txhash[32];       //!<�ҵ��Ľ���hash
	unsigned int height;          //!<ÿ�����׻��ڵĳ�ʱ�߶�
	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			for(int i = 0; i < 32; i++)
			{
				READWRITE(txhash[i]);
			}
			READWRITE(height);
	)
} TX_CONTRACT;
typedef struct {
	unsigned char type;            //!<��������
	unsigned char txhash[32];       //!<�ҵ��Ľ���hash
	unsigned int height;          //!<ÿ�����׻��ڵĳ�ʱ�߶�
	char  arbitationID[6];       //!<�ٲ���ID������6�ֽڵ��˻�ID��
	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
			for(int i = 0; i < 32; i++)
			{
				READWRITE(txhash[i]);
			}
			READWRITE(height);
			for(int i = 0; i < 6; i++)
			{
				READWRITE(arbitationID[i]);
			}
	)
} TX_Arbitration;  //!<�����ٲ�

typedef struct {
	unsigned char type;            //!<��������
	unsigned char arbitHash[32];      //!<�����ٲõĽ���hash
	unsigned int overtimeheightT;//!<�о�����ʱ��T
	char 	winner[6];      	//!<Ӯ��ID������6�ֽڵ��˻�ID��
	uint64_t winnerMoney;            //!<���ջ�õĽ��
	char  loser[6];       //!<���ID������6�ֽڵ��˻�ID��
	uint64_t loserMoney;            //!<���ջ�õĽ��
	IMPLEMENT_SERIALIZE
	(
		READWRITE(type);
		for(int i = 0; i < 32; i++)
		{
			READWRITE(arbitHash[i]);
		}
		READWRITE(overtimeheightT);
		for(int i = 0; i < 6; i++)
		{
			READWRITE(winner[i]);
		}
		READWRITE(winnerMoney);
		for(int i = 0; i < 6; i++)
		{
			READWRITE(loser[i]);
		}
		READWRITE(loserMoney);
	)
}TX_FINALRESULT_CONTRACT;        //!<���ղþ�


class CGuaranteeTest: public CycleTestBase {
	int nNum;
	int nStep;
	string strTxHash;
	string strAppRegId;//ע��Ӧ�ú��Id
public:
	CGuaranteeTest();
	~CGuaranteeTest(){};
	virtual TEST_STATE Run() ;
	bool RegistScript();

	bool Recharge(void);
	bool Withdraw(void);
	bool WithdrawSomemoney(void);

	bool Register(unsigned char type);
	bool ArbitONOrOFF(unsigned char type);
	bool UnRegister(void);
	bool SendStartTrade(void);
	bool SendCancelTrade(void);
	bool AcceptTrade(void);
	bool DeliveryTrade(void);
	bool BuyerConfirm(void);
    bool Arbitration(void);
    bool RunFinalResult(void);

};

#endif /* CANONY_TESTS_H */
