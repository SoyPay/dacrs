#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "init.h"
#include "miner.h"
#include "uint256.h"
#include "util.h"
#include <boost/test/unit_test.hpp>
#include "rpc/rpcclient.h"
#include "tx.h"
#include "wallet/wallet.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "vm/script.h"
#include "rpc/rpcserver.h"
#include "noui.h"
#include "ui_interface.h"
#include "systestbase.h"
#include <boost/algorithm/string/predicate.hpp>
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_reader.h"
#include "json/json_spirit_writer.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_stream_reader.h"
#include "tx.h"
using namespace std;
using namespace boost;
std::string g_strTxHash("");
const int g_knNewAddrs = 1420;

class CSysScriptTest : public SysTestBase {
 public:
	CSysScriptTest() {
		StartServer();
	}

	~CSysScriptTest() {
		StopServer();
	}

 private:
	void StartServer() {
	}

	void StopServer() {
	}

 public:
	uint64_t GetValue(Value val, string strCompare) {
		if (val.type() != obj_type) {
			return 0;
		}
		json_spirit::Value::Object obj = val.get_obj();
		string strRet;
		for (size_t i = 0; i < obj.size(); ++i) {
			const json_spirit::Pair& pair = obj[i];
			const std::string& str_name = pair.name_;
			const json_spirit::Value& val_val = pair.value_;

			if (str_name.compare(strCompare) == 0) {
				return val_val.get_int64();
			}

			if (strCompare == "value") {
				if (str_name == "FreedomFund") {
					json_spirit::Value::Array narray = val_val.get_array();
					if (narray.size() == 0)
						return false;
					json_spirit::Value::Object obj1 = narray[0].get_obj();
					for (size_t j = 0; j < obj1.size(); ++j) {
						const json_spirit::Pair& pair = obj1[j];
						const std::string& str_name = pair.name_;
						const json_spirit::Value& val_val = pair.value_;
						if (str_name == "value") {
							return val_val.get_int64();
						}
					}

				}
			}

		}
		return 0;
	}

