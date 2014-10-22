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
vector<uint256> g_vHash[5];
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

	for (int i = 0; i < 5; i++) {
		GenerateRandomHash(g_vHash[i]);
	}

	for (int i = 0; i < VECTOR_SIZE; i++) {
		account.vRewardFund.push_back(CFund(REWARD_FUND, g_vHash[0].at(i), random(10), random(20)));
	}

	for (int i = 0; i < VECTOR_SIZE; i++) {
		account.vFreedomFund.push_back(CFund(FREEDOM_FUND, g_vHash[1].at(i), random(5), random(20)));
	}

	for (int i = 0; i < VECTOR_SIZE; i++) {
		account.vInputFreeze.push_back(
				CFund(INPUT_FREEZD_FUND, g_vHash[2].at(i), random(10), CHAIN_HEIGHT - 10 + random(30)));
	}

	for (int i = 0; i < VECTOR_SIZE; i++) {
		account.vOutputFreeze.push_back(
				CFund(OUTPUT_FREEZD_FUND, g_vHash[3].at(i), random(10), CHAIN_HEIGHT - 10 + random(30)));
	}

	for (int i = 0; i < VECTOR_SIZE; i++) {
		account.vSelfFreeze.push_back(
				CFund(SELF_FREEZD_FUND, g_vHash[4].at(i), random(10), CHAIN_HEIGHT - 10 + random(30)));
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
		BOOST_CHECK(IsEqual(accBeforOperate.vInputFreeze, accOperate.vInputFreeze));
		BOOST_CHECK(IsEqual(accBeforOperate.vOutputFreeze, accOperate.vOutputFreeze));
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
		GenerateRandomHash(vRandomHash, VECTOR_SIZE / 10);
		vHashCopy.insert(vHashCopy.end(), vRandomHash.begin(), vRandomHash.end());
		random_shuffle(vHashCopy.begin(), vHashCopy.end());
	}

	void CheckInOut(OperType emType) {
		BOOST_CHECK(emType == MINUS_OUTPUT || emType == MINUS_INPUT);
		accOperate.CompactAccount(CHAIN_HEIGHT);

		vector<CFund>& vFund = emType == MINUS_OUTPUT ? accOperate.vOutputFreeze : accOperate.vInputFreeze;
		vector<uint256> vHashCopy;
		FillSpecificAndRandData(vHashCopy, vFund);

		//start test random data
		int nExecBranchCount[3] = { 0 };
		uint64_t nOperaeValue = 0;
		int nCount = 0;
		for (vector<uint256>::iterator it = vHashCopy.begin(); it != vHashCopy.end(); it++) {
			nOperaeValue = 0;
			uint64_t nRandomValue = random(10);
			CFund fund(OUTPUT_FREEZD_FUND, *it, nRandomValue, random(20));
			vector<CFund>::iterator iter = find_if(vFund.begin(), vFund.end(), CFindIterByHash(*it));

			if (vFund.end() == iter) {
				//can't find the specific fund by *it,so operatreAccount should return false
				BOOST_CHECK(!accOperate.OperateAccount(emType, fund, &nOperaeValue));
				nExecBranchCount[0]++;
				continue;
			}

			uint64_t nOldVectorValue = GetTotalValue(vFund);
			uint64_t nOldIterValue = iter->value;
			if (nRandomValue > iter->value) {
				BOOST_CHECK(!accOperate.OperateAccount(emType, fund, &nOperaeValue));
				BOOST_CHECK(GetTotalValue(vFund) + nOldIterValue == nOldVectorValue);
				BOOST_CHECK(nOldIterValue == nOperaeValue);
				nExecBranchCount[1]++;
			} else {
				BOOST_CHECK(accOperate.OperateAccount(emType, fund, &nOperaeValue));
				BOOST_CHECK(GetTotalValue(vFund) + nRandomValue == nOldVectorValue);
				BOOST_CHECK(nRandomValue == nOperaeValue);
				nExecBranchCount[2]++;
			}
		}

		for (int i = 0; i < sizeof(nExecBranchCount) / sizeof(nExecBranchCount[0]); i++) {
			//cout << "execute branch " << i << " times is " << nExecBranchCount[i] << endl;
		}
		accOperate.UndoOperateAccount(accOperate.accountOperLog);
		CheckAccountEqual();
	}

	void CheckInOutFree(OperType emType) {
		BOOST_CHECK(emType == MINUS_OUTPUT_OR_FREE || emType == MINUS_INPUT_OR_FREE);
		accOperate.CompactAccount(CHAIN_HEIGHT);

		vector<CFund>& vFund = emType == MINUS_OUTPUT_OR_FREE ? accOperate.vOutputFreeze : accOperate.vInputFreeze;
		vector<uint256> vHashCopy;
		FillSpecificAndRandData(vHashCopy, vFund);

		int nExecBranchCount[4] = { 0 };
		uint64_t nOperaeValue = 0;
		int nCount = 0;
		for (vector<uint256>::iterator it = vHashCopy.begin(); it != vHashCopy.end(); it++) {
			nOperaeValue = 0;
			uint64_t nRandomValue = random(200);
			CFund fund(OUTPUT_FREEZD_FUND, *it, nRandomValue, random(20));
			vector<CFund>::iterator iter = find_if(vFund.begin(), vFund.end(), CFindIterByHash(*it));

			if (vFund.end() == iter) {
				//can't find the specific fund by *it,so operatreAccount should return false
				BOOST_CHECK(!accOperate.OperateAccount(emType, fund, &nOperaeValue));
				nExecBranchCount[0]++;
				continue;
			}

			uint64_t nOldVectorValue = GetTotalValue(vFund);
			uint64_t nOldIterValue = iter->value;
			if (iter->value >= nRandomValue) {
				BOOST_CHECK(accOperate.OperateAccount(emType, fund, &nOperaeValue));
				BOOST_CHECK(GetTotalValue(vFund) + nRandomValue == nOldVectorValue);
				BOOST_CHECK(nRandomValue == nOperaeValue);
				nExecBranchCount[1]++;
			} else {
				uint64_t nOldFree = GetTotalValue(accOperate.vFreedomFund) + accOperate.llValues;
				if (iter->value + nOldFree >= nRandomValue) {
					BOOST_CHECK(accOperate.OperateAccount(emType, fund, &nOperaeValue));
					BOOST_CHECK(
							nOldVectorValue + nOldFree - nRandomValue
									== GetTotalValue(accOperate.vFreedomFund) + accOperate.llValues
											+ GetTotalValue(vFund));
					BOOST_CHECK(nRandomValue == nOperaeValue);
					nExecBranchCount[2]++;
				} else {
					BOOST_CHECK(!accOperate.OperateAccount(emType, fund, &nOperaeValue));
					BOOST_CHECK(!GetTotalValue(accOperate.vFreedomFund) && !accOperate.llValues);
					BOOST_CHECK(nOldVectorValue + nOldFree - nOperaeValue == GetTotalValue(vFund));
					nExecBranchCount[3]++;
				}
			}
		}

		for (int i = 0; i < sizeof(nExecBranchCount) / sizeof(nExecBranchCount[0]); i++) {
			//cout << "execute branch " << i << " times is " << nExecBranchCount[i] << endl;
		}
		accOperate.UndoOperateAccount(accOperate.accountOperLog);
		CheckAccountEqual();
	}

	void CheckInOutFreeSelf(OperType emType) {
		BOOST_CHECK(emType == MINUS_OUTPUT_OR_FREE_OR_SELF || emType == MINUS_INPUT_OR_FREE_OR_SELF);
		accOperate.CompactAccount(CHAIN_HEIGHT);

		vector<CFund>& vFund =
				emType == MINUS_OUTPUT_OR_FREE_OR_SELF ? accOperate.vOutputFreeze : accOperate.vInputFreeze;
		vector<uint256> vHashCopy;
		FillSpecificAndRandData(vHashCopy, vFund);

		int nExecBranchCount[5] = { 0 };
		uint64_t nOperaeValue = 0;
		int nCount = 0;
		for (vector<uint256>::iterator it = vHashCopy.begin(); it != vHashCopy.end(); it++) {
			nOperaeValue = 0;
			uint64_t nRandomValue = random(70);
			CFund fund(OUTPUT_FREEZD_FUND, *it, nRandomValue, random(20));
			vector<CFund>::iterator iter = find_if(vFund.begin(), vFund.end(), CFindIterByHash(*it));

			if (vFund.end() == iter) {
				//can't find the specific fund by *it,so operatreAccount should return false
				BOOST_CHECK(!accOperate.OperateAccount(emType, fund, &nOperaeValue));
				nExecBranchCount[0]++;
				continue;
			}

			uint64_t nOldVectorValue = GetTotalValue(vFund);
			uint64_t nIterValue = iter->value;
			if (nIterValue >= nRandomValue) {
				BOOST_CHECK(accOperate.OperateAccount(emType, fund, &nOperaeValue));
				BOOST_CHECK(GetTotalValue(vFund) + nRandomValue == nOldVectorValue);
				BOOST_CHECK(nRandomValue == nOperaeValue);
				nExecBranchCount[1]++;
			} else {
				uint64_t nOldTotalFree = GetTotalValue(accOperate.vFreedomFund) + accOperate.llValues;
				if (nIterValue + nOldTotalFree >= nRandomValue) {
					BOOST_CHECK(accOperate.OperateAccount(emType, fund, &nOperaeValue));
					BOOST_CHECK(
							nOldVectorValue + nOldTotalFree - nRandomValue
									== GetTotalValue(accOperate.vFreedomFund) + accOperate.llValues
											+ GetTotalValue(vFund));
					BOOST_CHECK(nRandomValue == nOperaeValue);
					nExecBranchCount[2]++;
				} else {
					uint64_t nOldSelfValue = GetTotalValue(accOperate.vSelfFreeze);
					if (nIterValue + nOldTotalFree + nOldSelfValue >= nRandomValue) {
						BOOST_CHECK(accOperate.OperateAccount(emType, fund, &nOperaeValue));
						uint64_t nSum = GetTotalValue(vFund) + accOperate.llValues
								+ GetTotalValue(accOperate.vFreedomFund) + GetTotalValue(accOperate.vSelfFreeze);
						BOOST_CHECK(nOldVectorValue + nOldTotalFree + nOldSelfValue - nRandomValue == nSum);
						BOOST_CHECK(nOperaeValue == nRandomValue);
						nExecBranchCount[3]++;
					} else {
						BOOST_CHECK(!accOperate.OperateAccount(emType, fund, &nOperaeValue));
						BOOST_CHECK(!GetTotalValue(accOperate.vFreedomFund) && !accOperate.llValues);
						BOOST_CHECK(!GetTotalValue(accOperate.vSelfFreeze));

						BOOST_CHECK(
								nOldVectorValue + nOldTotalFree + nOldSelfValue - nOperaeValue == GetTotalValue(vFund));
						nExecBranchCount[4]++;
					}
				}
			}
		}

		for (int i = 0; i < sizeof(nExecBranchCount) / sizeof(nExecBranchCount[0]); i++) {
			//cout<<"execute branch "<<i<<" times is "<<nExecBranchCount[i]<<endl;
		}

		accOperate.UndoOperateAccount(accOperate.accountOperLog);
		CheckAccountEqual();
	}

	void CheckFreeSelf() {

	}
};

