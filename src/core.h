// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_CORE_H
#define DACRS_CORE_H

#include "key.h"
#include "serialize.h"
#include "uint256.h"

#include "base58.h"
#include <stdint.h>
#include <memory>

/** No amount larger than this (in satoshi) is valid */
static const int64_t MAX_MONEY = 1000000000 * COIN;
static const int64_t MAX_MONEY_REG_NET = 20 * MAX_MONEY;

static const int g_BlockVersion2 = 2;
static const int g_BlockVersion3 = 3;

inline int64_t GetMaxMoney()
{
	if(SysCfg().NetworkID() == CBaseParams::REGTEST) {
		return MAX_MONEY_REG_NET;
	}
	else {
		return MAX_MONEY;
	}
}
inline bool MoneyRange(int64_t nValue) { return (nValue >= 0 && nValue <= GetMaxMoney()); }

class CAccountViewCache;

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader
{
public:
    // header
    static const int CURRENT_VERSION=g_BlockVersion3;
protected:
    int nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint256 hashPos;   //nVersion;hashPrevBlock;hashMerkleRoot;nValues(���);nTime;nNonce;nHeight;nFuel;nFuelRate; �����hash
    unsigned int nTime;
    unsigned int nBits;
    unsigned int nNonce;
    unsigned int nHeight;
    int64_t    nFuel;
    int nFuelRate;
    vector<unsigned char> vSignature;

public:
    CBlockHeader()
    {
        SetNull();
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(hashPos);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
        READWRITE(nHeight);
        READWRITE(nFuel);
        READWRITE(nFuelRate);
        READWRITE(vSignature);
    )

    void SetNull()
    {
        nVersion = CBlockHeader::CURRENT_VERSION;
        hashPrevBlock = uint256();
        hashMerkleRoot = uint256();
        hashPos = uint256();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
        nHeight = 0;
        nFuel = 0;
        nFuelRate = 100;
        vSignature.clear();
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const;

    uint256 SignatureHash() const;

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }

    int GetVersion() const  {
    	return  nVersion;
    }
    void SetVersion(int nVersion) {
    	this->nVersion = nVersion;
    }
    uint256 GetHashPrevBlock() const  {
    	return hashPrevBlock;
    }
    void SetHashPrevBlock(uint256 prevBlockHash) {
    	this->hashPrevBlock = prevBlockHash;
    }
    uint256 GetHashMerkleRoot() const{
    	return hashMerkleRoot;
    }
    void SetHashMerkleRoot(uint256 merkleRootHash) {
    	this->hashMerkleRoot = merkleRootHash;
    }
    uint256 GetHashPos() const{
    	return hashPos;
    }
    void SetHashPos(uint256 posHash) {
    	this->hashPos = posHash;
    }
    unsigned int GetTime() const{
    	return nTime;
    }
    void SetTime(unsigned int time) {
    	this->nTime = time;
    }
    unsigned int GetBits() const{
    	return nBits;
    }
    void SetBits(unsigned int bits) {
    	this->nBits = bits;
    }
    unsigned int GetNonce() const{
    	return nNonce;
    }
    void SetNonce(unsigned int nonce) {
    	this->nNonce = nonce;
    }
    unsigned int GetHeight() const{
    	return nHeight;
    }
    void SetHeight(unsigned int height);

    unsigned int GetFuel() const{
    	return nFuel;
    }
    void SetFuel(int64_t fuel) {
    	this->nFuel = fuel;
    }
    int GetFuelRate() const{
    	return nFuelRate;
    }
    void SetFuelRate(int fuelRalte) {
    	this->nFuelRate = fuelRalte;
    }
    const vector<unsigned char> &GetSignature() const{
    	return vSignature;
    }
    void SetSignature(const vector<unsigned char> &signature) {
    	this->vSignature = signature;
    }
    void ClearSignature() {
    	this->vSignature.clear();
    }
};


class CBlock : public CBlockHeader
{
public:
    // network and disk
    vector<std::shared_ptr<CBaseTransaction> > vptx;

    // memory only
    mutable vector<uint256> vMerkleTree;  //�������н��׵Ľ���hash����

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        SetNull();
        *((CBlockHeader*)this) = header;
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(*(CBlockHeader*)this);
        READWRITE(vptx);
    )

    void SetNull()
    {
        CBlockHeader::SetNull();
        vptx.clear();
        vMerkleTree.clear();
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.SetVersion(nVersion);
        block.SetHashPrevBlock(hashPrevBlock);
        block.SetHashMerkleRoot(hashMerkleRoot);
        block.SetHashPos(hashPos);
        block.SetTime(nTime);
        block.SetBits(nBits);
        block.SetNonce(nNonce);
        block.SetHeight(nHeight);
        block.SetFuel(nFuel);
        block.SetFuelRate(nFuelRate);
        block.SetSignature(vSignature);
        return block;
    }

    uint256 BuildMerkleTree() const;

    std::tuple<bool, int> GetTxIndex(const uint256& txHash) const;

    const uint256 &GetTxHash(unsigned int nIndex) const {
        assert(vMerkleTree.size() > 0); // BuildMerkleTree must have been called first
        assert(nIndex < vptx.size());
        return vMerkleTree[nIndex];
    }

    vector<uint256> GetMerkleBranch(int nIndex) const;
    static uint256 CheckMerkleBranch(uint256 hash, const vector<uint256>& vMerkleBranch, int nIndex);

    int64_t GetFee() const;

    void print(CAccountViewCache &view) const;
};


/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    vector<uint256> vHave;

    CBlockLocator() {}

    CBlockLocator(const vector<uint256>& vHaveIn)
    {
        vHave = vHaveIn;
    }

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    )

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull()
    {
        return vHave.empty();
    }
};

#endif
