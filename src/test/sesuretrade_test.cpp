#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "init.h"
#include "miner.h"
#include "uint256.h"
#include "util.h"
#include <boost/test/unit_test.hpp>
#include "rpcclient.h"
#include "tx.h"
#include "wallet.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "./VmScript/VmScript.h"
#include "rpcserver.h"
#include "noui.h"
#include "ui_interface.h"
#include "cuiserve.h"
#include <boost/algorithm/string/predicate.hpp>
using namespace std;
using namespace boost;

#define BUYER	"01"
#define SELLER	"02"
#define ARBIT	"03"

#define BUYER_ADDR 		"mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc"
#define SELLER_ADDR 	"mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y"
#define ARBIT_ADDR		"moZJZgsGFC4qvwRdjzS7Bj3fHrtpUfEVEE"

#define BUYER_ID 		"000000000200"
#define SELLER_ID 		"000000000300"
#define ARBIT_ID		"000000000400"

#define ACCOUNT_ID_SIZE 6
#define MAX_ARBITRATOR 3
#define HASH_SIZE		32

#pragma pack(1)
typedef struct tag_INT64 {
	unsigned char data[8];
} Int64;

typedef struct tagACCOUNT_ID
{
	char accounid[ACCOUNT_ID_SIZE];
}ACCOUNT_ID;

typedef struct  {
	unsigned char nType;					//!<ÀàÐÍ
	unsigned char nArbitratorCount;			//!<ÖÙ²ÃÕß¸öÊý
	ACCOUNT_ID 	buyer;						//!<Âò¼ÒID£¨²ÉÓÃ6×Ö½ÚµÄÕË»§ID£©
	ACCOUNT_ID seller;						//!<Âô¼ÒID£¨²ÉÓÃ6×Ö½ÚµÄÕË»§ID£©
	ACCOUNT_ID arbitrator[MAX_ARBITRATOR];	//!<ÖÙ²ÃÕßID£¨²ÉÓÃ6×Ö½ÚµÄÕË»§ID£©
	long nHeight;							//!<³¬Ê±¾ø¶Ô¸ß¶È
	Int64 nFineMoney;						//!<Âô¼ÒÎ¥Ô¼ºó×î´ó·£¿î½ð¶î
	Int64 nPayMoney;						//!<Âò¼ÒÏòÂô¼ÒÖ§¸¶µÄ½ð¶î
	Int64 nFee;								//!<ÖÙ²ÃÊÖÐø·Ñ
	Int64 ndeposit;							//!<ÖÙ²ÃÑº½ð,ÉêËßÊ±´ÓÖÙ²ÃÕß¿Û³ýµÄÑº½ð(Èç¹ûÖÙ²Ã²»ÏìÓ¦Ç®¹éÂò¼Ò·ñÔòÂò¼ÒÍË»¹¸øÖÙ²ÃÕß)
}FIRST_CONTRACT;

typedef struct {
	unsigned char nType;					//!<½»Ò×ÀàÐÍ
	unsigned char hash[HASH_SIZE];			//!<ÉÏÒ»¸ö½»Ò×°üµÄ¹þÏ£
} NEXT_CONTRACT;

typedef struct {
	unsigned char nType;					//!<½»Ò×ÀàÐÍ
	unsigned char hash[HASH_SIZE];			//!<ÉÏÒ»¸ö½»Ò×µÄ¹þÏ£
	Int64 nMinus;
}ARBIT_RES_CONTRACT;
#pragma popup()

class CSesureTrade
{
public:
	bool GetHashFromCreatedTx(const Value& valueRes,string& strHash)
	{
		if (valueRes.type() == null_type) {
			return false;
		}

		const Value& result = find_value(valueRes.get_obj(), "hash");
		if (result.type() == null_type){
			return false;
		}

		strHash = result.get_str();
		return true;
	}

	bool CommandLineRPC_GetValue(int argc, char *argv[], Value &value) {
		string strPrint;
		bool nRes = false;
		try {
			// Skip switches
			while (argc > 1 && IsSwitchChar(argv[1][0])) {
				argc--;
				argv++;
			}

			// Method
			if (argc < 2)
				throw runtime_error("too few parameters");
			string strMethod = argv[1];

			// Parameters default to strings
			std::vector<std::string> strParams(&argv[2], &argv[argc]);
			Array params = RPCConvertValues(strMethod, strParams);

			// Execute
			Object reply = CallRPC(strMethod, params);

			// Parse reply
			const Value& result = find_value(reply, "result");
			const Value& error = find_value(reply, "error");

			if (error.type() != null_type) {
				// Error
				strPrint = "error: " + write_string(error, false);
				int code = find_value(error.get_obj(), "code").get_int();
			} else {
				value = result;
				// Result
				if (result.type() == null_type)
					strPrint = "";
				else if (result.type() == str_type)
					strPrint = result.get_str();
				else
					strPrint = write_string(result, true);
				nRes = true;
			}
		} catch (boost::thread_interrupted) {
			throw;
		} catch (std::exception& e) {
			strPrint = string("error: ") + e.what();
		} catch (...) {
			PrintExceptionContinue(NULL, "CommandLineRPC()");
			throw;
		}

		if (strPrint != "") {
			if (false == nRes) {
	//			cout<<strPrint<<endl;
			}
	//	    fprintf((nRes == 0 ? stdout : stderr), "%s\n", strPrint.c_str());
		}

		return nRes;
	}