BOOST_FIXTURE_TEST_SUITE(tx_tests,CTxTest)

#if 0

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
		} else {
			if (accOperate.llValues + nOldVectorSum >= randValue) {
				nOperateValue = 0;
				BOOST_CHECK(accOperate.OperateAccount(MINUS_FREE, fund, &nOperateValue));
				BOOST_CHECK(
						GetTotalValue(accOperate.vFreedomFund) + accOperate.llValues
						== nOldValue + nOldVectorSum - randValue);
				BOOST_CHECK(nOperateValue == randValue);
			} else {
				nOperateValue = 0;
				accOperate.OperateAccount(MINUS_FREE, fund, &nOperateValue);
				BOOST_CHECK(!GetTotalValue(accOperate.vFreedomFund) && !accOperate.llValues);
				BOOST_CHECK(nOperateValue == nOldValue + nOldVectorSum);
			}
		}
	}

	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();
}

BOOST_FIXTURE_TEST_CASE(tx_add_self,CTxTest) {
	//invalid data
	CFund fundInvalid(NULL_FUNDTYPE, g_vHash[0].at(0), 1, CHAIN_HEIGHT + 1);
	BOOST_CHECK(!accOperate.OperateAccount(ADD_SELF_FREEZD, fundInvalid));
	fundInvalid.nFundType = 0;
	BOOST_CHECK(!accOperate.OperateAccount(ADD_SELF_FREEZD, fundInvalid));
	fundInvalid.nFundType = FREEDOM_FUND;
	fundInvalid.value = MAX_MONEY;
	BOOST_CHECK(!accOperate.OperateAccount(ADD_SELF_FREEZD, fundInvalid));

	//random test
	vector<uint256> vSelf;
	GenerateRandomHash(vSelf);

	accOperate.CompactAccount(CHAIN_HEIGHT);
	for (int i = 0; i < VECTOR_SIZE; i++) {
		uint64_t nOldVectorSum = GetTotalValue(accOperate.vFreedomFund);
		uint64_t nOldValue = accOperate.llValues;
		uint64_t randValue = random(30);
		uint64_t nOperateValue = 0;
		uint64_t nOldSelfValue = GetTotalValue(accOperate.vSelfFreeze);
		CFund fund(SELF_FREEZD_FUND, vSelf.at(i), randValue, CHAIN_HEIGHT + 1);

		if (nOldVectorSum && nOldVectorSum >= randValue) {
			nOperateValue = 0;
			BOOST_CHECK(accOperate.OperateAccount(ADD_SELF_FREEZD, fund, &nOperateValue));
			BOOST_CHECK(GetTotalValue(accOperate.vFreedomFund) == nOldVectorSum - randValue);
			BOOST_CHECK(nOperateValue == randValue);

			BOOST_CHECK(nOldSelfValue + randValue == GetTotalValue(accOperate.vSelfFreeze));
		} else {
			if (accOperate.llValues + nOldVectorSum >= randValue) {
				nOperateValue = 0;
				BOOST_CHECK(accOperate.OperateAccount(ADD_SELF_FREEZD, fund, &nOperateValue));
				BOOST_CHECK(
						GetTotalValue(accOperate.vFreedomFund) + accOperate.llValues
						== nOldValue + nOldVectorSum - randValue);
				BOOST_CHECK(nOperateValue == randValue);

				BOOST_CHECK(nOldSelfValue + randValue == GetTotalValue(accOperate.vSelfFreeze));
			} else {
				nOperateValue = 0;
				accOperate.OperateAccount(ADD_SELF_FREEZD, fund, &nOperateValue);
				BOOST_CHECK(!GetTotalValue(accOperate.vFreedomFund) && !accOperate.llValues);
				BOOST_CHECK(nOperateValue == nOldValue + nOldVectorSum);
			}
		}
	}

	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();
}

