#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include "txdb.h"
#include "account.h"
#include <iostream>
using namespace std;

#define VECTOR_SIZE 1000
class CAccountViewTest {
public:
	CAccountViewTest() {
		pAccountViewDB = new CAccountViewDB(500, false, false);
		pAccountViewTip = new CAccountViewCache(*pAccountViewDB);
		Init();
	}

	bool EraseAccount();
	bool EraseKeyID();
	bool HaveAccount();
	bool CheckKeyMap(bool bCheckExist);
	bool SetKeyID();
	bool TestGetAccount(bool bCheckExist);
	void Init();

public:
	vector<CKeyID> vRandomKeyID;
	vector<CRegID> vRandomRegID;
	vector<CSecureAccount> vAccount;
	CAccountViewDB* pAccountViewDB;
	CAccountViewCache* pAccountViewTip;
};

bool CAccountViewTest::EraseKeyID() {
	for (int i = 0; i < VECTOR_SIZE; i++) {
		pAccountViewTip->EraseKeyId(vRandomRegID.at(i).vRegID);
	}

	return true;
}

bool CAccountViewTest::EraseAccount() {
	for (int i = 0; i < VECTOR_SIZE; i++) {
		pAccountViewTip->EraseAccount(vRandomKeyID.at(i));
	}

	return true;
}

bool CAccountViewTest::TestGetAccount(bool bCheckExist) {
	CSecureAccount account;

	//get account by keyID
	for (int i = 0; i < VECTOR_SIZE; i++) {
		if (bCheckExist) {
			if (!pAccountViewTip->GetAccount(vRandomKeyID.at(i), account)) {
				return false;
			}
		} else {
			if (pAccountViewTip->GetAccount(vRandomKeyID.at(i), account)) {
				return false;
			}
		}
	}

	//get account by accountID
	for (int i = 0; i < VECTOR_SIZE; i++) {
		if (bCheckExist) {
			if (!pAccountViewTip->GetAccount(vRandomRegID.at(i).vRegID, account)) {
				return false;
			}
		} else {
			if (pAccountViewTip->GetAccount(vRandomRegID.at(i).vRegID, account)) {
				return false;
			}
		}
	}

	return true;
}

bool CAccountViewTest::SetKeyID() {
	for (int i = 0; i < VECTOR_SIZE; i++) {
		pAccountViewTip->SetKeyId(vRandomRegID.at(i).vRegID, vRandomKeyID.at(i));
	}

	return true;
}

void CAccountViewTest::Init() {
	vRandomKeyID.reserve(VECTOR_SIZE);
	vRandomRegID.reserve(VECTOR_SIZE);
	vAccount.reserve(VECTOR_SIZE);

	for (int i = 0; i < VECTOR_SIZE; i++) {
		CKey key;
		key.MakeNewKey(false);
		CPubKey pubkey = key.GetPubKey();
		CKeyID keyID = pubkey.GetID();
		vRandomKeyID.push_back(keyID);
	}

	for (int j = 0; j < VECTOR_SIZE; j++) {
		CRegID accountId(10000 + j, j);
		vRandomRegID.push_back(accountId);
	}

	for (int k = 0; k < VECTOR_SIZE; k++) {
		CSecureAccount account;
		account.llValues = k + 1;
		account.keyID = vRandomKeyID.at(k);
		vAccount.push_back(account);
	}
}

bool CAccountViewTest::CheckKeyMap(bool bCheckExist) {
	CKeyID keyID;
	for (int i = 0; i < VECTOR_SIZE; i++) {
		if (bCheckExist) {
			if (!pAccountViewTip->GetKeyId(vRandomRegID.at(i).vRegID, keyID)) {
				return false;
			}
		} else {
			if (pAccountViewTip->GetKeyId(vRandomRegID.at(i).vRegID, keyID)) {
				return false;
			}
		}
	}

	return true;
}

bool CAccountViewTest::HaveAccount() {
	for (int i = 0; i < VECTOR_SIZE; i++) {
		if (pAccountViewTip->HaveAccount(vRandomKeyID.at(i)))
			return false;
	}

	return true;
}
BOOST_FIXTURE_TEST_SUITE(accountview_tests,CAccountViewTest)

#if 0

BOOST_FIXTURE_TEST_CASE(regid_test,CAccountViewTest)
{
	BOOST_CHECK(pAccountViewTip);
	BOOST_CHECK(CheckKeyMap(false) );
	BOOST_CHECK(SetKeyID() );
	BOOST_CHECK(HaveAccount() );

	BOOST_CHECK(CheckKeyMap(true) );
	BOOST_CHECK(pAccountViewTip->Flush() );
	BOOST_CHECK(CheckKeyMap(true) );

	EraseKeyID();
	BOOST_CHECK(pAccountViewTip->Flush() );
	BOOST_CHECK(CheckKeyMap(false) );
}

BOOST_FIXTURE_TEST_CASE(setaccount_test1,CAccountViewTest)
{
	BOOST_CHECK(SetKeyID() );
	for (int i = 0; i < VECTOR_SIZE; i++) {
		BOOST_CHECK(pAccountViewTip->SetAccount(vRandomKeyID.at(i), vAccount.at(i)) );
	}

	BOOST_CHECK(TestGetAccount(true) );
	BOOST_CHECK(pAccountViewTip->Flush() );
	BOOST_CHECK(TestGetAccount(true) );

	EraseAccount();
	EraseKeyID();
	BOOST_CHECK(pAccountViewTip->Flush() );
	BOOST_CHECK(TestGetAccount(false) );
}

BOOST_FIXTURE_TEST_CASE(setaccount_test2,CAccountViewTest)
{
	BOOST_CHECK(SetKeyID());
	for (int i = 0; i < VECTOR_SIZE; i++) {
		BOOST_CHECK(pAccountViewTip->SetAccount(vRandomRegID.at(i).vRegID, vAccount.at(i)) );
	}

	BOOST_CHECK(TestGetAccount(true));
	BOOST_CHECK(pAccountViewTip->Flush());
	BOOST_CHECK(TestGetAccount(true));

	EraseAccount();
	EraseKeyID();
	BOOST_CHECK(pAccountViewTip->Flush());
	BOOST_CHECK(TestGetAccount(false));
}

BOOST_FIXTURE_TEST_CASE(BatchWrite_test,CAccountViewTest)
{
	BOOST_CHECK(SetKeyID() );
	pAccountViewTip->BatchWrite(vAccount);

	BOOST_CHECK(TestGetAccount(true));
	BOOST_CHECK(pAccountViewTip->Flush());
	BOOST_CHECK(TestGetAccount(true));

	EraseAccount();
	EraseKeyID();
	BOOST_CHECK(pAccountViewTip->Flush());
	BOOST_CHECK(TestGetAccount(false));
}

#else
BOOST_AUTO_TEST_CASE(xxxx) {
	BOOST_ERROR("ERROR:THE SUITE NEED TO MODIFY!");
}
#endif

BOOST_AUTO_TEST_SUITE_END()
