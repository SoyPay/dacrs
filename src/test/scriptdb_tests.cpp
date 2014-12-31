#include "main.h"
#include "txdb.h"
#include "account.h"
#include <iostream>
#include <boost/test/unit_test.hpp>
#include  "boost/filesystem/operations.hpp"
#include  "boost/filesystem/path.hpp"
using namespace std;
CScriptDBViewCache *pTestView = NULL;
CScriptDB *pTestDB = NULL;
void init() {
	 pTestDB = new CScriptDB("testdb",size_t(4<<20), false , true);
	 pTestView=  new CScriptDBViewCache(*pTestDB, false);
}

void closedb() {

	if (pTestView != NULL) {
		delete pTestView;
		pTestView = NULL;
	}
	if (pTestDB != NULL) {
		delete pTestDB;
		pTestDB = NULL;
	}
	const boost::filesystem::path p=GetDataDir() / "blocks" / "testdb";
	boost::filesystem::remove_all(p);

}
void testscriptdb() {
	vector<unsigned char> vScriptId = {0x01,0x00,0x00,0x00,0x01,0x00};
	vector<unsigned char> vScriptId1 = {0x01,0x00,0x00,0x00,0x02,0x00};
	vector<unsigned char> vScriptContent = {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
	vector<unsigned char> vScriptContent1 = {0x01,0x02,0x03,0x04,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
	CRegID regScriptId(vScriptId);
	CRegID regScriptId1(vScriptId1);
	//write script content to db
	BOOST_CHECK(pTestView->SetScript(regScriptId, vScriptContent));
	BOOST_CHECK(pTestView->SetScript(regScriptId1, vScriptContent1));
	//write all data in caches to db
	BOOST_CHECK(pTestView->Flush());
	//test if the script id is exist in db
	BOOST_CHECK(pTestView->HaveScript(regScriptId));
	vector<unsigned char> vScript;
	//read script content from db by scriptId
	BOOST_CHECK(pTestView->GetScript(regScriptId,vScript));
	// if the readed script content equals with original
	BOOST_CHECK(vScriptContent == vScript);
	int nCount;
	//get script numbers from db
	BOOST_CHECK(pTestView->GetScriptCount(nCount));
	//if the number is one
	BOOST_CHECK_EQUAL(nCount, 2);
	//get index 0 script from db
	vScript.clear();
	vector<unsigned char> vId;

	int nIndex = 0;
	CRegID regId;
	BOOST_CHECK(pTestView->GetScript(0, regId, vScript));
	BOOST_CHECK(vScriptId == regId.GetVec6());
	BOOST_CHECK(vScriptContent == vScript);
	nIndex = 1;
	BOOST_CHECK(pTestView->GetScript(1, regId, vScript));
	BOOST_CHECK(vScriptId1 == regId.GetVec6());
	BOOST_CHECK(vScriptContent1 == vScript);
	//delete script from db
	BOOST_CHECK(pTestView->EraseScript(regScriptId));
	BOOST_CHECK(pTestView->GetScriptCount(nCount));
	BOOST_CHECK_EQUAL(nCount, 1);
	//write all data in caches to db
	BOOST_CHECK(pTestView->Flush());
}


void testscriptdatadb() {
	vector<unsigned char> vScriptId = {0x01,0x00,0x00,0x00,0x02,0x00};
//  vector<unsigned char> vScriptKey = {0x01,0x00,0x02,0x03,0x04,0x05,0x06,0x07};
//	vector<unsigned char> vScriptKey1 = {0x01,0x00,0x02,0x03,0x04,0x05,0x07,0x06};
	vector<unsigned char> vScriptKey = {0x01,0x00,0x01};
	vector<unsigned char> vScriptKey1 = {0x01,0x00,0x02};
	vector<unsigned char> vScriptKey2 = {0x01,0x00,0x03};
	vector<unsigned char> vScriptKey3 = {0x01,0x00,0x04};
	vector<unsigned char> vScriptKey4 = {0x01,0x00,0x05};
	vector<unsigned char> vScriptKey5 = {0x01,0x00,0x06};
	vector<unsigned char> vScriptData = {0x01,0x01,0x01,0x01,0x01};
	vector<unsigned char> vScriptData1 = {0x01,0x01,0x01,0x00,0x00};
	CScriptDBOperLog operlog;
	CRegID regScriptId(vScriptId);

	//测试数据库中有vScriptKey1， vScriptKey3，在缓存中有vScriptKey， vScriptKey2，是否能正确遍历脚本数据库
	BOOST_CHECK(pTestView->SetScriptData(regScriptId, vScriptKey1, vScriptData, 100, operlog));
	BOOST_CHECK(pTestView->SetScriptData(regScriptId, vScriptKey3, vScriptData, 100, operlog));
	BOOST_CHECK(pTestView->SetScriptData(regScriptId, vScriptKey5, vScriptData, 100, operlog));
	BOOST_CHECK(pTestView->Flush());
	BOOST_CHECK(pTestView->SetScriptData(regScriptId, vScriptKey, vScriptData, 100, operlog));
	BOOST_CHECK(pTestView->SetScriptData(regScriptId, vScriptKey2, vScriptData, 100, operlog));
	BOOST_CHECK(pTestView->SetScriptData(regScriptId, vScriptKey4, vScriptData, 100, operlog));
	BOOST_CHECK(pTestView->EraseScriptData(regScriptId, vScriptKey3, operlog));
	BOOST_CHECK(pTestView->EraseScriptData(regScriptId, vScriptKey2, operlog));

	int height(0);
	int curheight(0);
	vector<unsigned char> vKey;
	vector<unsigned char> vScript;
	int nValidHeight(0);
	set<CScriptDBOperLog> setOperLog;
	bool ret = pTestView->GetScriptData(curheight,regScriptId, 0, vKey, vScript, nValidHeight, setOperLog);
	if(ret) cout << "script key:" << HexStr(vKey) << endl;
	while(ret) {
		ret = pTestView->GetScriptData(curheight,regScriptId, 1, vKey, vScript, nValidHeight, setOperLog);
		if(ret) cout << "script key:" << HexStr(vKey) << endl;
	}
	BOOST_CHECK(pTestView->EraseScriptData(regScriptId, vScriptKey, operlog));
	BOOST_CHECK(pTestView->EraseScriptData(regScriptId, vScriptKey1, operlog));
	BOOST_CHECK(pTestView->EraseScriptData(regScriptId, vScriptKey4, operlog));
	BOOST_CHECK(pTestView->EraseScriptData(regScriptId, vScriptKey5, operlog));

	BOOST_CHECK(pTestView->SetScriptData(regScriptId, vScriptKey, vScriptData, 100, operlog));
//	int height = 0;
//	int curheight = 0;
	BOOST_CHECK(pTestView->GetScriptData(curheight,regScriptId,vScriptKey,vScriptData,height,operlog));
	pTestView->GetScriptCount(height);

	BOOST_CHECK(pTestView->GetScriptData(curheight,regScriptId, 0, vScriptKey, vScriptData, height, setOperLog));
	BOOST_CHECK(pTestView->SetScriptData(regScriptId, vScriptKey, vScriptData, 100, operlog));

	//write script data to db
	BOOST_CHECK(pTestView->SetScriptData(regScriptId, vScriptKey, vScriptData, 100, operlog));
	//write all data in caches to db
	BOOST_CHECK(pTestView->Flush());
	BOOST_CHECK(pTestView->SetScriptData(regScriptId, vScriptKey1, vScriptData1, 101, operlog));
	//test if the script id is exist in db
	BOOST_CHECK(pTestView->HaveScriptData(regScriptId, vScriptKey));
	vScript.clear();


	//read script content from db by scriptId
	BOOST_CHECK(pTestView->GetScriptData(curheight,regScriptId, vScriptKey, vScript, nValidHeight, operlog));
	// if the readed script content equals with original
	BOOST_CHECK(vScriptData == vScript);
	BOOST_CHECK_EQUAL(nValidHeight, 100);
	int nCount;
	//get script numbers from db
	BOOST_CHECK(pTestView->GetScriptDataCount(regScriptId, nCount));
	//if the number is one
	BOOST_CHECK_EQUAL(nCount, 2);
	//get index 0 script from db
	vScript.clear();
	vKey.clear();
	nValidHeight = 0;
	BOOST_CHECK(pTestView->GetScriptData(curheight,regScriptId, 0, vKey, vScript, nValidHeight, setOperLog));
	BOOST_CHECK(vKey == vScriptKey);
	BOOST_CHECK(vScript == vScriptData);
	BOOST_CHECK_EQUAL(nValidHeight,100);
	BOOST_CHECK(pTestView->GetScriptData(curheight,regScriptId, 1, vKey, vScript, nValidHeight, setOperLog));
	BOOST_CHECK(vKey == vScriptKey1);
	BOOST_CHECK(vScript == vScriptData1);
	BOOST_CHECK_EQUAL(nValidHeight,101);
	//delete script from db
	BOOST_CHECK(pTestView->EraseScriptData(regScriptId, vScriptKey, operlog));
	vKey.clear();
	vScript.clear();
	BOOST_CHECK(pTestView->GetScriptData(curheight,regScriptId, 0, vKey, vScript, nValidHeight, setOperLog));
	BOOST_CHECK(vKey == vScriptKey1);
	BOOST_CHECK(vScript == vScriptData1);
	BOOST_CHECK_EQUAL(nValidHeight,101);
	BOOST_CHECK(pTestView->GetScriptDataCount(regScriptId, nCount));
	BOOST_CHECK_EQUAL(nCount, 1);
	//write all data in caches to db
	BOOST_CHECK(pTestView->Flush());
}

BOOST_AUTO_TEST_SUITE(scriptdb_test)
BOOST_AUTO_TEST_CASE(test)
{
//	BOOST_ERROR("THE SUITE NEED TO MODIFY!");
	init();
	testscriptdb();
	testscriptdatadb();
	closedb();
}
BOOST_AUTO_TEST_SUITE_END()
