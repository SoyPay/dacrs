// Copyright (c) 2011-2013 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "arith_uint256.h"
#include "uint256.h"
#include "version.h"

#include <boost/test/unit_test.hpp>
#include <stdint.h>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <string>
#include <stdio.h>

BOOST_AUTO_TEST_SUITE(uint256_tests)

const unsigned char g_kuchR1Array[] =
    "\x9c\x52\x4a\xdb\xcf\x56\x11\x12\x2b\x29\x12\x5e\x5d\x35\xd2\xd2"
    "\x22\x81\xaa\xb5\x33\xf0\x08\x32\xd5\x56\xb1\xf9\xea\xe5\x1d\x7d";
const char g_kchR1ArrayHex[] = "7D1DE5EAF9B156D53208F033B5AA8122D2d2355d5e12292b121156cfdb4a529c";
const uint256 g_kcR1L = uint256(std::vector<unsigned char>(g_kuchR1Array,g_kuchR1Array+32));
const uint160 g_kcR1S = uint160(std::vector<unsigned char>(g_kuchR1Array,g_kuchR1Array+20));

const unsigned char g_kuchR2Array[] =
    "\x70\x32\x1d\x7c\x47\xa5\x6b\x40\x26\x7e\x0a\xc3\xa6\x9c\xb6\xbf"
    "\x13\x30\x47\xa3\x19\x2d\xda\x71\x49\x13\x72\xf0\xb4\xca\x81\xd7";
const uint256 g_kcR2L = uint256(std::vector<unsigned char>(g_kuchR2Array,g_kuchR2Array+32));
const uint160 g_kcR2S = uint160(std::vector<unsigned char>(g_kuchR2Array,g_kuchR2Array+20));

const unsigned char g_kuchZeroArray[] =
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
const uint256 g_kcZeroL = uint256(std::vector<unsigned char>(g_kuchZeroArray,g_kuchZeroArray+32));
const uint160 g_kcZeroS = uint160(std::vector<unsigned char>(g_kuchZeroArray,g_kuchZeroArray+20));

const unsigned char g_kuchOneArray[] =
    "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
const uint256 g_kcOneL = uint256(std::vector<unsigned char>(g_kuchOneArray,g_kuchOneArray+32));
const uint160 g_kcOneS = uint160(std::vector<unsigned char>(g_kuchOneArray,g_kuchOneArray+20));

const unsigned char g_kuchMaxArray[] =
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
const uint256 g_kcMaxL = uint256(std::vector<unsigned char>(g_kuchMaxArray,g_kuchMaxArray+32));
const uint160 g_kcMaxS = uint160(std::vector<unsigned char>(g_kuchMaxArray,g_kuchMaxArray+20));

std::string ArrayToString(const unsigned char A[], unsigned int width) {
	std::stringstream Stream;
	Stream << std::hex;
	for (unsigned int i = 0; i < width; ++i) {
		Stream << std::setw(2) << std::setfill('0') << (unsigned int) A[width - i - 1];
	}
	return Stream.str();
}

inline uint160 uint160S(const char *str) {
	uint160 cRv;
	cRv.SetHex(str);
	return cRv;
}

inline uint160 uint160S(const std::string& str) {
	uint160 cRv;
	cRv.SetHex(str);
	return cRv;
}

