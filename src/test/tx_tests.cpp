#include <iostream>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include "serialize.h"
#include "txdb.h"
#include "account.h"
#include "main.h"
using namespace std;

#define CHAIN_HEIGHT (nMaxCoinDay/nTargetSpacing)+10
#define VECTOR_SIZE 1000
#define random(x) (rand()%x)
vector<uint256> g_vHash[4];
class CFindIterByHash {
public:
	CFindIterByHash(uint256 hash) {
		m_hash = hash;
	}
	bool operator()(const CFund& fund) {
		return fund.uTxHash == m_hash;
	}

private:
	uint256 m_hash;

};

void GenerateRandomHash(vector<uint256>& vHash, uint64_t nSize = VECTOR_SIZE) {
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

void Init(CBlockIndex* pBlockIndex, CSecureAccount& account) {
	srand((unsigned) time(NULL));
	account.llValues = 100;
	pBlockIndex->nHeight = CHAIN_HEIGHT;
	chainActive.SetTip(pBlockIndex);
	BOOST_CHECK(NULL != chainActive.Tip());

	for (int i = 0; i < 4; i++) {
		GenerateRandomHash(g_vHash[i]);
	}

	for (int i = 0; i < VECTOR_SIZE; i++) {
		account.vRewardFund.push_back(CFund(REWARD_FUND, g_vHash[0].at(i), random(10), random(20)));
	}

	for (int i = 0; i < VECTOR_SIZE; i++) {
		account.vFreedomFund.push_back(CFund(FREEDOM_FUND, g_vHash[1].at(i), random(5), random(20)));
	}

	for (int i = 0; i < VECTOR_SIZE; i++) {
		account.vSelfFreeze.push_back(
				CFund(SELF_FREEZD_FUND, g_vHash[3].at(i), random(10), CHAIN_HEIGHT - 10 + random(30)));
	}

	for (int i = 0; i < VECTOR_SIZE; i++) {
		account.vFreeze.push_back(
				CFund(FREEZD_FUND, g_vHash[2].at(i), random(10), CHAIN_HEIGHT - 1 + random(3)));
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
	CBlockIndex block;
	CSecureAccount accOperate;
	CSecureAccount accBeforOperate;

	CTxTest() {
		::Init(&block, accOperate);
		accBeforOperate = accOperate;
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
			vHashCopy.push_back(it->uTxHash);
		}

		//insert 100 data that not in the specific vector
		vector<uint256> vRandomHash;
		GenerateRandomHash(vRandomHash, VECTOR_SIZE/10);
		vHashCopy.insert(vHashCopy.end(), vRandomHash.begin(), vRandomHash.end());
		random_shuffle(vHashCopy.begin(), vHashCopy.end());
	}

};

BOOST_FIXTURE_TEST_SUITE(tx_tests,CTxTest)

BOOST_FIXTURE_TEST_CASE(tx_add_free,CTxTest) {
	//invalid data
	CFund fund(NULL_FUNDTYPE, g_vHash[0].at(0), 1, CHAIN_HEIGHT + 1);
	BOOST_CHECK(!accOperate.OperateAccount(ADD_FREE, fund));
	fund.nFundType = FREEDOM_FUND;
	fund.value = MAX_MONEY;
	BOOST_CHECK(!accOperate.OperateAccount(ADD_FREE, fund));

	vector<uint256> vReward;
	vector<uint256> vFree;
	GenerateRandomHash(vReward);
	GenerateRandomHash(vFree);
	accOperate.CompactAccount(10);
	for (int i = 0; i < VECTOR_SIZE; i++) {
		uint64_t nOld = GetTotalValue(accOperate.vRewardFund);
		uint64_t randValue = random(10);
		CFund fundReward(REWARD_FUND, vReward.at(i), randValue, CHAIN_HEIGHT + 1);
		BOOST_CHECK(accOperate.OperateAccount(ADD_FREE, fundReward));
		BOOST_CHECK(GetTotalValue(accOperate.vRewardFund) == nOld + randValue);

		nOld = GetTotalValue(accOperate.vFreedomFund);
		CFund fundFree(FREEDOM_FUND, vFree.at(i), randValue, CHAIN_HEIGHT + 1);
		BOOST_CHECK(accOperate.OperateAccount(ADD_FREE, fundFree));
		BOOST_CHECK(GetTotalValue(accOperate.vFreedomFund) == nOld + randValue);
	}

	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();
}

BOOST_FIXTURE_TEST_CASE(tx_minus_free,CTxTest) {
	accOperate.CompactAccount(CHAIN_HEIGHT);

	int nBranch[3] = {0};
	for (int i = 0; i < VECTOR_SIZE; i++) {
		uint64_t nOldVectorSum = GetTotalValue(accOperate.vFreedomFund);
		uint64_t nOldValue = accOperate.llValues;
		uint64_t randValue = random(30);
		uint64_t nOperateValue = 0;
		CFund fund(REWARD_FUND, 0, randValue, random(20));

		if (nOldVectorSum && nOldVectorSum >= randValue) {
			nOperateValue = 0;
			BOOST_CHECK(accOperate.OperateAccount(MINUS_FREE, fund, &nOperateValue));
			BOOST_CHECK(GetTotalValue(accOperate.vFreedomFund) == nOldVectorSum - randValue);
			BOOST_CHECK(nOperateValue == randValue);
			nBranch[0]++;
		} else {
			if (accOperate.llValues + nOldVectorSum >= randValue) {
				nOperateValue = 0;
				BOOST_CHECK(accOperate.OperateAccount(MINUS_FREE, fund, &nOperateValue));
				BOOST_CHECK(
						GetTotalValue(accOperate.vFreedomFund) + accOperate.llValues
								== nOldValue + nOldVectorSum - randValue);
				BOOST_CHECK(nOperateValue == randValue);
				nBranch[1]++;
			} else {
				nOperateValue = 0;
				BOOST_CHECK(!accOperate.OperateAccount(MINUS_FREE, fund, &nOperateValue) );
				BOOST_CHECK(GetTotalValue(accOperate.vFreedomFund) == nOldVectorSum);
				BOOST_CHECK(nOldValue == accOperate.llValues);
				BOOST_CHECK(nOperateValue == 0);
				nBranch[2]++;
			}
		}
	}

	cout<<"branch 1 is "<<nBranch[0]<<endl<<" branch 2 is "<<nBranch[1]<<endl<<" branch 3 is "<<nBranch[2]<<endl;
	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();
}

BOOST_FIXTURE_TEST_CASE(tx_add_self,CTxTest) {
	vector<uint256> vSelf;
	GenerateRandomHash(vSelf);
	accOperate.CompactAccount(CHAIN_HEIGHT);

	//invalid param check
	CFund fundvalid(SELF_FREEZD_FUND, vSelf.at(0), 1, CHAIN_HEIGHT + 1);
	CFund fundInvalid1(fundvalid);
	fundInvalid1.nFundType = 0;
	BOOST_CHECK(!accOperate.OperateAccount(ADD_SELF_FREEZD, fundInvalid1));
	fundInvalid1.nFundType = NULL_OPERTYPE;
	BOOST_CHECK(!accOperate.OperateAccount(ADD_SELF_FREEZD, fundInvalid1));

	CFund fundInvalid2(fundvalid);
	fundInvalid2.uTxHash = uint256(0);
	BOOST_CHECK(!accOperate.OperateAccount(ADD_SELF_FREEZD, fundInvalid2));

	//random check
	for (int i = 0; i < VECTOR_SIZE; i++) {
		uint64_t nOldInput = GetTotalValue(accOperate.vSelfFreeze);
		uint64_t randValue = random(10);
		CFund fund(SELF_FREEZD_FUND, vSelf.at(i), randValue, CHAIN_HEIGHT + 1);
		BOOST_CHECK(accOperate.OperateAccount(ADD_SELF_FREEZD, fund));
		BOOST_CHECK(GetTotalValue(accOperate.vSelfFreeze) == nOldInput + randValue);
	}

	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();
}

BOOST_FIXTURE_TEST_CASE(tx_minus_self,CTxTest) {
	accOperate.CompactAccount(CHAIN_HEIGHT);
	int nBranch[2] = {0};
	for (int i = 0; i < VECTOR_SIZE; i++) {
		uint64_t nOldVectorSum = GetTotalValue(accOperate.vSelfFreeze);
		uint64_t randValue = random(30);
		uint64_t nOperateValue = 0;
		CFund fund(REWARD_FUND, 0, randValue, random(20));

		if (nOldVectorSum >= randValue) {
			nOperateValue = 0;
			BOOST_CHECK(accOperate.OperateAccount(MINUS_SELF_FREEZD, fund, &nOperateValue));
			BOOST_CHECK(GetTotalValue(accOperate.vSelfFreeze) == nOldVectorSum - randValue);
			BOOST_CHECK(nOperateValue == randValue);
			nBranch[0]++;
		} else {
			nOperateValue = 0;
			BOOST_CHECK(!accOperate.OperateAccount(MINUS_SELF_FREEZD, fund, &nOperateValue));
			BOOST_CHECK(GetTotalValue(accOperate.vSelfFreeze) == nOldVectorSum);
			BOOST_CHECK(nOperateValue == 0);
			nBranch[1]++;
		}
	}

	cout<<"branch 1 is "<<nBranch[0]<<endl<<" branch 2 is "<<nBranch[1]<<endl;
	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();
}

BOOST_FIXTURE_TEST_CASE(tx_add_freezed,CTxTest) {
	vector<uint256> vSelf;
	GenerateRandomHash(vSelf);
	accOperate.CompactAccount(CHAIN_HEIGHT);

	//invalid param check
	CFund fundvalid(FREEDOM_FUND, vSelf.at(0), 1, CHAIN_HEIGHT + 1);
	CFund fundInvalid1(fundvalid);
	fundInvalid1.nFundType = 0;
	BOOST_CHECK(!accOperate.OperateAccount(ADD_FREEZD, fundInvalid1));
	fundInvalid1.nFundType = NULL_FUNDTYPE;
	BOOST_CHECK(!accOperate.OperateAccount(ADD_FREEZD, fundInvalid1));

	CFund fundInvalid2(fundvalid);
	fundInvalid2.uTxHash = uint256(0);
	BOOST_CHECK(!accOperate.OperateAccount(ADD_FREEZD, fundInvalid2));

	//random check
	for (int i = 0; i < VECTOR_SIZE; i++) {
		uint64_t nOldInput = GetTotalValue(accOperate.vFreeze);
		uint64_t randValue = random(10);
		CFund fund(FREEZD_FUND, vSelf.at(i), randValue, CHAIN_HEIGHT + 1);
		BOOST_CHECK(accOperate.OperateAccount(ADD_FREEZD, fund));
		BOOST_CHECK(GetTotalValue(accOperate.vFreeze) == nOldInput + randValue);
	}

	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();
}

BOOST_FIXTURE_TEST_CASE(tx_minus_freezed,CTxTest) {
	accOperate.CompactAccount(CHAIN_HEIGHT);
	vector<uint256> vFreezedHashCopy;
	for(vector<CFund>::iterator it = accOperate.vFreeze.begin();it !=accOperate.vFreeze.end();it++)
	{
		vFreezedHashCopy.push_back(it->uTxHash);
	}

	//insert 100 data that not in the specific vector
	vector<uint256> vRandomHash;
	GenerateRandomHash(vRandomHash,100);
	vFreezedHashCopy.insert(vFreezedHashCopy.end(),vRandomHash.begin(),vRandomHash.end() );
	random_shuffle(vFreezedHashCopy.begin(),vFreezedHashCopy.end() );

	//start test random data
	uint64_t nOperaeValue = 0;
	int nBranch[4] = {0};
	for (vector<uint256>::iterator it = vFreezedHashCopy.begin();it !=vFreezedHashCopy.end();it++)
	{
		nOperaeValue = 0;
		uint64_t nRandomValue = random(10)+1;
		int nHeight = CHAIN_HEIGHT - 1 + random(3);
		CFund fund(FREEZD_FUND, *it, nRandomValue, nHeight);
		vector<CFund>::iterator iter = find_if(accOperate.vFreeze.begin(),accOperate.vFreeze.end(),
				CFindIterByHash(*it) );

		if(accOperate.vFreeze.end() == iter){
			//can't find the specific fund by *it,so operatreAccount should return false
			BOOST_CHECK(!accOperate.OperateAccount(MINUS_FREEZD, fund,&nOperaeValue) );
			nBranch[0]++;
			continue;
		}

		if (iter->nHeight != nHeight) {
			BOOST_CHECK(!accOperate.OperateAccount(MINUS_FREEZD, fund,&nOperaeValue) );
			nBranch[1]++;
			continue;
		}

		uint64_t nOldFreezeValue = GetTotalValue(accOperate.vFreeze);
		uint64_t nOldIterValue = iter->value;
		if(nRandomValue>iter->value)
		{
			BOOST_CHECK(!accOperate.OperateAccount(MINUS_FREEZD, fund,&nOperaeValue) );
			BOOST_CHECK(GetTotalValue(accOperate.vFreeze) == nOldFreezeValue);
			BOOST_CHECK(0 == nOperaeValue);
			nBranch[2]++;
		}
		else
		{
			BOOST_CHECK(accOperate.OperateAccount(MINUS_FREEZD, fund,&nOperaeValue) );
			BOOST_CHECK(GetTotalValue(accOperate.vFreeze) + nRandomValue == nOldFreezeValue);
			BOOST_CHECK(nRandomValue == nOperaeValue);
			nBranch[3]++;
		}
	}

	cout<<"branch 1 is "<<nBranch[0]<<endl<<" branch 2 is "<<nBranch[1]<<endl
			<<" branch 3 is "<<nBranch[2]<<endl<<" branch 4 is "<<nBranch[3]<<endl;
	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();

}

BOOST_AUTO_TEST_SUITE_END()
