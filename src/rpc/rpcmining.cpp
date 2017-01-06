// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/foreach.hpp>
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "chainparams.h"
#include "core.h"
#include "init.h"
#include "main.h"
#include "miner.h"
//#include "net.h"
#include "rpcprotocol.h"
#include "rpcserver.h"
#include "serialize.h"
#include "sync.h"
#include "txmempool.h"
#include "uint256.h"
#include "util.h"
#include "version.h"

#include "../wallet/wallet.h"

#include <stdint.h>

#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"

using namespace json_spirit;
using namespace std;


// Key used by getwork miners.
// Allocated in InitRPCMining, free'd in ShutdownRPCMining
void InitRPCMining() {
	if (!g_pwalletMain) {
		return;
	}
	LogPrint("TODO", "InitRPCMining");
}

void ShutdownRPCMining() {
//    if (!pMiningKey)
//        return;
//
//    delete pMiningKey; pMiningKey = NULL;
}


// Return average network hashes per second based on the last 'lookup' blocks,
// or from the last difficulty change if 'lookup' is nonpositive.
// If 'height' is nonnegative, compute the estimate at the time when a given block was found.
Value GetNetworkHashPS(int nLookup, int nHeight) {
    CBlockIndex *pBlockIndex = g_cChainActive.Tip();

	if (nHeight >= 0 && nHeight < g_cChainActive.Height()) {
		pBlockIndex = g_cChainActive[nHeight];
	}
	if (pBlockIndex == NULL || !pBlockIndex->m_nHeight) {
		return 0;
	}
    // If lookup is -1, then use blocks since last difficulty change.
	if (nLookup <= 0) {
		nLookup = pBlockIndex->m_nHeight % 2016 + 1;
	}
    // If lookup is larger than chain, then set it to chain length.
	if (nLookup > pBlockIndex->m_nHeight) {
		nLookup = pBlockIndex->m_nHeight;
	}
    CBlockIndex *pcBlockIndex0 = pBlockIndex;
    int64_t llMinTime = pcBlockIndex0->GetBlockTime();
    int64_t llMaxTime = llMinTime;
    for (int i = 0; i < nLookup; i++) {
        pcBlockIndex0 = pcBlockIndex0->m_pPrevBlockIndex;
        int64_t llTime = pcBlockIndex0->GetBlockTime();
        llMinTime = min(llTime, llMinTime);
        llMaxTime = max(llTime, llMaxTime);
    }

    // In case there's a situation where minTime == maxTime, we don't want a divide by zero exception.
	if (llMinTime == llMaxTime) {
		return 0;
	}
    arith_uint256 cWorkDiff = pBlockIndex->m_cChainWork - pcBlockIndex0->m_cChainWork;
    int64_t llTimeDiff = llMaxTime - llMinTime;

    return (int64_t)(cWorkDiff.getdouble() / llTimeDiff);
}

Value getnetworkhashps(const Array& params, bool bHelp) {
	if (bHelp || params.size() > 2) {
		throw runtime_error(
				"getnetworkhashps ( blocks height )\n"
						"\nReturns the estimated network hashes per second based on the last n blocks.\n"
						"Pass in [blocks] to override # of blocks, -1 specifies since last difficulty change.\n"
						"Pass in [height] to estimate the network speed at the time when a certain block was found.\n"
						"\nArguments:\n"
						"1. blocks     (numeric, optional, default=120) The number of blocks, or -1 for blocks since last difficulty change.\n"
						"2. height     (numeric, optional, default=-1) To estimate at the time of the given height.\n"
						"\nResult:\n"
						"x             (numeric) Hashes per second estimated\n"
						"\nExamples:\n" + HelpExampleCli("getnetworkhashps", "")
						+ HelpExampleRpc("getnetworkhashps", ""));
	}
	return GetNetworkHashPS(params.size() > 0 ? params[0].get_int() : 120, params.size() > 1 ? params[1].get_int() : -1);
}

static bool g_sIsMining = false;

void SetMinerStatus(bool bstatue) {
	g_sIsMining = bstatue;
}

static bool getMiningInfo() {
	return g_sIsMining;
}

Value setgenerate(const Array& params, bool bHelp) {
	if (bHelp || (params.size() != 1 && params.size() != 2)) {
		throw runtime_error(
				"setgenerate generate ( genproclimit )\n"
						"\nSet 'generate' true or false to turn generation on or off.\n"
						"Generation is limited to 'genproclimit' processors, -1 is unlimited.\n"
						"See the getgenerate call for the current setting.\n"
						"\nArguments:\n"
						"1. generate         (boolean, required) Set to true to turn on generation, off to turn off.\n"
						"2. genproclimit     (numeric, optional) Set the processor limit for when generation is on. Can be -1 for unlimited.\n"
						"                    Note: in -regtest mode, genproclimit controls how many blocks are generated immediately.\n"
						"\nExamples:\n"
						"\nSet the generation on with a limit of one processor\n"
						+ HelpExampleCli("setgenerate", "true 1") + "\nTurn off generation\n"
						+ HelpExampleCli("setgenerate", "false") + "\nUsing json rpc\n"
						+ HelpExampleRpc("setgenerate", "true, 1"));
	}

	static bool bGenerate = false;

	set<CKeyID> setKeyId;
	setKeyId.clear();
	g_pwalletMain->GetKeys(setKeyId, true);

	bool bSetEmpty(true);
	for (auto & keyId : setKeyId) {
		CUserID cUserId(keyId);
		CAccount cAcctInfo;
		if (g_pAccountViewTip->GetAccount(cUserId, cAcctInfo)) {
			bSetEmpty = false;
			break;
		}
	}

	if (bSetEmpty) {
		throw JSONRPCError(RPC_INVALID_PARAMS, "no key for mining");
	}
	if (params.size() > 0) {
		bGenerate = params[0].get_bool();
	}
	int nGenProcLimit = 1;
	if (params.size() == 2) {
		nGenProcLimit = params[1].get_int();
		if (nGenProcLimit <= 0) {
			throw JSONRPCError(RPC_INVALID_PARAMS, "limit conter err for mining");
		}
	}
	Object obj;
	if (bGenerate == false) {
		GenerateDacrsBlock(false, g_pwalletMain, 1);
		obj.push_back(Pair("msg", "stoping  mining"));
		return obj;
	}

	GenerateDacrsBlock(true, g_pwalletMain, nGenProcLimit); //跑完之后需要退出
	obj.push_back(Pair("msg", "in  mining"));
	return obj;
}

