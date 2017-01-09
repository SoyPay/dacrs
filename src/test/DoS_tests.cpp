// Copyright (c) 2011-2014 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//
// Unit tests for denial-of-service detection/prevention code
//

#include "bignum.h"
#include "keystore.h"
#include "main.h"
#include "net.h"
#include "key.h"
#include "serialize.h"

#include <stdint.h>

#include <boost/assign/list_of.hpp> // for 'map_list_of()'#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

// Tests this internal-to-main.cpp method:
extern bool AddOrphanTx(const CTransaction& tx);
extern unsigned int LimitOrphanTxSize(unsigned int nMaxOrphans);
extern std::map<uint256, CTransaction> g_mapOrphanTransactions;
extern std::map<uint256, std::set<uint256> > g_mapOrphanTransactionsByPrev;

CService ip(uint32_t i) {
	struct in_addr s;
	s.s_addr = i;
	return CService(CNetAddr(s), SysCfg().GetDefaultPort());
}

BOOST_AUTO_TEST_SUITE(DoS_tests)

BOOST_AUTO_TEST_CASE(DoS_banning) {
	CNode::ClearBanned();
	CAddress cAddr1(ip(0xa0b0c001));
	CNode cDummyNode1(INVALID_SOCKET, cAddr1, "", true);
	cDummyNode1.m_nVersion = 1;
	Misbehaving(cDummyNode1.GetId(), 100); // Should get banned
	SendMessages(&cDummyNode1, false);
	BOOST_CHECK(CNode::IsBanned(cAddr1));
	BOOST_CHECK(!CNode::IsBanned(ip(0xa0b0c001 | 0x0000ff00))); // Different IP, not banned

	CAddress cAddr2(ip(0xa0b0c002));
	CNode cDummyNode2(INVALID_SOCKET, cAddr2, "", true);
	cDummyNode2.m_nVersion = 1;
	Misbehaving(cDummyNode2.GetId(), 50);
	SendMessages(&cDummyNode2, false);
	BOOST_CHECK(!CNode::IsBanned(cAddr2)); // 2 not banned yet...
	BOOST_CHECK(CNode::IsBanned(cAddr1)); // ... but 1 still should be
	Misbehaving(cDummyNode2.GetId(), 50);
	SendMessages(&cDummyNode2, false);
	BOOST_CHECK(CNode::IsBanned(cAddr2));
}

BOOST_AUTO_TEST_CASE(DoS_banscore) {
	CNode::ClearBanned();
	string strOldValue = SysCfg().GetArg("-banscore", "");
	SysCfg().SoftSetArgCover("-banscore", "111"); // because 11 is my favorite number
	CAddress cAddr1(ip(0xa0b0c001));
	CNode cDummyNode1(INVALID_SOCKET, cAddr1, "", true);
	cDummyNode1.m_nVersion = 1;
	Misbehaving(cDummyNode1.GetId(), 100);
	SendMessages(&cDummyNode1, false);
	BOOST_CHECK(!CNode::IsBanned(cAddr1));
	Misbehaving(cDummyNode1.GetId(), 10);
	SendMessages(&cDummyNode1, false);
	BOOST_CHECK(!CNode::IsBanned(cAddr1));
	Misbehaving(cDummyNode1.GetId(), 1);
	SendMessages(&cDummyNode1, false);
	BOOST_CHECK(CNode::IsBanned(cAddr1));
	if ("" == strOldValue) {
		SysCfg().EraseArg("-banscore");
	} else {
		SysCfg().SoftSetArgCover("-banscore", strOldValue.c_str());
	}
}

