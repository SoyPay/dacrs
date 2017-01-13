#include <iostream>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include "serialize.h"
#include "txdb.h"
#include "database.h"
#include "main.h"
#include "systestbase.h"
using namespace std;
#define DAY_BLOCKS	((24 * 60 * 60)/(10*60))
#define MONTH_BLOCKS (30*DAY_BLOCKS)
#define CHAIN_HEIGHT (10*MONTH_BLOCKS+10)
#define TEST_SIZE 10000
#define MAX_FUND_MONEY 15
#define RANDOM_FUND_MONEY (random(MAX_FUND_MONEY)+1)
#define random(x) (rand()%x)

#define TX_HASH "022596466a"
#define AMOUNT 100*COIN
#define NUMBER 20

bool GetRpcHash(const string &strHash, string &strRetHash) {
	const char *argv[] = { "rpctest", "gethash", strHash.c_str() };
	int argc = sizeof(argv) / sizeof(char*);
	Value value;
	if (!SysTestBase::CommandLineRPC_GetValue(argc, argv, value)) {
		return false;
	}
	const Value& result = find_value(value.get_obj(), "hash");
	if (result == null_type) {
		return false;
	}
	strRetHash = result.get_str();
	return true;
}

struct CTxTest : public SysTestBase {
	int m_nRunTimeHeight;
	string m_strRegID;
	string m_strKeyID;
	string m_strSignAddr;
	CAccount m_cAccOperate;
	CAccount m_cAccBeforOperate;
	vector<unsigned char> m_vcAuthorScript;
	vector_unsigned_char v[11]; //0~9 is valid,10 is used to for invalid scriptID

	CTxTest() {
		ResetEnv();
		m_cAccOperate.m_cKeyID.SetNull();
		m_cAccBeforOperate = m_cAccOperate;
		Init();
	}

	~CTxTest() {
	}

	void InitFund() {
		srand((unsigned) time(NULL));
		// for (int i = 0; i < TEST_SIZE/100; i++) {
		// m_cAccOperate.vRewardFund.push_back(CFund(RANDOM_FUND_MONEY, random(5)));
		// }

		// for (int i = 0; i < TEST_SIZE; i++) {
		// int nFundHeight = CHAIN_HEIGHT - MONTH_BLOCKS;
		// m_cAccOperate.OperateAccount(EM_ADD_FREE, nFundHeight+random(MONTH_BLOCKS), nFundHeight);
		// }
	}

	void Init() {
		m_nRunTimeHeight = 0;
		m_strRegID = "000000000900";
		m_strKeyID = "a4529134008a4e09e68bec89045ccea6c013bd0b";
		m_strSignAddr = "dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem";
		CKeyID ckeyID;
		ckeyID.SetHex(m_strKeyID);
		m_cAccOperate.m_cKeyID = ckeyID;
		m_cAccOperate.m_ullValues = TEST_SIZE * 5;
		InitFund();
	}

	void CheckAccountEqual(bool bCheckAuthority = true) {
		// BOOST_CHECK(IsEqual(m_cAccBeforOperate.vRewardFund, m_cAccOperate.vRewardFund));
		// BOOST_CHECK(m_cAccBeforOperate.llValues == m_cAccOperate.llValues);
		// cout<<"old: "<<GetTotalValue(m_cAccBeforOperate.vSelfFreeze)<<" new: "<<GetTotalValue(m_cAccOperate.vSelfFreeze)<<endl;
	}
};

BOOST_FIXTURE_TEST_SUITE(tx_tests,CTxTest)

BOOST_FIXTURE_TEST_CASE(tx_add_free,CTxTest) {
	// invalid data
	// CFund fund(1, CHAIN_HEIGHT + 1);
	int nHeight = g_cChainActive.Tip()->m_nHeight;
	BOOST_CHECK(m_cAccOperate.OperateAccount(EM_ADD_FREE, 1, nHeight));
	// fund.value = MAX_MONEY;

    // accOperate.CompactAccount(CHAIN_HEIGHT);

	for (int i = 0; i < TEST_SIZE; i++) {
		// uint64_t nOld = accOperate.GetRewardAmount(CHAIN_HEIGHT)+accOperate.GetRawBalance(CHAIN_HEIGHT);
		uint64_t ullRandValue = random(10);
		// CFund fundReward(randValue, CHAIN_HEIGHT - 1);
		BOOST_CHECK(m_cAccOperate.OperateAccount(EM_ADD_FREE, ullRandValue, nHeight));
		// BOOST_CHECK(accOperate.GetRewardAmount(CHAIN_HEIGHT)+accOperate.GetRawBalance(CHAIN_HEIGHT) == nOld + randValue);

	}
	BOOST_CHECK(!m_cAccOperate.OperateAccount(EM_ADD_FREE, GetMaxMoney(), nHeight));

	CheckAccountEqual();
}

/**
 * brief	:each height test 10 times
 */
BOOST_FIXTURE_TEST_CASE(tx_minus_free,CTxTest) {
	for (int i = 1; i <= TEST_SIZE / 10; i++) {
	// for (int j = 0; j < 10; j++) {
	// int64_t nOldVectorSum = GetTotalValue(accOperate.vFreedomFund);
	// uint64_t minusValue = random(40) + 1;
	// CFund fund(minusValue, random(20));
	// if (nOldVectorSum >= minusValue) {
	//
	// BOOST_CHECK(accOperate.OperateAccount(MINUS_FREE, fund));
	// BOOST_CHECK(GetTotalValue(accOperate.vFreedomFund) == nOldVectorSum - minusValue);
	// }
	//
	// }

	// CAccount accountAfterOper = accOperate;
	// accOperate.UndoOperateAccount(accOperate.accountOperLog);
	// CheckAccountEqual();
	//
	// accOperate = accountAfterOper;
	// CAccountOperLog log;
	// accOperate.accountOperLog = log;
	//
	// accBeforOperate = accOperate;
	}
}

BOOST_FIXTURE_TEST_CASE(red_packet, CTxTest) {
	//gethash
	string strRetHash;
	vector<int> vnRetPacket;
	int64_t llTotal = 0;
	string strInitHash = TX_HASH;
	do {
		BOOST_CHECK(GetRpcHash(strInitHash, strRetHash));
		strInitHash = strRetHash;
		vector<unsigned char> vuchRet = ParseHex(strRetHash);
		for (size_t i = 0; i < vuchRet.size();) {
			int nData = vuchRet[i] << 16 | vuchRet[i + 1];
			vnRetPacket.push_back(nData % 1000 + 1000);
			llTotal += nData % 1000 + 1000;
			if (vnRetPacket.size() == NUMBER) {
				break;
			}
			i += 2;
		}
	} while (vnRetPacket.size() != NUMBER);
	vector<int> vnRedPacket;
	int64_t nllTotal_Packet = 0;
	for (size_t j = 0; j < vnRetPacket.size(); ++j) {
		int64_t llRedAmount = vnRetPacket[j] * AMOUNT / llTotal;
		vnRedPacket.push_back(llRedAmount);
		nllTotal_Packet += llRedAmount;
		double dRedAmount = llRedAmount / COIN;
		cout << "index" << j << ", redPackets:" << llRedAmount << " " << dRedAmount << endl;
	}
	double dTotalPacket = nllTotal_Packet / COIN;
	cout << "total:" << nllTotal_Packet << " " << dTotalPacket << endl;
}

BOOST_AUTO_TEST_SUITE_END()

