#include <iostream>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include "serialize.h"
#include "txdb.h"
#include "account.h"
#include "main.h"
using namespace std;
#define DAY_BLOCKS	((24 * 60 * 60)/(10*60))
#define MONTH_BLOCKS (30*DAY_BLOCKS)
#define CHAIN_HEIGHT (10*MONTH_BLOCKS+10)
#define TEST_SIZE 10000
#define MAX_FUND_MONEY 15
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
	for (int i = 1; i <= nSize; i++) {
		CKey key;
		key.MakeNewKey();
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
//	CBlockIndex block;
	CAccount accOperate;
	CAccount accBeforOperate;
	CAccount accYesterDayLastOper;
	vector<unsigned char> scriptID;
	vector_unsigned_char v[11]; //0~9 is valid,10 is used to for invalid scriptID


	CTxTest() {
		Init();
		accOperate.keyID = uint160(1);
		accBeforOperate = accOperate;

	}
	~CTxTest(){
		delete pScriptDB;
		pScriptDB = NULL;
		delete pScriptDBTip;
		pScriptDBTip = NULL;
	}

	void InitAuthorization(const string& str) {
		scriptID = ParseHex(str.c_str());
		CAuthorizate author;
		author.SetAuthorizeTime(CHAIN_HEIGHT + 100);
		author.SetMaxMoneyPerDay(800);
		author.SetCurMaxMoneyPerDay(0);
		author.SetLastOperHeight(1);
		author.SetMaxMoneyPerTime(37);
		author.SetMaxMoneyTotal(10 * TEST_SIZE);
		accOperate.mapAuthorizate[scriptID] = author;
		accOperate.keyID = uint160(1);

		if(pScriptDBTip !=NULL) {
			pScriptDBTip->Flush();
			delete pScriptDBTip;
			pScriptDBTip = NULL;
		}

		if(pScriptDB != NULL) {
			delete pScriptDB;
			pScriptDB = NULL;
		}
		pScriptDB = new CScriptDB(1024 * 1024, false, false);
		pScriptDBTip = new CScriptDBViewCache(*pScriptDB, false);
		vector<unsigned char> vScriptContent = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
				0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };
		CRegID regScriptId(scriptID);
		BOOST_CHECK(pScriptDBTip->SetScript(regScriptId, vScriptContent));
	}

	void InitFund() {
		char buf[12];
		for (int j = 100000, i = 0; j < 100011; j++, i++) {
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "%012d", j);
			v[i] = ParseHex(buf);
		}

		for (int i = 0; i < TEST_SIZE/100; i++) {
			accOperate.vRewardFund.push_back(CFund(REWARD_FUND, RANDOM_FUND_MONEY, random(5)));
		}

		for (int i = 0; i < TEST_SIZE; i++) {
			int nFundHeight = CHAIN_HEIGHT - MONTH_BLOCKS;
			accOperate.AddToFreedom(CFund(FREEDOM_FUND, RANDOM_FUND_MONEY, nFundHeight+random(MONTH_BLOCKS)), false);
		}

		for (int i = 0; i < TEST_SIZE; i++) {
			accOperate.AddToFreeze(CFund(FREEZD_FUND, RANDOM_FUND_MONEY, random(15), v[random(10)]), false);
		}

		for (int i = 0; i < TEST_SIZE; i++) {
			accOperate.AddToSelfFreeze(CFund(SELF_FREEZD_FUND, RANDOM_FUND_MONEY*2, random(20)) ,false);
		}

		vector<unsigned char> vScriptContent1 = {0x01,0x02,0x03,0x04,0x01,0x01,0x01,0x01,0x01,
				0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};

		CRegID regId;
		for(auto &item:v) {
			regId.SetRegID(item);
			BOOST_CHECK(pScriptDBTip->SetScript(regId, vScriptContent1));
		}
	}

	void Init() {
		srand((unsigned) time(NULL));
		accOperate.llValues = TEST_SIZE*5;
//		block.nHeight = CHAIN_HEIGHT;
//		chainActive.SetTip(&block);
//		BOOST_CHECK(NULL != chainActive.Tip());

		nRunTimeHeight = 0;

		InitAuthorization("001122334455");

		InitFund();
	}

	void IsAuthorityEqual(const vector_unsigned_char& scriptID) {
		auto itBefore = accBeforOperate.mapAuthorizate.find(scriptID);
		BOOST_CHECK(accBeforOperate.mapAuthorizate.end() != itBefore);

		auto it = accOperate.mapAuthorizate.find(scriptID);
		BOOST_CHECK(accBeforOperate.mapAuthorizate.end() != it);

		CAuthorizate& authorizate = it->second;
		CAuthorizate& authorizateBefor = itBefore->second;
		BOOST_CHECK(authorizate.GetCurMaxMoneyPerDay() == authorizateBefor.GetCurMaxMoneyPerDay() &&
				authorizate.GetMaxMoneyTotal() == authorizateBefor.GetMaxMoneyTotal() &&
				authorizate.GetLastOperHeight() == authorizateBefor.GetLastOperHeight() );
		//cout<<"old oper height: "<<authorizateBefor.GetLastOperHeight()<<" new oper height: "<<authorizate.GetLastOperHeight()<<endl;
	}

	void CheckAccountEqual(bool bCheckAuthority = true) {
		BOOST_CHECK(IsEqual(accBeforOperate.vRewardFund, accOperate.vRewardFund));
		BOOST_CHECK(IsEqual(accBeforOperate.vFreedomFund, accOperate.vFreedomFund));
		BOOST_CHECK(IsEqual(accBeforOperate.vFreeze, accOperate.vFreeze));
		BOOST_CHECK(IsEqual(accBeforOperate.vSelfFreeze, accOperate.vSelfFreeze));
		BOOST_CHECK(accBeforOperate.llValues == accOperate.llValues);

		//cout<<"old: "<<GetTotalValue(accBeforOperate.vSelfFreeze)<<" new: "<<GetTotalValue(accOperate.vSelfFreeze)<<endl;
		if (bCheckAuthority)
			IsAuthorityEqual(scriptID);
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
		if (OldAuth.GetLastOperHeight() / nBlocksPerDay < nHeight / nBlocksPerDay && 0 != nMoney) {
			BOOST_CHECK(OldAuth.GetMaxMoneyPerDay() == newAuthor.GetCurMaxMoneyPerDay() + nMoney);
		}
		else
		{
			//cout<<"old: "<<OldAuth.GetCurMaxMoneyPerDay()<<" Money:"<<nMoney<<" new: "<<newAuthor.GetCurMaxMoneyPerDay()<<endl;
			BOOST_CHECK(OldAuth.GetCurMaxMoneyPerDay() == newAuthor.GetCurMaxMoneyPerDay() + nMoney);
		}
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
	int nBranch[7] = { 0 };
	int nRunTimeHeight = 0;
	int nLastOperHeight = 0;
	bool bOverDay = false;
	bool bCheckAuthority = true;

	for (int i = 1; i <= TEST_SIZE / 10; i++) {
		nRunTimeHeight += 10;
		bOverDay = nRunTimeHeight / DAY_BLOCKS > nLastOperHeight / DAY_BLOCKS;
		bCheckAuthority = ((0 == i % 2) ? true : false);

		for (int j = 0; j < 10; j++) {
			uint64_t nOldVectorSum = GetTotalValue(accOperate.vFreedomFund);
			uint64_t nOldValue = accOperate.llValues;
			uint64_t minusValue = random(40) + 1;
			CFund fund(REWARD_FUND, minusValue, random(20));
			CAuthorizate OldAuthor = accOperate.mapAuthorizate[scriptID];

			if (nOldVectorSum >= minusValue) {
				if (bCheckAuthority) {
					if (accOperate.IsAuthorized(fund.value, nRunTimeHeight, scriptID)) {
						//run branch 0:enough money in vector and not signed but authorized by script
						BOOST_CHECK(accOperate.OperateAccount(MINUS_FREE, fund, nRunTimeHeight, &scriptID, true));
						BOOST_CHECK(GetTotalValue(accOperate.vFreedomFund) == nOldVectorSum - minusValue);
						CheckAuthorization(OldAuthor, fund.value, nRunTimeHeight, scriptID);
						nBranch[0]++;

						nLastOperHeight = nRunTimeHeight;
						if (!bOverDay)
							accYesterDayLastOper = accOperate;
					} else {
						//run branch 1:enough money in vector but not signed and not authorized by script
						BOOST_CHECK(!accOperate.OperateAccount(MINUS_FREE, fund, nRunTimeHeight, &scriptID, true));
						nBranch[1]++;
					}
				} else {
					//run branch 2:enough money in vector and signed by account(no need to check authority)
					BOOST_CHECK(accOperate.OperateAccount(MINUS_FREE, fund, nRunTimeHeight, &scriptID, false));
					BOOST_CHECK(GetTotalValue(accOperate.vFreedomFund) == nOldVectorSum - minusValue);
					nBranch[2]++;
				}

			} else {
				if (accOperate.llValues + nOldVectorSum >= minusValue) {
					if (bCheckAuthority) {
						if (accOperate.IsAuthorized(fund.value, nRunTimeHeight, scriptID)) {
							//run branch 3:enough money in (vector+llvalue) authorized by script but not signed
							BOOST_CHECK(accOperate.OperateAccount(MINUS_FREE, fund, nRunTimeHeight, &scriptID, true));
							BOOST_CHECK(
									GetTotalValue(accOperate.vFreedomFund) + accOperate.llValues
											== nOldValue + nOldVectorSum - minusValue);
							CheckAuthorization(OldAuthor, fund.value, nRunTimeHeight, scriptID);
							nBranch[3]++;

							nLastOperHeight = nRunTimeHeight;
							if (!bOverDay)
								accYesterDayLastOper = accOperate;
						} else {
							//run branch 4:enough money in (vector+llvalue) but not signed and not authorized by script
							BOOST_CHECK(!accOperate.OperateAccount(MINUS_FREE, fund, nRunTimeHeight, &scriptID, true));
							nBranch[4]++;
						}

					} else {
						//run branch 5:enough money in (vector+llvalue) and signed by account(no need to check authority)
						BOOST_CHECK(accOperate.OperateAccount(MINUS_FREE, fund, nRunTimeHeight, &scriptID, false));
						BOOST_CHECK(
								GetTotalValue(accOperate.vFreedomFund) + accOperate.llValues
										== nOldValue + nOldVectorSum - minusValue);
						nBranch[5]++;
					}

				} else {
					//run branch 6:not enough money
					BOOST_CHECK(!accOperate.OperateAccount(MINUS_FREE, fund, nRunTimeHeight, &scriptID, true));
					BOOST_CHECK(GetTotalValue(accOperate.vFreedomFund) == nOldVectorSum);
					BOOST_CHECK(nOldValue == accOperate.llValues);
					nBranch[6]++;
				}
			}
		}

		CAccount accountAfterOper = accOperate;
		accOperate.UndoOperateAccount(accOperate.accountOperLog);
		CheckAccountEqual(bCheckAuthority);

		accOperate = accountAfterOper;
		CAccountOperLog log;
		accOperate.accountOperLog = log;

		accBeforOperate = accOperate;
	}

	for (int i = 0; i < sizeof(nBranch) / sizeof(nBranch[0]); i++)
		BOOST_TEST_MESSAGE(strprintf("branch %d is %d" ,i , nBranch[i]));
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
	int nBranch[4] = { 0 };
	int nRunTimeHeight = 0;
	int nLastOperHeight = 0;
	bool bOverDay = false;
	bool bCheckAuthority = true;

	for (int i = 1; i <= TEST_SIZE / 10; i++) {
		nRunTimeHeight += 10;
		bOverDay = nRunTimeHeight / DAY_BLOCKS > nLastOperHeight / DAY_BLOCKS;
		bCheckAuthority = ((0 == i % 2) ? true : false);

		for (int j = 0; j < 10; j++) {
			uint64_t nOldVectorSum = GetTotalValue(accOperate.vSelfFreeze);
			uint64_t randValue = random(40);
			CFund fund(REWARD_FUND, randValue, random(20));
			CAuthorizate OldAuthor = accOperate.mapAuthorizate[scriptID];

			if (nOldVectorSum >= randValue) {
				if (bCheckAuthority) {
					if (accOperate.IsAuthorized(fund.value, nRunTimeHeight, scriptID)) {
						//branch 0:enough money and authorized by script
						BOOST_CHECK(
								accOperate.OperateAccount(MINUS_SELF_FREEZD, fund, nRunTimeHeight, &scriptID, true));
						BOOST_CHECK(GetTotalValue(accOperate.vSelfFreeze) == nOldVectorSum - randValue);
						CheckAuthorization(OldAuthor, fund.value, nRunTimeHeight, scriptID);
						nBranch[0]++;
						nLastOperHeight = nRunTimeHeight;
					} else {
						//branch 0:enough money but not authorized by script
						BOOST_CHECK(
								!accOperate.OperateAccount(MINUS_SELF_FREEZD, fund, nRunTimeHeight, &scriptID, true));
						nBranch[1]++;
					}
				} else {
					BOOST_CHECK(accOperate.OperateAccount(MINUS_SELF_FREEZD, fund, nRunTimeHeight, &scriptID, false));
					BOOST_CHECK(GetTotalValue(accOperate.vSelfFreeze) == nOldVectorSum - randValue);
					nBranch[2]++;
				}
			} else {
				//not enough money
				BOOST_CHECK(!accOperate.OperateAccount(MINUS_SELF_FREEZD, fund, nRunTimeHeight, &scriptID, true));
				BOOST_CHECK(GetTotalValue(accOperate.vSelfFreeze) == nOldVectorSum);
				nBranch[3]++;
			}
		}

		CAccount accountAfterOper = accOperate;
		accOperate.UndoOperateAccount(accOperate.accountOperLog);
		CheckAccountEqual(bCheckAuthority);

		accOperate = accountAfterOper;
		CAccountOperLog log;
		accOperate.accountOperLog = log;

		accBeforOperate = accOperate;
	}

	for (int i = 0; i < sizeof(nBranch) / sizeof(nBranch[0]); i++)
		BOOST_TEST_MESSAGE(strprintf("branch %d is %d" ,i , nBranch[i]));
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
			//branch 1:enough money and execute minus-operation
			BOOST_CHECK(accOperate.OperateAccount(MINUS_FREEZD, fund));
			BOOST_CHECK(GetTotalValue(accOperate.vFreeze) + nRandomValue == nOldFreezeValue);
			nBranch[1]++;

		} else {
			//branch 2:not enough money
			BOOST_CHECK(!accOperate.OperateAccount(MINUS_FREEZD, fund));
			BOOST_CHECK(GetTotalValue(accOperate.vFreeze) == nOldFreezeValue);
			nBranch[2]++;
		}
	}

	for (int i = 0; i < sizeof(nBranch) / sizeof(nBranch[0]); i++)
		BOOST_TEST_MESSAGE(strprintf("branch %d is %d" ,i , nBranch[i]));
	accOperate.UndoOperateAccount(accOperate.accountOperLog);
	CheckAccountEqual();

}

BOOST_AUTO_TEST_SUITE_END()
