#include "rpcclient.h"
#include "util.h"
#include <boost/test/unit_test.hpp>
#include "txdb.h"
#include "account.h"
#include <iostream>
using namespace std;
vector<string> g_vScriptID;
void ReadData() {
	auto_ptr<CAccountViewDB> pAccountViewDB(new CAccountViewDB(1000, false, false));
	auto_ptr<CAccountViewCache> pAccountViewTip(new CAccountViewCache(*pAccountViewDB));
	CAccountViewCache view(*pAccountViewTip, true);
	map<string, CContractScript> &mapScript = pContractScriptTip->GetScriptCache();
	map<string, CContractScript>::iterator iterScript = mapScript.begin();
	for (; iterScript != mapScript.end(); ++iterScript) {
		g_vScriptID.push_back(iterScript->first);
		//cout<<"RegScriptId:"<<iterScript->first.c_str()<<endl;
		//cout<<"RegScriptContent:"<<HexStr(iterScript->second.scriptContent.begin(), iterScript->second.scriptContent.end())<<endl;

		set<string>::iterator iterArbit = iterScript->second.setArbitratorAccId.begin();
		for (; iterArbit != iterScript->second.setArbitratorAccId.end(); ++iterArbit) {
			CKeyID keyId;
			view.GetKeyId(ParseHex(*iterArbit), keyId);
			//cout<<"RegId:"<<(*iterArbit).c_str()<<endl;
			//cout<<"KeyId:"<<keyId.GetHex()<<endl;
		}
	}
}

void GenerateMinerEx() {
	//cout <<"Generate miner" << endl;
	int argc = 3;
	char *argv[3] = { "rpctest", "setgenerate", "true" };
	CommandLineRPC(argc, argv);
}

void CreateRegScriptTxEx() {
	//cout <<"CreateRegScriptTx" << endl;
	int argc = 6;
	char *argv[6] = { "rpctest", "registerscripttx", "5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9", "321654hddf",
			"1000000", "10" };
	CommandLineRPC(argc, argv);
}
BOOST_AUTO_TEST_SUITE(scriptdb_test)

#if 0

BOOST_AUTO_TEST_CASE(scriptdb_Mine)
{
	//cout<<"start mine"<<endl;
	GenerateMinerEx();

}

BOOST_AUTO_TEST_CASE(scriptdb_createregtx)
{
	//cout<<"Create RegScript"<<endl;
	CreateRegScriptTxEx();
}

BOOST_AUTO_TEST_CASE(scriptdb_readwrite)
{

	map<string, CContractScript> &mapScript = pContractScriptTip->GetScriptCache();
	BOOST_CHECK(mapScript.empty() );
	auto_ptr<CScriptDB> pScriptDB(new CScriptDB(1000,false,false) );
	BOOST_CHECK(NULL !=pScriptDB.get() && pScriptDB->LoadRegScript(mapScript) );
	ReadData();

	//set data
	string strData("654321ab");
	set<string> setData;
	setData.insert(string("arbitrator"));
	for(auto& item:g_vScriptID) {
		vector<unsigned char> scriptID;
		scriptID = ParseHex(item.c_str() );
		vector<unsigned char> scriptContent;
		scriptContent = ParseHex(strData);
		pScriptDB->SetScript(scriptID,scriptContent);
		pScriptDB->SetArbitrator(scriptID,setData);
	}

	BOOST_CHECK(pScriptDB->Flush(mapScript) );

	//get and compare
	mapScript.clear();
	BOOST_CHECK(NULL !=pScriptDB.get() && pScriptDB->LoadRegScript(mapScript) );
	for (auto& item : g_vScriptID) {
		vector<unsigned char> scriptID;
		scriptID = ParseHex(item.c_str());
		vector<unsigned char> scriptContent;
		pScriptDB->GetScript(scriptID, scriptContent);

		set<string> getData;
		pScriptDB->GetArbitrator(scriptID,getData);
		BOOST_CHECK(strData == string(HexStr(scriptContent)));
		BOOST_CHECK(setData == getData);
	}

	//erase
	map<string, CContractScript>::iterator iterScript = mapScript.begin();
	for (; iterScript != mapScript.end(); ++iterScript) {
		//iterScript->second.scriptId.clear();
		vector<unsigned char> scriptID;
		scriptID = ParseHex(iterScript->first);
		BOOST_CHECK(pScriptDB->EraseScript(scriptID));
	}

	BOOST_CHECK(pScriptDB->Flush(mapScript) );
	mapScript.clear();
	BOOST_CHECK(NULL !=pScriptDB.get() && pScriptDB->LoadRegScript(mapScript) );
	BOOST_CHECK(mapScript.empty() );
	ReadData();
}

#else
BOOST_AUTO_TEST_CASE(xxxx) {
	BOOST_ERROR("ERROR:THE SUITE NEED TO MODIFY!");
}
#endif

BOOST_AUTO_TEST_SUITE_END()