// constructors, equality, inequality
BOOST_AUTO_TEST_CASE( basics ) {
    BOOST_CHECK(1 == 0+1);
    // constructor uint256(vector<char>):
    BOOST_CHECK(g_kcR1L.ToString() == ArrayToString(g_kuchR1Array,32));
    BOOST_CHECK(g_kcR1S.ToString() == ArrayToString(g_kuchR1Array,20));
    BOOST_CHECK(g_kcR2L.ToString() == ArrayToString(g_kuchR2Array,32));
    BOOST_CHECK(g_kcR2S.ToString() == ArrayToString(g_kuchR2Array,20));
    BOOST_CHECK(g_kcZeroL.ToString() == ArrayToString(g_kuchZeroArray,32));
    BOOST_CHECK(g_kcZeroS.ToString() == ArrayToString(g_kuchZeroArray,20));
    BOOST_CHECK(g_kcOneL.ToString() == ArrayToString(g_kuchOneArray,32));
    BOOST_CHECK(g_kcOneS.ToString() == ArrayToString(g_kuchOneArray,20));
    BOOST_CHECK(g_kcMaxL.ToString() == ArrayToString(g_kuchMaxArray,32));
    BOOST_CHECK(g_kcMaxS.ToString() == ArrayToString(g_kuchMaxArray,20));
    BOOST_CHECK(g_kcOneL.ToString() != ArrayToString(g_kuchZeroArray,32));
    BOOST_CHECK(g_kcOneS.ToString() != ArrayToString(g_kuchZeroArray,20));

    // == and !=
    BOOST_CHECK(g_kcR1L != g_kcR2L && g_kcR1S != g_kcR2S);
    BOOST_CHECK(g_kcZeroL != g_kcOneL && g_kcZeroS != g_kcOneS);
    BOOST_CHECK(g_kcOneL != g_kcZeroL && g_kcOneS != g_kcZeroS);
    BOOST_CHECK(g_kcMaxL != g_kcZeroL && g_kcMaxS != g_kcZeroS);

    // String Constructor and Copy Constructor
    BOOST_CHECK(uint256S("0x"+g_kcR1L.ToString()) == g_kcR1L);
    BOOST_CHECK(uint256S("0x"+g_kcR2L.ToString()) == g_kcR2L);
    BOOST_CHECK(uint256S("0x"+g_kcZeroL.ToString()) == g_kcZeroL);
    BOOST_CHECK(uint256S("0x"+g_kcOneL.ToString()) == g_kcOneL);
    BOOST_CHECK(uint256S("0x"+g_kcMaxL.ToString()) == g_kcMaxL);
    BOOST_CHECK(uint256S(g_kcR1L.ToString()) == g_kcR1L);
    BOOST_CHECK(uint256S("   0x"+g_kcR1L.ToString()+"   ") == g_kcR1L);
    BOOST_CHECK(uint256S("") == g_kcZeroL);
    BOOST_CHECK(g_kcR1L == uint256S(g_kchR1ArrayHex));
    BOOST_CHECK(uint256(g_kcR1L) == g_kcR1L);
    BOOST_CHECK(uint256(g_kcZeroL) == g_kcZeroL);
    BOOST_CHECK(uint256(g_kcOneL) == g_kcOneL);

    BOOST_CHECK(uint160S("0x"+g_kcR1S.ToString()) == g_kcR1S);
    BOOST_CHECK(uint160S("0x"+g_kcR2S.ToString()) == g_kcR2S);
    BOOST_CHECK(uint160S("0x"+g_kcZeroS.ToString()) == g_kcZeroS);
    BOOST_CHECK(uint160S("0x"+g_kcOneS.ToString()) == g_kcOneS);
    BOOST_CHECK(uint160S("0x"+g_kcMaxS.ToString()) == g_kcMaxS);
    BOOST_CHECK(uint160S(g_kcR1S.ToString()) == g_kcR1S);
    BOOST_CHECK(uint160S("   0x"+g_kcR1S.ToString()+"   ") == g_kcR1S);
    BOOST_CHECK(uint160S("") == g_kcZeroS);
    BOOST_CHECK(g_kcR1S == uint160S(g_kchR1ArrayHex));

    BOOST_CHECK(uint160(g_kcR1S) == g_kcR1S);
    BOOST_CHECK(uint160(g_kcZeroS) == g_kcZeroS);
    BOOST_CHECK(uint160(g_kcOneS) == g_kcOneS);
}

// <= >= < >
BOOST_AUTO_TEST_CASE( comparison ) {
	uint256 cLastL;
	for (int i = 255; i >= 0; --i) {
		uint256 TmpL;
		*(TmpL.begin() + (i >> 3)) |= 1 << (7 - (i & 7));
		BOOST_CHECK(cLastL < TmpL);
		cLastL = TmpL;
	}

    BOOST_CHECK( g_kcZeroL < g_kcR1L );
    BOOST_CHECK( g_kcR2L < g_kcR1L );
    BOOST_CHECK( g_kcZeroL < g_kcOneL );
    BOOST_CHECK( g_kcOneL < g_kcMaxL );
    BOOST_CHECK( g_kcR1L < g_kcMaxL );
    BOOST_CHECK( g_kcR2L < g_kcMaxL );

	uint160 cLastS;
	for (int i = 159; i >= 0; --i) {
		uint160 cTmpS;
		*(cTmpS.begin() + (i >> 3)) |= 1 << (7 - (i & 7));
		BOOST_CHECK(cLastS < cTmpS);
		cLastS = cTmpS;
	}
    BOOST_CHECK( g_kcZeroS < g_kcR1S );
    BOOST_CHECK( g_kcR2S < g_kcR1S );
    BOOST_CHECK( g_kcZeroS < g_kcOneS );
    BOOST_CHECK( g_kcOneS < g_kcMaxS );
    BOOST_CHECK( g_kcR1S < g_kcMaxS );
    BOOST_CHECK( g_kcR2S < g_kcMaxS );
}

