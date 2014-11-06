#include <iostream>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include "serialize.h"
#include "txdb.h"
#include "account.h"
#include "main.h"
using namespace std;
#define MONTH_BLOCKS (30*(24 * 60 * 60)/(10*60))
#define CHAIN_HEIGHT (10*MONTH_BLOCKS+10)
#define TEST_SIZE 10000
#define MAX_FUND_MONEY 10
#define RANDOM_FUND_MONEY (random(MAX_FUND_MONEY)+1)
#define random(x) (rand()%x)

class CFindIterByID {
public:
	CFindIterByID(const vector_unsigned_char& scriptID) {
		m_scriptID = scriptID;
	}
	bool operator()(const CFund& fund) {
		return fund.scriptID == m_scriptID;
	}

private:
	vector_unsigned_char m_scriptID;

};

void GenerateRandomHash(vector<uint256>& vHash, uint64_t nSize = TEST_SIZE) {
	vHash.reserve(nSize);
	vector<unsigned char> vRandom;
	vRandom.reserve(32);

	//generate key hash
	for (int i = 0; i < nSize; i++) {
		CKey key;
		key.MakeNewKey(false);
		string str(key.begin(), key.end());
		vRandom.clear();
		vRandom.insert(vRandom.begin(), key.begin(), key.end());
		uint256 hash(vRandom);
		vHash.push_back(hash);
		//cout << hash.GetHex() << endl;
	}
}

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
struct CTxTest {
	int nRunTimeHeight;
	CBlockIndex block;
	CAccount accOperate;
	CAccount accBeforOperate;
	vector_unsigned_char v[11]; //0~9 is valid,10 is used to for invalid scriptID

	CTxTest() {
		Init();
		accBeforOperate = accOperate;
	}

	void InitAuthorization(const string& str) {
		vector<unsigned char> scriptID = ParseHex(str.c_str());
		CAuthorizate author;
		author.SetAuthorizeTime(CHAIN_HEIGHT + 100);
		author.SetMaxMoneyPerDay(1000);
		author.SetCurMaxMoneyPerDay(author.GetMaxMoneyPerDay());
		author.SetLastOperHeight(0);
		author.SetMaxMoneyPerTime(27);
		author.SetMaxMoneyTotal(20*TEST_SIZE);
		accOperate.mapAuthorizate[scriptID] = author;
	}

	void InitFund() {
		char buf[10];
		for (int j = 100000, i = 0; j < 100011; j++, i++) {
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "%d", j);
			v[i] = ParseHex(buf);
		}

		for (int i = 0; i < TEST_SIZE; i++) {
			accOperate.vRewardFund.push_back(CFund(REWARD_FUND, RANDOM_FUND_MONEY, random(20)));
		}

		for (int i = 0; i < TEST_SIZE; i++) {
			int nFundHeight = CHAIN_HEIGHT - MONTH_BLOCKS;
			accOperate.AddToFreedom(CFund(FREEDOM_FUND, RANDOM_FUND_MONEY, nFundHeight+random(MONTH_BLOCKS)), false);
		}

		for (int i = 0; i < TEST_SIZE; i++) {
			accOperate.AddToFreeze(CFund(FREEZD_FUND, RANDOM_FUND_MONEY, random(15), v[random(10)]), false);
		}

