#include "CycleTestBase.h"
#include "../test/systestbase.h"
#include "../rpc/rpcclient.h"
#include "CDarkAndAnony.h"
#include "tx.h"
#include <vector>
using namespace std;


bool CCreateNormalTxTest::SelectAccounts() {
	const char *argv[] = { "rpctest", "listaddr"};
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		if (basetest.CommandLineRPC_GetValue(argc, argv, value)) {
			if (value.type() == null_type) {
				return false;
			}
			for(auto & item : value.get_array()) {
				const Value& balance = find_value(item.get_obj(), "balance");
				if(balance.get_real() > 5000.0) {
					const Value& regId = find_value(item.get_obj(), "regid");
					if("" != regId.get_str())
						vAccount.push_back(regId.get_str());
				}
			}
		}
	return !vAccount.empty();
}

string CCreateNormalTxTest::SelectOneAccount() {
	srand(time(NULL));
	int r = (rand() % vAccount.size());
	return vAccount.at(r);
}

bool CCreateNormalTxTest::WaitComfirms(){
	string index = "";
	if (basetest.GetTxConfirmedRegID(sendhash, index)) {
			return true;
	}
	return false;
}


TEST_STATE CCreateNormalTxTest::Run() {
	static int conter = 0 ;
	switch(nStep) {
	case 0:
		if(SelectAccounts())
			++nStep;
		break;
	case 1:
		if(CreateTx())
			++nStep;
		break;
	case 2:
		if(WaitComfirms()== true){
		IncSentTotal();
			if (++conter > 10) {
				conter = 0;
				nStep = 0;
			} else {
				nStep = 1; //等待确认完成 后从新创建，但不是初始化
			}
		}
		break;
	}
	return next_state;
}
bool CCreateNormalTxTest::CreateTx()
{
	string srcAcct = SelectOneAccount();
	string desAcct("");
	desAcct= SelectOneAccount();
	Value value = basetest.CreateNormalTx(srcAcct, desAcct, 100 * COIN);
	if (basetest.GetHashFromCreatedTx(value, sendhash)) {
		return true;
	}
	return false;
}

