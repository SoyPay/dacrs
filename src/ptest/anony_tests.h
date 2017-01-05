/*
 * anony_tests.h
 *
 *  Created on: 2015-04-24
 *      Author: frank.shi
 */

#ifndef DACRS_PTEST_ANONY_TEST_H_
#define DACRS_PTEST_ANONY_TEST_H_

#include "cycle_test_base.h"
#include "../test/systestbase.h"
#include "../rpc/rpcclient.h"
#include "tx.h"

using namespace std;
using namespace boost;
using namespace json_spirit;


typedef struct  {
	unsigned char arruchSender[6];					//!<转账人ID（采用6字节的账户ID）
	int64_t llPayMoney;								//!<转账的人支付的金额
	unsigned short usLen;             		        //!<接受钱账户信息长度
	IMPLEMENT_SERIALIZE
	(
			for (int i = 0; i < 6; i++) {
			    READWRITE(arruchSender[i]);
			}
			READWRITE(llPayMoney);
			READWRITE(usLen);
	)
}ST_CONTRACT;

typedef struct  {
	char  arrchAccount[6];						    	//!<接受钱的ID（采用6字节的账户ID）
	int64_t llReciMoney;						    	//!<	收到钱的金额
	IMPLEMENT_SERIALIZE
	(
			for(int i = 0; i < 6; i++){
				READWRITE(arrchAccount[i]);
			}
			READWRITE(llReciMoney);
	)
}ST_ACCOUNT_INFO;


class CAnonyTest: public CycleTestBase {
 public:
	CAnonyTest();
	~CAnonyTest(){};
	virtual emTEST_STATE Run() ;
	bool RegistScript();
	bool CreateAnonyTx();
	void Initialize();

 private:
	int m_nNum;
	int m_nStep;
	string m_strTxHash;
	string m_strAppRegId;
	string m_strRegId;
};

#endif /* DACRS_PTEST_ANONY_TEST_H_ */