BOOST_FIXTURE_TEST_CASE(tx_add_input,CTxTest) {
	vector<uint256> vSelf;
	GenerateRandomHash(vSelf);
	accOperate.CompactAccount(CHAIN_HEIGHT);

	//invalid param check
	CFund fundvalid(INPUT_FREEZD_FUND, vSelf.at(0), 1, CHAIN_HEIGHT + 1);
	CFund fundInvalid1(fundvalid);
	fundInvalid1.nFundType = 0;
	BOOST_CHECK(!accOperate.OperateAccount(ADD_INPUT_FREEZD, fundInvalid1));
	fundInvalid1.nFundType = NULL_OPERTYPE;
	BOOST_CHECK(!accOperate.OperateAccount(ADD_INPUT_FREEZD, fundInvalid1));

	CFund fundInvalid2(fundvalid);
	fundInvalid2.uTxHash = uint256(0);
	BOOST_CHECK(!accOperate.OperateAccount(ADD_INPUT_FREEZD, fundInvalid2));

//	CFund fundInvalid3(fundvalid);
//	fundInvalid3.nHeight = 0;
//	BOOST_CHECK(!accOperate.OperateAccount(ADD_INPUT_FREEZD, fundInvalid3));

//random check
	for (int i = 0; i < VECTOR_SIZE; i++) {
		uint64_t nOldInput = GetTotalValue(accOperate.vInputFreeze);
		uint64_t randValue = random(10);
		CFund fund(INPUT_FREEZD_FUND, vSelf.at(i), randValue, CHAIN_HEIGHT + 1);
		BOOST_CHECK(accOperate.OperateAccount(ADD_INPUT_FREEZD, fund));
		BOOST_CHECK(GetTotalValue(accOperate.vInputFreeze) == nOldInput + randValue);
	}

	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();
}

