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
struct AccState {
	int64_t dUnmatureMoney;
	int64_t dFreeMoney;
	int64_t dFrozenMoney;
	AccState() {
		dUnmatureMoney = 0;
		dFreeMoney = 0;
		dFrozenMoney = 0;
	}
	AccState(int64_t a, int64_t b, int64_t c) {
		dUnmatureMoney = a;
		dFreeMoney = b;
		dFrozenMoney = c;
	}

	bool operator==(AccState &a) {
		if (dUnmatureMoney == a.dUnmatureMoney && dFreeMoney == a.dFreeMoney && dFrozenMoney == a.dFrozenMoney) {
			return true;
		}
		return false;
	}

	bool SumEqual(AccState &a) {
		int64_t l = dUnmatureMoney + dFreeMoney + dFrozenMoney;
		int64_t r = a.dUnmatureMoney + a.dFreeMoney + a.dFrozenMoney;

		return l == r;
	}

	AccState& operator+=(AccState &a) {
		dUnmatureMoney += a.dUnmatureMoney;
		dFreeMoney += a.dFreeMoney;
		dFrozenMoney += a.dFrozenMoney;
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
	}

	void MergeAcc(int nHeight) {
		for (auto &item : mapAccState) {
			if (nHeight > item.first + nFrozenHeight) {
				item.second.dFreeMoney += item.second.dFrozenMoney;
				item.second.dFrozenMoney = 0.0;
			}

			if (nHeight > item.first + nMatureHeight) {
				item.second.dFreeMoney += item.second.dUnmatureMoney;
				item.second.dUnmatureMoney = 0.0;
			}
		}
	}
};

class SysTestBase {
protected:
	int GetRandomFee();

	int GetRandomMoney();



	Value GetAccountInfo(const string& strID);

public:


	SysTestBase();

	~SysTestBase();

	static void StartServer(int argc,char* argv[]);

	static void StopServer();

	bool GetHashFromCreatedTx(const Value& valueRes,string& strHash)
	{
		if (valueRes.type() == null_type) {
			//cout<<write_string(valueRes, true)<<endl;
			return false;
		}

		const Value& result = find_value(valueRes.get_obj(), "hash");
		if (result.type() == null_type){
			//cout<<write_string(valueRes, true)<<endl;
			return false;
		}

		strHash = result.get_str();
		return true;
	}

	bool GetTxConfirmedRegID(const string& haseh,string& strRegID)
	{
		char *argv[] = { "rpctest", "getscriptid", (char*) haseh.c_str() };
		int argc = sizeof(argv) / sizeof(char*);

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

	Value CreateRegScriptTx(const string& strAddress, const string& strScript, bool bRigsterScript, int nFee,
			int nHeight, const CNetAuthorizate& author);

	bool ResetEnv();

	bool CommandLineRPC_GetValue(int argc, char *argv[], Value &value);

	bool IsScriptAccCreated(const string& strScript);

	uint64_t GetFreeMoney(const string& strID);

	bool GetOneScriptId(std::string &regscriptid);

	bool GetNewAddr(std::string &addr,bool flag);

	bool GetAccState(const std::string &addr, AccState &accstate);

	bool GetBlockHeight(int &nHeight);

	bool CreateNormalTx(const std::string &srcAddr, const std::string &desAddr, const int nHeight);

	Value CreateFreezeTx(const std::string &addr, const int nHeight);

	Value registaccounttx(const std::string &addr, const int nHeight);

	Value PCreateContractTx(const std::string &scriptid, const std::string &addrs, const std::string &contract,
			int nHeight,int nFee = 10000);

	Value CreateContractTxEx(const std::string &scriptid, const std::string &addrs, const std::string &contract,
			int nHeight,int nFee = 10000);

	bool CreateContractTx(const std::string &scriptid, const std::string &addrs, const std::string &contract,
			int nHeight,int nFee = 10000);
	Value CreateContractTx1(const std::string &scriptid, const std::string &addrs, const std::string &contract,
				const int nHeight);
	Value RegisterScriptTx(const string& strAddress, const string& strScript, int nHeight, int nFee = 10000);

	Value ModifyAuthor(const string& strAddress, const string& strScript, int nHeight, int nFee,
			const CNetAuthorizate& author);

	Value SignSecureTx(const string &securetx);

	bool IsAllTxInBlock();

	bool GetBlockHash(const int nHeight, std::string &blockhash);

	bool GetBlockMinerAddr(const std::string &blockhash, std::string &addr);

	bool GenerateOneBlock();

	bool DisConnectBlock(int nNum);
	Value GetScriptID(string txhash);
	bool GetStrFromObj(const Value& valueRes,string& str);
	bool ImportWalletKey(char**address,int nCount);
protected:
	static boost::thread* pThreadShutdown ;
	std::map<string, AccState> mapAccState;
	std::map<string, AccOperLog> mapAccOperLog;
	int nCurHeight;
	int64_t nCurMoney;
	int64_t nCurFee;
};

#endif /* CRPCREQUEST2_H_ */
