#include <iostream>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include "serialize.h"
#include "txdb.h"
#include "account.h"
#include "main.h"
#include "SysTestBase.h"
using namespace std;
#define DAY_BLOCKS	((24 * 60 * 60)/(10*60))
#define MONTH_BLOCKS (30*DAY_BLOCKS)
#define CHAIN_HEIGHT (10*MONTH_BLOCKS+10)
#define TEST_SIZE 10000
#define MAX_FUND_MONEY 15
#define RANDOM_FUND_MONEY (random(MAX_FUND_MONEY)+1)
#define random(x) (rand()%x)


bool IsEqual(const vector<CFund>& vSrc, const vector<CFund>& vDest) {
	if (vSrc.size() != vDest.size()) {
		return false;
	}

	for (vector<CFund>::const_iterator it = vDest.begin(); it != vDest.end(); it++) {
		if (vSrc.end() == find(vSrc.begin(), vSrc.end(), *it)) {
			return false;
		}
	}

	return true;
}
struct CTxTest :public SysTestBase{
	int nRunTimeHeight;
	string strRegID;
	string strKeyID;
	string strSignAddr;
	CAccount accOperate;
	CAccount accBeforOperate;
	CAccount accYesterDayLastOper;
	vector<unsigned char> authorScript;
	vector_unsigned_char v[11]; //0~9 is valid,10 is used to for invalid scriptID

	CTxTest() {
		ResetEnv();

		Init();
		accOperate.keyID = uint160(1);
		accBeforOperate = accOperate;
	}
	~CTxTest(){

	}


	void InitFund() {
		for (int i = 0; i < TEST_SIZE/100; i++) {
			accOperate.vRewardFund.push_back(CFund(REWARD_FUND, RANDOM_FUND_MONEY, random(5)));
		}

		for (int i = 0; i < TEST_SIZE; i++) {
			int nFundHeight = CHAIN_HEIGHT - MONTH_BLOCKS;
			accOperate.AddToFreedom(CFund(FREEDOM_FUND, RANDOM_FUND_MONEY, nFundHeight+random(MONTH_BLOCKS)), false);
		}
	}

	void Init() {

		CKeyID keyID;
		keyID.SetHex(strKeyID);
		accOperate.keyID = keyID;

		nRunTimeHeight = 0;
		strRegID = "000000000900";
		strKeyID = "a4529134008a4e09e68bec89045ccea6c013bd0b";
		strSignAddr = "mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA";

		srand((unsigned) time(NULL));
		accOperate.llValues = TEST_SIZE*5;

		InitFund();
	}


	void CheckAccountEqual(bool bCheckAuthority = true) {
		BOOST_CHECK(IsEqual(accBeforOperate.vRewardFund, accOperate.vRewardFund));
		BOOST_CHECK(IsEqual(accBeforOperate.vFreedomFund, accOperate.vFreedomFund));
		BOOST_CHECK(accBeforOperate.llValues == accOperate.llValues);

		//cout<<"old: "<<GetTotalValue(accBeforOperate.vSelfFreeze)<<" new: "<<GetTotalValue(accOperate.vSelfFreeze)<<endl;
	}

	uint64_t GetTotalValue(const vector<CFund>& vFund) {
		uint64_t nSum = 0;
		for (vector<CFund>::const_iterator it = vFund.begin(); it != vFund.end(); it++) {
			nSum += it->value;
		}

		return nSum;
	}


};

BOOST_FIXTURE_TEST_SUITE(tx_tests,CTxTest)

BOOST_FIXTURE_TEST_CASE(tx_add_free,CTxTest) {
	//invalid data
	CFund fund(NULL_FUNDTYPE, 1, CHAIN_HEIGHT + 1);
	BOOST_CHECK(!accOperate.OperateAccount(ADD_FREE, fund));
	fund.nFundType = FREEDOM_FUND;
	fund.value = MAX_MONEY;
	BOOST_CHECK(!accOperate.OperateAccount(ADD_FREE, fund));

	accOperate.CompactAccount(CHAIN_HEIGHT);

	for (int i = 0; i < TEST_SIZE; i++) {
		uint64_t nOld = GetTotalValue(accOperate.vRewardFund);
		uint64_t randValue = random(10);
		CFund fundReward(REWARD_FUND, randValue, CHAIN_HEIGHT - 1);
		BOOST_CHECK(accOperate.OperateAccount(ADD_FREE, fundReward));
		BOOST_CHECK(GetTotalValue(accOperate.vRewardFund) == nOld + randValue);

		nOld = GetTotalValue(accOperate.vFreedomFund);
		CFund fundFree(FREEDOM_FUND, randValue, CHAIN_HEIGHT - 1);
		BOOST_CHECK(accOperate.OperateAccount(ADD_FREE, fundFree));
		BOOST_CHECK(GetTotalValue(accOperate.vFreedomFund) == nOld + randValue);
	}

	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();
}

/**
 * brief	:each height test 10 times
 */
BOOST_FIXTURE_TEST_CASE(tx_minus_free,CTxTest) {

	accOperate.CompactAccount(10000);
	for (int i = 1; i <= TEST_SIZE / 10; i++) {

		for (int j = 0; j < 10; j++) {
			uint64_t nOldVectorSum = GetTotalValue(accOperate.vFreedomFund);
			uint64_t minusValue = random(40) + 1;
			CFund fund(REWARD_FUND, minusValue, random(20));
			if (nOldVectorSum >= minusValue) {

				BOOST_CHECK(accOperate.OperateAccount(MINUS_FREE, fund));
				BOOST_CHECK(GetTotalValue(accOperate.vFreedomFund) == nOldVectorSum - minusValue);
			}

		}

		CAccount accountAfterOper = accOperate;
		accOperate.UndoOperateAccount(accOperate.accountOperLog);
		CheckAccountEqual();

		accOperate = accountAfterOper;
		CAccountOperLog log;
		accOperate.accountOperLog = log;

		accBeforOperate = accOperate;
	}

}


BOOST_AUTO_TEST_SUITE_END()
