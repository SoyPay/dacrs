// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

#include <stdint.h>
#include "syncdatadb.h"

#include <boost/assign/list_of.hpp> // for 'map_list_of()'

namespace Checkpoints {
typedef map<int, uint256> MapCheckPoints; // the first parameter is  nHeight;
CCriticalSection g_cs_checkPoint;

// How many times we expect transactions after the last checkpoint to
// be slower. This number is a compromise, as it can't be accurate for
// every system. When reindexing from a fast disk with a slow CPU, it
// can be up to 20, while when downloading from a slow network with a
// fast multicore CPU, it won't be much higher than 1.
static const double g_sSigCheckVerificationFactor = 5.0;

struct CCheckPointData {
	MapCheckPoints *pmapCheckPoints;
	int64_t llTimeLastCheckpoint;
	int64_t llTransactionsLastCheckpoint;
	double dTransactionsPerDay;
};

bool g_bEnabled = true;

// What makes a good checkpoint block?
// + Is surrounded by blocks with reasonable timestamps
//   (no blocks before with a timestamp after, none after with
//    timestamp before)
// + Contains no strange transactions
static MapCheckPoints g_smapCheckPointsMainnet = boost::assign::map_list_of(0,
		uint256S("1f0d05a703a917511558f046529c48ad53b55c5b16c5d432fab8773a4b5ed4f1"));
static const CCheckPointData g_sCheckPointDataMainnet = { &g_smapCheckPointsMainnet, 0,      // * UNIX timestamp of last checkpoint block
		0,      // * total number of transactions between genesis and last checkpoint
				//   (the tx=... number in the SetBestChain debug.log lines)
		0       // * estimated number of transactions per day after checkpoint
		};

static MapCheckPoints g_smapCheckPointsTestnet = boost::assign::map_list_of(0,
		uint256S("c28af610f0fb593e6194cef9195f154327577fc20b50018ccc822a7940d2b92d"));
static const CCheckPointData g_sCheckPointDataTestnet = { &g_smapCheckPointsTestnet, 0, 0, 0 };

static MapCheckPoints g_smapCheckPointsRegtest = boost::assign::map_list_of(0,
		uint256S("708d5c14424395963cd11bb3f2ff791f584efbeb59fe5922f2131bfc879cd1f7"));
static const CCheckPointData g_sCheckPointDataDataRegtest = { &g_smapCheckPointsRegtest, 0, 0, 0 };

const CCheckPointData &Checkpoints() {
	if (SysCfg().NetworkID() == CBaseParams::EM_TESTNET) {
		return g_sCheckPointDataTestnet;
	} else if (SysCfg().NetworkID() == CBaseParams::EM_MAIN) {
		return g_sCheckPointDataMainnet;
	} else {
		return g_sCheckPointDataDataRegtest;
	}
}

bool CheckBlock(int nHeight, const uint256& cHash) { //nHeight 找不到或 高度和hash都能找到，则返回true
	if (!g_bEnabled) {
		return true;
	}

	const MapCheckPoints& mapCheckPoints = *Checkpoints().pmapCheckPoints;
	MapCheckPoints::const_iterator iterMapCheckPoints = mapCheckPoints.find(nHeight);
	if (iterMapCheckPoints == mapCheckPoints.end()) {
		return true;
	}

	return cHash == iterMapCheckPoints->second;
}

// Guess how far we are in the verification process at the given block index
double GuessVerificationProgress(CBlockIndex *pBlockIndex, bool bSigchecks) {
	if (pBlockIndex == NULL) {
		return 0.0;
	}

	int64_t llNow = time(NULL);
	double dSigcheckVerificationFactor = bSigchecks ? g_sSigCheckVerificationFactor : 1.0;
	double dWorkBefore 	= 0.0; 	// Amount of work done before pindex
	double dWorkAfter 	= 0.0;  // Amount of work left after pindex (estimated)
	// Work is defined as: 1.0 per transaction before the last checkpoint, and
	// fSigcheckVerificationFactor per transaction after.
	const CCheckPointData &cCheckPointData = Checkpoints();

	if (pBlockIndex->m_unChainTx <= cCheckPointData.llTransactionsLastCheckpoint) {
		double dCheapBefore = pBlockIndex->m_unChainTx;
		double dCheapAfter 	= cCheckPointData.llTransactionsLastCheckpoint - pBlockIndex->m_unChainTx;
		double dExpensiveAfter = (llNow - cCheckPointData.llTimeLastCheckpoint) / 86400.0 * cCheckPointData.dTransactionsPerDay;
		dWorkBefore = dCheapBefore;
		dWorkAfter 	= dCheapAfter + dExpensiveAfter * dSigcheckVerificationFactor;
	} else {
		double dCheapBefore 	= cCheckPointData.llTransactionsLastCheckpoint;
		double dExpensiveBefore = pBlockIndex->m_unChainTx - cCheckPointData.llTransactionsLastCheckpoint;
		double dExpensiveAfter 	= (llNow - pBlockIndex->m_unTime) / 86400.0 * cCheckPointData.dTransactionsPerDay;
		dWorkBefore = dCheapBefore + dExpensiveBefore * dSigcheckVerificationFactor;
		dWorkAfter 	= dExpensiveAfter * dSigcheckVerificationFactor;
	}

	return dWorkBefore / (dWorkBefore + dWorkAfter);
}

int GetTotalBlocksEstimate() {    // 获取mapCheckpoints 中保存最后一个checkpoint 的高度
	if (!g_bEnabled) {
		return 0;
	}
	const MapCheckPoints& mapCheckPoints = *Checkpoints().pmapCheckPoints;

	return mapCheckPoints.rbegin()->first;
}

CBlockIndex* GetLastCheckpoint(const map<uint256, CBlockIndex*>& mapBlockIndex) {
	if (!g_bEnabled) {
		return NULL;
	}
	const MapCheckPoints& mapCheckPoints = *Checkpoints().pmapCheckPoints;

	BOOST_REVERSE_FOREACH(const MapCheckPoints::value_type& i, mapCheckPoints)
	{
		const uint256& hash = i.second;
		map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
		if (t != mapBlockIndex.end())
		return t->second;
	}

	return NULL;
}

bool LoadCheckpoint() {
	LOCK(g_cs_checkPoint);
	SyncData::CSyncDataDb cSyncDataDb;
	return cSyncDataDb.LoadCheckPoint(*Checkpoints().pmapCheckPoints);
}

bool GetCheckpointByHeight(const int nHeight, std::vector<int> &vnCheckPoints) {
	LOCK(g_cs_checkPoint);
	MapCheckPoints& mapCheckPoints = *Checkpoints().pmapCheckPoints;
	std::map<int, uint256>::iterator iterMap = mapCheckPoints.upper_bound(nHeight);
	while (iterMap != mapCheckPoints.end()) {
		vnCheckPoints.push_back(iterMap->first);
		++iterMap;
	}

	return !vnCheckPoints.empty();
}

bool AddCheckpoint(int nHeight, uint256 cHash) {
	LOCK(g_cs_checkPoint);
	MapCheckPoints& mapCheckPoints = *Checkpoints().pmapCheckPoints;
	mapCheckPoints.insert(mapCheckPoints.end(), make_pair(nHeight, cHash));

	return true;
}

void GetCheckpointMap(std::map<int, uint256> &mapCheckPoints) {
	LOCK(g_cs_checkPoint);
	const MapCheckPoints& mapCheckpoints = *Checkpoints().pmapCheckPoints;
	mapCheckPoints = mapCheckpoints;
}

}