BOOST_FIXTURE_TEST_CASE(tx_minus_free_to_output,CTxTest) {
	accOperate.CompactAccount(CHAIN_HEIGHT);
	vector<CFund> vFreeCopy(accOperate.vFreedomFund);

	for (vector<CFund>::iterator it = vFreeCopy.begin(); it != vFreeCopy.end(); it++) {
		size_t nOldFreeSize = accOperate.vFreedomFund.size();
		uint64_t nOldFreeValue = GetTotalValue(accOperate.vFreedomFund);
		size_t nOldOutputSize = accOperate.vOutputFreeze.size();
		uint64_t nOldOutputValue = GetTotalValue(accOperate.vOutputFreeze);
		uint64_t nChangeValue = it->value;

		BOOST_CHECK(accOperate.OperateAccount(MINUS_FREE_TO_OUTPUT, *it));

		BOOST_CHECK(nOldFreeValue - nChangeValue == GetTotalValue(accOperate.vFreedomFund));
		BOOST_CHECK(nOldOutputValue + nChangeValue == GetTotalValue(accOperate.vOutputFreeze));

		BOOST_CHECK(
				((nOldFreeSize == accOperate.vFreedomFund.size() + 1)
						&& nOldOutputSize + 1 == accOperate.vOutputFreeze.size())
				|| (nOldFreeSize == accOperate.vFreedomFund.size()
						&& nOldOutputSize == accOperate.vOutputFreeze.size()));
	}

	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();
}

