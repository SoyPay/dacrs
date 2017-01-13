
#include "create_tx_tests.h"
#include "cycle_test_manger.h"

int CCreateTxTest::m_snCount = 0;

CCreateTxTest::CCreateTxTest() :
		m_nTxType(0), m_nNum(0), m_nStep(0), m_strSendHash(""), m_strNewAddr("") {
	for (int i = 1; i < boost::unit_test::framework::master_test_suite().argc; ++i) {
		string strArgv = boost::unit_test::framework::master_test_suite().argv[i];
		string::size_type pos = strArgv.find("-number=");
		if (string::npos != pos) {
			string strNum = strArgv.substr(pos + strlen("-number="), string::npos);
			m_nNum = Str2Int(strNum);
			break;
		}
	}
}

void CCreateTxTest::Initialize() {
	CycleTestManger cCycleManager = CycleTestManger::GetNewInstance();
	vector<std::shared_ptr<CycleTestBase> > vecTest;
	if (0 == m_nNum) { 	//if don't have input param -number, default create 100 CCreateTxText instance defalue;
		m_nNum = 100;
	}
	for (int i = 0; i < m_nNum; ++i) {
		vecTest.push_back(std::make_shared<CCreateTxTest>());
	}
	cCycleManager.Initialize(vecTest);
	cCycleManager.Run();
}

emTEST_STATE CCreateTxTest::Run() {
	switch (m_nStep) {
	case 0: {
		if (!m_vstrAccount.empty() || SelectAccounts(m_vstrAccount)) {
			++m_nStep;
		}
		break;
	}
	case 1: {
		if (CreateTx(m_nTxType)) {
			++m_nStep;
		}
		if (++m_nTxType > 3) {
			m_nTxType = 0;
		}
		break;
	}
	case 2: {
		string strComfirmedPos;
		if (WaitComfirmed(m_strSendHash, strComfirmedPos) == true) {
			IncSentTotal();
			if (++m_snCount >= m_nNum) {
				m_snCount = 0;
				m_nStep = 0;
				m_vstrAccount.clear();
			} else {
				m_nStep = 1; //等待确认完成 后从新创建，但不是初始化
			}
		}
		break;
	}
	}
	return EM_NEXT_STATE;
}

bool CCreateTxTest::CreateTx(int nTxType) {
	switch (nTxType) {
	case 0: {
		string strSrcAcct("");
		if (!SelectOneAccount(strSrcAcct)) {
			return false;
		}
		string strDesAcct = strSrcAcct;
		if (!SelectOneAccount(strDesAcct, true)) {
			return false;
		}
		Value value = m_cBasetest.CreateNormalTx(strSrcAcct, strDesAcct, 100 * COIN);
		if (m_cBasetest.GetHashFromCreatedTx(value, m_strSendHash)) {
			cout << "src regId:" << strSrcAcct << " create normal tx hash:" << m_strSendHash << endl;
			return true;
		}
		break;
	}
	case 1: {
		string strSrcAcct("");
		if (!SelectOneAccount(strSrcAcct)) {
			return false;
		}
		BOOST_CHECK(m_cBasetest.GetNewAddr(m_strNewAddr, false));
		Value value = m_cBasetest.CreateNormalTx(strSrcAcct, m_strNewAddr, 100 * COIN);
		if (m_cBasetest.GetHashFromCreatedTx(value, m_strSendHash)) {
			cout << "src regId:" << strSrcAcct << " create regist account step 1 tx hash:" << m_strSendHash << endl;
			return true;
		}
		break;
	}
	case 2: {
		Value value = m_cBasetest.RegistAccountTx(m_strNewAddr, 100000);
		cout << "register address:" << m_strNewAddr << endl;
		if (m_cBasetest.GetHashFromCreatedTx(value, m_strSendHash)) {
			cout << "register address:" << m_strNewAddr << " create regist account step 2 tx hash:" << m_strSendHash
					<< endl;
			return true;
		}
		break;
	}
	case 3: {
		string strRegAddress("");
		if (!SelectOneAccount(strRegAddress)) {
			return false;
		}
		uint64_t ullFee = m_cBasetest.GetRandomFee() + 1 * COIN;
		Value value = m_cBasetest.RegisterAppTx(strRegAddress, "unit_test.bin", 0, ullFee);
		if (m_cBasetest.GetHashFromCreatedTx(value, m_strSendHash)) {
			cout << "regid " << strRegAddress << " create regist app tx hash:" << m_strSendHash << endl;
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
