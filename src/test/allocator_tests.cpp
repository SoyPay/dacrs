// Copyright (c) 2012-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "util.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(allocator_tests)

// Dummy memory page locker for platform independent tests
static const void *g_psLastLockAddr, *g_psLastUnlockAddr;
static size_t g_sLastLockLen, g_sLastUnlockLen;

class TestLocker {
 public:
	bool Lock(const void *pAddr, size_t unLen) {
		g_psLastLockAddr = pAddr;
		g_sLastLockLen = unLen;
		return true;
	}

	bool Unlock(const void *pAddr, size_t unLen) {
		g_psLastUnlockAddr = pAddr;
		g_sLastUnlockLen = unLen;
		return true;
	}
};

BOOST_AUTO_TEST_CASE(test_LockedPageManagerBase) {
	const size_t kTestPageSize = 4096;
	LockedPageManagerBase<TestLocker> cLpm(kTestPageSize);
	size_t unAddr;
	g_psLastLockAddr = g_psLastUnlockAddr = 0;
	g_sLastLockLen = g_sLastUnlockLen = 0;

	/* Try large number of small objects */
	unAddr = 0;
	for (int i = 0; i < 1000; ++i) {
		cLpm.LockRange(reinterpret_cast<void*>(unAddr), 33);
		unAddr += 33;
	}
	/* Try small number of page-sized objects, straddling two pages */
	unAddr = kTestPageSize * 100 + 53;
	for (int i = 0; i < 100; ++i) {
		cLpm.LockRange(reinterpret_cast<void*>(unAddr), kTestPageSize);
		unAddr += kTestPageSize;
	}
	/* Try small number of page-sized objects aligned to exactly one page */
	unAddr = kTestPageSize * 300;
	for (int i = 0; i < 100; ++i) {
		cLpm.LockRange(reinterpret_cast<void*>(unAddr), kTestPageSize);
		unAddr += kTestPageSize;
	}
	/* one very large object, straddling pages */
	cLpm.LockRange(reinterpret_cast<void*>(kTestPageSize * 600 + 1), kTestPageSize * 500);
	BOOST_CHECK(g_psLastLockAddr == reinterpret_cast<void*>(kTestPageSize * (600 + 500)));
	/* one very large object, page aligned */
	cLpm.LockRange(reinterpret_cast<void*>(kTestPageSize * 1200), kTestPageSize * 500 - 1);
	BOOST_CHECK(g_psLastLockAddr == reinterpret_cast<void*>(kTestPageSize * (1200 + 500 - 1)));

	BOOST_CHECK(cLpm.GetLockedPageCount() == ((1000 * 33 + kTestPageSize - 1) / kTestPageSize + // small objects
			101 + 100 +  // page-sized objects
			501 + 500)); // large objects
	BOOST_CHECK((g_sLastLockLen & (kTestPageSize - 1)) == 0); // always lock entire pages
	BOOST_CHECK(g_sLastUnlockLen == 0); // nothing unlocked yet

	/* And unlock again */
	unAddr = 0;
	for (int i = 0; i < 1000; ++i) {
		cLpm.UnlockRange(reinterpret_cast<void*>(unAddr), 33);
		unAddr += 33;
	}
	unAddr = kTestPageSize * 100 + 53;
	for (int i = 0; i < 100; ++i) {
		cLpm.UnlockRange(reinterpret_cast<void*>(unAddr), kTestPageSize);
		unAddr += kTestPageSize;
	}
	unAddr = kTestPageSize * 300;
	for (int i = 0; i < 100; ++i) {
		cLpm.UnlockRange(reinterpret_cast<void*>(unAddr), kTestPageSize);
		unAddr += kTestPageSize;
	}
	cLpm.UnlockRange(reinterpret_cast<void*>(kTestPageSize * 600 + 1), kTestPageSize * 500);
	cLpm.UnlockRange(reinterpret_cast<void*>(kTestPageSize * 1200), kTestPageSize * 500 - 1);

	/* Check that everything is released */
	BOOST_CHECK(cLpm.GetLockedPageCount() == 0);

	/* A few and unlocks of size zero (should have no effect) */
	unAddr = 0;
	for (int i = 0; i < 1000; ++i) {
		cLpm.LockRange(reinterpret_cast<void*>(unAddr), 0);
		unAddr += 1;
	}
	BOOST_CHECK(cLpm.GetLockedPageCount() == 0);
	unAddr = 0;
	for (int i = 0; i < 1000; ++i) {
		cLpm.UnlockRange(reinterpret_cast<void*>(unAddr), 0);
		unAddr += 1;
	}
	BOOST_CHECK(cLpm.GetLockedPageCount() == 0);
	BOOST_CHECK((g_sLastUnlockLen & (kTestPageSize - 1)) == 0); // always unlock entire pages
}

BOOST_AUTO_TEST_SUITE_END()
