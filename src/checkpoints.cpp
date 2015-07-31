// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

#include <stdint.h>
#include "syncdatadb.h"

#include <boost/assign/list_of.hpp> // for 'map_list_of()'

namespace Checkpoints
{
    typedef map<int, uint256> MapCheckpoints; // the first parameter is  nHeight;
    CCriticalSection cs_checkPoint;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double SIGCHECK_VERIFICATION_FACTOR = 5.0;

    struct CCheckpointData {
        MapCheckpoints *mapCheckpoints;
        int64_t nTimeLastCheckpoint;
        int64_t nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    bool fEnabled = true;

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        ( 0, uint256("1f0d05a703a917511558f046529c48ad53b55c5b16c5d432fab8773a4b5ed4f1"));
    static const CCheckpointData data = {
        &mapCheckpoints,
        0,      // * UNIX timestamp of last checkpoint block
        0,      // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the SetBestChain debug.log lines)
        0       // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        ( 0, uint256("c28af610f0fb593e6194cef9195f154327577fc20b50018ccc822a7940d2b92d"));

    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        0,
        0,
        0
    };

    static MapCheckpoints mapCheckpointsRegtest =
        boost::assign::map_list_of
        ( 0, uint256("708d5c14424395963cd11bb3f2ff791f584efbeb59fe5922f2131bfc879cd1f7"))
        ;
    static const CCheckpointData dataRegtest = {
        &mapCheckpointsRegtest,
        0,
        0,
        0
    };

    const CCheckpointData &Checkpoints() {
        if (SysCfg().NetworkID() == CBaseParams::TESTNET)
            return dataTestnet;
        else if (SysCfg().NetworkID() == CBaseParams::MAIN)
            return data;
        else
            return dataRegtest;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    { //nHeight 找不到或 高度和hash都能找到，则返回true
        if (!fEnabled)
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex, bool fSigchecks) {
        if (pindex==NULL)
            return 0.0;

        int64_t nNow = time(NULL);

        double fSigcheckVerificationFactor = fSigchecks ? SIGCHECK_VERIFICATION_FACTOR : 1.0;
        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkpoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {    // 获取mapCheckpoints 中保存最后一个checkpoint 的高度
        if (!fEnabled)
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (!fEnabled)
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

	bool LoadCheckpoint() {
		LOCK(cs_checkPoint);
		SyncData::CSyncDataDb db;
		return db.LoadCheckPoint(*Checkpoints().mapCheckpoints);
	}

	bool GetCheckpointByHeight(const int nHeight, std::vector<int> &vCheckpoints) {
		LOCK(cs_checkPoint);
		MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;
		std::map<int, uint256>::iterator iterMap = checkpoints.upper_bound(nHeight);
		while (iterMap != checkpoints.end()) {
			vCheckpoints.push_back(iterMap->first);
			++iterMap;
		}
		return !vCheckpoints.empty();
	}

	bool AddCheckpoint(int nHeight, uint256 hash) {
		LOCK(cs_checkPoint);
		MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;
		checkpoints.insert(checkpoints.end(), make_pair(nHeight, hash));
		return true;
	}

	void GetCheckpointMap(std::map<int, uint256> &mapCheckpoints) {
		LOCK(cs_checkPoint);
		const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;
		mapCheckpoints = checkpoints;
	}

}