		for (int i = 0; i < TEST_SIZE; i++) {
			accOperate.vSelfFreeze.push_back(CFund(SELF_FREEZD_FUND, RANDOM_FUND_MONEY, random(20)));
		}
	}

	void Init() {
		srand((unsigned) time(NULL));
		accOperate.llValues = TEST_SIZE;
		block.nHeight = CHAIN_HEIGHT;
		chainActive.SetTip(&block);
		BOOST_CHECK(NULL != chainActive.Tip());

		nRunTimeHeight = 0;

		InitAuthorization("00112233");

		InitFund();
	}

	void CheckAccountEqual() {
		BOOST_CHECK(IsEqual(accBeforOperate.vRewardFund, accOperate.vRewardFund));
		BOOST_CHECK(IsEqual(accBeforOperate.vFreedomFund, accOperate.vFreedomFund));
		BOOST_CHECK(IsEqual(accBeforOperate.vFreeze, accOperate.vFreeze));
		BOOST_CHECK(IsEqual(accBeforOperate.vSelfFreeze, accOperate.vSelfFreeze));
		BOOST_CHECK(accBeforOperate.llValues == accOperate.llValues);
	}

	uint64_t GetTotalValue(const vector<CFund>& vFund) {
		uint64_t nSum = 0;
		for (vector<CFund>::const_iterator it = vFund.begin(); it != vFund.end(); it++) {
			nSum += it->value;
		}

		return nSum;
	}

	void FillSpecificAndRandData(vector<uint256>& vHashCopy, vector<CFund>& vSpecificFund) {
		for (vector<CFund>::iterator it = vSpecificFund.begin(); it != vSpecificFund.end(); it++) {
			//vHashCopy.push_back(it->uTxHash);
		}

		//insert 100 data that not in the specific vector
		vector<uint256> vRandomHash;
		GenerateRandomHash(vRandomHash, TEST_SIZE / 10);
		vHashCopy.insert(vHashCopy.end(), vRandomHash.begin(), vRandomHash.end());
		random_shuffle(vHashCopy.begin(), vHashCopy.end());
	}

	void CheckAuthorization(const CAuthorizate OldAuth, uint64_t nMoney, int nHeight,
			const vector<unsigned char>& scriptID) {
		CAuthorizate newAuthor = accOperate.mapAuthorizate[scriptID];
		BOOST_CHECK(OldAuth.GetMaxMoneyTotal() == newAuthor.GetMaxMoneyTotal() + nMoney);

		const uint64_t nBlocksPerDay = 24 * 60 / 10;	//amount of blocks that connected into chain per day
		if (OldAuth.GetLastOperHeight() / nBlocksPerDay < nHeight / nBlocksPerDay)
			BOOST_CHECK(OldAuth.GetMaxMoneyPerDay() == newAuthor.GetCurMaxMoneyPerDay() + nMoney);
		else
			BOOST_CHECK(OldAuth.GetCurMaxMoneyPerDay() == newAuthor.GetCurMaxMoneyPerDay() + nMoney);
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

BOOST_FIXTURE_TEST_CASE(tx_minus_free,CTxTest) {
	vector<unsigned char> scriptID = ParseHex("00112233");
	int nBranch[5] = { 0 };
	int nRunTimeHeight = 0;
	for (int i = 0; i < TEST_SIZE; i++) {
		nRunTimeHeight += 1;
		uint64_t nOldVectorSum = GetTotalValue(accOperate.vFreedomFund);
		uint64_t nOldValue = accOperate.llValues;
		uint64_t randValue = random(40);
		CFund fund(REWARD_FUND, randValue, random(20));
		CAuthorizate OldAuthor = accOperate.mapAuthorizate[scriptID];

		if (nOldVectorSum >= randValue) {
			if (accOperate.IsAuthorized(fund.value, nRunTimeHeight, scriptID)) {
				//run branch 0:enough money in vector and authorized by script
				BOOST_CHECK(accOperate.OperateAccount(MINUS_FREE, fund, nRunTimeHeight, &scriptID,true));
				BOOST_CHECK(GetTotalValue(accOperate.vFreedomFund) == nOldVectorSum - randValue);
				CheckAuthorization(OldAuthor, fund.value, nRunTimeHeight, scriptID);
				nBranch[0]++;
			} else {
				//run branch 1:enough money in vector but not authorized by script
				BOOST_CHECK(!accOperate.OperateAccount(MINUS_FREE, fund, nRunTimeHeight, &scriptID,true));
				nBranch[1]++;
			}
		} else {
			if (accOperate.llValues + nOldVectorSum >= randValue) {
				if (accOperate.IsAuthorized(fund.value, nRunTimeHeight, scriptID)) {
					//run branch 2:enough money in (vector+llvalue) and authorized by script
					BOOST_CHECK(accOperate.OperateAccount(MINUS_FREE, fund, nRunTimeHeight, &scriptID,true));
					BOOST_CHECK(
							GetTotalValue(accOperate.vFreedomFund) + accOperate.llValues
									== nOldValue + nOldVectorSum - randValue);
					CheckAuthorization(OldAuthor, fund.value, nRunTimeHeight, scriptID);
					nBranch[2]++;
				} else {
					//run branch 3:enough money in (vector+llvalue) but not authorized by script
					BOOST_CHECK(!accOperate.OperateAccount(MINUS_FREE, fund, nRunTimeHeight, &scriptID,true));
					nBranch[3]++;
				}

			} else {
				//run branch 4:not enough money
				BOOST_CHECK(!accOperate.OperateAccount(MINUS_FREE, fund, nRunTimeHeight, &scriptID,true));
				BOOST_CHECK(GetTotalValue(accOperate.vFreedomFund) == nOldVectorSum);
				BOOST_CHECK(nOldValue == accOperate.llValues);
				nBranch[4]++;
			}
		}
	}

	for (int i = 0; i < sizeof(nBranch) / sizeof(nBranch[0]); i++)
		cout << "branch " << i << " is " << nBranch[i] << endl;

	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();
}

BOOST_FIXTURE_TEST_CASE(tx_add_self,CTxTest) {
	accOperate.CompactAccount(CHAIN_HEIGHT);

	//invalid param check
	CFund fundvalid(SELF_FREEZD_FUND, 1, CHAIN_HEIGHT - 1);
	CFund fundInvalid1(fundvalid);
	fundInvalid1.nFundType = 0;
	BOOST_CHECK(!accOperate.OperateAccount(ADD_SELF_FREEZD, fundInvalid1));
	fundInvalid1.nFundType = NULL_OPERTYPE;
	BOOST_CHECK(!accOperate.OperateAccount(ADD_SELF_FREEZD, fundInvalid1));

	//random check
	for (int i = 0; i < TEST_SIZE; i++) {
		uint64_t nOldInput = GetTotalValue(accOperate.vSelfFreeze);
		uint64_t randValue = random(10);
		CFund fund(SELF_FREEZD_FUND, randValue, CHAIN_HEIGHT - 1);
		BOOST_CHECK(accOperate.OperateAccount(ADD_SELF_FREEZD, fund));
		BOOST_CHECK(GetTotalValue(accOperate.vSelfFreeze) == nOldInput + randValue);
	}

	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();
}

BOOST_FIXTURE_TEST_CASE(tx_minus_self,CTxTest) {
	accOperate.CompactAccount(5);
	int nBranch[3] = { 0 };
	int nRunTimeHeight = 0;
	vector<unsigned char> scriptID = ParseHex("00112233");
	CAuthorizate OldAuthor = accOperate.mapAuthorizate[scriptID];
	for (int i = 0; i < TEST_SIZE; i++) {
		nRunTimeHeight += 1;
		uint64_t nOldVectorSum = GetTotalValue(accOperate.vSelfFreeze);
		uint64_t randValue = random(30);
		CFund fund(REWARD_FUND, randValue, random(20));

		if (nOldVectorSum >= randValue) {
			if (accOperate.IsAuthorized(fund.value, nRunTimeHeight, scriptID)) {
				//branch 0:enough money and authorized by script
				BOOST_CHECK(accOperate.OperateAccount(MINUS_SELF_FREEZD, fund, nRunTimeHeight, &scriptID,true));
				BOOST_CHECK(GetTotalValue(accOperate.vSelfFreeze) == nOldVectorSum - randValue);
				nBranch[0]++;
			} else {
				//branch 0:enough money but not authorized by script
				BOOST_CHECK(!accOperate.OperateAccount(MINUS_SELF_FREEZD, fund, nRunTimeHeight, &scriptID,true));
				nBranch[1]++;
			}

		} else {
			//not enough money
			BOOST_CHECK(!accOperate.OperateAccount(MINUS_SELF_FREEZD, fund, nRunTimeHeight, &scriptID,true));
			BOOST_CHECK(GetTotalValue(accOperate.vSelfFreeze) == nOldVectorSum);
			nBranch[2]++;
		}
	}

	for (int i = 0; i < sizeof(nBranch) / sizeof(nBranch[0]); i++)
		cout << "branch " << i << " is " << nBranch[i] << endl;
	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();
}

BOOST_FIXTURE_TEST_CASE(tx_add_freezed,CTxTest) {
	accOperate.CompactAccount(CHAIN_HEIGHT);

	//invalid param check
	CFund fundvalid(FREEDOM_FUND, 1, CHAIN_HEIGHT - 1);
	CFund fundInvalid1(fundvalid);
	fundInvalid1.nFundType = 0;
	BOOST_CHECK(!accOperate.OperateAccount(ADD_FREEZD, fundInvalid1));
	fundInvalid1.nFundType = NULL_FUNDTYPE;
	BOOST_CHECK(!accOperate.OperateAccount(ADD_FREEZD, fundInvalid1));

	//random check
	for (int i = 0; i < TEST_SIZE; i++) {
		uint64_t nOldInput = GetTotalValue(accOperate.vFreeze);
		uint64_t randValue = random(10);
		CFund fund(FREEZD_FUND, randValue, CHAIN_HEIGHT - 1);
		BOOST_CHECK(accOperate.OperateAccount(ADD_FREEZD, fund));
		BOOST_CHECK(GetTotalValue(accOperate.vFreeze) == nOldInput + randValue);
	}

	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();
}

/**
 * branch 1:scriptID or height in target fund not match any funds in vFreeze
 * branch 2:minus money from vFreeze
 * branch 3:not enough money in vFreeze
 */
BOOST_FIXTURE_TEST_CASE(tx_minus_freezed,CTxTest) {
	accOperate.CompactAccount(4);

	int nBranch[3] = { 0 };
	int nHeight = 0;
	for (int i = 0; i < TEST_SIZE; i++) {
		uint64_t nRandomValue = random(20) + 1;
		int nHeight = random(15);
		CFund fund(FREEZD_FUND, nRandomValue, nHeight, v[random(11)]);
		auto iter = find_if(accOperate.vFreeze.begin(), accOperate.vFreeze.end(), [&](const CFund& fundInVec)
		{
			return fundInVec.scriptID == fund.scriptID && fundInVec.nHeight == fund.nHeight;
		});

		if (accOperate.vFreeze.end() == iter) {
			//branch 0:scriptID or nHeight not in target fund not match any funds in vector(vFreezed)
			BOOST_CHECK(!accOperate.OperateAccount(MINUS_FREEZD, fund));
			nBranch[0]++;
			continue;
		}

		uint64_t nOldFreezeValue = GetTotalValue(accOperate.vFreeze);
		uint64_t nOldIterValue = iter->value;
		if (nRandomValue <= iter->value) {
			//branch 0:enough money and execute minus-operation
			BOOST_CHECK(accOperate.OperateAccount(MINUS_FREEZD, fund));
			BOOST_CHECK(GetTotalValue(accOperate.vFreeze) + nRandomValue == nOldFreezeValue);
			nBranch[1]++;

		} else {
			//branch 0:not enough money
			BOOST_CHECK(!accOperate.OperateAccount(MINUS_FREEZD, fund));
			BOOST_CHECK(GetTotalValue(accOperate.vFreeze) == nOldFreezeValue);
			nBranch[2]++;
		}
	}

	for (int i = 0; i < sizeof(nBranch) / sizeof(nBranch[0]); i++)
		cout << "branch " << i << " is " << nBranch[i] << endl;
	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();

}

BOOST_AUTO_TEST_SUITE_END()
