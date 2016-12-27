/*
 * CreateTx_tests.h
 *
 *  Created on: 2015-04-22
 *      Author: frank
 */

#ifndef CREATETX_TESTS_H_
#define CREATETX_TESTS_H_
#include "cycle_test_base.h"
#include "../test/systestbase.h"
#include "../rpc/rpcclient.h"
#include "tx.h"
#include <vector>
using namespace std;

class CCreateTxTest : public CycleTestBase{
 public:
	CCreateTxTest();
	 ~CCreateTxTest(){};
	 bool  CreateTx(int nTxType);
	 void Initialize();
	 emTEST_STATE Run();

 private:
	 static  int nCount ;
	 int nTxType;
	 int nNum;
	 int nStep ;
	 string strSendHash;
	 string strNewAddr;

};
#endif /* CDARKANDANONY_H_ */
