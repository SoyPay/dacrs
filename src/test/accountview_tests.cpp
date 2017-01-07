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
		// pAccountViewDB = new CAccountViewDB("test",500, false, false);
		m_pcViewTip1 = new CAccountViewCache(*g_pAccountViewTip, true);
		m_pcViewTip2 = new CAccountViewCache(*m_pcViewTip1, true);
		Init();
	}

	~CAccountViewTest() {
		// if(pAccountViewDB != NULL) {
		// delete pAccountViewDB;
		// }
		if (m_pcViewTip2 != NULL) {
			delete m_pcViewTip2;
		}
		if (m_pcViewTip1 != NULL) {
			delete m_pcViewTip1;
		}
		// const boost::filesystem::path p=GetDataDir() / "blocks" / "test";
		// boost::filesystem::remove_all(p);
		// boost::filesystem::remove_all(GetDataDir() / "blocks" / "test");
	}
	bool EraseAccount();
	bool EraseKeyID();
	bool HaveAccount();
	bool CheckKeyMap(bool bCheckExist);
	bool SetKeyID();
	bool TestGetAccount(bool bCheckExist);
	void Init();

 public:
	vector<CKeyID> m_vcRandomKeyID;
	vector<CRegID> m_vcRandomRegID;
	vector<CAccount> m_vcAccount;
	CAccountViewCache* m_pcViewTip1;
	CAccountViewCache* m_pcViewTip2;
};

bool CAccountViewTest::EraseKeyID() {
	for (int i = 0; i < VECTOR_SIZE; i++) {
		m_pcViewTip2->EraseId(m_vcRandomRegID.at(i));
	}
	return true;
}

bool CAccountViewTest::EraseAccount() {
	for (int i = 0; i < VECTOR_SIZE; i++) {
		CUserID cUserId = m_vcRandomKeyID.at(i);
		m_pcViewTip2->EraseAccount(cUserId);
	}
	return true;
}

bool CAccountViewTest::TestGetAccount(bool bCheckExist) {
	CAccount cAccount;
	//get account by keyID
	for (int i = 0; i < VECTOR_SIZE; i++) {
		CUserID cUserId = m_vcRandomKeyID.at(i);
		if (bCheckExist) {
			if (!m_pcViewTip2->GetAccount(cUserId, cAccount)) {
				return false;
			}
		} else {
			if (m_pcViewTip2->GetAccount(cUserId, cAccount)) {
				return false;
			}
		}
	}

	//get account by accountID
	for (int i = 0; i < VECTOR_SIZE; i++) {
		CUserID cUserId = m_vcRandomKeyID.at(i);
		if (bCheckExist) {
			if (!m_pcViewTip2->GetAccount(cUserId, cAccount)) {
				return false;
			}
		} else {
			if (m_pcViewTip2->GetAccount(cUserId, cAccount)) {
				return false;
			}
		}
	}

	return true;
}

bool CAccountViewTest::SetKeyID() {
	for (int i = 0; i < VECTOR_SIZE; i++) {
		m_pcViewTip2->SetKeyId(m_vcRandomRegID.at(i), m_vcRandomKeyID.at(i));
	}
	return true;
}

void CAccountViewTest::Init() {
	m_vcRandomKeyID.reserve(VECTOR_SIZE);
	m_vcRandomRegID.reserve(VECTOR_SIZE);
	m_vcAccount.reserve(VECTOR_SIZE);

	for (int i = 0; i < VECTOR_SIZE; i++) {
		CKey cKey;
		cKey.MakeNewKey();
		CPubKey cPubkey = cKey.GetPubKey();
		CKeyID cKeyID = cPubkey.GetKeyID();
		m_vcRandomKeyID.push_back(cKeyID);
	}

	for (int j = 0; j < VECTOR_SIZE; j++) {
		CRegID cAccountId(10000 + j, j);
		m_vcRandomRegID.push_back(cAccountId);
	}

	for (int k = 0; k < VECTOR_SIZE; k++) {
		CAccount cAccount;
		cAccount.m_ullValues = k + 1;
		cAccount.m_cKeyID = m_vcRandomKeyID.at(k);
		m_vcAccount.push_back(cAccount);
	}
}

