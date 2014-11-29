// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_MINER_H
#define BITCOIN_MINER_H

#include <stdint.h>
#include <vector>
#include <map>
#include <set>

#include "uint256.h"
#include "key.h"
#include "boost/tuple/tuple.hpp"
#include <memory>

class CBlock;
class CBlockIndex;
struct CBlockTemplate;
//class CReserveKey;
//class CScript;
class CWallet;
class CBaseTransaction;
class COrphan;
class CAccountViewCache;
class CTransactionCache;
class CScriptDBViewCache;

typedef boost::tuple<double, double, std::shared_ptr<CBaseTransaction> > TxPriority;
class TxPriorityCompare
{
    bool byFee;
public:
    TxPriorityCompare(bool _byFee) : byFee(_byFee) { }
    bool operator()(const TxPriority& a, const TxPriority& b)
    {
        if (byFee)
        {
            if (a.get<1>() == b.get<1>())
                return a.get<0>() < b.get<0>();
            return a.get<1>() < b.get<1>();
        }
        else
        {
            if (a.get<0>() == b.get<0>())
                return a.get<1>() < b.get<1>();
            return a.get<0>() < b.get<0>();
        }
    }
};

/** Run the miner threads */
void GenerateBitcoins(bool fGenerate, CWallet* pwallet, int nThreads);
/** Generate a new block, without valid proof-of-work */
//CBlockTemplate* CreateNewBlock(const CScript& scriptPubKeyIn);
//CBlockTemplate* CreateNewBlockWithKey(CReserveKey& reservekey);

CBlockTemplate* CreateNewBlock();
/** Modify the extranonce in a block */
void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce);
/** Do mining precalculation */
void FormatHashBuffers(CBlock* pblock, char* pmidstate, char* pdata, char* phash1);

bool CreatePosTx(const CBlockIndex *pPrevIndex, CBlock *pBlock,set<CKeyID>&setCreateKey);

bool VerifyPosTx(const CBlockIndex *pPrevIndex, CAccountViewCache &accView, const CBlock *pBlock, uint64_t &nInterest, CTransactionCache &txCache, CScriptDBViewCache &scriptCache, bool bJustCheckSign = false);
/** Check mined block */
bool CheckWork(CBlock* pblock, CWallet& wallet);
/** Base sha256 mining transform */
void SHA256Transform(void* pstate, void* pinput, const void* pinit);
/** Get burn element */
uint64_t GetElementForBurn(CBlockIndex *pindex);

void GetPriorityTx(vector<TxPriority> &vecPriority, map<uint256, vector<COrphan*> > &mapDependers);
extern double dHashesPerSec;
extern int64_t nHPSTimerStart;

extern uint256 CreateBlockWithAppointedAddr(CKeyID const &keyID);

#endif // BITCOIN_MINER_H
