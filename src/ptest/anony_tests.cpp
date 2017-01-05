/*
 * Anony_tests.cpp
 *
 *  Created on: 2015-04-24
 *      Author: frank
 */

#include "anony_tests.h"
#include "cycle_test_manger.h"

CAnonyTest::CAnonyTest() :
		m_nNum(0), m_nStep(0), m_strTxHash(""), m_strAppRegId(""), m_strRegId("") {
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

emTEST_STATE CAnonyTest::Run() {
	switch (m_nStep) {
	case 0: {
		if (RegistScript()) {
			m_nStep = 2;
		}
		break;
	}
	case 1: {
		CreateAnonyTx();
		break;
	}
	case 2: {
		if (WaitComfirmed(m_strTxHash, m_strAppRegId)) {
			m_nStep = 1;
		}
		break;
	}
	default:
		m_nStep = 1;
		break;
	}
	return EM_NEXT_STATE;
}

bool CAnonyTest::RegistScript() {
	string strFileName("anony.bin");
	int nFee = m_cBasetest.GetRandomFee();
	int nCurHight;
	m_cBasetest.GetBlockHeight(nCurHight);
	string strRegAddr = "";
	if (!SelectOneAccount(strRegAddr)) {
		return false;
	}
	Value regscript = m_cBasetest.RegisterAppTx(strRegAddr, strFileName, nCurHight, nFee + 20 * COIN);
	if (m_cBasetest.GetHashFromCreatedTx(regscript, m_strTxHash)) {
		return true;
	}
	return false;
}

bool CAnonyTest::CreateAnonyTx() {
	srand(time(NULL));
	int nNum = (rand() % 3) + 3;
	vector<string> vstrRegId;
	string strRegId;
	std::pair<std::set<string>::iterator, bool> ret;
	for (int i = 0; i < nNum;) {
		if (SelectOneAccount(strRegId)) {
			if (vstrRegId.end() == find(vstrRegId.begin(), vstrRegId.end(), strRegId)) {
				CRegID cRegid(strRegId);
				string strTemp = HexStr(cRegid.GetVec6());
				vstrRegId.push_back(strTemp);
				++i;
			}
		} else {
			return false;
		}
	}
	ST_ACCOUNT_INFO tAccountInfo;
	int nAccountInfoSize = ::GetSerializeSize(tAccountInfo, SER_NETWORK, g_sProtocolVersion);
	unsigned short usLength = (nNum - 1) * nAccountInfoSize;

	ST_ACCOUNT_INFO arrtAccountInfo[nNum - 1];
	memset(arrtAccountInfo, 0, usLength);

	vector<unsigned char> vuchSendRegId = ParseHex(vstrRegId[0]);
	int64_t llSendTotalMoney(0);
	for (int i = 1; i < nNum; ++i) {
		vector<unsigned char> vuchTemp = ParseHex(vstrRegId[i]);
		memcpy(&tAccountInfo.arrchAccount, &vuchTemp.at(0), vuchTemp.size());
		tAccountInfo.llReciMoney = i * 100 * COIN;
		llSendTotalMoney += tAccountInfo.llReciMoney;
		arrtAccountInfo[i - 1] = tAccountInfo;
		//memcpy(pAccountInfo+(i-1)*accountInfoSize, &accountInfo, accountInfoSize);
	}
	ST_CONTRACT tHead;
	memcpy(tHead.arruchSender, &vuchSendRegId.at(0), vuchSendRegId.size());
	tHead.llPayMoney = llSendTotalMoney;
	tHead.usLen = usLength;
	CDataStream cDataStream(SER_DISK, g_sClientVersion);
	cDataStream << tHead;
	for (int i = 1; i < nNum; ++i) {
		cDataStream << arrtAccountInfo[i - 1];
	}
	string strContract = HexStr(cDataStream);
	Value retValue = m_cBasetest.CreateContractTx(m_strAppRegId, vstrRegId[0], strContract, 0, 0, llSendTotalMoney);
	if (m_cBasetest.GetHashFromCreatedTx(retValue, m_strTxHash)) {
		return true;
	}
	return false;
}

void CAnonyTest::Initialize() {
	CycleTestManger cCycleManager = CycleTestManger::GetNewInstance();
	vector<std::shared_ptr<CycleTestBase> > vecTest;
	if (0 == m_nNum) { 	//if don't have input param -number, default create 100 CCreateTxText instance defalue;
		m_nNum = 100;
	}
	for (int i = 0; i < m_nNum; ++i) {
		vecTest.push_back(std::make_shared<CAnonyTest>());
	}
	cCycleManager.Initialize(vecTest);
	cCycleManager.Run();
}

BOOST_FIXTURE_TEST_SUITE(CreateAnonyTxTest,CAnonyTest)

BOOST_FIXTURE_TEST_CASE(Test,CAnonyTest) {
	Initialize();
}

BOOST_AUTO_TEST_SUITE_END()