Value getmininginfo(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		throw runtime_error(
				"getmininginfo\n"
						"\nReturns a json object containing mining-related information."
						"\nResult:\n"
						"{\n"
						"  \"blocks\": nnn,             (numeric) The current block\n"
						"  \"currentblocksize\": nnn,   (numeric) The last block size\n"
						"  \"currentblocktx\": nnn,     (numeric) The last block transaction\n"
						"  \"difficulty\": xxx.xxxxx    (numeric) The current difficulty\n"
						"  \"errors\": \"...\"          (string) Current errors\n"
						"  \"generate\": true|false     (boolean) If the generation is on or off (see getgenerate or setgenerate calls)\n"
						"  \"genproclimit\": n          (numeric) The processor limit for generation. -1 if no generation. (see getgenerate or setgenerate calls)\n"
						"  \"hashespersec\": n          (numeric) The hashes per second of the generation, or 0 if no generation.\n"
						"  \"pooledtx\": n              (numeric) The size of the mem pool\n"
						"  \"testnet\": true|false      (boolean) If using testnet or not\n"
						"}\n"
						"\nExamples:\n" + HelpExampleCli("getmininginfo", "") + HelpExampleRpc("getmininginfo", ""));
	}
	Object obj;
	obj.push_back(Pair("blocks", (int) g_cChainActive.Height()));
    obj.push_back(Pair("currentblocksize", (uint64_t)g_ullLastBlockSize));
    obj.push_back(Pair("currentblocktx",   (uint64_t)g_ullLastBlockTx));
	obj.push_back(Pair("difficulty", (double) GetDifficulty()));
	obj.push_back(Pair("errors", GetWarnings("statusbar")));
	obj.push_back(Pair("genproclimit", 1));
	obj.push_back(Pair("networkhashps", getnetworkhashps(params, false)));
	obj.push_back(Pair("pooledtx", (uint64_t) g_cTxMemPool.size()));
	static const string name[] = { "MAIN", "TESTNET", "REGTEST" };
	obj.push_back(Pair("nettype", name[SysCfg().NetworkID()]));
	obj.push_back(Pair("posmaxnonce", SysCfg().GetBlockMaxNonce()));

	obj.push_back(Pair("generate", getMiningInfo()));
	return obj;
}

Value submitblock(const Array& params, bool bHelp) {
	if (bHelp || params.size() < 1 || params.size() > 2) {
		throw runtime_error(
				"submitblock \"hexdata\" ( \"jsonparametersobject\" )\n"
						"\nAttempts to submit new block to network.\n"
						"The 'jsonparametersobject' parameter is currently ignored.\n"
						"See https://en.Dacrs.it/wiki/BIP_0022 for full specification.\n"

						"\nArguments\n"
						"1. \"hexdata\"    (string, required) the hex-encoded block data to submit\n"
						"2. \"jsonparametersobject\"     (string, optional) object of optional parameters\n"
						"    {\n"
						"      \"workid\" : \"id\"    (string, optional) if the server provided a workid, it MUST be included with submissions\n"
						"    }\n"
						"\nResult:\n"
						"\nExamples:\n" + HelpExampleCli("submitblock", "\"mydata\"")
						+ HelpExampleRpc("submitblock", "\"mydata\""));
	}
	vector<unsigned char> vuchBlockData(ParseHex(params[0].get_str()));
	CDataStream cDSBlock(vuchBlockData, SER_NETWORK, g_sProtocolVersion);
	CBlock pBlock;
	try {
		cDSBlock >> pBlock;
	} catch (std::exception &e) {
		throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Block decode failed");
	}

	CValidationState cValidationState;
	bool bAccepted = ProcessBlock(cValidationState, NULL, &pBlock);
	Object obj;
	if (!bAccepted) {
		obj.push_back(Pair("status", "rejected"));
		obj.push_back(Pair("reject code", cValidationState.GetRejectCode()));
		obj.push_back(Pair("info", cValidationState.GetRejectReason()));
	} else {
		obj.push_back(Pair("status", "OK"));
		obj.push_back(Pair("hash", pBlock.GetHash().ToString()));
	}
	return obj;
}
