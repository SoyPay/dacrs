#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include "txdb.h"
#include "database.h"
#include <iostream>
#include  "boost/filesystem/operations.hpp"
#include  "boost/filesystem/path.hpp"
using namespace std;

#define VECTOR_SIZE 1000

class CAccountViewTest {
 public:
	CAccountViewTest() {
//		pAccountViewDB = new CAccountViewDB("test",500, false, false);
		pcViewTip1 = new CAccountViewCache(*g_pAccountViewTip, true);
		pcViewTip2 = new CAccountViewCache(*pcViewTip1, true);
		Init();
	}
	~CAccountViewTest() {
//		if(pAccountViewDB != NULL) {
//			delete pAccountViewDB;
//		}
		if (pcViewTip2 != NULL) {
			delete pcViewTip2;
		}
		if (pcViewTip1 != NULL) {
			delete pcViewTip1;
		}
//		const boost::filesystem::path p=GetDataDir() / "blocks" / "test";
//		boost::filesystem::remove_all(p);
//		boost::filesystem::remove_all(GetDataDir() / "blocks" / "test");
	}
	bool EraseAccount();
	bool EraseKeyID();
	bool HaveAccount();
	bool CheckKeyMap(bool bCheckExist);
	bool SetKeyID();
	bool TestGetAccount(bool bCheckExist);
	void Init();

public:
	vector<CKeyID> vcRandomKeyID;
	vector<CRegID> vcRandomRegID;
	vector<CAccount> vcAccount;
	CAccountViewCache* pcViewTip1;
	CAccountViewCache* pcViewTip2;
};

bool CAccountViewTest::EraseKeyID() {
	for (int i = 0; i < VECTOR_SIZE; i++) {
		pcViewTip2->EraseId(vcRandomRegID.at(i));
	}
	return true;
}

bool CAccountViewTest::EraseAccount() {
	for (int i = 0; i < VECTOR_SIZE; i++) {
		CUserID userId = vcRandomKeyID.at(i);
		pcViewTip2->EraseAccount(userId);
	}
	return true;
}

bool CAccountViewTest::TestGetAccount(bool bCheckExist) {
	CAccount cAccount;

	//get account by keyID
	for (int i = 0; i < VECTOR_SIZE; i++) {
		CUserID userId = vcRandomKeyID.at(i);
		if (bCheckExist) {
			if (!pcViewTip2->GetAccount(userId, cAccount)) {
				return false;
			}
		} else {
			if (pcViewTip2->GetAccount(userId, cAccount)) {
				return false;
			}
		}
	}

	//get account by accountID
	for (int i = 0; i < VECTOR_SIZE; i++) {
		CUserID userId = vcRandomKeyID.at(i);
		if (bCheckExist) {
			if (!pcViewTip2->GetAccount(userId, cAccount)) {
				return false;
			}
		} else {
			if (pcViewTip2->GetAccount(userId, cAccount)) {
				return false;
			}
		}
	}

	return true;
}

bool CAccountViewTest::SetKeyID() {
	for (int i = 0; i < VECTOR_SIZE; i++) {
		pcViewTip2->SetKeyId(vcRandomRegID.at(i), vcRandomKeyID.at(i));
	}
	return true;
}

void CAccountViewTest::Init() {
	vcRandomKeyID.reserve(VECTOR_SIZE);
	vcRandomRegID.reserve(VECTOR_SIZE);
	vcAccount.reserve(VECTOR_SIZE);

	for (int i = 0; i < VECTOR_SIZE; i++) {
		CKey cKey;
		cKey.MakeNewKey();
		CPubKey cPubkey = cKey.GetPubKey();
		CKeyID cKeyID = cPubkey.GetKeyID();
		vcRandomKeyID.push_back(cKeyID);
	}

	for (int j = 0; j < VECTOR_SIZE; j++) {
		CRegID accountId(10000 + j, j);
		vcRandomRegID.push_back(accountId);
	}

	for (int k = 0; k < VECTOR_SIZE; k++) {
		CAccount account;
		account.llValues = k + 1;
		account.keyID = vcRandomKeyID.at(k);
		vcAccount.push_back(account);
	}
}

bool CAccountViewTest::CheckKeyMap(bool bCheckExist) {
	CKeyID cKeyID;
	for (int i = 0; i < VECTOR_SIZE; i++) {
		if (bCheckExist) {
			if (!pcViewTip2->GetKeyId(vcRandomRegID.at(i), cKeyID)) {
				return false;
			}
		} else {
			if (pcViewTip2->GetKeyId(vcRandomRegID.at(i), cKeyID)) {
				return false;
			}
		}
	}
	return true;
}

bool CAccountViewTest::HaveAccount() {
	for (int i = 0; i < VECTOR_SIZE; i++) {
		CUserID userId = vcRandomKeyID.at(i);
		if (pcViewTip2->HaveAccount(userId))
			return false;
	}
	return true;
}
BOOST_FIXTURE_TEST_SUITE(accountview_tests,CAccountViewTest)

BOOST_FIXTURE_TEST_CASE(regid_test,CAccountViewTest) {
	BOOST_CHECK(pcViewTip2);
	BOOST_CHECK(CheckKeyMap(false));
	BOOST_CHECK(SetKeyID());
	BOOST_CHECK(HaveAccount());

	BOOST_CHECK(CheckKeyMap(true));
	BOOST_CHECK(pcViewTip2->Flush());
	BOOST_CHECK(CheckKeyMap(true));

	EraseKeyID();
	BOOST_CHECK(pcViewTip2->Flush());
	BOOST_CHECK(CheckKeyMap(false));
}

BOOST_FIXTURE_TEST_CASE(setaccount_test1,CAccountViewTest) {
	BOOST_CHECK(SetKeyID());
	for (int i = 0; i < VECTOR_SIZE; i++) {
		CUserID userId = vcRandomKeyID.at(i);
		BOOST_CHECK(pcViewTip2->SetAccount(userId, vcAccount.at(i)));
	}

	BOOST_CHECK(TestGetAccount(true));
	BOOST_CHECK(pcViewTip2->Flush());
	BOOST_CHECK(TestGetAccount(true));

	EraseAccount();
	EraseKeyID();
	BOOST_CHECK(pcViewTip2->Flush());
	BOOST_CHECK(TestGetAccount(false));
}

BOOST_FIXTURE_TEST_CASE(setaccount_test2,CAccountViewTest) {
	BOOST_CHECK(SetKeyID());
	for (int i = 0; i < VECTOR_SIZE; i++) {
		CUserID userId = vcRandomRegID.at(i);
		BOOST_CHECK(pcViewTip2->SetAccount(userId, vcAccount.at(i)));
	}

	BOOST_CHECK(TestGetAccount(true));
	BOOST_CHECK(pcViewTip2->Flush());
	BOOST_CHECK(TestGetAccount(true));

	EraseAccount();
	EraseKeyID();
	BOOST_CHECK(pcViewTip2->Flush());
	BOOST_CHECK(TestGetAccount(false));
}

BOOST_FIXTURE_TEST_CASE(BatchWrite_test,CAccountViewTest) {
	BOOST_CHECK(SetKeyID());
	pcViewTip2->BatchWrite(vcAccount);

	BOOST_CHECK(TestGetAccount(true));
	BOOST_CHECK(pcViewTip2->Flush());
	BOOST_CHECK(TestGetAccount(true));

	EraseAccount();
	EraseKeyID();
	BOOST_CHECK(pcViewTip2->Flush());
	BOOST_CHECK(TestGetAccount(false));
}

BOOST_AUTO_TEST_SUITE_END()