BOOST_AUTO_TEST_CASE( methods ) // GetHex SetHex begin() end() size() GetLow64 GetSerializeSize, Serialize, Unserialize
{
    BOOST_CHECK(g_kcR1L.GetHex() == g_kcR1L.ToString());
    BOOST_CHECK(g_kcR2L.GetHex() == g_kcR2L.ToString());
    BOOST_CHECK(g_kcOneL.GetHex() == g_kcOneL.ToString());
    BOOST_CHECK(g_kcMaxL.GetHex() == g_kcMaxL.ToString());
    uint256 cTmpL(g_kcR1L);
    BOOST_CHECK(cTmpL == g_kcR1L);
    cTmpL.SetHex(g_kcR2L.ToString());   BOOST_CHECK(cTmpL == g_kcR2L);
    cTmpL.SetHex(g_kcZeroL.ToString()); BOOST_CHECK(cTmpL == uint256());

    cTmpL.SetHex(g_kcR1L.ToString());
    BOOST_CHECK(memcmp(g_kcR1L.begin(), g_kuchR1Array, 32)==0);
    BOOST_CHECK(memcmp(cTmpL.begin(), g_kuchR1Array, 32)==0);
    BOOST_CHECK(memcmp(g_kcR2L.begin(), g_kuchR2Array, 32)==0);
    BOOST_CHECK(memcmp(g_kcZeroL.begin(), g_kuchZeroArray, 32)==0);
    BOOST_CHECK(memcmp(g_kcOneL.begin(), g_kuchOneArray, 32)==0);
    BOOST_CHECK(g_kcR1L.size() == sizeof(g_kcR1L));
    BOOST_CHECK(sizeof(g_kcR1L) == 32);
    BOOST_CHECK(g_kcR1L.size() == 32);
    BOOST_CHECK(g_kcR2L.size() == 32);
    BOOST_CHECK(g_kcZeroL.size() == 32);
    BOOST_CHECK(g_kcMaxL.size() == 32);
    BOOST_CHECK(g_kcR1L.begin() + 32 == g_kcR1L.end());
    BOOST_CHECK(g_kcR2L.begin() + 32 == g_kcR2L.end());
    BOOST_CHECK(g_kcOneL.begin() + 32 == g_kcOneL.end());
    BOOST_CHECK(g_kcMaxL.begin() + 32 == g_kcMaxL.end());
    BOOST_CHECK(cTmpL.begin() + 32 == cTmpL.end());
    BOOST_CHECK(g_kcR1L.GetSerializeSize(0,g_sProtocolVersion) == 32);
    BOOST_CHECK(g_kcZeroL.GetSerializeSize(0,g_sProtocolVersion) == 32);

    std::stringstream ss;
    g_kcR1L.Serialize(ss,0,g_sProtocolVersion);
    BOOST_CHECK(ss.str() == std::string(g_kuchR1Array,g_kuchR1Array+32));
    cTmpL.Unserialize(ss,0,g_sProtocolVersion);
    BOOST_CHECK(g_kcR1L == cTmpL);
    ss.str("");
    g_kcZeroL.Serialize(ss,0,g_sProtocolVersion);
    BOOST_CHECK(ss.str() == std::string(g_kuchZeroArray,g_kuchZeroArray+32));
    cTmpL.Unserialize(ss,0,g_sProtocolVersion);
    BOOST_CHECK(g_kcZeroL == cTmpL);
    ss.str("");
    g_kcMaxL.Serialize(ss,0,g_sProtocolVersion);
    BOOST_CHECK(ss.str() == std::string(g_kuchMaxArray,g_kuchMaxArray+32));
    cTmpL.Unserialize(ss,0,g_sProtocolVersion);
    BOOST_CHECK(g_kcMaxL == cTmpL);
    ss.str("");

    BOOST_CHECK(g_kcR1S.GetHex() == g_kcR1S.ToString());
    BOOST_CHECK(g_kcR2S.GetHex() == g_kcR2S.ToString());
    BOOST_CHECK(g_kcOneS.GetHex() == g_kcOneS.ToString());
    BOOST_CHECK(g_kcMaxS.GetHex() == g_kcMaxS.ToString());
    uint160 TmpS(g_kcR1S);
    BOOST_CHECK(TmpS == g_kcR1S);
    TmpS.SetHex(g_kcR2S.ToString());   BOOST_CHECK(TmpS == g_kcR2S);
    TmpS.SetHex(g_kcZeroS.ToString()); BOOST_CHECK(TmpS == uint160());

    TmpS.SetHex(g_kcR1S.ToString());
    BOOST_CHECK(memcmp(g_kcR1S.begin(), g_kuchR1Array, 20)==0);
    BOOST_CHECK(memcmp(TmpS.begin(), g_kuchR1Array, 20)==0);
    BOOST_CHECK(memcmp(g_kcR2S.begin(), g_kuchR2Array, 20)==0);
    BOOST_CHECK(memcmp(g_kcZeroS.begin(), g_kuchZeroArray, 20)==0);
    BOOST_CHECK(memcmp(g_kcOneS.begin(), g_kuchOneArray, 20)==0);
    BOOST_CHECK(g_kcR1S.size() == sizeof(g_kcR1S));
    BOOST_CHECK(sizeof(g_kcR1S) == 20);
    BOOST_CHECK(g_kcR1S.size() == 20);
    BOOST_CHECK(g_kcR2S.size() == 20);
    BOOST_CHECK(g_kcZeroS.size() == 20);
    BOOST_CHECK(g_kcMaxS.size() == 20);
    BOOST_CHECK(g_kcR1S.begin() + 20 == g_kcR1S.end());
    BOOST_CHECK(g_kcR2S.begin() + 20 == g_kcR2S.end());
    BOOST_CHECK(g_kcOneS.begin() + 20 == g_kcOneS.end());
    BOOST_CHECK(g_kcMaxS.begin() + 20 == g_kcMaxS.end());
    BOOST_CHECK(TmpS.begin() + 20 == TmpS.end());
    BOOST_CHECK(g_kcR1S.GetSerializeSize(0,g_sProtocolVersion) == 20);
    BOOST_CHECK(g_kcZeroS.GetSerializeSize(0,g_sProtocolVersion) == 20);

    g_kcR1S.Serialize(ss,0,g_sProtocolVersion);
    BOOST_CHECK(ss.str() == std::string(g_kuchR1Array,g_kuchR1Array+20));
    TmpS.Unserialize(ss,0,g_sProtocolVersion);
    BOOST_CHECK(g_kcR1S == TmpS);
    ss.str("");
    g_kcZeroS.Serialize(ss,0,g_sProtocolVersion);
    BOOST_CHECK(ss.str() == std::string(g_kuchZeroArray,g_kuchZeroArray+20));
    TmpS.Unserialize(ss,0,g_sProtocolVersion);
    BOOST_CHECK(g_kcZeroS == TmpS);
    ss.str("");
    g_kcMaxS.Serialize(ss,0,g_sProtocolVersion);
    BOOST_CHECK(ss.str() == std::string(g_kuchMaxArray,g_kuchMaxArray+20));
    TmpS.Unserialize(ss,0,g_sProtocolVersion);
    BOOST_CHECK(g_kcMaxS == TmpS);
    ss.str("");
}

