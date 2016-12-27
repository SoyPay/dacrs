/*
 * CycleTestBase.h
 *
 *  Created on: 2014Äê12ÔÂ30ÈÕ
 *      Author: ranger.shi
 */

#ifndef CYCLE_TEST_BASE_H_
#define CYCLE_TEST_BASE_H_
#include "../test/systestbase.h"
enum emTEST_STATE {
	EM_THES_STATE, EM_NEXT_STATE, EM_END_STATE,
};

class CycleTestBase {

public:
	CycleTestBase();
	bool IncSentTotal() {
		m_cBasetest.ShowProgressTotal("Send Cycle:", ++m_snTotalsend);
		return true;
	}
	static bool SelectAccounts(vector<string> &vstrAccount);
	bool SelectOneAccount(string &strSelectAddr, bool bFalg = false);
	bool WaitComfirmed(string &strTxHash, string &strRegId);
	int Str2Int(string &strValue);
	virtual emTEST_STATE Run();
	virtual ~CycleTestBase();

protected:
	SysTestBase m_cBasetest;
	static int m_snTotalsend;
	static vector<string> svstrAccount;

};



#endif /* CYCLETESTBASE_H_ */
