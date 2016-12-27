// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_CHECKPOINTS_H_
#define DACRS_CHECKPOINTS_H_

#include <map>
#include <vector>

class CBlockIndex;
class uint256;

/** Block-chain checkpoints are compiled-in sanity checks.
 * They are updated every release or three.
 */
namespace Checkpoints {
// Returns true if block passes checkpoint checks
bool CheckBlock(int nHeight, const uint256& cHash);

// Return conservative estimate of total number of blocks, 0 if unknown
int GetTotalBlocksEstimate();

// Returns last CBlockIndex* in mapBlockIndex that is a checkpoint
CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex);

double GuessVerificationProgress(CBlockIndex *pBlockIndex, bool bSigchecks = true);

bool AddCheckpoint(int nHeight, uint256 cHash);

bool GetCheckpointByHeight(const int nHeight, std::vector<int> &vnCheckPoints);

bool LoadCheckpoint();

void GetCheckpointMap(std::map<int, uint256> &mapCheckPoints);

extern bool bEnabled;
}

#endif