BOOST_AUTO_TEST_CASE(DoS_bantime) {
	CNode::ClearBanned();
	int64_t llStartTime = GetTime();
	SetMockTime(llStartTime); // Overrides future calls to GetTime()

	CAddress cAddr(ip(0xa0b0c001));
	CNode cDummyNode(INVALID_SOCKET, cAddr, "", true);
	cDummyNode.m_nVersion = 1;

	Misbehaving(cDummyNode.GetId(), 100);
	SendMessages(&cDummyNode, false);
	BOOST_CHECK(CNode::IsBanned(cAddr));

	SetMockTime(llStartTime + 60 * 60);
	BOOST_CHECK(CNode::IsBanned(cAddr));

	SetMockTime(llStartTime + 60 * 60 * 24 + 1);
	BOOST_CHECK(!CNode::IsBanned(cAddr));
}

static bool CheckNBits(unsigned int unbits1, int64_t llTime1, unsigned int unbits2, int64_t llTime2) {
	if (llTime1 > llTime2) {
		return CheckNBits(unbits2, llTime2, unbits1, llTime1);
	}
	int64_t llDeltaTime = llTime2 - llTime1;

	CBigNum cRequired;
	cRequired.SetCompact(ComputeMinWork(unbits1, llDeltaTime));
	CBigNum cHave;
	cHave.SetCompact(unbits2);
	return (cHave <= cRequired);
}

BOOST_AUTO_TEST_CASE(DoS_checknbits) {
	using namespace boost::assign;
	// for 'map_list_of()'
	// Timestamps,nBits from the Dacrs block chain.
	// These are the block-chain checkpoint blocks
	typedef std::map<int64_t, unsigned int> BlockData;
	BlockData cChainData =
	map_list_of(1239852051,486604799)(1262749024,486594666)
	(1279305360,469854461)(1280200847,469830746)(1281678674,469809688)
	(1296207707,453179945)(1302624061,453036989)(1309640330,437004818)
	(1313172719,436789733);

	// Make sure CheckNBits considers every combination of block-chain-lock-in-points
	// "sane":
	for (const auto& i : cChainData) {
		for (const auto& j : cChainData) {
			BOOST_CHECK(CheckNBits(i.second, i.first, j.second, j.first));
		}
	}

	// Test a couple of insane combinations:
	BlockData::value_type firstcheck = *(cChainData.begin());
	BlockData::value_type lastcheck = *(cChainData.rbegin());

	// First checkpoint difficulty at or a while after the last checkpoint time should fail when
	// compared to last checkpoint
	BOOST_CHECK(!CheckNBits(firstcheck.second, lastcheck.first + 60 * 10, lastcheck.second, lastcheck.first));
	BOOST_CHECK(!CheckNBits(firstcheck.second, lastcheck.first + 60 * 60 * 24 * 14, lastcheck.second, lastcheck.first));

	// ... but OK if enough time passed for difficulty to adjust downward:
	BOOST_CHECK(
			CheckNBits(firstcheck.second, lastcheck.first + 60 * 60 * 24 * 365 * 4, lastcheck.second, lastcheck.first));
}

CTransaction RandomOrphan() {
	std::map<uint256, CTransaction>::iterator it;
	it = g_mapOrphanTransactions.lower_bound(GetRandHash());
	if (it == g_mapOrphanTransactions.end()) {
		it = g_mapOrphanTransactions.begin();
	}
	return it->second;
}

