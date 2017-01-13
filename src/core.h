// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_CORE_H_
#define DACRS_CORE_H_

#include "key.h"
#include "serialize.h"
#include "uint256.h"

#include "base58.h"
#include <stdint.h>
#include <memory>

/** No amount larger than this (in satoshi) is valid */
static const int64_t g_sMaxMoney 		= 1000000000 * COIN;
static const int64_t g_sMaxMoneyRegNet 	= 20 * g_sMaxMoney;

static const int g_sBlockVersion2 	= 2;
static const int g_sBlockVersion3 	= 3;

inline int64_t GetMaxMoney() {
	if (SysCfg().NetworkID() == CBaseParams::EM_REGTEST) {
		return g_sMaxMoneyRegNet;
	} else {
		return g_sMaxMoney;
	}
}

inline bool MoneyRange(int64_t llValue) {
	return (llValue >= 0 && llValue <= GetMaxMoney());
}

class CAccountViewCache;

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader {
 public:
	// header
	static const int m_sCurrentVersion = g_sBlockVersion3;

 public:
	CBlockHeader() {
		SetNull();
	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(this->m_nVersion);
			nVersion = this->m_nVersion;
			READWRITE(m_cHashPrevBlock);
			READWRITE(m_cHashMerkleRoot);
			READWRITE(m_cHashPos);
			READWRITE(m_unTime);
			READWRITE(m_unBits);
			READWRITE(m_unNonce);
			READWRITE(m_unHeight);
			READWRITE(m_llFuel);
			READWRITE(m_nFuelRate);
			READWRITE(m_vchSignature);
	)

	void SetNull() {
		m_nVersion 			= CBlockHeader::m_sCurrentVersion;
		m_cHashPrevBlock 	= uint256();
		m_cHashMerkleRoot 	= uint256();
		m_cHashPos 			= uint256();
		m_unTime 			= 0;
		m_unBits 			= 0;
		m_unNonce 			= 0;
		m_unHeight 			= 0;
		m_llFuel 			= 0;
		m_nFuelRate 		= 100;
		m_vchSignature.clear();
	}

	bool IsNull() const {
		return (m_unBits == 0);
	}

	uint256 GetHash() const;
	uint256 SignatureHash() const;

	int64_t GetBlockTime() const {
		return (int64_t) m_unTime;
	}

	int GetVersion() const {
		return m_nVersion;
	}

	void SetVersion(int nVersion) {
		this->m_nVersion = nVersion;
	}

	uint256 GetHashPrevBlock() const {
		return m_cHashPrevBlock;
	}

	void SetHashPrevBlock(uint256 prevBlockHash) {
		this->m_cHashPrevBlock = prevBlockHash;
	}

	uint256 GetHashMerkleRoot() const {
		return m_cHashMerkleRoot;
	}

	void SetHashMerkleRoot(uint256 merkleRootHash) {
		this->m_cHashMerkleRoot = merkleRootHash;
	}

	uint256 GetHashPos() const {
		return m_cHashPos;
	}

	void SetHashPos(uint256 posHash) {
		this->m_cHashPos = posHash;
	}

	unsigned int GetTime() const {
		return m_unTime;
	}

	void SetTime(unsigned int time) {
		this->m_unTime = time;
	}

	unsigned int GetBits() const {
		return m_unBits;
	}

	void SetBits(unsigned int bits) {
		this->m_unBits = bits;
	}

	unsigned int GetNonce() const {
		return m_unNonce;
	}

	void SetNonce(unsigned int nonce) {
		this->m_unNonce = nonce;
	}

	unsigned int GetHeight() const {
		return m_unHeight;
	}

	void SetHeight(unsigned int height);

	unsigned int GetFuel() const {
		return m_llFuel;
	}

	void SetFuel(int64_t fuel) {
		this->m_llFuel = fuel;
	}

	int GetFuelRate() const {
		return m_nFuelRate;
	}

	void SetFuelRate(int fuelRalte) {
		this->m_nFuelRate = fuelRalte;
	}

	const vector<unsigned char> &GetSignature() const {
		return m_vchSignature;
	}

	void SetSignature(const vector<unsigned char> &signature) {
		this->m_vchSignature = signature;
	}

	void ClearSignature() {
		this->m_vchSignature.clear();
	}

 protected:
	int m_nVersion;
	uint256 m_cHashPrevBlock;
	uint256 m_cHashMerkleRoot;
	uint256 m_cHashPos;   		//nVersion;hashPrevBlock;hashMerkleRoot;nValues(余额);nTime;nNonce;nHeight;nFuel;nFuelRate; 计算的hash
	unsigned int m_unTime;
	unsigned int m_unBits;
	unsigned int m_unNonce;
	unsigned int m_unHeight;
	int64_t m_llFuel;
	int m_nFuelRate;
	vector<unsigned char> m_vchSignature;
};

class CBlock: public CBlockHeader {
 public:
	// network and disk
	vector<std::shared_ptr<CBaseTransaction> > vptx;
	// memory only
	mutable vector<uint256> vMerkleTree;  //块中所有交易的交易hash集合

	CBlock() {
		SetNull();
	}

	CBlock(const CBlockHeader &cBlockHeader) {
		SetNull();
		*((CBlockHeader*)this) = cBlockHeader;
	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(*(CBlockHeader*)this);
			READWRITE(vptx);
	)

	void SetNull() {
		CBlockHeader::SetNull();
		vptx.clear();
		vMerkleTree.clear();
	}

	CBlockHeader GetBlockHeader() const {
		CBlockHeader cBlockHeader;
		cBlockHeader.SetVersion(m_nVersion);
		cBlockHeader.SetHashPrevBlock(m_cHashPrevBlock);
		cBlockHeader.SetHashMerkleRoot(m_cHashMerkleRoot);
		cBlockHeader.SetHashPos(m_cHashPos);
		cBlockHeader.SetTime(m_unTime);
		cBlockHeader.SetBits(m_unBits);
		cBlockHeader.SetNonce(m_unNonce);
		cBlockHeader.SetHeight(m_unHeight);
		cBlockHeader.SetFuel(m_llFuel);
		cBlockHeader.SetFuelRate(m_nFuelRate);
		cBlockHeader.SetSignature(m_vchSignature);
		return cBlockHeader;
	}

	uint256 BuildMerkleTree() const;
	std::tuple<bool, int> GetTxIndex(const uint256& cTxHash) const;

	const uint256 &GetTxHash(unsigned int unIndex) const {
		assert(vMerkleTree.size() > 0); // BuildMerkleTree must have been called first
		assert(unIndex < vptx.size());
		return vMerkleTree[unIndex];
	}

	vector<uint256> GetMerkleBranch(int nIndex) const;
	static uint256 CheckMerkleBranch(uint256 cHash, const vector<uint256>& vcMerkleBranch, int nIndex);
	int64_t GetFee() const;
	void print(CAccountViewCache &cView) const;
};


/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct ST_BlockLocator {
	ST_BlockLocator() {
	}

	ST_BlockLocator(const vector<uint256>& vHaveIn) {
		vcHave = vHaveIn;
	}

	IMPLEMENT_SERIALIZE
	(
			if (!(nType & SER_GETHASH))
			READWRITE(nVersion);
			READWRITE(vcHave);
	)

	void SetNull() {
		vcHave.clear();
	}

	bool IsNull() {
		return vcHave.empty();
	}

	vector<uint256> vcHave;
};

#endif
