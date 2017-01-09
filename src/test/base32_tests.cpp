// Copyright (c) 2012-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "util.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(base32_tests)

BOOST_AUTO_TEST_CASE(base32_testvectors) {
	static const string sstrVstrIn[] = { "", "f", "fo", "foo", "foob", "fooba", "foobar" };
	static const string sstrVstrOut[] = { "", "my======", "mzxq====", "mzxw6===", "mzxw6yq=", "mzxw6ytb","mzxw6ytboi======" };
	for (unsigned int i = 0; i < sizeof(sstrVstrIn) / sizeof(sstrVstrIn[0]); i++) {
		string strEnc = EncodeBase32(sstrVstrIn[i]);
		BOOST_CHECK(strEnc == sstrVstrOut[i]);
		string strDec = DecodeBase32(sstrVstrOut[i]);
		BOOST_CHECK(strDec == sstrVstrIn[i]);
	}
}

BOOST_AUTO_TEST_SUITE_END()
