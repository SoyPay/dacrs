/*
 * create_tx_tests.h
 *
 *  Created on: 2015-04-22
 *      Author: frank
 */

#ifndef DACRS_PTEST_CREATE_TX_TESTS_H_
#define DACRS_PTEST_CREATE_TX_TESTS_H_

#include "cycle_test_base.h"
#include "../test/systestbase.h"
#include "../rpc/rpcclient.h"
#include "tx.h"
#include <vector>
using namespace std;

class CCreateTxTest: public CycleTestBase {
 public:
	CCreateTxTest();
	~CCreateTxTest() {
	};
	bool CreateTx(int nTxType);
	void Initialize();
	emTEST_STATE Run();

 private:
	static int m_snCount;
	int m_nTxType;
	int m_nNum;
	int m_nStep;
	string m_strSendHash;
	string m_strNewAddr;
};

#endif /* DACRS_PTEST_CREATE_TX_TESTS_H_ */
