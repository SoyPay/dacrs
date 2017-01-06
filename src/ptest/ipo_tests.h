/*
 * ipo_tests.h
 *
 *  Created on: 2015-04-24
 *      Author: frank.shi
 */

#ifndef DACRS_PTEST_IPO_TESTS_H_
#define DACRS_PTEST_IPO_TESTS_H_

#include "cycle_test_base.h"
#include "../test/systestbase.h"
#include "../rpc/rpcclient.h"
#include "tx.h"

using namespace std;
using namespace boost;
using namespace json_spirit;

//#define SEND_A    "DmtzzT99HYUGAV6ejkWTWXF8pcYXtkpU4g"
#define SEND_A    "DmtzzT99HYUGAV6ejkWTWXF8pcYXtkpU4g"
//#define SEND_A    "dtKsuK9HUvLLHtBQL8Psk5fUnTLTFC83GS"  // 0-20

class CIpoTest: public CycleTestBase {
 public:
	CIpoTest();
	~CIpoTest(){};
	virtual emTEST_STATE Run() ;
	bool RegistScript();
	bool CreateIpoTx(string strContact,int64_t llSendTotal);
	bool SendIpoTx(unsigned char uchType);
	void RunIpo(unsigned char uchType);
	void SendErrorIopTx();

 private:
	int m_nNum;
	int m_nStep;
	string m_strTxHash;
	string m_strAppRegId;	//注册应用后的Id
};

#endif /* DACRS_PTEST_IPO_TESTS_H_ */
