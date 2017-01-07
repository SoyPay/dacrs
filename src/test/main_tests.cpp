// Copyright (c) 2014 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "core.h"
#include "main.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(main_tests)

BOOST_AUTO_TEST_CASE(subsidy_limit_test) {
	uint64_t ullSum = 0;
	if (CBaseParams::EM_MAIN == SysCfg().NetworkID()) {
		for (int nHeight = 0; nHeight < 14000000; nHeight += 1000) {
			uint64_t ullSubsidy = GetBlockValue(nHeight, 0);
			BOOST_CHECK(ullSubsidy <= 50 * COIN);
			ullSum += ullSubsidy * 1000;
			BOOST_CHECK(MoneyRange(ullSum));
		}
		BOOST_CHECK(ullSum == 2099999997690000ULL);
	} else {
		// uint64_t sSum = 0;
		for (int nHeight = 0; nHeight < 10000; nHeight += 150) {
			uint64_t ullSubsidy = GetBlockValue(nHeight, 0);
			BOOST_CHECK(ullSubsidy <= 50 * COIN);
			ullSum += ullSubsidy * 150;
			BOOST_CHECK(MoneyRange(ullSum));
		}
		BOOST_CHECK(ullSum == 1499999998350ULL);
	}
}

BOOST_AUTO_TEST_SUITE_END()
