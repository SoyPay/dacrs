#include "red_packet_tests.h"
#include "cycle_test_manger.h"

typedef struct {
	unsigned char uchDnType;
	uint64_t ullMoney;
	int nNumber;
	unsigned char arruchMessage[200];
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchDnType);
			READWRITE(ullMoney);
			READWRITE(nNumber);
			for (int i = 0;i < 200;i++) {
				READWRITE(arruchMessage[i]);
			}
	)
}ST_RED_PACKET;

typedef struct {
	unsigned char uchDnType;
	unsigned char arruchRedHash[32];
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchDnType);
			for (int i = 0;i < 32;i++) {
				READWRITE(arruchRedHash[i]);
			}
	)
}ST_ACCEPT_RED_PACKET;

typedef struct {
	unsigned char uchSysType;               //0xff
	unsigned char uchType;            // 0x01 提现
	unsigned char uchTypeAddr;            // 0x01 regid 0x02 base58
	IMPLEMENT_SERIALIZE
	(
		READWRITE(uchSysType);
		READWRITE(uchType);
		READWRITE(uchTypeAddr);
	)
} ST_APPACC;

enum emTX_TYPE{
	EM_TX_COMM_SENDREDPACKET 		= 0x01,
	EM_TX_COMM_ACCEPTREDPACKET 		= 0x02,
	EM_TX_SPECIAL_SENDREDPACKET 	= 0x03,
	EM_TX_SPECIAL_ACCEPTREDPACKET 	= 0x04
};

emTEST_STATE CRedPacketTest::Run() {
	switch (m_nStep) {
	case 0: {
		RegistScript();
		break;
	}
	case 1: {
		WaitRegistScript();
		break;
	}
	case 2: {
		WithDraw();
		break;
	}
	case 3: {
		WaitTxConfirmedPackage(m_strTxHash);
		break;
	}
	case 4: {
		SendSpecailRedPacketTx();
		break;
	}
	case 5: {
		WaitTxConfirmedPackage(m_strRedHash);
		break;
	}
	case 6: {
		AcceptSpecailRedPacketTx();
		break;
	}
	case 7: {
		WaitTxConfirmedPackage(m_strTxHash);
		break;
	}
	default: {
		m_nStep = 6;
		break;
	}
	}
	return EM_NEXT_STATE;
}

CRedPacketTest::CRedPacketTest(){
	m_nStep 			= 0;
	m_strTxHash 		= "";
	m_strAppRegId 		= "";
	m_nNum 				= 0;
	m_strAppAddr 		= "";
	m_ullSpecailmM 		= 0;
	m_srtRchangeAddr 	= "";
}

bool CRedPacketTest::RegistScript() {
	m_cBasetest.ImportAllPrivateKey();
	string strFileName("redpacket.bin");
	int nFee = m_cBasetest.GetRandomFee();
	int nCurHight;
	m_cBasetest.GetBlockHeight(nCurHight);
	string strRegAddr="";
	if (!SelectOneAccount(strRegAddr)) {
		return false;
	}
	//reg anony app
	Value regscript = m_cBasetest.RegisterAppTx(strRegAddr, strFileName, nCurHight, nFee+20*COIN);
	if (m_cBasetest.GetHashFromCreatedTx(regscript, m_strTxHash)) {
		m_nStep++;
		return true;
	}
	return false;
}

bool CRedPacketTest::WaitRegistScript() {
	m_cBasetest.GenerateOneBlock();
	if (m_cBasetest.GetTxConfirmedRegID(m_strTxHash, m_strAppRegId)) {
		m_nStep++;
		return true;
	}
	return true;
}

bool CRedPacketTest::WaitTxConfirmedPackage(string m_strTxHash) {
	m_cBasetest.GenerateOneBlock();
	string strRegid ="";
	if (m_cBasetest.GetTxConfirmedRegID(m_strTxHash, strRegid)) {
		m_nStep++;
		return true;
	}
	return true;
}

bool CRedPacketTest::WithDraw() {
	ST_APPACC tAccData;
	memset(&tAccData,0,sizeof(ST_APPACC));
	tAccData.uchSysType = 0xff;
	tAccData.uchType = 0x02;  /// 0xff 表示提现 或者充值 0x01 提现 0x02 充值
	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tAccData;
	string strSendContract = HexStr(cScriptData);

	if (!SelectOneAccount(m_strAppAddr)) {
		return false;
	}

	m_srtRchangeAddr = m_strAppAddr;
	uint64_t ullMoney = 100000000000;
	Value  darwpack= m_cBasetest.CreateContractTx(m_strAppRegId,m_strAppAddr,strSendContract,0,100000000,ullMoney);

	if (m_cBasetest.GetHashFromCreatedTx(darwpack, m_strTxHash)) {
		m_nStep++;
		return true;
	}
	return true;
}

