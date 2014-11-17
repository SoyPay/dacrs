#include "main.h"
#include "txdb.h"
#include "account.h"
#include <iostream>
#include <boost/test/unit_test.hpp>

using namespace std;

void init() {
	 pScriptDB = new CScriptDB(size_t(4<<20), false , Params().IsReindex());
	 pScriptDBTip = new CScriptDBViewCache(*pScriptDB, false);
}

void testscriptdb() {
	vector<unsigned char> vScriptId = {0x01,0x00,0x00,0x00,0x01,0x00};
	vector<unsigned char> vScriptId1 = {0x01,0x00,0x00,0x00,0x02,0x00};
	vector<unsigned char> vScriptContent = {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
	vector<unsigned char> vScriptContent1 = {0x01,0x02,0x03,0x04,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
	//write script content to db
	BOOST_CHECK(pScriptDBTip->SetScript(vScriptId, vScriptContent));
	BOOST_CHECK(pScriptDBTip->SetScript(vScriptId1, vScriptContent1));
	//write all data in caches to db
	BOOST_CHECK(pScriptDBTip->Flush());
	//test if the script id is exist in db
	BOOST_CHECK(pScriptDBTip->HaveScript(vScriptId));
	vector<unsigned char> vScript;
	//read script content from db by scriptId
	BOOST_CHECK(pScriptDBTip->GetScript(vScriptId,vScript));
	// if the readed script content equals with original
	BOOST_CHECK(vScriptContent == vScript);
	int nCount;
	//get script numbers from db
	BOOST_CHECK(pScriptDBTip->GetScriptCount(nCount));
	//if the number is one
	BOOST_CHECK_EQUAL(nCount, 2);
	//get index 0 script from db
	vScript.clear();
	vector<unsigned char> vId;
	BOOST_CHECK(pScriptDBTip->GetScript(0,vId,vScript));
	BOOST_CHECK(vScriptId == vId);
	BOOST_CHECK(vScriptContent == vScript);
	BOOST_CHECK(pScriptDBTip->GetScript(1,vId,vScript));
	BOOST_CHECK(vScriptId1 == vId);
	BOOST_CHECK(vScriptContent1 == vScript);
	//delete script from db
	BOOST_CHECK(pScriptDBTip->EraseScript(vScriptId));
	BOOST_CHECK(pScriptDBTip->GetScriptCount(nCount));
	BOOST_CHECK_EQUAL(nCount, 1);
	//write all data in caches to db
	BOOST_CHECK(pScriptDBTip->Flush());
}


void testscriptdatadb() {
	vector<unsigned char> vScriptId = {0x01,0x00,0x00,0x00,0x02,0x00};
//  vector<unsigned char> vScriptKey = {0x01,0x00,0x02,0x03,0x04,0x05,0x06,0x07};
//	vector<unsigned char> vScriptKey1 = {0x01,0x00,0x02,0x03,0x04,0x05,0x07,0x06};
	vector<unsigned char> vScriptKey = {0x01,0x00,0x02};
	vector<unsigned char> vScriptKey1 = {0x01,0x00,0x02,0x03};
	vector<unsigned char> vScriptData = {0x01,0x01,0x01,0x01,0x01};
	vector<unsigned char> vScriptData1 = {0x01,0x01,0x01,0x00,0x00};
	CScriptDBOperLog operlog;
	BOOST_CHECK(pScriptDBTip->SetScriptData(vScriptId, vScriptKey, vScriptData, 100, operlog));
	int height = 0;
	BOOST_CHECK(pScriptDBTip->GetScriptData(vScriptId,vScriptKey,vScriptData,height));
	pScriptDBTip->GetScriptCount(height);
	BOOST_CHECK(pScriptDBTip->GetScriptData(vScriptId, 0, vScriptKey, vScriptData, height));
	BOOST_CHECK(pScriptDBTip->SetScriptData(vScriptId, vScriptKey, vScriptData, 100, operlog));

	//write script data to db
	BOOST_CHECK(pScriptDBTip->SetScriptData(vScriptId, vScriptKey, vScriptData, 100, operlog));
	//write all data in caches to db
//	BOOST_CHECK(pScriptDBTip->Flush());
	BOOST_CHECK(pScriptDBTip->SetScriptData(vScriptId, vScriptKey1, vScriptData1, 101, operlog));
	//test if the script id is exist in db
	BOOST_CHECK(pScriptDBTip->HaveScriptData(vScriptId, vScriptKey));
	vector<unsigned char> vScript;
	int nValidHeight;
	//read script content from db by scriptId
	BOOST_CHECK(pScriptDBTip->GetScriptData(vScriptId, vScriptKey, vScript, nValidHeight));
	// if the readed script content equals with original
	BOOST_CHECK(vScriptData == vScript);
	BOOST_CHECK_EQUAL(nValidHeight, 100);
	int nCount;
	//get script numbers from db
	BOOST_CHECK(pScriptDBTip->GetScriptDataCount(vScriptId, nCount));
	//if the number is one
	BOOST_CHECK_EQUAL(nCount, 2);
	//get index 0 script from db
	vScript.clear();
	vector<unsigned char> vKey;
	vKey.clear();
	nValidHeight = 0;
	BOOST_CHECK(pScriptDBTip->GetScriptData(vScriptId, 0, vKey, vScript, nValidHeight));
	BOOST_CHECK(vKey == vScriptKey);
	BOOST_CHECK(vScript == vScriptData);
	BOOST_CHECK_EQUAL(nValidHeight,100);
	BOOST_CHECK(pScriptDBTip->GetScriptData(vScriptId, 1, vKey, vScript, nValidHeight));
	BOOST_CHECK(vKey == vScriptKey1);
	BOOST_CHECK(vScript == vScriptData1);
	BOOST_CHECK_EQUAL(nValidHeight,101);
	//delete script from db
	BOOST_CHECK(pScriptDBTip->EraseScriptData(vScriptId, vScriptKey, operlog));
	vKey.clear();
	vScript.clear();
	BOOST_CHECK(pScriptDBTip->GetScriptData(vScriptId, 0, vKey, vScript, nValidHeight));
	BOOST_CHECK(vKey == vScriptKey1);
	BOOST_CHECK(vScript == vScriptData1);
	BOOST_CHECK_EQUAL(nValidHeight,101);
	BOOST_CHECK(pScriptDBTip->GetScriptDataCount(vScriptId, nCount));
	BOOST_CHECK_EQUAL(nCount, 1);
	//write all data in caches to db
	BOOST_CHECK(pScriptDBTip->Flush());
}

BOOST_AUTO_TEST_SUITE(scriptdb_test)
BOOST_AUTO_TEST_CASE(test)
{
	init();
	testscriptdb();
	testscriptdatadb();
}
BOOST_AUTO_TEST_SUITE_END()