	bool GenerateOneBlock() {
		char *argv[] = { "rpctest", "setgenerate", "true" };
		int argc = sizeof(argv) / sizeof(char*);
	    int high= chainActive.Height();
		Value value;
		if (CommandLineRPC_GetValue(argc, argv, value)) {
			BOOST_CHECK(high+1==chainActive.Height());
			return true;
		}
		return false;
	}

	bool GetScriptID(const string& strTxHash, string& strScriptID) {
		uint256 txhash(strTxHash);

		int nIndex = 0;
		int BlockHeight = GetTxComfirmHigh(txhash);
		if (BlockHeight > chainActive.Height() || BlockHeight == -1) {
			return false;
		}
		CBlockIndex* pindex = chainActive[BlockHeight];
		CBlock block;
		if (!ReadBlockFromDisk(block, pindex))
			return false;

		block.BuildMerkleTree();
		std::tuple<bool, int> ret = block.GetTxIndex(txhash);
		if (!std::get<0>(ret)) {
			return false;
		}

		nIndex = std::get<1>(ret);

		CRegID striptID(BlockHeight, nIndex);

		strScriptID = HexStr(striptID.GetVec6());
		return true;
	}

	void VerifyTxInBlock(const string& strTxHash) {
		string strScriptID;
		while (true) {
			if (GetScriptID(strTxHash, strScriptID)) {
				if (!strRegScriptID.empty())
					break;
			}
		}
	}


	Value RegisterScriptTx(const string& strAddress, const string& strScript, int nHeight, int nFee) {
		return CreateRegScriptTx(strAddress, strScript, true, nFee, nHeight, CNetAuthorizate());
	}

	Value ModifyAuthor(const string& strAddress, const string& strScript, int nHeight, int nFee,
			const CNetAuthorizate& author) {
		return CreateRegScriptTx(strAddress, strScript, false, nFee, nHeight, author);
	}

	bool CreateSecureTx(const string &scriptid, const vector<string> &obaddrs, const vector<string> &addrs,
			const string&contract, const int nHeight) {
		//CommanRpc
		char cscriptid[64] = { 0 };
		strncpy(cscriptid, scriptid.c_str(), sizeof(cscriptid) - 1);

		char cobstr[512] = { 0 };
		{
			Array array;
			array.clear();
			for (const auto &str : obaddrs) {
				array.push_back(str);
			}
			string arraystr = write_string(Value(array), false);
			strncpy(cobstr, arraystr.c_str(), sizeof(cobstr) - 1);
		}
		char addrstr[512] = { 0 };
		{
			Array array;
			array.clear();
			for (const auto &str : addrs) {
				array.push_back(str);
			}
			string arraystr = write_string(Value(array), false);
			strncpy(addrstr, arraystr.c_str(), sizeof(addrstr) - 1);
		}

		char ccontract[10 * 1024] = { 0 };
		strncpy(ccontract, contract.c_str(), sizeof(ccontract) - 1);

		char height[16] = { 0 };
		sprintf(height, "%d", nHeight);

		char *argv[] = { "rpctest", "createsecuretx", cscriptid, cobstr, addrstr, ccontract, "1000000", height };
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		int ret = CommandLineRPC_GetValue(argc, argv, value);
		if (!ret) {
			LogPrint("test_miners", "CreateSecureTx:%s\r\n", value.get_str().c_str());
			return true;
		}
		return false;
	}

	bool ModifyAuthor(unsigned char nUserData,string& strTxHash) {
		vector<unsigned char> vUserDefine;
		string strHash1, strHash2;
		vUserDefine.push_back(nUserData);
		CNetAuthorizate author(100, vUserDefine, 100000, 1000000, 1000000);
		Value valueRes = ModifyAuthor(BUYER_ADDR, strRegScriptID, 200, 10000, author);

		if (!GetHashFromCreatedTx(valueRes,strTxHash)) {
			return false;
		}

		return true;
	}

