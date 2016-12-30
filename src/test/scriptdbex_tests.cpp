#include "main.h"
#include "txdb.h"
#include "database.h"
#include <iostream>
#include <boost/test/unit_test.hpp>
#include  "boost/filesystem/operations.hpp"
#include  "boost/filesystem/path.hpp"
using namespace std;

class CScriptDBTest {
public:
	CScriptDBTest();
	~CScriptDBTest();
	void Init();
	void InsertData(CScriptDBViewCache* pcViewCache);
	void CheckRecordCount(CScriptDBViewCache* pcViewCache, size_t unComparCount);
	void CheckReadData(CScriptDBViewCache* pcViewCache);
	void Flush(CScriptDBViewCache* pViewCache1, CScriptDBViewCache* pcViewCache2, CScriptDBViewCache* pcViewCache3);
	void Flush(CScriptDBViewCache* pcViewCache);
	void EraseData(CScriptDBViewCache* pcViewCache);
	void GetScriptData(CScriptDBViewCache* pcViewCache);
protected:
	static const int TEST_SIZE = 10000;
	CScriptDB* m_pcTestDB;
	CScriptDBViewCache* m_pcTestView;
	map<vector<unsigned char>, vector<unsigned char> > m_mapScript;
};

CScriptDBTest::CScriptDBTest() {
	Init();
}


CScriptDBTest::~CScriptDBTest() {
	if (m_pcTestView != NULL) {
		delete m_pcTestView;
		m_pcTestView = NULL;
	}
	if (m_pcTestDB != NULL) {
		delete m_pcTestDB;
		m_pcTestDB = NULL;
	}
	const boost::filesystem::path p = GetDataDir() / "blocks" / "testdb";
	boost::filesystem::remove_all(p);
}

void CScriptDBTest::Init() {
	m_pcTestDB = new CScriptDB("testdb", size_t(4 << 20), false, true);
	m_pcTestView = new CScriptDBViewCache(*m_pcTestDB, false);

	vector<unsigned char> vScriptBase = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
			0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };

	for (int i = 0; i < TEST_SIZE; ++i) {
		CRegID regID(i, 1);
		vector<unsigned char> vcScriptData(vScriptBase.begin(), vScriptBase.end());
		vector<unsigned char> vcRegV6 = regID.GetVec6();
		vcScriptData.insert(vcScriptData.end(), vcRegV6.begin(), vcRegV6.end());
		m_mapScript.insert(std::make_pair(vcRegV6, vcScriptData));
	}
}

void CScriptDBTest::CheckReadData(CScriptDBViewCache* pcViewCache) {
	BOOST_CHECK(pcViewCache);
	vector<unsigned char> vScriptContent;
	for (const auto& item : m_mapScript) {
		CRegID regID(item.first);
		BOOST_CHECK(pcViewCache->HaveScript(regID));
		BOOST_CHECK(pcViewCache->GetScript(regID, vScriptContent));
		BOOST_CHECK(vScriptContent == item.second);
	}
}

void CScriptDBTest::InsertData(CScriptDBViewCache* pcViewCache) {
	BOOST_CHECK(pcViewCache);
	for (const auto& item : m_mapScript) {
		CRegID regID(item.first);
		BOOST_CHECK(pcViewCache->SetScript(regID, item.second));
	}
}

void CScriptDBTest::CheckRecordCount(CScriptDBViewCache* pcViewCache, size_t unComparCount) {
	BOOST_CHECK(pcViewCache);
	int nCount = 0;
	if (0 == unComparCount) {
		BOOST_CHECK(!pcViewCache->GetScriptCount(nCount));
	} else {
		BOOST_CHECK(pcViewCache->GetScriptCount(nCount));
		BOOST_CHECK((unsigned int )nCount == unComparCount);
	}
}

void CScriptDBTest::Flush(CScriptDBViewCache* pViewCache1, CScriptDBViewCache* pcViewCache2,CScriptDBViewCache* pcViewCache3) {
	BOOST_CHECK(pViewCache1 && pcViewCache2 && pcViewCache3);
	BOOST_CHECK(pViewCache1->Flush());
	BOOST_CHECK(pcViewCache2->Flush());
	BOOST_CHECK(pcViewCache3->Flush());
}

void CScriptDBTest::Flush(CScriptDBViewCache* pcViewCache) {
	BOOST_CHECK(pcViewCache);
	BOOST_CHECK(pcViewCache->Flush());
}

void CScriptDBTest::EraseData(CScriptDBViewCache* pcViewCache) {
	BOOST_CHECK(pcViewCache);
	for (const auto& item : m_mapScript) {
		CRegID regID(item.first);
		BOOST_CHECK(pcViewCache->EraseScript(regID));
	}
}

void CScriptDBTest::GetScriptData(CScriptDBViewCache* pcViewCache) {
	BOOST_CHECK(pcViewCache);
	int nCount = 0;
//	int nHeight = 0;
	int nCurHeight = TEST_SIZE / 2;
	vector<unsigned char> vcScriptData;
	vector<unsigned char> vcScriptKey;
	set<CScriptDBOperLog> setOperLog;
	auto it = m_mapScript.begin();
	BOOST_CHECK(it != m_mapScript.end());

	BOOST_CHECK(pcViewCache->GetScriptDataCount(CRegID(it->first), nCount));
	bool bRet = pcViewCache->GetScriptData(nCurHeight, CRegID(it->first), 0, vcScriptKey, vcScriptData);

	while (bRet) {
		if (++it == m_mapScript.end()) {
			break;
		}
		bRet = pcViewCache->GetScriptData(nCurHeight, CRegID(it->first), 1, vcScriptKey, vcScriptData);
		pcViewCache->GetScriptDataCount(CRegID(it->first), nCount);
	}
}

BOOST_FIXTURE_TEST_SUITE(scriptdbex_test,CScriptDBTest)
BOOST_AUTO_TEST_CASE(add_erase) {
	CScriptDBViewCache cCache2(*m_pcTestView, true);
	CScriptDBViewCache cCache3(cCache2, true);
	InsertData(&cCache3);

	Flush(&cCache3, &cCache2, m_pcTestView);

	CheckRecordCount(m_pcTestView, m_mapScript.size());

	CheckReadData(m_pcTestView);

	//test erase data
	EraseData(&cCache3);

	Flush(&cCache3, &cCache2, m_pcTestView);

	CheckRecordCount(m_pcTestView, 0);
}

BOOST_AUTO_TEST_CASE(overtime) {
//	CScriptDBViewCache cCache2(*m_pcTestView,true);
//	CScriptDBViewCache cCache3(cCache2,true);
//	InsertData(&cCache3);
//
//	Flush(&cCache3,&cCache2,m_pcTestView);
//
//	GetScriptData(&cCache3);
//
//	Flush(&cCache3,&cCache2,m_pcTestView);
//
//	CheckRecordCount(m_pcTestView,TEST_SIZE/2);
}

BOOST_AUTO_TEST_SUITE_END()