	void CheckSdk() {
		string strParam = "01";
		Value resut = CreateContractTx("010000000100", "e21rEzVwkPFQYfgxcg7xLp7DKeYrW4Fpoz", strParam, 10, 10000000);
		BOOST_CHECK(GetHashFromCreatedTx(resut, g_strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		uint256 cHash(uint256S(g_strTxHash.c_str()));
		strParam = "02";
		strParam += HexStr(cHash);
		string strTemp;
		resut = CreateContractTx("010000000100", "e21rEzVwkPFQYfgxcg7xLp7DKeYrW4Fpoz", strParam, 10, 10000000);
		BOOST_CHECK(GetHashFromCreatedTx(resut, strTemp));
		BOOST_CHECK(GenerateOneBlock());

		strParam = "03";
		resut = CreateContractTx("010000000100", "e21rEzVwkPFQYfgxcg7xLp7DKeYrW4Fpoz", strParam, 10, 10000000);
		BOOST_CHECK(GetHashFromCreatedTx(resut, strTemp));
		BOOST_CHECK(GenerateOneBlock());

		strParam = "05";
		strParam += HexStr(cHash);

		resut = CreateContractTx("010000000100", "e21rEzVwkPFQYfgxcg7xLp7DKeYrW4Fpoz", strParam, 10, 10000000);
		BOOST_CHECK(GetHashFromCreatedTx(resut, strTemp));
		BOOST_CHECK(GenerateOneBlock());
	}

	string CreateRegScript(const char* strAddr, const char* sourceCode) {
		int nFee = 1 * COIN + 10000000;
		string strTxHash;
		string strFileName(sourceCode);
		Value valueRes = RegisterAppTx(strAddr, strFileName, 100, nFee);
		BOOST_CHECK(GetHashFromCreatedTx(valueRes, strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		return strTxHash;
	}

	string CreateContactTx(int param) {
		char arrchBuffer[3] = { 0 };
		sprintf(arrchBuffer, "%02x", param);
		string strTemp;
		// Value resut =CreateContractTx("010000000100", "5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d", buffer,10);
		Value resut = CreateContractTx("010000000100", "ddMuEBkAwhcb5K5QJ83MqQHrgHRn4EbRdh", arrchBuffer, 10, 1 * COIN);
		BOOST_CHECK(GetHashFromCreatedTx(resut, strTemp));
		BOOST_CHECK(GenerateOneBlock());
		return strTemp;
	}

	void CheckRollBack() {
		CreateContactTx(6);    //新增脚本数据
		//cout<<6<<endl;
		CreateContactTx(7);;   //修改脚本数据
		//cout<<7<<endl;
		CreateContactTx(8);    //删除脚本数据
		//		cout<<8<<endl;
		//		DisConnectBlock(1);           //删除1个block
		//		g_cTxMemPool.mapTx.clear();
		//		CreateContactTx(9);    //check删除的脚本是否恢复
		//		cout<<9<<endl;
		//		DisConnectBlock(2);
		//		g_cTxMemPool.mapTx.clear();
		//		CreateContactTx(10);    //check修改的脚本数据是否恢复
		//		cout<<10<<endl;
		//		DisConnectBlock(2);
		//		g_cTxMemPool.mapTx.clear();
		//		CreateContactTx(11);   //check新增的脚本数据是否恢复
	}

	bool CheckScriptid(Value val, string strScriptid) {
		if (val.type() != obj_type) {
			return false;
		}
		const Value& value = val.get_obj();

		json_spirit::Value::Object obj = value.get_obj();
		for (size_t i = 0; i < obj.size(); ++i) {
			const json_spirit::Pair& pair = obj[i];
			const std::string& str_name = pair.name_;
			const json_spirit::Value& val_val = pair.value_;
			if (str_name == "PublicKey") {
				if (val_val.get_str() != "") {
					return false;
				}
			} else if (str_name == "KeyID") {
				CRegID cRegId(strScriptid);
				CKeyID cKeyId = Hash160(cRegId.GetVec6());
				string strKey = HexStr(cKeyId.begin(), cKeyId.end()).c_str();
				if (val_val.get_str() != strKey) {
					return false;
				}
			}

		}
		return true;
	}

	bool CreateScriptAndCheck() {
		CreateRegScript("dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem", "unit_test.bin");
		CreateRegScript("dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem", "unit_test.bin");
		string strRegID = "010000000100";
		Value temp1 = GetAccountInfo(strRegID);
		if (!CheckScriptid(temp1, strRegID)) {
			return false;
		}
		strRegID = "020000000100";
		temp1 = GetAccountInfo(strRegID);
		if (!CheckScriptid(temp1, strRegID)) {
			return false;
		}
		return true;
	}

	void CheckScriptAccount() {
		// 检查脚本账户创建的合法性
		BOOST_CHECK_EQUAL(CreateScriptAndCheck(), true);

		//// 给脚本账户打钱
		string strAccountid = "010000000100";
		int nParam = 13;
		string strTemp = "";
		strTemp += tinyformat::format("%02x%s", nParam, strAccountid);
		Value resut = CreateContractTx("010000000100", "dsGb9GyDGYnnHdjSvRfYbj9ox2zPbtgtpo", strTemp, 10, 100000000,
				10000);
		BOOST_CHECK(GetHashFromCreatedTx(resut, strTemp));

		BOOST_CHECK(SetAddrGenerteBlock("dggsWmQ7jH46dgtA5dEZ9bhFSAK1LASALw"));
		Value temp1 = GetAccountInfo("010000000100");
		temp1 = GetAccountInfo("dsGb9GyDGYnnHdjSvRfYbj9ox2zPbtgtpo");
		BOOST_CHECK_EQUAL(GetValue(temp1, "Balance"), 99999999899990000);

		/// 脚本账户给普通账户打钱
		nParam = 14;
		strTemp = "";
		strTemp += tinyformat::format("%02x%s", nParam, strAccountid);
		resut = CreateContractTx("010000000100", "dsGb9GyDGYnnHdjSvRfYbj9ox2zPbtgtpo", strTemp, 10, 100000000);
		BOOST_CHECK(GetHashFromCreatedTx(resut, strTemp));

		BOOST_CHECK(SetAddrGenerteBlock("dps9hqUmBAVGVg7ijLGPcD9CJz9HHiTw6H"));
		temp1 = GetAccountInfo("010000000100");
		BOOST_CHECK_EQUAL(GetValue(temp1, "Balance"), 0);
		temp1 = GetAccountInfo("dsGb9GyDGYnnHdjSvRfYbj9ox2zPbtgtpo");
		BOOST_CHECK_EQUAL(GetValue(temp1, "Balance"), 99999999800000000);

		//测试不能从其他脚本打钱到本APP脚本账户中
		strAccountid = "020000000100";
		nParam = 19;
		strTemp = "";
		strTemp += tinyformat::format("%02x%s", nParam, strAccountid);
		resut = CreateContractTx("010000000100", "dsGb9GyDGYnnHdjSvRfYbj9ox2zPbtgtpo", strTemp, 10);
		BOOST_CHECK(!GetHashFromCreatedTx(resut, strTemp));
	}

	void GetScriptDataSize() {
		const char *pParam[] = { "rpctest", "getscriptdbsize", "010000000100" };
		//		CommandLineRPC(3, param);
		Value dummy;
		CommandLineRPC_GetValue(3, pParam, dummy);
	}

	bool CheckScriptDB(int nheigh, string strSrcipt, string strHash, int flag) {
		int nCurtiph = g_cChainActive.Height();
		string strHash2 = "hash";

		CRegID cRegid(strSrcipt);
		if (cRegid.IsEmpty() == true) {
			return false;
		}

		CScriptDBViewCache cContractScriptTemp(*g_pScriptDBTip, true);
		if (!cContractScriptTemp.HaveScript(cRegid)) {
			return false;
		}
		int nDbsize;
		cContractScriptTemp.GetScriptDataCount(cRegid, nDbsize);
		if (nCurtiph < nheigh) {
			BOOST_CHECK(0 == nDbsize);
			return true;
		}
		BOOST_CHECK(1000 == nDbsize);

		vector<unsigned char> vuchValue;
		vector<unsigned char> vuchScriptKey;

		if (!cContractScriptTemp.GetScriptData(nCurtiph, cRegid, 0, vuchScriptKey, vuchValue)) {
			return false;
		}
		uint256 cHash1(vuchValue);
		string strPvalue(vuchValue.begin(), vuchValue.end());
		if (flag) {
			BOOST_CHECK(strHash == cHash1.GetHex() || strPvalue == strHash2);
		} else {
			BOOST_CHECK(strHash == cHash1.GetHex());
		}
		unsigned short usKey = 0;
		memcpy(&usKey, &vuchScriptKey.at(0), sizeof(usKey));

		int nCount = nDbsize - 1;
		while (nCount--) {
			if (!cContractScriptTemp.GetScriptData(nCurtiph, cRegid, 1, vuchScriptKey, vuchValue)) {
				return false;
			}
			uint256 cHash3(vuchValue);
			string pvalue(vuchValue.begin(), vuchValue.end());
			if (flag) {
				BOOST_CHECK(strHash == cHash3.GetHex() || pvalue == strHash2);
			} else {
				BOOST_CHECK(strHash == cHash1.GetHex());
			}
		}
		return true;
	}

	void CreateTx(string strContact, string strAddr) {
		string strTemp = strAddr;
		Value resut = CreateContractTx("010000000100", strTemp, strContact, 10, 1 * COIN);
		string strReturn;
		BOOST_CHECK(GetHashFromCreatedTx(resut, strReturn));
		BOOST_CHECK(GenerateOneBlock());
		return;
	}

	bool GetScriptData(string strSrcipt, vector<unsigned char> vuchKey) {
		CRegID cRegid(strSrcipt);
		if (cRegid.IsEmpty() == true) {
			return false;
		}
		CScriptDBViewCache cContractScriptTemp(*g_pScriptDBTip, true);
		if (!cContractScriptTemp.HaveScript(cRegid)) {
			return false;
		}
		vector<unsigned char> vuchValue;
		int nTipH = g_cChainActive.Height();
		CScriptDBOperLog cOperLog;
		if (!cContractScriptTemp.GetScriptData(nTipH, cRegid, vuchKey, vuchValue)) {
			return false;
		}
		return true;
	}

	int GetScriptSize(string strSrcipt) {
		CRegID cRegid(strSrcipt);
		if (cRegid.IsEmpty() == true) {
			return 0;
		}

		if (!g_pScriptDBTip->HaveScript(cRegid)) {
			return 0;
		}
		int nDBsize;
		g_pScriptDBTip->GetScriptDataCount(cRegid, nDBsize);
		return nDBsize;
	}

	string CreatWriteTx(string &strHash) {
		string strShash = CreateRegScript("dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem", "unit_test.bin");
		string strScriptid;
		GetTxConfirmedRegID(strShash, strScriptid);
		BOOST_CHECK(strScriptid != "");
		//// first tx
		string strPhash = CreateContactTx(15);
		int nHeight = g_cChainActive.Height();

		BOOST_CHECK(CheckScriptDB(nHeight, strScriptid, strPhash, false));

		strHash = strPhash;
		return strScriptid;
	}

	void testdb() {
		string strPhash = "";
		string strScriptid = CreatWriteTx(strPhash);
		int nHeight = g_cChainActive.Height();
		int nCircle = 4;
		while (nCircle--) {
			BOOST_CHECK(GenerateOneBlock());
		}

		int nCount = 15;
		while (nCount > 1) {
			//// second tx
			uint256 cHash(uint256S(strPhash.c_str()));
			int nParam = 16;
			string strTemp = "";
			strTemp += tinyformat::format("%02x%s%02x", nParam, HexStr(cHash), nHeight);
			//	cout<<"cont:"<<strTemp<<endl;
			CreateTx(strTemp, "e21rEzVwkPFQYfgxcg7xLp7DKeYrW4Fpoz");

			vector<unsigned char> vuchKey;
			const char *pkszKey1 = "2_error";
			vuchKey.insert(vuchKey.begin(), pkszKey1, pkszKey1 + strlen(pkszKey1) + 1);
			BOOST_CHECK(!GetScriptData(strScriptid, vuchKey));

			CheckScriptDB((nHeight), strScriptid, strPhash, false);
			nCount--;
		}

		while (true) {
			DisConnectBlock(1);
			CheckScriptDB(nHeight, strScriptid, strPhash, false);
			nCount = GetScriptSize(strScriptid);
			if (nCount == 1000) {
				break;
			}
		}

	}

	void testdeletmodifydb() {
		string strWriteTxHash = "";
		string strScriptid = CreatWriteTx(strWriteTxHash);
		int nHeight = g_cChainActive.Height();

		///// 修改删除包
		int nParam = 17;
		string strTemp = "";
		strTemp += tinyformat::format("%02x%02x", nParam, 11);
		CreateTx(strTemp, "e21rEzVwkPFQYfgxcg7xLp7DKeYrW4Fpoz");
		vector<unsigned char> vuchKey;
		const char *pkszkey1 = "3_error";
		vuchKey.insert(vuchKey.begin(), pkszkey1, pkszkey1 + strlen(pkszkey1) + 1);
		BOOST_CHECK(!GetScriptData(strScriptid, vuchKey));
		CheckScriptDB(nHeight, strScriptid, strWriteTxHash, true);
		int nModHeight = g_cChainActive.Height();

		//	cout<<"end:"<<endl;
		//// 遍历
		int nCount = 15;
		while (nCount > 1) {
			int nParam = 18;
			string strTemp = "";
			strTemp += tinyformat::format("%02x", nParam);
			CreateTx(strTemp, "e21rEzVwkPFQYfgxcg7xLp7DKeYrW4Fpoz");
			//  		cout<<"cont:"<<endl;
			// 		cout<<g_cChainActive.Height()<<endl;
			CheckScriptDB(nHeight, strScriptid, strWriteTxHash, true);
			nCount--;
		}

		/// 回滚
		while (true) {
			DisConnectBlock(1);
			nCount = GetScriptSize(strScriptid);
			if (g_cChainActive.Height() > nModHeight) {
				CheckScriptDB(nHeight, strScriptid, strWriteTxHash, true);
			} else {
				CheckScriptDB(nHeight, strScriptid, strWriteTxHash, false);
			}
			if (nCount == 1000)
				break;
		}
	}

	void TestMinner() {
		string strHash = "";
		vector<string> vstrNewAddress;
		string strAddress("0-8");
		for (int i = 0; i < g_knNewAddrs; i++) {
			string strNewAddr;
			BOOST_CHECK(GetNewAddr(strNewAddr, false));
			vstrNewAddress.push_back(strNewAddr);
			uint64_t ullMoney = 10000 * COIN;
			if (i == 800) {
				strAddress = "0-7";
			}
			Value value = CreateNormalTx(strAddress, strNewAddr, ullMoney);
			BOOST_CHECK(GetHashFromCreatedTx(value, strHash));
		}
		cout << "create new address completed!" << endl;
		while (!IsMemoryPoolEmpty()) {
			BOOST_CHECK(GenerateOneBlock());
		}
		cout << "new transation have been confirmed, current height:" << g_cChainActive.Height() << endl;
		for (size_t i = 0; i < vstrNewAddress.size(); i++) {
			int nfee = GetRandomFee();
			Value value1 = RegistAccountTx(vstrNewAddress[i], nfee);
			BOOST_CHECK(GetHashFromCreatedTx(value1, strHash));

			//	BOOST_CHECK(GenerateOneBlock());
		}
		cout << "new address register account transactions have been created completed!" << endl;
		while (!IsMemoryPoolEmpty()) {
			BOOST_CHECK(GenerateOneBlock());
		}
		int nTotalhigh = 20;
		while (g_cChainActive.Height() != nTotalhigh) {
			BOOST_CHECK(GenerateOneBlock());
			ShowProgress("GenerateOneBlock progress: ", ((float) g_cChainActive.Height() / (float) nTotalhigh) * 100);
			MilliSleep(15);
		}

		//		BOOST_CHECK(DisConnectBlock(g_cChainActive.Height()-1));
		//		BOOST_CHECK(GenerateOneBlock());
		//		BOOST_CHECK(GenerateOneBlock());
	}
};


BOOST_FIXTURE_TEST_SUITE(sysScript_test,CSysScriptTest)

BOOST_FIXTURE_TEST_CASE(script_test,CSysScriptTest) {
#if 0

	//// pass
	ResetEnv();
	BOOST_CHECK(0==g_cChainActive.Height());
	CreateRegScript("dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem","unit_test.bin");
	CheckSdk();

	ResetEnv();
	BOOST_CHECK(0==g_cChainActive.Height());
	CreateRegScript("dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem","unit_test.bin");
	CheckRollBack();

	//// pass
	ResetEnv();
	BOOST_CHECK(0==g_cChainActive.Height());
	CheckScriptAccount();

	ResetEnv();
	BOOST_CHECK(0==g_cChainActive.Height());
	testdb();

	ResetEnv();
	BOOST_CHECK(0==g_cChainActive.Height());
	testdeletmodifydb();

#endif //0
}

// 测试各种地址挖矿
BOOST_FIXTURE_TEST_CASE(minier,CSysScriptTest) {
#if 0
	ResetEnv();
	TestMinner();
#endif //0
}

BOOST_FIXTURE_TEST_CASE(appacc,CSysScriptTest){
#if 0
	ResetEnv();
	BOOST_CHECK(0==g_cChainActive.Height());
	string strShash = CreateRegScript("dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem","unit_test.bin");
	string sriptid ="";
	BOOST_CHECK(GetTxConfirmedRegID(strShash,sriptid));
	int strTemp = 22;

	string param = strprintf("%02x",strTemp);
	string strHash ="";
	uint64_t nMoney = 1000000000;
	Value resut =CreateContractTx(sriptid, "e21rEzVwkPFQYfgxcg7xLp7DKeYrW4Fpoz", param,10,10000000,nMoney);
	BOOST_CHECK(GetHashFromCreatedTx(resut,strHash));
	BOOST_CHECK(GenerateOneBlock());

	nMoney = nMoney/5;
	vector<unsigned char> vtemp;
	vtemp.assign((char*)&nMoney,(char*)&nMoney+sizeof(nMoney));
	for(int i = 1;i<5;i++){
		strTemp += 1;
		param = strprintf("%02x%s",strTemp,HexStr(vtemp));
		//cout<<i<<endl;
		resut =CreateContractTx(sriptid, "e21rEzVwkPFQYfgxcg7xLp7DKeYrW4Fpoz", param,10,10000000,0);
		BOOST_CHECK(GetHashFromCreatedTx(resut,strHash));
		BOOST_CHECK(GenerateOneBlock());
	}

	strTemp += 1;
	param = strprintf("%02x%s",strTemp,HexStr(vtemp));
	resut =CreateContractTx(sriptid, "e21rEzVwkPFQYfgxcg7xLp7DKeYrW4Fpoz", param,10,10000000,0);
	BOOST_CHECK(GetHashFromCreatedTx(resut,strHash));
	BOOST_CHECK(GenerateOneBlock());

	strTemp += 1;
	param = strprintf("%02x%s",strTemp,HexStr(vtemp));
	resut =CreateContractTx(sriptid, "e21rEzVwkPFQYfgxcg7xLp7DKeYrW4Fpoz", param,10,10000000,0);
	BOOST_CHECK(!GetHashFromCreatedTx(resut,strHash));


	BOOST_CHECK(DisConnectBlock(5));



	CScriptDBViewCache cContractScriptTemp(*g_pScriptDBTip, true);
	CRegID script(sriptid);
	CRegID strreg;
	string address = "e21rEzVwkPFQYfgxcg7xLp7DKeYrW4Fpoz";

	BOOST_CHECK(SysTestBase::GetRegID(address,strreg));
	std::shared_ptr<CAppUserAccout> tem = std::make_shared<CAppUserAccout>();
	cContractScriptTemp.GetScriptAcc(script,strreg.GetVec6(),*tem.get());
	BOOST_CHECK(tem.get()->getllValues() == nMoney);
#endif //0
}

BOOST_AUTO_TEST_SUITE_END()

