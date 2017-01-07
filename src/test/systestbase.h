/*
 * CRPCRequest2.h
 *
 *  Created on: Dec 9, 2014
 *      Author: ranger.shi
 */

#ifndef DACRS_TEST_SYSTESTBASE_H_
#define DACRS_TEST_SYSTESTBASE_H_

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
#include "cuiserver.h"
#include <boost/algorithm/string/predicate.hpp>

using namespace std;
using namespace boost;
struct ST_ACC_STATE {
	int64_t lldUnmatureMoney;
	int64_t lldFreeMoney;
	int64_t lldFrozenMoney;

	ST_ACC_STATE() {
		lldUnmatureMoney = 0;
		lldFreeMoney = 0;
		lldFrozenMoney = 0;
	}

	ST_ACC_STATE(int64_t a, int64_t b, int64_t c) {
		lldUnmatureMoney = a;
		lldFreeMoney = b;
		lldFrozenMoney = c;
	}

	bool operator==(ST_ACC_STATE &a) {
		if (lldUnmatureMoney == a.lldUnmatureMoney && lldFreeMoney == a.lldFreeMoney && lldFrozenMoney == a.lldFrozenMoney) {
			return true;
		}
		return false;
	}

	bool SumEqual(ST_ACC_STATE &a) {
		int64_t l = lldUnmatureMoney + lldFreeMoney + lldFrozenMoney;
		int64_t r = a.lldUnmatureMoney + a.lldFreeMoney + a.lldFrozenMoney;

		return l == r;
	}

	ST_ACC_STATE& operator+=(ST_ACC_STATE &a) {
		lldUnmatureMoney += a.lldUnmatureMoney;
		lldFreeMoney += a.lldFreeMoney;
		lldFrozenMoney += a.lldFrozenMoney;
		return *this;
	}
};

struct ST_ACC_OPER_LOG {
	const static int knFrozenHeight = 100;
	const static int knMatureHeight = 100;
	std::map<int, ST_ACC_STATE> mapAccState;

	ST_ACC_OPER_LOG() {
		mapAccState.clear();
	}

	bool Add(int &nHeight, ST_ACC_STATE &accstate) {
		mapAccState[nHeight] = accstate;
		return true;
	}

	void MergeAcc(int nHeight) {
		for (auto &item : mapAccState) {
			if (nHeight > item.first + knFrozenHeight) {
				item.second.lldFreeMoney += item.second.lldFrozenMoney;
				item.second.lldFrozenMoney = 0.0;
			}

			if (nHeight > item.first + knMatureHeight) {
				item.second.lldFreeMoney += item.second.lldUnmatureMoney;
				item.second.lldUnmatureMoney = 0.0;
			}
		}
	}
};

class SysTestBase {
 protected:
	int GetRandomMoney();
	Value GetAccountInfo(const string& strID);

 public:
	SysTestBase();
	~SysTestBase();
	static void StartServer(int argc,const char* argv[]);
	static void StopServer();
	int GetRandomFee();
	bool GetMemPoolSize(int &nSize);
	bool ImportAllPrivateKey();

	bool GetHashFromCreatedTx(const Value& valueRes, string& strHash) {
		if (valueRes.type() == null_type) {
			return false;
		}

		const Value& result = find_value(valueRes.get_obj(), "hash");
		const Value& result1 = find_value(valueRes.get_obj(), "rawtx");
		const Value& result3 = find_value(valueRes.get_obj(), "script");
		if (result.type() == null_type && result1.type() == null_type && result3.type() == null_type) {
			return false;
		}
		if (result.type() != null_type) {
			strHash = result.get_str();
		} else if (result1.type() != null_type) {
			strHash = result1.get_str();
		} else if (result3.type() != null_type) {
			strHash = result3.get_str();
		}

		return true;
	}

	bool GetTxConfirmedRegID(const string& kstrHaseh, string& strRegID) {
		const char *argv[] = { "rpctest", "getscriptid", (char*) kstrHaseh.c_str() };

		Value value;
		if (!CommandLineRPC_GetValue(sizeof(argv) / sizeof(argv[0]), argv, value)) {
			return false;
		}

		if (value.type() == null_type) {
			//cout<<write_string(value, true)<<endl;
			return false;
		}

		const Value& result = find_value(value.get_obj(), "script");
		if (result.type() == null_type) {
			//cout<<write_string(value, true)<<endl;
			return false;
		}
		strRegID = result.get_str();
		return true;
	}

	Value CreateRegAppTx(const string& strAddress, const string& strScript, bool bRigsterScript, int nFee,int nHeight);
	bool ResetEnv();
	static bool CommandLineRPC_GetValue(int argc, const char *argv[], Value &value);
	bool IsScriptAccCreated(const string& strScript);
	uint64_t GetBalance(const string& strID);
	bool GetNewAddr(std::string &strAddr,bool bFlag);
	bool GetBlockHeight(int &nHeight);
	Value CreateNormalTx(const std::string &strSrcAddr, const std::string &strDesAddr,uint64_t ullMoney);
	Value CreateNormalTx(const std::string &strDesAddr,uint64_t ullMoney);
	Value RegistAccountTx(const std::string &strAddr, const int nfee);
	Value CreateContractTx(const std::string &strScriptid, const std::string &strAddrs, const std::string &strContract,
			int nHeight,int nFee=0,uint64_t ullMoney=0 );
	Value RegisterAppTx(const string& strAddress, const string& strScript, int nHeight, int nFee = 100000000);

	Value SignSecureTx(const string &strSecuretx);

	bool IsAllTxInBlock();

	bool GetBlockHash(const int nHeight, std::string &strBlockHash);

	bool GetBlockMinerAddr(const std::string &strBlockHash, std::string &strAddr);

	bool GenerateOneBlock();

	bool SetAddrGenerteBlock(const char *addr);

	bool DisConnectBlock(int nNum);

	bool GetStrFromObj(const Value& valueRes,string& str);

	bool ImportWalletKey(const char**address,int nCount);

	bool ShowProgress(string const &msg, float fRate) {
		for (int j = 0; j < 100; ++j) {
			cout << '\b';
		}
		cout << msg << fRate << "%";
		return true;
	}

	bool ShowProgressTotal(string const &strMsg, int nRate) {
		for (int j = 0; j < 100; ++j) {
			cout << '\b';
		}
		cout << strMsg << " Total: " << nRate;
		return true;
	}

	uint64_t GetRandomBetfee();

	bool GetKeyId(string const &strAddr,CKeyID &cKeyId);

	bool IsTxInMemorypool(const uint256& cTxHash);

	bool IsTxUnConfirmdInWallet(const uint256& cTxHash) ;

	bool IsTxInTipBlock(const uint256& cTxHash);

	bool GetRegID(string& strAddr,CRegID& cRegID);

	bool GetRegID(string& strAddr,string& strRegID);

	bool GetTxOperateLog(const uint256& cTxHash, vector<CAccountLog>& vcLog) ;

	bool PrintLog();

	bool IsMemoryPoolEmpty();

	Value GetAppAccountInfo(const string& strScriptId,const string& strAddr);

 protected:
	static boost::thread* pThreadShutdown;
	std::map<string, ST_ACC_STATE> m_mapAccState;
	std::map<string, ST_ACC_OPER_LOG> m_mapAccOperLog;
	int m_nCurHeight;
	int64_t m_llCurMoney;
	int64_t m_llCurFee;
};

#endif /* DACRS_TEST_SYSTESTBASE_H_ */
