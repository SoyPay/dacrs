
#include "CreateTx_tests.h"
#include "CycleTestManger.h"
#include <ctype.h>


int CCreateTxTest::conter;
vector<string> CCreateTxTest::vAccount;

CCreateTxTest::CCreateTxTest():	nTxType(0), nNum(0), nStep(0), sendhash(""), newAddr("") {
	for (int i = 1; i < boost::unit_test::framework::master_test_suite().argc; ++i) {
		string strArgv = boost::unit_test::framework::master_test_suite().argv[i];
		string::size_type pos = strArgv.find("-number=");
		if (string::npos != pos) {
			string strNum = strArgv.substr(pos + strlen("-number="), string::npos);
			nNum = Str2Int(strNum);
			break;
		}
	}
}

void CCreateTxTest::Initialize(){
	CycleTestManger aCycleManager  = CycleTestManger::GetNewInstance();
	vector<std::shared_ptr<CycleTestBase> > vTest;
	if(0 == nNum) { //if don't have input param -number, default create 100 CCreateTxText instance defalue;
		nNum = 100;
	}
	for(int i=0; i< nNum; ++i)
	{
		vTest.push_back(std::make_shared<CCreateTxTest>());
	}
	aCycleManager.Initialize(vTest);
	aCycleManager.Run();
}

bool CCreateTxTest::SelectAccounts() {
	cout << "SelectAccounts" << endl;
	const char *argv[] = { "rpctest", "listaddr"};
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		if (SysTestBase::CommandLineRPC_GetValue(argc, argv, value)) {
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
	bool bRet = vAccount.empty();
	cout << "SelectAccount ret:"<< !bRet << endl;
	return !bRet;
}

bool CCreateTxTest::SelectOneAccount(string &selectAddr) {
	srand(time(NULL));
	if(vAccount.empty())
	{
		if(!SelectAccounts())
			return false;
	}
	int r = (rand() % vAccount.size());
	selectAddr = vAccount.at(r);
	return true;
}

bool CCreateTxTest::WaitComfirms(){
	string index = "";
	if (basetest.GetTxConfirmedRegID(sendhash, index)) {
			return true;
	}
	return false;
}

TEST_STATE CCreateTxTest::Run() {

	switch (nStep) {
		case 0: {
			if (!vAccount.empty() || SelectAccounts())
				++nStep;
			break;
		}
		case 1: {
			if (CreateTx(nTxType)) {
				++nStep;
			}
			if (++nTxType > 3)
				nTxType = 0;
			break;
		}
		case 2: {
			if (WaitComfirms() == true) {
				IncSentTotal();
				if (++conter >= nNum) {
					conter = 0;
					nStep = 0;
					vAccount.clear();
				} else {
					nStep = 1; //等待确认完成 后从新创建，但不是初始化
				}
			}
			break;
		}
	}
	return next_state;
}

bool CCreateTxTest::CreateTx(int nTxType)
{
	switch(nTxType) {
	case 0:
	{
		string srcAcct("");
		if(!SelectOneAccount(srcAcct))
			return false;
		string desAcct("");
		if(!SelectOneAccount(desAcct))
			return false;
		Value value = basetest.CreateNormalTx(srcAcct, desAcct, 100 * COIN);
		if (basetest.GetHashFromCreatedTx(value, sendhash)) {
			cout << "src regId:" << srcAcct<<" create normal tx hash:" << sendhash << endl;
			return true;
		}
		break;
	}
	case 1:
	{
		string srcAcct("");
		if(!SelectOneAccount(srcAcct))
			return false;
		BOOST_CHECK(basetest.GetNewAddr(newAddr, false));
		Value value = basetest.CreateNormalTx(srcAcct, newAddr, 100 * COIN);
		if (basetest.GetHashFromCreatedTx(value, sendhash)) {
			cout << "src regId:" << srcAcct <<" create regist account step 1 tx hash:" << sendhash <<endl;
			return true;
		}

		break;
	}
	case 2:
	{
		Value value = basetest.RegistAccountTx(newAddr, 100000);
		cout << "register address:" << newAddr << endl;
		if (basetest.GetHashFromCreatedTx(value, sendhash)) {
			cout << "register address:" << newAddr << " create regist account step 2 tx hash:" << sendhash <<endl;
			return true;
		}
		break;
	}
	case 3:
	{
		string regAddress("");
		if(!SelectOneAccount(regAddress))
			return false;
		uint64_t nFee = basetest.GetRandomFee() + 1 * COIN;
		Value value = basetest.RegisterScriptTx(regAddress, "unit_test.bin", 0, nFee);
		if (basetest.GetHashFromCreatedTx(value, sendhash)) {
			cout << "regid "<< regAddress <<" create regist app tx hash:" << sendhash << endl;
			return true;
		}
		break;
	}
	default:
		break;
	}
	return false;
}

int CCreateTxTest::Str2Int(string &strValue) {
	if("" == strValue) {
		return 0;
	}
	int iRet(0);
	int len = strValue.length();
	for(int i=0; i<len; ++i) {
		if(!isdigit(strValue[i])){
			return 0;
		}
		iRet += (strValue[i]-'0') * pow(10, len-1-i);
	}
	return iRet;
}

BOOST_FIXTURE_TEST_SUITE(CreateTxTest,CCreateTxTest)

BOOST_FIXTURE_TEST_CASE(Test,CCreateTxTest)
{
	Initialize();
}

BOOST_AUTO_TEST_SUITE_END()
