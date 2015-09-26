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

//#define SEND_A    "DmtzzT99HYUGAV6ejkWTWXF8pcYXtkpU4g"
#define SEND_A    "DmtzzT99HYUGAV6ejkWTWXF8pcYXtkpU4g"
//#define SEND_A    "dtKsuK9HUvLLHtBQL8Psk5fUnTLTFC83GS"  // 0-20

class CIpoTest: public CycleTestBase {
	int nNum;
	int nStep;
	string strTxHash;
	string strAppRegId;//注册应用后的Id
public:
	CIpoTest();
	~CIpoTest(){};
	virtual TEST_STATE Run() ;
	bool RegistScript();
	bool CreateIpoTx(string contact,int64_t llSendTotal);
	bool SendIpoTx(unsigned char type);
	void RunIpo(unsigned char type);
	void SendErrorIopTx();

};

#endif /* CANONY_TESTS_H */
