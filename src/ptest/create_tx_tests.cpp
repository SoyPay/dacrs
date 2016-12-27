
#include "create_tx_tests.h"
#include "cycle_test_manger.h"


int CCreateTxTest::nCount = 0;

CCreateTxTest::CCreateTxTest():	nTxType(0), nNum(0), nStep(0), strSendHash(""), strNewAddr("") {
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
	vector<std::shared_ptr<CycleTestBase> > vecTest;
	if (0 == nNum) { //if don't have input param -number, default create 100 CCreateTxText instance defalue;
		nNum = 100;
	}
	for (int i=0; i< nNum; ++i) {
		vecTest.push_back(std::make_shared<CCreateTxTest>());
	}
	aCycleManager.Initialize(vecTest);
	aCycleManager.Run();
}


emTEST_STATE CCreateTxTest::Run() {

	switch (nStep) {
		case 0: {
			if (!svstrAccount.empty() || SelectAccounts(svstrAccount)) {
				++nStep;
			}
			break;
		}
		case 1: {
			if (CreateTx(nTxType)) {
				++nStep;
			}
			if (++nTxType > 3) {
				nTxType = 0;
			}
			break;
		}
		case 2: {
			string strComfirmedPos;
			if (WaitComfirmed(strSendHash, strComfirmedPos) == true) {
				IncSentTotal();
				if (++nCount >= nNum) {
					nCount = 0;
					nStep = 0;
					svstrAccount.clear();
				} else {
					nStep = 1; //等待确认完成 后从新创建，但不是初始化
				}
			}
			break;
		}
	}
	return EM_NEXT_STATE;
}

bool CCreateTxTest::CreateTx(int nTxType) {
	switch(nTxType) {
		case 0:	{
			string srcAcct("");
			if (!SelectOneAccount(srcAcct)) {
				return false;
			}
			string strDesAcct = srcAcct;
			if (!SelectOneAccount(strDesAcct, true)) {
				return false;
			}
			Value value = m_cBasetest.CreateNormalTx(srcAcct, strDesAcct, 100 * COIN);
			if (m_cBasetest.GetHashFromCreatedTx(value, strSendHash)) {
				cout << "src regId:" << srcAcct<<" create normal tx hash:" << strSendHash << endl;
				return true;
			}
			break;
		}
		case 1:	{
			string srcAcct("");
			if(!SelectOneAccount(srcAcct)) {
				return false;
			}
			BOOST_CHECK(m_cBasetest.GetNewAddr(strNewAddr, false));
			Value value = m_cBasetest.CreateNormalTx(srcAcct, strNewAddr, 100 * COIN);
			if (m_cBasetest.GetHashFromCreatedTx(value, strSendHash)) {
				cout << "src regId:" << srcAcct <<" create regist account step 1 tx hash:" << strSendHash <<endl;
				return true;
			}
			break;
		}
		case 2:	{
			Value value = m_cBasetest.RegistAccountTx(strNewAddr, 100000);
			cout << "register address:" << strNewAddr << endl;
			if (m_cBasetest.GetHashFromCreatedTx(value, strSendHash)) {
				cout << "register address:" << strNewAddr << " create regist account step 2 tx hash:" << strSendHash <<endl;
				return true;
			}
			break;
		}
		case 3:	{
			string regAddress("");
			if(!SelectOneAccount(regAddress)) {
				return false;
			}
			uint64_t llFee = m_cBasetest.GetRandomFee() + 1 * COIN;
			Value value = m_cBasetest.RegisterAppTx(regAddress, "unit_test.bin", 0, llFee);
			if (m_cBasetest.GetHashFromCreatedTx(value, strSendHash)) {
				cout << "regid "<< regAddress <<" create regist app tx hash:" << strSendHash << endl;
				return true;
			}
			break;
		}
		default:
			break;
	}
	return false;
}

BOOST_FIXTURE_TEST_SUITE(CreateTxTest,CCreateTxTest)

BOOST_FIXTURE_TEST_CASE(Test,CCreateTxTest){
	Initialize();
}

BOOST_AUTO_TEST_SUITE_END()
