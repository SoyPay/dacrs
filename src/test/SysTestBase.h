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

	Value CreateRegScriptTx(const string& strAddress, const string& strScript, bool bRigsterScript, int nFee,
			int nHeight, const CNetAuthorizate& author);

	Value GetAccountInfo(const string& strID);

public:
	SysTestBase();

	~SysTestBase();

	static void StartServer(int argc,char* argv[]);

	static void StopServer();

	bool CommandLineRPC_GetValue(int argc, char *argv[], Value &value);

	bool IsScriptAccCreated(const string& strScript);

	uint64_t GetFreeMoney(const string& strID);

	bool GetOneAddr(std::string &addr, char *pStrMinMoney, char *bpBoolReg);

	bool GetOneScriptId(std::string &regscriptid);

	bool GetNewAddr(std::string &addr);

	bool GetAccState(const std::string &addr, AccState &accstate);

	bool GetBlockHeight(int &nHeight);

	bool CreateNormalTx(const std::string &srcAddr, const std::string &desAddr, const int nHeight);

	bool CreateFreezeTx(const std::string &addr, const int nHeight);

	bool RegisterAccountTx(const std::string &addr, const int nHeight);

	bool CreateContractTx(const std::string &scriptid, const std::string &addrs, const std::string &contract,
			const int nHeight);

	Value RegisterScriptTx(const string& strAddress, const string& strScript, int nHeight, int nFee = 10000);

	Value ModifyAuthor(const string& strAddress, const string& strScript, int nHeight, int nFee,
			const CNetAuthorizate& author);

	bool CreateSecureTx(const string &scriptid, const vector<string> &obaddrs, const vector<string> &addrs,
			const string&contract, const int nHeight);

	bool SignSecureTx(const string &securetx);

	bool IsAllTxInBlock();

	bool GetBlockHash(const int nHeight, std::string &blockhash);

	bool GetBlockMinerAddr(const std::string &blockhash, std::string &addr);

	bool GenerateOneBlock();

protected:
	static boost::thread* pThreadShutdown ;
	std::map<string, AccState> mapAccState;
	std::map<string, AccOperLog> mapAccOperLog;
	int nCurHeight;
	int64_t nCurMoney;
	int64_t nCurFee;
};

#endif /* CRPCREQUEST2_H_ */
