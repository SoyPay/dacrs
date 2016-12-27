/*
 * CRPCRequest2.h
 *
 *  Created on: Dec 9, 2014
 *      Author: ranger.shi
 */

#ifndef CRPCREQUEST_H_
#define CRPCREQUEST_H_

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
struct AccState {
	int64_t lldUnmatureMoney;
	int64_t lldFreeMoney;
	int64_t lldFrozenMoney;
	AccState() {
		lldUnmatureMoney = 0;
		lldFreeMoney = 0;
		lldFrozenMoney = 0;
	}
	AccState(int64_t a, int64_t b, int64_t c) {
		lldUnmatureMoney = a;
		lldFreeMoney = b;
		lldFrozenMoney = c;
	}

	bool operator==(AccState &a) {
		if (lldUnmatureMoney == a.lldUnmatureMoney && lldFreeMoney == a.lldFreeMoney && lldFrozenMoney == a.lldFrozenMoney) {
			return true;
		}
		return false;
	}

	bool SumEqual(AccState &a) {
		int64_t l = lldUnmatureMoney + lldFreeMoney + lldFrozenMoney;
		int64_t r = a.lldUnmatureMoney + a.lldFreeMoney + a.lldFrozenMoney;

		return l == r;
	}

	AccState& operator+=(AccState &a) {
		lldUnmatureMoney += a.lldUnmatureMoney;
		lldFreeMoney += a.lldFreeMoney;
		lldFrozenMoney += a.lldFrozenMoney;
		return *this;
	}
};

struct AccOperLog {
	const static int nFrozenHeight = 100;
	const static int nMatureHeight = 100;
	std::map<int, AccState> mapAccState;
	AccOperLog() {
		mapAccState.clear();
	}
	bool Add(int &nHeight, AccState &accstate) {
		mapAccState[nHeight] = accstate;
		return true;
	}

	void MergeAcc(int nHeight) {
		for (auto &item : mapAccState) {
			if (nHeight > item.first + nFrozenHeight) {
				item.second.lldFreeMoney += item.second.lldFrozenMoney;
				item.second.lldFrozenMoney = 0.0;
			}

			if (nHeight > item.first + nMatureHeight) {
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

	bool GetMemPoolSize(int &size);

	bool ImportAllPrivateKey();

	bool GetHashFromCreatedTx(const Value& valueRes,string& strHash)
	{
		if (valueRes.type() == null_type) {
			return false;
		}

		const Value& result = find_value(valueRes.get_obj(), "hash");
		const Value& result1 = find_value(valueRes.get_obj(), "rawtx");
		const Value& result3 = find_value(valueRes.get_obj(), "script");
		if (result.type() == null_type && result1.type() == null_type && result3.type() == null_type){
			return false;
		}
		if (result.type() != null_type){
			strHash = result.get_str();
			}
		else if(result1.type() != null_type)
		{
			strHash = result1.get_str();
		}else if(result3.type() != null_type)
		{
			strHash = result3.get_str();
		}

		return true;
	}

	bool GetTxConfirmedRegID(const string& haseh,string& strRegID)
	{
		const char *argv[] = { "rpctest", "getscriptid", (char*) haseh.c_str() };

		Value value;
		if (!CommandLineRPC_GetValue(sizeof(argv) / sizeof(argv[0]), argv, value)) {
			return false;
		}

		if (value.type() == null_type) {
			//cout<<write_string(value, true)<<endl;
			return false;
		}

		const Value& result = find_value(value.get_obj(), "script");
		if (result.type() == null_type){
			//cout<<write_string(value, true)<<endl;
			return false;
		}
		strRegID = result.get_str();
		return true;
	}

	Value CreateRegAppTx(const string& strAddress, const string& strScript, bool bRigsterScript, int nFee,
			int nHeight);

	bool ResetEnv();

	static bool CommandLineRPC_GetValue(int argc, const char *argv[], Value &value);

	bool IsScriptAccCreated(const string& strScript);

	uint64_t GetBalance(const string& strID);

	bool GetNewAddr(std::string &addr,bool flag);

	bool GetBlockHeight(int &nHeight);

	Value CreateNormalTx(const std::string &srcAddr, const std::string &desAddr,uint64_t nMoney);

	Value CreateNormalTx(const std::string &desAddr,uint64_t nMoney);

	Value RegistAccountTx(const std::string &addr, const int nfee =0);

	Value CreateContractTx(const std::string &scriptid, const std::string &addrs, const std::string &contract,
			int nHeight,int nFee = 0,uint64_t nMoney = 0);

	Value RegisterAppTx(const string& strAddress, const string& strScript, int nHeight, int nFee = 100000000);

	Value SignSecureTx(const string &securetx);

	bool IsAllTxInBlock();

	bool GetBlockHash(const int nHeight, std::string &blockhash);

	bool GetBlockMinerAddr(const std::string &blockhash, std::string &addr);

	bool GenerateOneBlock();

	bool SetAddrGenerteBlock(const char *addr);

	bool DisConnectBlock(int nNum);

	bool GetStrFromObj(const Value& valueRes,string& str);

	bool ImportWalletKey(const char**address,int nCount);

	bool ShowProgress(string const &msg,float rate)
    {
    	for(int j=0; j<100 ;++j)
    	cout<<'\b';
    	cout << msg <<  rate << "%";
    	return true;
    }


	bool ShowProgressTotal(string const &msg,int rate)
    {
    	for(int j=0; j<100 ;++j)
    	cout<<'\b';
    	cout << msg << " Total: " << rate;
    	return true;
    }

	uint64_t GetRandomBetfee();

	bool GetKeyId(string const &addr,CKeyID &cKeyId);

	bool IsTxInMemorypool(const uint256& txHash);

	bool IsTxUnConfirmdInWallet(const uint256& txHash) ;

	bool IsTxInTipBlock(const uint256& txHash);

	bool GetRegID(string& strAddr,CRegID& regID);

	bool GetRegID(string& strAddr,string& regID);

	bool GetTxOperateLog(const uint256& txHash, vector<CAccountLog>& vLog) ;

	bool PrintLog();

	bool IsMemoryPoolEmpty();

	Value GetAppAccountInfo(const string& scriptId,const string& strAddr);
protected:
	static boost::thread* pThreadShutdown ;
	std::map<string, AccState> mapAccState;
	std::map<string, AccOperLog> mapAccOperLog;
	int nCurHeight;
	int64_t nCurMoney;
	int64_t nCurFee;
};

#endif /* CRPCREQUEST2_H_ */