BOOST_FIXTURE_TEST_CASE(tx_minus_free_or_self,CTxTest) {
	accOperate.CompactAccount(CHAIN_HEIGHT);
	for (int i = 0; i < VECTOR_SIZE; i++) {
		uint64_t nOldTotalFree = GetTotalValue(accOperate.vFreedomFund)+accOperate.llValues;
		uint64_t nOldSelf = GetTotalValue(accOperate.vSelfFreeze);
		uint64_t randValue = random(30);
		uint64_t nOperateValue = 0;
		CFund fund(REWARD_FUND, 0, randValue, random(20));

		if (nOldTotalFree >= randValue) {
			nOperateValue = 0;
			BOOST_CHECK(accOperate.OperateAccount(MINUS_FREE_OR_SELF, fund, &nOperateValue));
			BOOST_CHECK(GetTotalValue(accOperate.vFreedomFund)+accOperate.llValues == nOldTotalFree - randValue);
			BOOST_CHECK(nOperateValue == randValue);
		} else {
			if (nOldTotalFree + nOldSelf >= randValue) {
				nOperateValue = 0;
				BOOST_CHECK(accOperate.OperateAccount(MINUS_FREE_OR_SELF, fund, &nOperateValue));
				BOOST_CHECK(GetTotalValue(accOperate.vFreedomFund) + accOperate.llValues+
						GetTotalValue(accOperate.vSelfFreeze)== nOldTotalFree + nOldSelf - randValue);
				BOOST_CHECK(nOperateValue == randValue);
			} else {
				nOperateValue = 0;
				accOperate.OperateAccount(MINUS_FREE_OR_SELF, fund, &nOperateValue);
				BOOST_CHECK(!GetTotalValue(accOperate.vFreedomFund) && !accOperate.llValues);
				BOOST_CHECK(nOperateValue == nOldSelf + nOldTotalFree);
			}
		}
	}

	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();
}