bool CAccountViewTest::CheckKeyMap(bool bCheckExist) {
	CKeyID cKeyID;
	for (int i = 0; i < VECTOR_SIZE; i++) {
		if (bCheckExist) {
			if (!m_pcViewTip2->GetKeyId(m_vcRandomRegID.at(i), cKeyID)) {
				return false;
			}
		} else {
			if (m_pcViewTip2->GetKeyId(m_vcRandomRegID.at(i), cKeyID)) {
				return false;
			}
		}
	}
	return true;
}

bool CAccountViewTest::HaveAccount() {
	for (int i = 0; i < VECTOR_SIZE; i++) {
		CUserID cUserId = m_vcRandomKeyID.at(i);
		if (m_pcViewTip2->HaveAccount(cUserId)) {
			return false;
		}
	}
	return true;
}

BOOST_FIXTURE_TEST_SUITE(accountview_tests,CAccountViewTest)

BOOST_FIXTURE_TEST_CASE(regid_test,CAccountViewTest) {
	BOOST_CHECK(m_pcViewTip2);
	BOOST_CHECK(CheckKeyMap(false));
	BOOST_CHECK(SetKeyID());
	BOOST_CHECK(HaveAccount());

	BOOST_CHECK(CheckKeyMap(true));
	BOOST_CHECK(m_pcViewTip2->Flush());
	BOOST_CHECK(CheckKeyMap(true));

	EraseKeyID();
	BOOST_CHECK(m_pcViewTip2->Flush());
	BOOST_CHECK(CheckKeyMap(false));
}

BOOST_FIXTURE_TEST_CASE(setaccount_test1,CAccountViewTest) {
	BOOST_CHECK(SetKeyID());
	for (int i = 0; i < VECTOR_SIZE; i++) {
		CUserID cUserId = m_vcRandomKeyID.at(i);
		BOOST_CHECK(m_pcViewTip2->SetAccount(cUserId, m_vcAccount.at(i)));
	}

	BOOST_CHECK(TestGetAccount(true));
	BOOST_CHECK(m_pcViewTip2->Flush());
	BOOST_CHECK(TestGetAccount(true));

	EraseAccount();
	EraseKeyID();
	BOOST_CHECK(m_pcViewTip2->Flush());
	BOOST_CHECK(TestGetAccount(false));
}

BOOST_FIXTURE_TEST_CASE(setaccount_test2,CAccountViewTest) {
	BOOST_CHECK(SetKeyID());
	for (int i = 0; i < VECTOR_SIZE; i++) {
		CUserID cUserId = m_vcRandomRegID.at(i);
		BOOST_CHECK(m_pcViewTip2->SetAccount(cUserId, m_vcAccount.at(i)));
	}

	BOOST_CHECK(TestGetAccount(true));
	BOOST_CHECK(m_pcViewTip2->Flush());
	BOOST_CHECK(TestGetAccount(true));

	EraseAccount();
	EraseKeyID();
	BOOST_CHECK(m_pcViewTip2->Flush());
	BOOST_CHECK(TestGetAccount(false));
}

BOOST_FIXTURE_TEST_CASE(BatchWrite_test,CAccountViewTest) {
	BOOST_CHECK(SetKeyID());
	m_pcViewTip2->BatchWrite(m_vcAccount);

	BOOST_CHECK(TestGetAccount(true));
	BOOST_CHECK(m_pcViewTip2->Flush());
	BOOST_CHECK(TestGetAccount(true));

	EraseAccount();
	EraseKeyID();
	BOOST_CHECK(m_pcViewTip2->Flush());
	BOOST_CHECK(TestGetAccount(false));
}

BOOST_AUTO_TEST_SUITE_END()
