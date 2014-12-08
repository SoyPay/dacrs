// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
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


#include "db.h"
#include "wallet.h"

#include <stdint.h>

#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"

using namespace json_spirit;
using namespace std;


// Key used by getwork miners.
// Allocated in InitRPCMining, free'd in ShutdownRPCMining
//static CReserveKey* pMiningKey = NULL;

void InitRPCMining()
{
    if (!pwalletMain)
        return;

    // getwork/getblocktemplate mining rewards paid here:
//    pMiningKey = new CReserveKey(pwalletMain);
     //assert(0);
	 LogPrint("TODO","InitRPCMining");
}

void ShutdownRPCMining()
{
//    if (!pMiningKey)
//        return;
//
//    delete pMiningKey; pMiningKey = NULL;
}


// Return average network hashes per second based on the last 'lookup' blocks,
// or from the last difficulty change if 'lookup' is nonpositive.
// If 'height' is nonnegative, compute the estimate at the time when a given block was found.
Value GetNetworkHashPS(int lookup, int height) {
    CBlockIndex *pb = chainActive.Tip();

    if (height >= 0 && height < chainActive.Height())
        pb = chainActive[height];

    if (pb == NULL || !pb->nHeight)
        return 0;

    // If lookup is -1, then use blocks since last difficulty change.
    if (lookup <= 0)
        lookup = pb->nHeight % 2016 + 1;

    // If lookup is larger than chain, then set it to chain length.
    if (lookup > pb->nHeight)
        lookup = pb->nHeight;

    CBlockIndex *pb0 = pb;
    int64_t minTime = pb0->GetBlockTime();
    int64_t maxTime = minTime;
    for (int i = 0; i < lookup; i++) {
        pb0 = pb0->pprev;
        int64_t time = pb0->GetBlockTime();
        minTime = min(time, minTime);
        maxTime = max(time, maxTime);
    }

    // In case there's a situation where minTime == maxTime, we don't want a divide by zero exception.
    if (minTime == maxTime)
        return 0;

    uint256 workDiff = pb->nChainWork - pb0->nChainWork;
    int64_t timeDiff = maxTime - minTime;

    return (int64_t)(workDiff.getdouble() / timeDiff);
}

Value getnetworkhashps(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
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
            "\nExamples:\n"
            + HelpExampleCli("getnetworkhashps", "")
            + HelpExampleRpc("getnetworkhashps", "")
       );

    return GetNetworkHashPS(params.size() > 0 ? params[0].get_int() : 120, params.size() > 1 ? params[1].get_int() : -1);
}


Value getgenerate(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getgenerate\n"
            "\nReturn if the server is set to generate coins or not. The default is false.\n"
            "It is set with the command line argument -gen (or soypay.conf setting gen)\n"
            "It can also be set with the setgenerate call.\n"
            "\nResult\n"
            "true|false      (boolean) If the server is set to generate coins or not\n"
            "\nExamples:\n"
            + HelpExampleCli("getgenerate", "")
            + HelpExampleRpc("getgenerate", "")
        );

//    if (!pMiningKey)
//        return false;


    return SysCfg().GetBoolArg("-gen", false);
}


Value setgenerate(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
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
            + HelpExampleCli("setgenerate", "true 1") +
            "\nCheck the setting\n"
            + HelpExampleCli("getgenerate", "") +
            "\nTurn off generation\n"
            + HelpExampleCli("setgenerate", "false") +
            "\nUsing json rpc\n"
            + HelpExampleRpc("setgenerate", "true, 1")
        );

    if (pwalletMain == NULL)
        throw JSONRPCError(RPC_METHOD_NOT_FOUND, "Method not found (disabled)");

    bool fGenerate = true;
    if (params.size() > 0)
        fGenerate = params[0].get_bool();

    int nGenProcLimit = -1;
    if (params.size() > 1)
    {
        nGenProcLimit = params[1].get_int();
        if (nGenProcLimit == 0)
            fGenerate = false;
    }

    // -regtest mode: don't return until nGenProcLimit blocks are generated
    if (fGenerate && SysCfg().NetworkID() == CBaseParams::REGTEST)
    {
        int nHeightStart = 0;
        int nHeightEnd = 0;
        int nHeight = 0;
        int nGenerate = (nGenProcLimit > 0 ? nGenProcLimit : 1);
        {   // Don't keep cs_main locked
            LOCK(cs_main);
            nHeightStart = chainActive.Height();
            nHeight = nHeightStart;
            nHeightEnd = nHeightStart+nGenerate;
        }
        int nHeightLast = -1;
        while (nHeight < nHeightEnd)
        {
            if (nHeightLast != nHeight)
            {
                nHeightLast = nHeight;
                GenerateBitcoins(fGenerate, pwalletMain, 1);
            }
            MilliSleep(1);
            {   // Don't keep cs_main locked
                LOCK(cs_main);
                nHeight = chainActive.Height();
            }
            //break;//leo
        }
    }
    else // Not -regtest: start generate thread, return immediately
    {
    	SysCfg().SoftSetArgCover("-gen", fGenerate ?
    			"1" : "0");
    	SysCfg().SoftSetArgCover("-genproclimit", itostr(nGenProcLimit));
        GenerateBitcoins(fGenerate, pwalletMain, nGenProcLimit);
    }

    return Value::null;
}

