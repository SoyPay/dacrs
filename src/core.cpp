// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "core.h"
#include "util.h"
#include "main.h"
#include "hash.h"

uint256 CBlockHeader::GetHash() const {
	if (m_nVersion >= g_sBlockVersion3) {
		return SignatureHash();
	} else {
		return SerializeHash(*this, SER_GETHASH, g_sClientVersion);
	}
}

void CBlockHeader::SetHeight(unsigned int height) {
	if (height < g_sUpdateBlockVersionHeight && height > 0) {
		SetVersion(g_sBlockVersion2);
	}
	this->m_unHeight = height;
}

uint256 CBlockHeader::SignatureHash() const {
	CHashWriter ss(SER_GETHASH, g_sClientVersion);
	ss << m_nVersion << m_cHashPrevBlock << m_cHashMerkleRoot << m_cHashPos << m_unTime << m_unBits << m_unNonce
			<< m_unHeight << m_llFuel << m_nFuelRate;
	return ss.GetHash();
}

uint256 CBlock::BuildMerkleTree() const {
	vMerkleTree.clear();
	for (const auto& ptx : vptx) {
		vMerkleTree.push_back(std::move(ptx->GetHash()));
	}
	int j = 0;
	for (int nSize = vptx.size(); nSize > 1; nSize = (nSize + 1) / 2) {
		for (int i = 0; i < nSize; i += 2) {
			int i2 = min(i + 1, nSize - 1);
			vMerkleTree.push_back(
					std::move(
							Hash(BEGIN(vMerkleTree[j + i]), END(vMerkleTree[j + i]), BEGIN(vMerkleTree[j + i2]),
									END(vMerkleTree[j + i2]))));
		}
		j += nSize;
	}

	return (vMerkleTree.empty() ? std::move(uint256()) : std::move(vMerkleTree.back()));
}

vector<uint256> CBlock::GetMerkleBranch(int nIndex) const {
	if (vMerkleTree.empty()) {
		BuildMerkleTree();
	}
	vector<uint256> vcMerkleBranch;
	int j = 0;
	for (int nSize = vptx.size(); nSize > 1; nSize = (nSize + 1) / 2) {
		int i = min(nIndex ^ 1, nSize - 1);
		vcMerkleBranch.push_back(vMerkleTree[j + i]);
		nIndex >>= 1;
		j += nSize;
	}

	return vcMerkleBranch;
}

uint256 CBlock::CheckMerkleBranch(uint256 cHash, const vector<uint256>& vcMerkleBranch, int nIndex) {
	if (nIndex == -1) {
		return uint256();
	}
	for (const auto& otherside : vcMerkleBranch) {
		if (nIndex & 1) {
			cHash = Hash(BEGIN(otherside), END(otherside), BEGIN(cHash), END(cHash));
		} else {
			cHash = Hash(BEGIN(cHash), END(cHash), BEGIN(otherside), END(otherside));
		}
		nIndex >>= 1;
	}

	return cHash;
}

int64_t CBlock::GetFee() const {
	int64_t llFees = 0;
	for (unsigned int i = 1; i < vptx.size(); ++i) {
		llFees += vptx[i]->GetFee();
	}

	return llFees;
}

void CBlock::print(CAccountViewCache &cView) const {
	LogPrint("INFO",
			"CBlock(hash=%s, m_nVersion=%d, m_cHashPrevBlock=%s, m_cHashMerkleRoot=%s, m_cHashPos=%s, m_unTime=%u, m_unBits=%08x, m_unNonce=%u, vtx=%u, m_llFuel=%d, m_nFuelRate=%d)\n",
			GetHash().ToString(), m_nVersion, m_cHashPrevBlock.ToString(), m_cHashMerkleRoot.ToString(),
			m_cHashPos.ToString(), m_unTime, m_unBits, m_unNonce, vptx.size(), m_llFuel, m_nFuelRate);
//	LogPrint("INFO","list transactions: \n");
//    for (unsigned int i = 0; i < vptx.size(); i++)
//    {
//    	LogPrint("INFO","%s ", vptx[i]->ToString(view));
//    }
//    LogPrint("INFO","  vMerkleTree: ");
//    for (unsigned int i = 0; i < vMerkleTree.size(); i++)
//    	LogPrint("INFO","%s ", vMerkleTree[i].ToString());
	LogPrint("INFO", "\n");
}

std::tuple<bool, int> CBlock::GetTxIndex(const uint256& cTxHash) const {
	for (size_t i = 0; i < vMerkleTree.size(); i++) {
		if (cTxHash == vMerkleTree[i]) {
			return std::make_tuple(true, i);
		}
	}

	return std::make_tuple(false, 0);
}