bool CRedPacketTest::SendRedPacketTx() {
	if (m_strAppRegId == "" || m_strAppAddr == "") {
		return false;
	}
	ST_RED_PACKET tRedPacket;
	tRedPacket.uchDnType = EM_TX_COMM_SENDREDPACKET;
	tRedPacket.ullMoney = 100000000;
	tRedPacket.nNumber = 2;
	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tRedPacket;
	string strSendContract = HexStr(cScriptData);
	Value buyerpack = m_cBasetest.CreateContractTx(m_strAppRegId, m_strAppAddr, strSendContract, 0, 100000000,
			tRedPacket.ullMoney);
	if (m_cBasetest.GetHashFromCreatedTx(buyerpack, m_strRedHash)) {
		m_nStep++;
		return true;
	}
	return true;
}

bool CRedPacketTest::AcceptRedPacketTx() {
	if (m_strAppRegId == "") {
		return false;
	}
	ST_ACCEPT_RED_PACKET tAcceptredpacket;
	tAcceptredpacket.uchDnType = EM_TX_COMM_ACCEPTREDPACKET;
	memcpy(tAcceptredpacket.arruchRedHash, uint256S(m_strRedHash).begin(), sizeof(tAcceptredpacket.arruchRedHash));

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tAcceptredpacket;
	string strSendContract = HexStr(cScriptData);
	string strRegAddr="";
	if (!SelectOneAccount(strRegAddr)) {
		return false;
	}

	Value  buyerpack = m_cBasetest.CreateContractTx(m_strAppRegId,strRegAddr,strSendContract,0,100000000,0);
	if (m_cBasetest.GetHashFromCreatedTx(buyerpack, m_strTxHash)) {
		m_nStep++;
		return true;
	} else {
		m_nStep = 4;
	}
	return true;
}

void CRedPacketTest::Initialize() {
	CycleTestManger cCycleManager = CycleTestManger::GetNewInstance();
	vector<std::shared_ptr<CycleTestBase> > vTest;
	if (0 == m_nNum) { //if don't have input param -number, default create 100 CCreateTxText instance defalue;
		m_nNum = 1;
	}
	for (int i = 0; i < m_nNum; ++i) {
		vTest.push_back(std::make_shared<CRedPacketTest>());
	}
	cCycleManager.Initialize(vTest);
	cCycleManager.Run();
}

bool CRedPacketTest::SendSpecailRedPacketTx() {
	if (m_strAppRegId == "" || m_strAppAddr == "") {
		return false;
	}

	ST_RED_PACKET tRedpacket;
	tRedpacket.uchDnType = EM_TX_SPECIAL_SENDREDPACKET;
	tRedpacket.ullMoney = 2000000000;
	tRedpacket.nNumber = 2;
	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tRedpacket;
	string strSendContract = HexStr(cScriptData);
	Value  Specailbuyerpack= m_cBasetest.CreateContractTx(m_strAppRegId,m_strAppAddr,strSendContract,0,100000000,tRedpacket.ullMoney);
	if (m_cBasetest.GetHashFromCreatedTx(Specailbuyerpack, m_strRedHash)) {
		m_nStep++;
		return true;
	}
	return true;
}

bool CRedPacketTest::AcceptSpecailRedPacketTx() {
	if (m_strAppRegId == "") {
		return false;
	}

	if (WithDraw()) {
		ST_ACCEPT_RED_PACKET tAcceptredpacket;

		tAcceptredpacket.uchDnType = EM_TX_SPECIAL_ACCEPTREDPACKET;
		memcpy(tAcceptredpacket.arruchRedHash, uint256S(m_strRedHash).begin(), sizeof(tAcceptredpacket.arruchRedHash));

		CDataStream cScriptData(SER_DISK, g_sClientVersion);
		cScriptData << tAcceptredpacket;
		string strSendContract = HexStr(cScriptData);

		Value buyerpack = m_cBasetest.CreateContractTx(m_strAppRegId, m_srtRchangeAddr, strSendContract, 0, 1000000000,
				0);

		if (m_cBasetest.GetHashFromCreatedTx(buyerpack, m_strTxHash)) {
			m_nStep++;
			return true;
		} else {
			m_nStep = 4;
		}
	}
	return true;
}

BOOST_FIXTURE_TEST_SUITE(CredTest,CRedPacketTest)

BOOST_FIXTURE_TEST_CASE(Test,CRedPacketTest) {
	Initialize();
}

BOOST_AUTO_TEST_SUITE_END()
