#include "main.h"
#include "txdb.h"
#include "database.h"
#include <iostream>
#include <boost/test/unit_test.hpp>
#include  "boost/filesystem/operations.hpp"
#include  "boost/filesystem/path.hpp"
using namespace std;
CScriptDBViewCache *g_pcScriptDBView = NULL;
CScriptDBViewCache *g_pcTestView = NULL;
CScriptDB *g_pcTestDB = NULL;
vector< vector<unsigned char> > g_arrKey;
vector<unsigned char> g_vuchKey1 = {0x01, 0x02, 0x01};
vector<unsigned char> g_vuchKey2 = {0x01, 0x02, 0x02};
vector<unsigned char> g_vuchKey3 = {0x01, 0x02, 0x03};
vector<unsigned char> g_vuchKey4 = {0x01, 0x02, 0x04};
vector<unsigned char> g_vuchKey5 = {0x01, 0x02, 0x05};
vector<unsigned char> g_vuchKey6 = {0x01, 0x02, 0x06};
vector<unsigned char> g_vuchKeyValue = {0x06, 0x07, 0x08};

int nCount = 0;

void init() {
	 g_pcTestDB = new CScriptDB("testdb",size_t(4<<20), false , true);
	 g_pcTestView =  new CScriptDBViewCache(*g_pcTestDB, false);
	 //穷举数据分别位于scriptDBView pTestView, pTestDB 三级分布数据测试数据库中
	 g_pcScriptDBView = new CScriptDBViewCache(*g_pcTestView, true);

	 g_arrKey.push_back(g_vuchKey1);
	 g_arrKey.push_back(g_vuchKey2);
	 g_arrKey.push_back(g_vuchKey3);
	 g_arrKey.push_back(g_vuchKey4);
	 g_arrKey.push_back(g_vuchKey5);
	 g_arrKey.push_back(g_vuchKey6);
}

void closedb() {
	if (g_pcTestView != NULL) {
		delete g_pcTestView;
		g_pcTestView = NULL;
	}
	if (g_pcTestDB != NULL) {
		delete g_pcTestDB;
		g_pcTestDB = NULL;
	}
	const boost::filesystem::path p = GetDataDir() / "blocks" / "testdb";
	boost::filesystem::remove_all(p);
}

