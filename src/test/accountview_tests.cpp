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
	~CAccountViewTest() {
		if(pAccountViewDB != NULL) {
			delete pAccountViewDB;
		}
		if(pAccountViewTip != NULL) {
			delete pAccountViewTip;
		}
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
	vector<CAccount> vAccount;
	CAccountViewDB* pAccountViewDB;
	CAccountViewCache* pAccountViewTip;
};

bool CAccountViewTest::EraseKeyID() {
	for (int i = 0; i < VECTOR_SIZE; i++) {
		pAccountViewTip->EraseId(vRandomRegID.at(i));
	}

	return true;
}

bool CAccountViewTest::EraseAccount() {
	for (int i = 0; i < VECTOR_SIZE; i++) {
		CUserID userId = vRandomKeyID.at(i);
		pAccountViewTip->EraseAccount(userId);
	}

	return true;
}

bool CAccountViewTest::TestGetAccount(bool bCheckExist) {
	CAccount account;

	//get account by keyID
	for (int i = 0; i < VECTOR_SIZE; i++) {
		CUserID userId = vRandomKeyID.at(i);
		if (bCheckExist) {
			if (!pAccountViewTip->GetAccount(userId, account)) {
				return false;
			}
		} else {
			if (pAccountViewTip->GetAccount(userId, account)) {
				return false;
			}
		}
	}

	//get account by accountID
	for (int i = 0; i < VECTOR_SIZE; i++) {
		CUserID userId = vRandomKeyID.at(i);
		if (bCheckExist) {
			if (!pAccountViewTip->GetAccount(userId, account)) {
				return false;
			}
		} else {
			if (pAccountViewTip->GetAccount(userId, account)) {
				return false;
			}
		}
	}

	return true;
}

bool CAccountViewTest::SetKeyID() {
	for (int i = 0; i < VECTOR_SIZE; i++) {
		pAccountViewTip->SetKeyId(vRandomRegID.at(i), vRandomKeyID.at(i));
	}

	return true;
}

void CAccountViewTest::Init() {
	vRandomKeyID.reserve(VECTOR_SIZE);
	vRandomRegID.reserve(VECTOR_SIZE);
	vAccount.reserve(VECTOR_SIZE);

	for (int i = 0; i < VECTOR_SIZE; i++) {
		CKey key;
		key.MakeNewKey();
		CPubKey pubkey = key.GetPubKey();
		CKeyID keyID = pubkey.GetKeyID();
		vRandomKeyID.push_back(keyID);
	}

	for (int j = 0; j < VECTOR_SIZE; j++) {
		CRegID accountId(10000 + j, j);
		vRandomRegID.push_back(accountId);
	}

	for (int k = 0; k < VECTOR_SIZE; k++) {
		CAccount account;
		account.llValues = k + 1;
		account.keyID = vRandomKeyID.at(k);
		vAccount.push_back(account);
	}
}

bool CAccountViewTest::CheckKeyMap(bool bCheckExist) {
	CKeyID keyID;
	for (int i = 0; i < VECTOR_SIZE; i++) {
		if (bCheckExist) {
			if (!pAccountViewTip->GetKeyId(vRandomRegID.at(i), keyID)) {
				return false;
			}
		} else {
			if (pAccountViewTip->GetKeyId(vRandomRegID.at(i), keyID)) {
				return false;
			}
		}
	}

	return true;
}

bool CAccountViewTest::HaveAccount() {
	for (int i = 0; i < VECTOR_SIZE; i++) {
		CUserID userId = vRandomKeyID.at(i);
		if (pAccountViewTip->HaveAccount(userId))
			return false;
	}

	return true;
}
BOOST_FIXTURE_TEST_SUITE(accountview_tests,CAccountViewTest)



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
		CUserID userId = vRandomKeyID.at(i);
		BOOST_CHECK(pAccountViewTip->SetAccount(userId, vAccount.at(i)) );
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
		CUserID userId = vRandomRegID.at(i);
		BOOST_CHECK(pAccountViewTip->SetAccount(userId, vAccount.at(i)));
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


BOOST_AUTO_TEST_SUITE_END()