Value gethashespersec(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "gethashespersec\n"
            "\nReturns a recent hashes per second performance measurement while generating.\n"
            "See the getgenerate and setgenerate calls to turn generation on and off.\n"
            "\nResult:\n"
            "n            (numeric) The recent hashes per second when generation is on (will return 0 if generation is off)\n"
            "\nExamples:\n"
            + HelpExampleCli("gethashespersec", "")
            + HelpExampleRpc("gethashespersec", "")
        );

    if (GetTimeMillis() - nHPSTimerStart > 8000)
        return (int64_t)0;
    return (int64_t)dHashesPerSec;
}



Value getmininginfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
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
            "\nExamples:\n"
            + HelpExampleCli("getmininginfo", "")
            + HelpExampleRpc("getmininginfo", "")
        );

    Object obj;
    obj.push_back(Pair("blocks",           (int)chainActive.Height()));
    obj.push_back(Pair("currentblocksize", (uint64_t)nLastBlockSize));
    obj.push_back(Pair("currentblocktx",   (uint64_t)nLastBlockTx));
    obj.push_back(Pair("difficulty",       (double)GetDifficulty()));
    obj.push_back(Pair("errors",           GetWarnings("statusbar")));
    obj.push_back(Pair("genproclimit",     (int)SysCfg().GetArg("-genproclimit", -1)));
    obj.push_back(Pair("networkhashps",    getnetworkhashps(params, false)));
    obj.push_back(Pair("pooledtx",         (uint64_t)mempool.size()));
    obj.push_back(Pair("testnet",          TestNet()));
    obj.push_back(Pair("generate",         getgenerate(params, false)));
    obj.push_back(Pair("hashespersec",     gethashespersec(params, false)));
    return obj;
}