void testscriptdb() {
	vector<unsigned char> vuchScriptId = {0x01,0x00,0x00,0x00,0x01,0x00};
	vector<unsigned char> vuchScriptId1 = {0x01,0x00,0x00,0x00,0x02,0x00};
	vector<unsigned char> vuchScriptContent = {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
	vector<unsigned char> vuchScriptContent1 = {0x01,0x02,0x03,0x04,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
	CRegID cRegScriptId(vuchScriptId);
	CRegID cRegScriptId1(vuchScriptId1);
	//write script content to db
	BOOST_CHECK(g_pcTestView->SetScript(cRegScriptId, vuchScriptContent));
	BOOST_CHECK(g_pcTestView->SetScript(cRegScriptId1, vuchScriptContent1));
	//write all data in caches to db
	BOOST_CHECK(g_pcTestView->Flush());
	//test if the script id is exist in db
	BOOST_CHECK(g_pcTestView->HaveScript(cRegScriptId));
	vector<unsigned char> vuchScript;
	//read script content from db by scriptId
	BOOST_CHECK(g_pcTestView->GetScript(cRegScriptId,vuchScript));
	// if the readed script content equals with original
	BOOST_CHECK(vuchScriptContent == vuchScript);
	int nCount;
	//get script numbers from db
	BOOST_CHECK(g_pcTestView->GetScriptCount(nCount));
	//if the number is one
	BOOST_CHECK_EQUAL(nCount, 2);
	//get index 0 script from db
	vuchScript.clear();
	vector<unsigned char> vuchId;

	CRegID cRegId;
	BOOST_CHECK(g_pcTestView->GetScript(0, cRegId, vuchScript));
	BOOST_CHECK(vuchScriptId == cRegId.GetVec6());
	BOOST_CHECK(vuchScriptContent == vuchScript);
	BOOST_CHECK(g_pcTestView->GetScript(1, cRegId, vuchScript));
	BOOST_CHECK(vuchScriptId1 == cRegId.GetVec6());
	BOOST_CHECK(vuchScriptContent1 == vuchScript);
	//delete script from db
	BOOST_CHECK(g_pcTestView->EraseScript(cRegScriptId));
	BOOST_CHECK(g_pcTestView->GetScriptCount(nCount));
	BOOST_CHECK_EQUAL(nCount, 1);
	//write all data in caches to db
	BOOST_CHECK(g_pcTestView->Flush());
}

void settodb(int nType, vector<unsigned char> &vuchKey, vector<unsigned char> &vuchScriptData) {
	vector<unsigned char> vuchScriptId = { 0x01, 0x00, 0x00, 0x00, 0x02, 0x00 };
	CScriptDBOperLog cOperlog;
	CRegID cRegScriptId(vuchScriptId);
	if (0 == nType) {
		BOOST_CHECK(g_pcScriptDBView->SetScriptData(cRegScriptId, vuchKey, vuchScriptData, cOperlog));
		BOOST_CHECK(g_pcScriptDBView->Flush());
		BOOST_CHECK(g_pcTestView->Flush());
	} else if (1 == nType) {
		BOOST_CHECK(g_pcScriptDBView->SetScriptData(cRegScriptId, vuchKey, vuchScriptData, cOperlog));
		BOOST_CHECK(g_pcScriptDBView->Flush());
	} else {
		BOOST_CHECK(g_pcScriptDBView->SetScriptData(cRegScriptId, vuchKey, vuchScriptData, cOperlog));
	}
}

void cleandb(int nType, vector<unsigned char> vuchKey) {
	vector<unsigned char> vuchScriptId = { 0x01, 0x00, 0x00, 0x00, 0x02, 0x00 };
	CRegID cRegScriptId(vuchScriptId);
	CScriptDBOperLog cOperlog;
	if (0 == nType) {
		BOOST_CHECK(g_pcScriptDBView->EraseScriptData(cRegScriptId, vuchKey, cOperlog));
		BOOST_CHECK(g_pcScriptDBView->Flush());
		BOOST_CHECK(g_pcTestView->Flush());
	} else if (1 == nType) {
		BOOST_CHECK(g_pcScriptDBView->EraseScriptData(cRegScriptId, vuchKey, cOperlog));
		BOOST_CHECK(g_pcScriptDBView->Flush());
	} else {
		BOOST_CHECK(g_pcScriptDBView->EraseScriptData(cRegScriptId, vuchKey, cOperlog));
	}
}

void traversaldb(CScriptDBViewCache *pScriptDB, bool needEqual) {
	assert(pScriptDB!=NULL);
	vector<vector<unsigned char> > vuchTraversalKey;
	// int nHeight(0);
	int nCurHeight(0);
	vector<unsigned char> vuchKey;
	vector<unsigned char> vuchScript;
	// int nValidHeight(0);
	vector<unsigned char> vuchScriptId = { 0x01, 0x00, 0x00, 0x00, 0x02, 0x00 };
	CRegID cRegScriptId(vuchScriptId);
	bool bRet = pScriptDB->GetScriptData(nCurHeight, cRegScriptId, 0, vuchKey, vuchScript);
	// int nType(0);
	if (bRet) {
		vuchTraversalKey.push_back(vuchKey);
		vector<unsigned char> vuchDataKey = { 'd', 'a', 't', 'a' };
		vuchDataKey.insert(vuchDataKey.end(), cRegScriptId.GetVec6().begin(), cRegScriptId.GetVec6().end());
		vuchDataKey.push_back('_');
		vuchDataKey.insert(vuchDataKey.end(), vuchKey.begin(), vuchKey.end());
		if (g_pcScriptDBView->m_mapDatas.count(vuchDataKey)) {
			// nType = 0;
		} else if (g_pcTestView->m_mapDatas.count(vuchDataKey)) {
			// nType = 1;
		} else {
			// nType = 2;
		}
			// cout << "script key:" << HexStr(vuchKey) <<" data at level:"<< nType<< endl;
	}

	while (bRet) {
		bRet = pScriptDB->GetScriptData(nCurHeight, cRegScriptId, 1, vuchKey, vuchScript);
		if (bRet) {
			vector<unsigned char> vuchDataKey = { 'd', 'a', 't', 'a' };
			vuchDataKey.insert(vuchDataKey.end(), cRegScriptId.GetVec6().begin(), cRegScriptId.GetVec6().end());
			vuchDataKey.push_back('_');
			vuchDataKey.insert(vuchDataKey.end(), vuchKey.begin(), vuchKey.end());
			if (g_pcScriptDBView->m_mapDatas.count(vuchDataKey)) {
				// nType = 0;
			} else if (g_pcTestView->m_mapDatas.count(vuchDataKey)) {
				// nType = 1;
			} else {
				// nType = 2;
			}
			vuchTraversalKey.push_back(vuchKey);
				// cout << "script key:" << HexStr(vuchKey) <<" data at level:"<< nType<< endl;
		}
	}
	if (needEqual) {
		BOOST_CHECK(vuchTraversalKey == g_arrKey);
	}
	// cout << "=======================traversaldb end======="<<++nCount<<"==============================="<<endl;
}

void testscriptdatadb() {
	vector<unsigned char> vuchScriptId = {0x01,0x00,0x00,0x00,0x02,0x00};
	// vector<unsigned char> vuchScriptKey = {0x01,0x00,0x02,0x03,0x04,0x05,0x06,0x07};
	// vector<unsigned char> vuchScriptKey1 = {0x01,0x00,0x02,0x03,0x04,0x05,0x07,0x06};
	vector<unsigned char> vuchScriptKey = {0x01,0x00,0x01};
	vector<unsigned char> vuchScriptKey1 = {0x01,0x00,0x02};
	vector<unsigned char> vuchScriptKey2 = {0x01,0x00,0x03};
	vector<unsigned char> vuchScriptKey3 = {0x01,0x00,0x04};
	vector<unsigned char> vuchScriptKey4 = {0x01,0x00,0x05};
	vector<unsigned char> vuchScriptKey5 = {0x01,0x00,0x06};
	vector<unsigned char> vuchScriptData = {0x01,0x01,0x01,0x01,0x01};
	vector<unsigned char> vScriptData1 = {0x01,0x01,0x01,0x00,0x00};
	CScriptDBOperLog cOperlog;
	CRegID cRegScriptId(vuchScriptId);

	//测试数据库中有vuchScriptKey1， vuchScriptKey3，在缓存中有vuchScriptKey， vuchScriptKey2，是否能正确遍历脚本数据库
	BOOST_CHECK(g_pcTestView->SetScriptData(cRegScriptId, vuchScriptKey1, vuchScriptData,cOperlog));
	BOOST_CHECK(g_pcTestView->SetScriptData(cRegScriptId, vuchScriptKey3, vuchScriptData, cOperlog));
	BOOST_CHECK(g_pcTestView->SetScriptData(cRegScriptId, vuchScriptKey5, vuchScriptData,cOperlog));
	BOOST_CHECK(g_pcTestView->Flush());
	BOOST_CHECK(g_pcTestView->SetScriptData(cRegScriptId, vuchScriptKey, vuchScriptData,  cOperlog));
	BOOST_CHECK(g_pcTestView->SetScriptData(cRegScriptId, vuchScriptKey2, vuchScriptData,  cOperlog));
	BOOST_CHECK(g_pcTestView->SetScriptData(cRegScriptId, vuchScriptKey4, vuchScriptData,  cOperlog));
	BOOST_CHECK(g_pcTestView->EraseScriptData(cRegScriptId, vuchScriptKey3, cOperlog));
	BOOST_CHECK(g_pcTestView->EraseScriptData(cRegScriptId, vuchScriptKey2, cOperlog));

	traversaldb(g_pcTestView, false);

	BOOST_CHECK(g_pcTestView->EraseScriptData(cRegScriptId, vuchScriptKey, cOperlog));
	BOOST_CHECK(g_pcTestView->EraseScriptData(cRegScriptId, vuchScriptKey1, cOperlog));
	BOOST_CHECK(g_pcTestView->EraseScriptData(cRegScriptId, vuchScriptKey4, cOperlog));
	BOOST_CHECK(g_pcTestView->EraseScriptData(cRegScriptId, vuchScriptKey5, cOperlog));
	BOOST_CHECK(g_pcTestView->Flush());

//	for(int a1=0; a1<3; ++a1) {
//		settodb(a1, vuchKey1, vuchKeyValue);
//		for(int a2=0; a2<3; ++a2) {
//			settodb(a2, vuchKey2, vuchKeyValue);
//			for(int a3=0; a3<3; ++a3) {
//				settodb(a3, vuchKey3, vuchKeyValue);
//				for(int a4=0; a4<3; ++a4) {
//					settodb(a4, vuchKey4, vuchKeyValue);
//					for(int a5=0; a5<3; ++a5) {
//						settodb(a5, vuchKey5, vuchKeyValue);
//						for(int a6=0; a6<3; ++a6){
//							settodb(a6, vuchKey6, vuchKeyValue);
//							traversaldb(pscriptDBView, true);
//							cleandb(a6, vuchKey6);
//						}
//						cleandb(a5, vuchKey5);
//					}
//					cleandb(a4, vuchKey4);
//				}
//				cleandb(a3, vuchKey3);
//			}
//			cleandb(a2, vuchKey2);
//		}
//		cleandb(a1, vuchKey1);
//	}
	for (int i = 0; i < 5; ++i) {
		for (int ii = i + 1; ii < 6; ++ii) {
			settodb(0, g_arrKey[i], g_vuchKeyValue);
			settodb(0, g_arrKey[ii], g_vuchKeyValue);
			for (int j = 0; j < 5; ++j) {
				for (int jj = j + 1; jj < 6; ++jj) {
					if (i != j && i != jj && ii != j && ii != jj) {
						settodb(1, g_arrKey[j], g_vuchKeyValue);
						settodb(1, g_arrKey[jj], g_vuchKeyValue);
						for (int k = 0; k < 5; ++k) {
							for (int kk = k + 1; kk < 6; ++kk) {
								if (k != i && k != ii && kk != i && kk != ii && k != j && k != jj && kk != j
										&& kk != jj) {
									settodb(2, g_arrKey[k], g_vuchKeyValue);
									settodb(2, g_arrKey[kk], g_vuchKeyValue);
									traversaldb(g_pcScriptDBView, true);
									cleandb(2, g_arrKey[k]);
									cleandb(2, g_arrKey[kk]);
								}
							}
						}
						cleandb(1, g_arrKey[j]);
						cleandb(1, g_arrKey[jj]);
					}
				}
			}
			cleandb(0, g_arrKey[i]);
			cleandb(0, g_arrKey[ii]);
		}
	}

	int nHeight(0);
	int nCurHeight(0);
	vector<unsigned char> vuchKey;
	vector<unsigned char> vuchScript;
	set<CScriptDBOperLog> setOperLog;

	BOOST_CHECK(g_pcTestView->SetScriptData(cRegScriptId, vuchScriptKey, vuchScriptData,  cOperlog));
	// int nHeight = 0;
	// int nCurHeight = 0;
	BOOST_CHECK(g_pcTestView->GetScriptData(nCurHeight,cRegScriptId,vuchScriptKey,vuchScriptData));
	g_pcTestView->GetScriptCount(nHeight);

	BOOST_CHECK(g_pcTestView->GetScriptData(nCurHeight,cRegScriptId, 0, vuchScriptKey, vuchScriptData));
	BOOST_CHECK(g_pcTestView->SetScriptData(cRegScriptId, vuchScriptKey, vuchScriptData, cOperlog));

	//write script data to db
	BOOST_CHECK(g_pcTestView->SetScriptData(cRegScriptId, vuchScriptKey, vuchScriptData,  cOperlog));
	//write all data in caches to db
	BOOST_CHECK(g_pcTestView->Flush());
	BOOST_CHECK(g_pcTestView->SetScriptData(cRegScriptId, vuchScriptKey1, vScriptData1, cOperlog));
	//test if the script id is exist in db
	BOOST_CHECK(g_pcTestView->HaveScriptData(cRegScriptId, vuchScriptKey));
	vuchScript.clear();


	//read script content from db by scriptId
	BOOST_CHECK(g_pcTestView->GetScriptData(nCurHeight,cRegScriptId, vuchScriptKey, vuchScript));
	// if the readed script content equals with original
	BOOST_CHECK(vuchScriptData == vuchScript);
	int nCount;
	//get script numbers from db
	BOOST_CHECK(g_pcTestView->GetScriptDataCount(cRegScriptId, nCount));
	//if the number is one
	BOOST_CHECK_EQUAL(nCount, 2);
	//get index 0 script from db
	vuchScript.clear();
	vuchKey.clear();
	BOOST_CHECK(g_pcTestView->GetScriptData(nCurHeight,cRegScriptId, 0, vuchKey, vuchScript));
	BOOST_CHECK(vuchKey == vuchScriptKey);
	BOOST_CHECK(vuchScript == vuchScriptData);
	BOOST_CHECK(g_pcTestView->GetScriptData(nCurHeight,cRegScriptId, 1, vuchKey, vuchScript));
	BOOST_CHECK(vuchKey == vuchScriptKey1);
	BOOST_CHECK(vuchScript == vScriptData1);
	//delete script from db
	BOOST_CHECK(g_pcTestView->EraseScriptData(cRegScriptId, vuchScriptKey, cOperlog));
	vuchKey.clear();
	vuchScript.clear();
	BOOST_CHECK(g_pcTestView->GetScriptData(nCurHeight,cRegScriptId, 0, vuchKey, vuchScript));
	BOOST_CHECK(vuchKey == vuchScriptKey1);
	BOOST_CHECK(vuchScript == vScriptData1);
	BOOST_CHECK(g_pcTestView->GetScriptDataCount(cRegScriptId, nCount));
	BOOST_CHECK_EQUAL(nCount, 1);
	//write all data in caches to db
	BOOST_CHECK(g_pcTestView->Flush());
}

BOOST_AUTO_TEST_SUITE(scriptdb_test)
BOOST_AUTO_TEST_CASE(test) {
	// BOOST_ERROR("THE SUITE NEED TO MODIFY!");
	init();
	testscriptdb();
	testscriptdatadb();
	closedb();
}
BOOST_AUTO_TEST_SUITE_END()