	bool CreateContractTx(const std::string &scriptid, const std::string &addrs, const std::string &contract,
			int nHeight,int nFee) {
		char cscriptid[1024] = { 0 };

		string strFee;
		strFee = strprintf("%d",nFee);

		char height[16] = { 0 };
		sprintf(height, "%d", nHeight);

		vector<unsigned char> vTemp;
		vTemp.assign(contract.begin(),contract.end());
		string strContractData = HexStr(vTemp);

		char *argv[] = { "rpctest", "createcontracttx", (char *) (scriptid.c_str()), (char *) (addrs.c_str()),
				(char *) (strContractData.c_str()), (char*)strFee.c_str(), height };
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		if (CommandLineRPC_GetValue(argc, argv, value)) {
			return true;
		}
		return false;
	}

	Value CreateRegScriptTx(const string& strAddress, const string& strScript, bool bRigsterScript, int nFee,
			int nHeight, const CNetAuthorizate& author) {
		string strScriptData;
		char szType[1] = { 0 };
		if (bRigsterScript) {
			szType[0] = '0';
			strScriptData = SysCfg().GetDefaultTestDataPath() + strScript;
			if (!boost::filesystem::exists(strScriptData)) {
				BOOST_CHECK_MESSAGE(0, strScriptData + " not exist");
				return false;
			}
		} else {
			strScriptData = strScript;
			szType[0] = '1';
		}

		string strFee = strprintf("%d",nFee);
		string strHeight = strprintf("%d",nHeight);
		string strAuTime = strprintf("%d",author.GetAuthorizeTime());
		string strMoneyPerTime = strprintf("%d",author.GetMaxMoneyPerTime());
		string strMoneyTotal = strprintf("%d",author.GetMaxMoneyTotal());
		string strMoneyPerDay = strprintf("%d",author.GetMaxMoneyPerDay());

		string strUserData = HexStr(author.GetUserData());
		char *argv[13] = {						//
				"rpctest",						//
						"registerscripttx",				//
						(char*) strAddress.c_str(), 	//
						szType, 						//
						(char*) strScriptData.c_str(),	//
						(char*) strFee.c_str(), 		//
						(char*) strHeight.c_str(), 		//
						"this is description", 			//
						(char*) strAuTime.c_str(),		//
						(char*) strMoneyPerTime.c_str(),		//
						(char*) strMoneyTotal.c_str(), 	//
						(char*) strMoneyPerDay.c_str(),	//
						(char*) strUserData.c_str() };	//

		Value value;

		if (CommandLineRPC_GetValue(sizeof(argv) / sizeof(argv[0]), argv, value)) {
			LogPrint("test_miners", "RegisterSecureTx:%s\r\n", write_string(value, true));
			return value;
		}
		return value;
	}

	void PacketFirstContract(const char*pBuyID,const char* pSellID,const char* pArID,
		int nHeight,int nFine,int nPay,int nFee,int ndeposit,FIRST_CONTRACT* pContract)
	{
		BOOST_CHECK(pContract);
		memset(pContract,0,sizeof(FIRST_CONTRACT));
		pContract->nType = 1;
		pContract->nArbitratorCount = 1;
		pContract->nHeight = nHeight;

		unsigned char nSize = sizeof(int);
		vector<unsigned char> v = ParseHex(pBuyID);
		memcpy(pContract->buyer.accounid,&v[0],ACCOUNT_ID_SIZE);

		v = ParseHex(pSellID);
		memcpy(pContract->seller.accounid,&v[0],ACCOUNT_ID_SIZE);

		v = ParseHex(pArID);
		memcpy(pContract->arbitrator[0].accounid,&v[0],ACCOUNT_ID_SIZE);

		memcpy(&pContract->nFineMoney,(const char*)&nFine,nSize);//100
		memcpy(&pContract->nPayMoney,(const char*)&nPay,nSize);//80
		memcpy(&pContract->ndeposit,(const char*)&ndeposit,nSize);//20
		memcpy(&pContract->nFee,(const char*)&nFee,nSize);//10
	}
protected:
	string 				strRegScriptID;
	FIRST_CONTRACT		firstConstract;
};

BOOST_FIXTURE_TEST_SUITE(sesuretrade,CSesureTrade)
BOOST_AUTO_TEST_CASE(test)
{
	string strTxHash;
	Value valueRes = RegisterScriptTx(BUYER_ADDR,"SecuredTrade.bin",100,10000);
	BOOST_CHECK(GetHashFromCreatedTx(valueRes,strTxHash));
	BOOST_CHECK(GenerateOneBlock());
	VerifyTxInBlock(strTxHash);
	BOOST_CHECK(GetScriptID(strTxHash,strRegScriptID));

	BOOST_CHECK(ModifyAuthor(1,strTxHash));
	BOOST_CHECK(GenerateOneBlock());
	VerifyTxInBlock(strTxHash);

	PacketFirstContract(BUYER_ID,SELLER_ID,ARBIT_ID,100,10000,10000,10000,10000,&firstConstract);
	string strContract;
	BOOST_CHECK(CreateContractTx(strRegScriptID,BUYER_ADDR,strContract,100,10000));
}
BOOST_AUTO_TEST_SUITE_END()