#if 0
Value getwork(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "getwork ( \"data\" )\n"
            "\nIf 'data' is not specified, it returns the formatted hash data to work on.\n"
            "If 'data' is specified, tries to solve the block and returns true if it was successful.\n"
            "\nArguments:\n"
            "1. \"data\"       (string, optional) The hex encoded data to solve\n"
            "\nResult (when 'data' is not specified):\n"
            "{\n"
            "  \"midstate\" : \"xxxx\",   (string) The precomputed hash state after hashing the first half of the data (DEPRECATED)\n" // deprecated
            "  \"data\" : \"xxxxx\",      (string) The block data\n"
            "  \"hash1\" : \"xxxxx\",     (string) The formatted hash buffer for second hash (DEPRECATED)\n" // deprecated
            "  \"target\" : \"xxxx\"      (string) The little endian hash target\n"
            "}\n"
            "\nResult (when 'data' is specified):\n"
            "true|false       (boolean) If solving the block specified in the 'data' was successfull\n"
            "\nExamples:\n"
            + HelpExampleCli("getwork", "")
            + HelpExampleRpc("getwork", "")
        );

    if (vNodes.empty())
        throw JSONRPCError(RPC_CLIENT_NOT_CONNECTED, "Bitcoin is not connected!");

    if (IsInitialBlockDownload())
        throw JSONRPCError(RPC_CLIENT_IN_INITIAL_DOWNLOAD, "Bitcoin is downloading blocks...");

    typedef map<uint256, pair<CBlock*, CScript> > mapNewBlock_t;
    static mapNewBlock_t mapNewBlock;    // FIXME: thread safety
    static vector<CBlockTemplate*> vNewBlockTemplate;

    if (params.size() == 0)
    {
        // Update block
        static unsigned int nTransactionsUpdatedLast;
        static CBlockIndex* pindexPrev;
        static int64_t nStart;
        static CBlockTemplate* pblocktemplate;
        if (pindexPrev != chainActive.Tip() ||
            (mempool.GetTransactionsUpdated() != nTransactionsUpdatedLast && GetTime() - nStart > 60))
        {
            if (pindexPrev != chainActive.Tip())
            {
                // Deallocate old blocks since they're obsolete now
                mapNewBlock.clear();
                BOOST_FOREACH(CBlockTemplate* pblocktemplate, vNewBlockTemplate)
                    delete pblocktemplate;
                vNewBlockTemplate.clear();
            }

            // Clear pindexPrev so future getworks make a new block, despite any failures from here on
            pindexPrev = NULL;

            // Store the pindexBest used before CreateNewBlock, to avoid races
            nTransactionsUpdatedLast = mempool.GetTransactionsUpdated();
            CBlockIndex* pindexPrevNew = chainActive.Tip();
            nStart = GetTime();

            // Create new block
            pblocktemplate = CreateNewBlockWithKey(*pMiningKey);
            if (!pblocktemplate)
                throw JSONRPCError(RPC_OUT_OF_MEMORY, "Out of memory");
            vNewBlockTemplate.push_back(pblocktemplate);

            // Need to update only after we know CreateNewBlock succeeded
            pindexPrev = pindexPrevNew;
        }
        CBlock* pblock = &pblocktemplate->block; // pointer for convenience

        // Update nTime
        UpdateTime(*pblock, pindexPrev);
        pblock->nNonce = 0;

        // Update nExtraNonce
        static unsigned int nExtraNonce = 0;
        IncrementExtraNonce(pblock, pindexPrev, nExtraNonce);

        // Save
        mapNewBlock[pblock->hashMerkleRoot] = make_pair(pblock, pblock->vtx[0].vin[0].scriptSig);

        // Pre-build hash buffers
        char pmidstate[32];
        char pdata[128];
        char phash1[64];
        FormatHashBuffers(pblock, pmidstate, pdata, phash1);

        uint256 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint256();

        Object result;
        result.push_back(Pair("midstate", HexStr(BEGIN(pmidstate), END(pmidstate)))); // deprecated
        result.push_back(Pair("data",     HexStr(BEGIN(pdata), END(pdata))));
        result.push_back(Pair("hash1",    HexStr(BEGIN(phash1), END(phash1)))); // deprecated
        result.push_back(Pair("target",   HexStr(BEGIN(hashTarget), END(hashTarget))));
        return result;
    }
    else
    {
        // Parse parameters
        vector<unsigned char> vchData = ParseHex(params[0].get_str());
        if (vchData.size() != 128)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter");
        CBlock* pdata = (CBlock*)&vchData[0];

        // Byte reverse
        for (int i = 0; i < 128/4; i++)
            ((unsigned int*)pdata)[i] = ByteReverse(((unsigned int*)pdata)[i]);

        // Get saved block
        if (!mapNewBlock.count(pdata->hashMerkleRoot))
            return false;
        CBlock* pblock = mapNewBlock[pdata->hashMerkleRoot].first;

        pblock->nTime = pdata->nTime;
        pblock->nNonce = pdata->nNonce;
        pblock->vtx[0].vin[0].scriptSig = mapNewBlock[pdata->hashMerkleRoot].second;
        pblock->hashMerkleRoot = pblock->BuildMerkleTree();

        assert(pwalletMain != NULL);
        return CheckWork(pblock, *pwalletMain, *pMiningKey);
    }
}
#endif


Value submitblock(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "submitblock \"hexdata\" ( \"jsonparametersobject\" )\n"
            "\nAttempts to submit new block to network.\n"
            "The 'jsonparametersobject' parameter is currently ignored.\n"
            "See https://en.bitcoin.it/wiki/BIP_0022 for full specification.\n"

            "\nArguments\n"
            "1. \"hexdata\"    (string, required) the hex-encoded block data to submit\n"
            "2. \"jsonparametersobject\"     (string, optional) object of optional parameters\n"
            "    {\n"
            "      \"workid\" : \"id\"    (string, optional) if the server provided a workid, it MUST be included with submissions\n"
            "    }\n"
            "\nResult:\n"
            "\nExamples:\n"
            + HelpExampleCli("submitblock", "\"mydata\"")
            + HelpExampleRpc("submitblock", "\"mydata\"")
        );

    vector<unsigned char> blockData(ParseHex(params[0].get_str()));
    CDataStream ssBlock(blockData, SER_NETWORK, PROTOCOL_VERSION);
    CBlock pblock;
    try {
        ssBlock >> pblock;
    }
    catch (std::exception &e) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Block decode failed");
    }

    CValidationState state;
    bool fAccepted = ProcessBlock(state, NULL, &pblock);
    Object obj;
	if (!fAccepted) {
		obj.push_back(Pair("status", "rejected"));
		obj.push_back(Pair("reject code", state.GetRejectCode()));
		obj.push_back(Pair("info", state.GetRejectReason()));
	} else {

		obj.push_back(Pair("status", "OK"));
		obj.push_back(Pair("hash", pblock.GetHash().ToString()));
	}
	return obj;
}
