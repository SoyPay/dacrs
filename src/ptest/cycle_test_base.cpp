/*
 * CycleTestBase.cpp
 *
 *  Created on: 2014Äê12ÔÂ30ÈÕ
 *      Author: ranger.shi
 */

#include "cycle_test_base.h"
#include <ctype.h>

int CycleTestBase::m_snTotalsend = 0;
vector<string> CycleTestBase::svstrAccount;

CycleTestBase::CycleTestBase() {

}

emTEST_STATE CycleTestBase::Run() {
	return EM_END_STATE;
}

CycleTestBase::~CycleTestBase() {

}


bool CycleTestBase::SelectAccounts(vector<string> &vstrAccount) {
	const char *pkArgv[] = { "rpctest", "listaddr" };
	int nArgc = sizeof(pkArgv) / sizeof(char*);

	Value value;
	if (SysTestBase::CommandLineRPC_GetValue(nArgc, pkArgv, value)) {
		if (value.type() == null_type) {
			return false;
		}
		for (auto & item : value.get_array()) {
			const Value& balance = find_value(item.get_obj(), "balance");
			if (balance.get_real() > 5000.0) {
				const Value& regId = find_value(item.get_obj(), "regid");
				if ("" != regId.get_str()) {
					vstrAccount.push_back(regId.get_str());
				}
			}
		}
	}
	return !vstrAccount.empty();
}

bool CycleTestBase::SelectOneAccount(string &strSelectAddr, bool bFalg) {
	srand(time(NULL));
	if (svstrAccount.empty() || (bFalg && svstrAccount.size() < 2)) {
		if (!SelectAccounts(svstrAccount)) {
			return false;
		}
	}

	int nR = (rand() % svstrAccount.size());
	if (bFalg) {
		string strAddr("");
		while (true) {
			strAddr = svstrAccount.at(nR);
			if (strSelectAddr != strAddr) {
				break;
			}
			nR = (rand() % svstrAccount.size());
		}
		strSelectAddr = strAddr;
	} else {
		strSelectAddr = svstrAccount.at(nR);
	}
	return true;
}

bool CycleTestBase::WaitComfirmed(string &strTxHash, string &strRegId) {
	if (m_cBasetest.GetTxConfirmedRegID(strTxHash, strRegId)) {
		return true;
	}
	return false;
}

int CycleTestBase::Str2Int(string &strValue) {
	if ("" == strValue) {
		return 0;
	}
	int nRet(0);
	int nLen = strValue.length();
	for (int i = 0; i < nLen; ++i) {
		if (!isdigit(strValue[i])) {
			return 0;
		}
		nRet += (strValue[i] - '0') * pow(10, nLen - 1 - i);
	}
	return nRet;
}
