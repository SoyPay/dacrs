// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcserver.h"
#include "main.h"
#include "sync.h"
#include "checkpoints.h"
#include <stdint.h>

#include "json/json_spirit_value.h"

using namespace json_spirit;
using namespace std;

//void ScriptPubKeyToJSON(const CScript& scriptPubKey, Object& out, bool fIncludeHex);

double GetDifficulty(const CBlockIndex* pcBlockIndex) {
	// Floating point number that is a multiple of the minimum difficulty,
	// minimum difficulty = 1.0.
	if (pcBlockIndex == NULL) {
		if (g_cChainActive.Tip() == NULL) {
			return 1.0;
		} else {
			pcBlockIndex = g_cChainActive.Tip();
		}
	}

	int nShift = (pcBlockIndex->m_unBits >> 24) & 0xff;
	double dDiff = (double) 0x0000ffff / (double) (pcBlockIndex->m_unBits & 0x00ffffff);

	while (nShift < 29) {
		dDiff *= 256.0;
		nShift++;
	}
	while (nShift > 29) {
		dDiff /= 256.0;
		nShift--;
	}
	return dDiff;
}

Object blockToJSON(const CBlock& cBlock, const CBlockIndex* cBlockIndex) {
	Object result;
	result.push_back(Pair("hash", cBlock.GetHash().GetHex()));
	CMerkleTx cTxGen(cBlock.vptx[0]);
	cTxGen.SetMerkleBranch(&cBlock);
	result.push_back(Pair("confirmations", (int) cTxGen.GetDepthInMainChain()));
	result.push_back(Pair("size", (int) ::GetSerializeSize(cBlock, SER_NETWORK, g_sProtocolVersion)));
	result.push_back(Pair("height", cBlockIndex->m_nHeight));
	result.push_back(Pair("version", cBlock.GetVersion()));
	result.push_back(Pair("merkleroot", cBlock.GetHashMerkleRoot().GetHex()));
	result.push_back(Pair("txnumber", (int) cBlock.vptx.size()));
	Array txs;
	for (const auto& ptx : cBlock.vptx) {
		txs.push_back(ptx->GetHash().GetHex());
	}
	result.push_back(Pair("tx", txs));
	result.push_back(Pair("time", cBlock.GetBlockTime()));
	result.push_back(Pair("nonce", (uint64_t) cBlock.GetNonce()));
	result.push_back(Pair("bits", HexBits(cBlock.GetBits())));
	result.push_back(Pair("difficulty", GetDifficulty(cBlockIndex)));
	result.push_back(Pair("chainwork", cBlockIndex->m_cChainWork.GetHex()));
	result.push_back(Pair("fuel", cBlockIndex->m_llFuel));
	result.push_back(Pair("fuelrate", cBlockIndex->m_nFuelRate));
	if (cBlockIndex->m_pPrevBlockIndex) {
		result.push_back(Pair("previousblockhash", cBlockIndex->m_pPrevBlockIndex->GetBlockHash().GetHex()));
	}
	CBlockIndex *pNextBlock = g_cChainActive.Next(cBlockIndex);
	if (pNextBlock) {
		result.push_back(Pair("nextblockhash", pNextBlock->GetBlockHash().GetHex()));
	}
	return result;
}
/**
 * 获取区块总数
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value getblockcount(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		throw runtime_error("getblockcount\n"
				"\nReturns the number of blocks in the longest block chain.\n"
				"\nResult:\n"
				"n    (numeric) The current block count\n"
				"\nExamples:\n" + HelpExampleCli("getblockcount", "") + HelpExampleRpc("getblockcount", ""));
	}

	return g_cChainActive.Height();
}
/**
 * 获取最近的区块HASH
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value getbestblockhash(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		throw runtime_error("getbestblockhash\n"
				"\nReturns the hash of the best (tip) block in the longest block chain.\n"
				"\nResult\n"
				"\"hex\"      (string) the block hash hex encoded\n"
				"\nExamples\n" + HelpExampleCli("getbestblockhash", "") + HelpExampleRpc("getbestblockhash", ""));
	}
	return g_cChainActive.Tip()->GetBlockHash().GetHex();
}
/**
 * 获取当前难度值
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value getdifficulty(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		throw runtime_error("getdifficulty\n"
				"\nReturns the proof-of-work difficulty as a multiple of the minimum difficulty.\n"
				"\nResult:\n"
				"n.nnn       (numeric) the proof-of-work difficulty as a multiple of the minimum difficulty.\n"
				"\nExamples:\n" + HelpExampleCli("getdifficulty", "") + HelpExampleRpc("getdifficulty", ""));
	}

	return GetDifficulty();
}
/**
 * 获得内存池交易标识
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value getrawmempool(const Array& params, bool bHelp) {
	if (bHelp || params.size() > 1) {
		throw runtime_error(
				"getrawmempool ( verbose )\n"
						"\nReturns all transaction ids in memory pool as a json array of string transaction ids.\n"
						"\nArguments:\n"
						"1. verbose           (boolean, optional, default=false) true for a json object, false for array of transaction ids\n"
						"\nResult: (for verbose = false):\n"
						"[                     (json array of string)\n"
						"  \"transactionid\"     (string) The transaction id\n"
						"  ,...\n"
						"]\n"
						"\nResult: (for verbose = true):\n"
						"{                           (json object)\n"
						"  \"transactionid\" : {       (json object)\n"
						"    \"size\" : n,             (numeric) transaction size in bytes\n"
						"    \"fee\" : n,              (numeric) transaction fee in Dacrss\n"
						"    \"time\" : n,             (numeric) local time transaction entered pool in seconds since 1 Jan 1970 GMT\n"
						"    \"height\" : n,           (numeric) block height when transaction entered pool\n"
						"    \"startingpriority\" : n, (numeric) priority when transaction entered pool\n"
						"    \"currentpriority\" : n,  (numeric) transaction priority now\n"
						"    \"depends\" : [           (array) unconfirmed transactions used as inputs for this transaction\n"
						"        \"transactionid\",    (string) parent transaction id\n"
						"       ... ]\n"
						"  }, ...\n"
						"]\n"
						"\nExamples\n" + HelpExampleCli("getrawmempool", "true")
						+ HelpExampleRpc("getrawmempool", "true"));
	}

	bool bFVerbose = false;
	if (params.size() > 0) {
		bFVerbose = params[0].get_bool();
	}

	if (bFVerbose) {
		LOCK(g_cTxMemPool.m_cs);
		Object obj;
		for (const auto& entry : g_cTxMemPool.m_mapTx) {
			const uint256& cHash = entry.first;
			const CTxMemPoolEntry& cTxMemPoolEntry = entry.second;
			Object info;
			info.push_back(Pair("size", (int) cTxMemPoolEntry.GetTxSize()));
			info.push_back(Pair("fee", ValueFromAmount(cTxMemPoolEntry.GetFee())));
			info.push_back(Pair("time", cTxMemPoolEntry.GetTime()));
			info.push_back(Pair("height", (int) cTxMemPoolEntry.GetHeight()));
			info.push_back(Pair("startingpriority", cTxMemPoolEntry.GetPriority(cTxMemPoolEntry.GetHeight())));
			info.push_back(Pair("currentpriority", cTxMemPoolEntry.GetPriority(g_cChainActive.Height())));
			// const CTransaction& tx = e.GetTx();
			set<string> setDepends;
			// BOOST_FOREACH(const CTxIn& txin, tx.vin)
			// {
			//     if (g_cTxMemPool.exists(txin.prevout.hash))
			//     setDepends.insert(txin.prevout.hash.ToString());
			// }
			Array depends(setDepends.begin(), setDepends.end());
			info.push_back(Pair("depends", depends));
			obj.push_back(Pair(cHash.ToString(), info));
		}
		return obj;
	} else {
		vector<uint256> vcTxID;
		g_cTxMemPool.queryHashes(vcTxID);

		Array arr;
		for (const auto& hash : vcTxID) {
			arr.push_back(hash.ToString());
		}

		return arr;
	}
}
/**
 * 获取某一个区块的HASH
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value getblockhash(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		throw runtime_error("getblockhash index\n"
				"\nReturns hash of block in best-block-chain at index provided.\n"
				"\nArguments:\n"
				"1. height         (numeric, required) The block height\n"
				"\nResult:\n"
				"\"hash\"         (string) The block hash\n"
				"\nExamples:\n" + HelpExampleCli("getblockhash", "1000") + HelpExampleRpc("getblockhash", "1000"));
	}
	int nHeight = params[0].get_int();
	if (nHeight < 0 || nHeight > g_cChainActive.Height()) {
		throw runtime_error("Block number out of range.");
	}

	CBlockIndex* pBlockIndex = g_cChainActive[nHeight];
	Object result;
	result.push_back(Pair("hash", pBlockIndex->GetBlockHash().GetHex()));
	return result;
}
/**
 * 获取区块的信息
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value getblock(const Array& params, bool bHelp) {
	if (bHelp || params.size() < 1 || params.size() > 2)
		throw runtime_error(
				"getblock \"hash\" ( verbose )\n"
						"\nIf verbose is false, returns a string that is serialized, hex-encoded data for block 'hash'.\n"
						"If verbose is true, returns an Object with information about block <hash>.\n"
						"\nArguments:\n"
						"1. \"hash or height\"(string or numeric,required) string for The block hash,numeric for the block height\n"
						"2. verbose           (boolean, optional, default=true) true for a json object, false for the hex encoded data\n"
						"\nResult (for verbose = true):\n"
						"{\n"
						"  \"hash\" : \"hash\",     (string) the block hash (same as provided)\n"
						"  \"confirmations\" : n,   (numeric) The number of confirmations\n"
						"  \"size\" : n,            (numeric) The block size\n"
						"  \"height\" : n,          (numeric) The block height or index\n"
						"  \"version\" : n,         (numeric) The block version\n"
						"  \"merkleroot\" : \"xxxx\", (string) The merkle root\n"
						"  \"tx\" : [               (array of string) The transaction ids\n"
						"     \"transactionid\"     (string) The transaction id\n"
						"     ,...\n"
						"  ],\n"
						"  \"time\" : ttt,          (numeric) The block time in seconds since epoch (Jan 1 1970 GMT)\n"
						"  \"nonce\" : n,           (numeric) The nonce\n"
						"  \"bits\" : \"1d00ffff\", (string) The bits\n"
						"  \"difficulty\" : x.xxx,  (numeric) The difficulty\n"
						"  \"previousblockhash\" : \"hash\",  (string) The hash of the previous block\n"
						"  \"nextblockhash\" : \"hash\"       (string) The hash of the next block\n"
						"}\n"
						"\nResult (for verbose=false):\n"
						"\"data\"             (string) A string that is serialized, hex-encoded data for block 'hash'.\n"
						"\nExamples:\n"
						+ HelpExampleCli("getblock",
								"\"00000000c937983704a73af28acdec37b049d214adbda81d7e2a3dd146f6ed09\"")
						+ HelpExampleRpc("getblock",
								"\"00000000c937983704a73af28acdec37b049d214adbda81d7e2a3dd146f6ed09\""));

	std::string strHash;
	if (int_type == params[0].type()) {
		int nHeight = params[0].get_int();
		if (nHeight < 0 || nHeight > g_cChainActive.Height()) {
			throw runtime_error("Block number out of range.");
		}

		CBlockIndex* pcBlockIndex = g_cChainActive[nHeight];
		strHash = pcBlockIndex->GetBlockHash().GetHex();
	} else {
		strHash = params[0].get_str();
	}
	uint256 cHash(uint256S(strHash));

	bool bFVerbose = true;
	if (params.size() > 1) {
		bFVerbose = params[1].get_bool();
	}
	if (g_mapBlockIndex.count(cHash) == 0) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");
	}

	CBlock cBlock;
	CBlockIndex* pcBlockIndex = g_mapBlockIndex[cHash];
	ReadBlockFromDisk(cBlock, pcBlockIndex);

	if (!bFVerbose) {
		CDataStream cBlock(SER_NETWORK, g_sProtocolVersion);
		cBlock << cBlock;
		std::string strHex = HexStr(cBlock.begin(), cBlock.end());
		return strHex;
	}

	return blockToJSON(cBlock, pcBlockIndex);
}
/**
 * 验证区块数据库
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value verifychain(const Array& params, bool bHelp) {
	if (bHelp || params.size() > 2) {
		throw runtime_error(
				"verifychain ( checklevel numblocks )\n"
						"\nVerifies blockchain database.\n"
						"\nArguments:\n"
						"1. checklevel   (numeric, optional, 0-4, default=3) How thorough the block verification is.\n"
						"2. numblocks    (numeric, optional, default=288, 0=all) The number of blocks to check.\n"
						"\nResult:\n"
						"true|false       (boolean) Verified or not\n"
						"\nExamples:\n" + HelpExampleCli("verifychain", "( checklevel numblocks )")
						+ HelpExampleRpc("verifychain", "( checklevel numblocks )"));
	}

	int nCheckLevel = SysCfg().GetArg("-checklevel", 3);
	int nCheckDepth = SysCfg().GetArg("-checkblocks", 288);
	if (params.size() > 0) {
		nCheckLevel = params[0].get_int();
	}
	if (params.size() > 1) {
		nCheckDepth = params[1].get_int();
	}

	return VerifyDB(nCheckLevel, nCheckDepth);
}
/**
 * 获取最近一个区块信息
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value getblockchaininfo(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		throw runtime_error("getblockchaininfo\n"
				"Returns an object containing various state info regarding block chain processing.\n"
				"\nResult:\n"
				"{\n"
				"  \"chain\": \"xxxx\",        (string) current chain (main, testnet3, regtest)\n"
				"  \"blocks\": xxxxxx,         (numeric) the current number of blocks processed in the server\n"
				"  \"bestblockhash\": \"...\", (string) the hash of the currently best block\n"
				"  \"difficulty\": xxxxxx,     (numeric) the current difficulty\n"
				"  \"verificationprogress\": xxxx, (numeric) estimate of verification progress [0..1]\n"
				"  \"chainwork\": \"xxxx\"     (string) total amount of work in active chain, in hexadecimal\n"
				"}\n"
				"\nExamples:\n" + HelpExampleCli("getblockchaininfo", "") + HelpExampleRpc("getblockchaininfo", ""));
	}

	proxyType proxy;
	GetProxy(NET_IPV4, proxy);

	Object obj;
	std::string strChain = SysCfg().DataDir();
	if (strChain.empty()) {
		strChain = "main";
	}
	obj.push_back(Pair("chain", strChain));
	obj.push_back(Pair("blocks", (int) g_cChainActive.Height()));
	obj.push_back(Pair("bestblockhash", g_cChainActive.Tip()->GetBlockHash().GetHex()));
	obj.push_back(Pair("difficulty", (double) GetDifficulty()));
	obj.push_back(Pair("verificationprogress", Checkpoints::GuessVerificationProgress(g_cChainActive.Tip())));
	obj.push_back(Pair("chainwork", g_cChainActive.Tip()->m_cChainWork.GetHex()));
	return obj;
}
/**
 * 测试用
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value listsetblockindexvalid(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		throw runtime_error(
				"listsetblockindexvalid \n"
						"\ncall ListSetBlockIndexValid function\n"
						"\nArguments:\n"
						"\nResult:\n"
						"\nExamples:\n"
						+ HelpExampleCli("listsetblockindexvalid", "")
						+ HelpExampleRpc("listsetblockindexvalid", ""));
	}
	return ListSetBlockIndexValid();
}
/**
 * 获得脚本ID
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value getscriptid(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		throw runtime_error(
				"getscriptid \n"
						"\nreturn an object containing regid and script\n"
						"\nArguments:\n"
						"1. txhash   (string, required) the transaction hash.\n"
						"\nResult:\n"
						"\nExamples:\n"
						+ HelpExampleCli("getscriptid", "5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG")
						+ HelpExampleRpc("getscriptid", "5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG"));
	}

	uint256 cTxhash(uint256S(params[0].get_str()));

	int nIndex = 0;
	int nBlockHeight = GetTxComfirmHigh(cTxhash, *g_pScriptDBTip);
	if (nBlockHeight > g_cChainActive.Height()) {
		throw runtime_error("height larger than tip block \n");
	} else if (nBlockHeight == -1) {
		throw runtime_error("tx hash unconfirmed \n");
	}
	CBlockIndex* pcIndex = g_cChainActive[nBlockHeight];
	CBlock cBlock;
	if (!ReadBlockFromDisk(cBlock, pcIndex)) {
		return false;
	}

	cBlock.BuildMerkleTree();
	std::tuple<bool, int> ret = cBlock.GetTxIndex(cTxhash);
	if (!std::get<0>(ret)) {
		throw runtime_error("tx not exit in block");
	}

	nIndex = std::get<1>(ret);

	CRegID cStriptID(nBlockHeight, nIndex);

	Object result;
	result.push_back(Pair("regid:", cStriptID.ToString()));
	result.push_back(Pair("script", HexStr(cStriptID.GetVec6())));
	return result;
}
/**
 * 列出检查点
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value listcheckpoint(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		throw runtime_error("listcheckpoint index\n"
				"\nget the list of checkpoint.\n"
				"\nResult a object  contain checkpoint\n"
				// "\nResult:\n"
				// "\"hash\"         (string) The block hash\n"
				"\nExamples:\n" + HelpExampleCli("listcheckpoint", "") + HelpExampleRpc("listcheckpoint", ""));
	}

	Object result;
	std::map<int, uint256> mapCheckPoint;
	Checkpoints::GetCheckpointMap(mapCheckPoint);
	for (std::map<int, uint256>::iterator iterCheck = mapCheckPoint.begin(); iterCheck != mapCheckPoint.end();
			++iterCheck) {
		result.push_back(Pair(tfm::format("%d", iterCheck->first).c_str(), iterCheck->second.GetHex()));
	}
	return result;
}