BOOST_AUTO_TEST_CASE( conversion ) {
    BOOST_CHECK(ArithToUint256(UintToArith256(g_kcZeroL)) == g_kcZeroL);
    BOOST_CHECK(ArithToUint256(UintToArith256(g_kcOneL)) == g_kcOneL);
    BOOST_CHECK(ArithToUint256(UintToArith256(g_kcR1L)) == g_kcR1L);
    BOOST_CHECK(ArithToUint256(UintToArith256(g_kcR2L)) == g_kcR2L);
    BOOST_CHECK(UintToArith256(g_kcZeroL) == 0);
    BOOST_CHECK(UintToArith256(g_kcOneL) == 1);
    BOOST_CHECK(ArithToUint256(0) == g_kcZeroL);
    BOOST_CHECK(ArithToUint256(1) == g_kcOneL);
    BOOST_CHECK(arith_uint256(g_kcR1L.GetHex()) == UintToArith256(g_kcR1L));
    BOOST_CHECK(arith_uint256(g_kcR2L.GetHex()) == UintToArith256(g_kcR2L));
    BOOST_CHECK(g_kcR1L.GetHex() == UintToArith256(g_kcR1L).GetHex());
    BOOST_CHECK(g_kcR2L.GetHex() == UintToArith256(g_kcR2L).GetHex());
}

BOOST_AUTO_TEST_SUITE_END()