//BOOST_AUTO_TEST_CASE(DoS_checkSig)
//{
//    // Test signature caching code (see key.cpp Verify() methods)
//
//    CKey key;
//    key.MakeNewKey(true);
//    CBasicKeyStore keystore;
//    keystore.AddKey(key);
//    unsigned int flags = SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_STRICTENC;
//
//    // 100 orphan transactions:
//    static const int NPREV=100;
//    CTransaction orphans[NPREV];
//    for (int i = 0; i < NPREV; i++)
//    {
//        CTransaction& tx = orphans[i];
//        tx.vin.resize(1);
//        tx.vin[0].prevout.n = 0;
//        tx.vin[0].prevout.hash = GetRandHash();
//        tx.vin[0].scriptSig << OP_1;
//        tx.vout.resize(1);
//        tx.vout[0].nValue = 1*CENT;
//        tx.vout[0].scriptPubKey.SetDestination(key.GetPubKey().GetID());
//
//        AddOrphanTx(tx);
//    }
//
//    // Create a transaction that depends on orphans:
//    CTransaction tx;
//    tx.vout.resize(1);
//    tx.vout[0].nValue = 1*CENT;
//    tx.vout[0].scriptPubKey.SetDestination(key.GetPubKey().GetID());
//    tx.vin.resize(NPREV);
//    for (unsigned int j = 0; j < tx.vin.size(); j++)
//    {
//        tx.vin[j].prevout.n = 0;
//        tx.vin[j].prevout.hash = orphans[j].GetHash();
//    }
//    // Creating signatures primes the cache:
//    boost::posix_time::ptime mst1 = boost::posix_time::microsec_clock::local_time();
//    for (unsigned int j = 0; j < tx.vin.size(); j++)
//        BOOST_CHECK(SignSignature(keystore, orphans[j], tx, j));
//    boost::posix_time::ptime mst2 = boost::posix_time::microsec_clock::local_time();
//    boost::posix_time::time_duration msdiff = mst2 - mst1;
//    long nOneValidate = msdiff.total_milliseconds();
//    if (fDebug) printf("DoS_Checksig sign: %ld\n", nOneValidate);
//
//    // ... now validating repeatedly should be quick:
//    // 2.8GHz machine, -g build: Sign takes ~760ms,
//    // uncached Verify takes ~250ms, cached Verify takes ~50ms
//    // (for 100 single-signature inputs)
//    mst1 = boost::posix_time::microsec_clock::local_time();
//    for (unsigned int i = 0; i < 5; i++)
//        for (unsigned int j = 0; j < tx.vin.size(); j++)
//            BOOST_CHECK(VerifySignature(CCoins(orphans[j], MEMPOOL_HEIGHT), tx, j, flags, SIGHASH_ALL));
//    mst2 = boost::posix_time::microsec_clock::local_time();
//    msdiff = mst2 - mst1;
//    long nManyValidate = msdiff.total_milliseconds();
//    if (fDebug) printf("DoS_Checksig five: %ld\n", nManyValidate);
//
//    BOOST_CHECK_MESSAGE(nManyValidate < nOneValidate, "Signature cache timing failed");
//
//    // Empty a signature, validation should fail:
//    CScript save = tx.vin[0].scriptSig;
//    tx.vin[0].scriptSig = CScript();
//    BOOST_CHECK(!VerifySignature(CCoins(orphans[0], MEMPOOL_HEIGHT), tx, 0, flags, SIGHASH_ALL));
//    tx.vin[0].scriptSig = save;
//
//    // Swap signatures, validation should fail:
//    std::swap(tx.vin[0].scriptSig, tx.vin[1].scriptSig);
//    BOOST_CHECK(!VerifySignature(CCoins(orphans[0], MEMPOOL_HEIGHT), tx, 0, flags, SIGHASH_ALL));
//    BOOST_CHECK(!VerifySignature(CCoins(orphans[1], MEMPOOL_HEIGHT), tx, 1, flags, SIGHASH_ALL));
//    std::swap(tx.vin[0].scriptSig, tx.vin[1].scriptSig);
//
//    // Exercise -maxsigcachesize code:
//    mapArgs["-maxsigcachesize"] = "10";
//    // Generate a new, different signature for vin[0] to trigger cache clear:
//    CScript oldSig = tx.vin[0].scriptSig;
//    BOOST_CHECK(SignSignature(keystore, orphans[0], tx, 0));
//    BOOST_CHECK(tx.vin[0].scriptSig != oldSig);
//    for (unsigned int j = 0; j < tx.vin.size(); j++)
//        BOOST_CHECK(VerifySignature(CCoins(orphans[j], MEMPOOL_HEIGHT), tx, j, flags, SIGHASH_ALL));
//    mapArgs.erase("-maxsigcachesize");
//
//    LimitOrphanTxSize(0);
//}

BOOST_AUTO_TEST_SUITE_END()