BOOST_FIXTURE_TEST_CASE(tx_minus_output,CTxTest) {
//	accOperate.CompactAccount(CHAIN_HEIGHT);
//	vector<uint256> vOutputHashCopy;
//	for(vector<CFund>::iterator it = accOperate.vOutputFreeze.begin();it !=accOperate.vOutputFreeze.end();it++)
//	{
//		vOutputHashCopy.push_back(it->uTxHash);
//	}
//
//	//insert 100 data that not in the specific vector
//	vector<uint256> vRandomHash;
//	GenerateRandomHash(vRandomHash,100);
//	vOutputHashCopy.insert(vOutputHashCopy.end(),vRandomHash.begin(),vRandomHash.end() );
//	random_shuffle(vOutputHashCopy.begin(),vOutputHashCopy.end() );
//
//	//start test random data
//	uint64_t nOperaeValue = 0;
//	int nCount = 0;
//	for (vector<uint256>::iterator it = vOutputHashCopy.begin();it !=vOutputHashCopy.end();it++)
//	{
//		nOperaeValue = 0;
//		uint64_t nRandomValue = random(10);
//		CFund fund(OUTPUT_FREEZD_FUND, *it, nRandomValue, random(20));
//		vector<CFund>::iterator iter = find_if(accOperate.vOutputFreeze.begin(),accOperate.vOutputFreeze.end(),
//				CFindIterByHash(*it) );
//
//		if(accOperate.vOutputFreeze.end() == iter){
//			//can't find the specific fund by *it,so operatreAccount should return false
//			BOOST_CHECK(!accOperate.OperateAccount(MINUS_OUTPUT, fund,&nOperaeValue) );
//			continue;
//		}
//
//		uint64_t nOldOutputValue = GetTotalValue(accOperate.vOutputFreeze);
//		uint64_t nOldIterValue = iter->value;
//		if(nRandomValue>iter->value)
//		{
//			BOOST_CHECK(!accOperate.OperateAccount(MINUS_OUTPUT, fund,&nOperaeValue) );
//			BOOST_CHECK(GetTotalValue(accOperate.vOutputFreeze) + nOldIterValue == nOldOutputValue);
//			BOOST_CHECK(nOldIterValue == nOperaeValue);
//		}
//		else
//		{
//			BOOST_CHECK(accOperate.OperateAccount(MINUS_OUTPUT, fund,&nOperaeValue) );
//			BOOST_CHECK(GetTotalValue(accOperate.vOutputFreeze) + nRandomValue == nOldOutputValue);
//			BOOST_CHECK(nRandomValue == nOperaeValue);
//		}
//	}
//
//	accOperate.UndoOperateAccount(accOperate.accountOperLog);
//	CheckAccountEqual();

	CheckInOut(MINUS_OUTPUT);
}

BOOST_FIXTURE_TEST_CASE(tx_minus_input,CTxTest) {
	CheckInOut(MINUS_INPUT);
}

BOOST_FIXTURE_TEST_CASE(tx_minus_output_free,CTxTest) {
	CheckInOutFree(MINUS_OUTPUT_OR_FREE);
}

BOOST_FIXTURE_TEST_CASE(tx_minus_input_free,CTxTest) {
	CheckInOutFree(MINUS_INPUT_OR_FREE);
}

BOOST_FIXTURE_TEST_CASE(tx_minus_input_free_self,CTxTest) {
	CheckInOutFreeSelf(MINUS_INPUT_OR_FREE_OR_SELF);
}

BOOST_FIXTURE_TEST_CASE(tx_minus_output_free_self,CTxTest) {
	CheckInOutFreeSelf(MINUS_OUTPUT_OR_FREE_OR_SELF);
}

#else
BOOST_AUTO_TEST_CASE(xxxx) {
	BOOST_ERROR("ERROR:THE SUITE NEED TO MODIFY!");
}
#endif

BOOST_AUTO_TEST_SUITE_END()
