// Copyright (c) 2012-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "serialize.h"

#include <stdint.h>

#include <boost/test/unit_test.hpp>

using namespace std;

BOOST_AUTO_TEST_SUITE(serialize_tests)

BOOST_AUTO_TEST_CASE(varints) {
	// encode
	CDataStream cSs(SER_DISK, 0);
	CDataStream::size_type size = 0;
	for (int i = 0; i < 100000; i++) {
		cSs << VARINT(i);
		size += ::GetSerializeSize(VARINT(i), 0, 0);
		BOOST_CHECK(size == cSs.size());
	}

	for (uint64_t i = 0; i < 100000000000ULL; i += 999999937) {
		cSs << VARINT(i);
		size += ::GetSerializeSize(VARINT(i), 0, 0);
		BOOST_CHECK(size == cSs.size());
	}

	// decode
	for (int i = 0; i < 100000; i++) {
		int j = -1;
		cSs >> VARINT(j);
		BOOST_CHECK_MESSAGE(i == j, "decoded:" << j << " expected:" << i);
	}

	for (uint64_t i = 0; i < 100000000000ULL; i += 999999937) {
		uint64_t j = -1;
		cSs >> VARINT(j);
		BOOST_CHECK_MESSAGE(i == j, "decoded:" << j << " expected:" << i);
	}
}

BOOST_AUTO_TEST_CASE(compactsize) {
	CDataStream cSs(SER_DISK, 0);
	vector<char>::size_type i, j;

	for (i = 1; i <= g_sMaxSize; i *= 2) {
		WriteCompactSize(cSs, i - 1);
		WriteCompactSize(cSs, i);
	}
	for (i = 1; i <= g_sMaxSize; i *= 2) {
		j = ReadCompactSize(cSs);
		BOOST_CHECK_MESSAGE((i - 1) == j, "decoded:" << j << " expected:" << (i-1));
		j = ReadCompactSize(cSs);
		BOOST_CHECK_MESSAGE(i == j, "decoded:" << j << " expected:" << i);
	}
}

static bool isCanonicalException(const std::ios_base::failure& ex) {
	std::ios_base::failure expectedException("non-canonical ReadCompactSize()");

	// The string returned by what() can be different for different platforms.
	// Instead of directly comparing the ex.what() with an expected string,
	// create an instance of exception to see if ex.what() matches
	// the expected explanatory string returned by the exception instance.
	return strcmp(expectedException.what(), ex.what()) == 0;
}

BOOST_AUTO_TEST_CASE(noncanonical) {
	// Write some non-canonical CompactSize encodings, and
	// make sure an exception is thrown when read back.
	CDataStream cSs(SER_DISK, 0);
	vector<char>::size_type n;

	// zero encoded with three bytes:
	cSs.write("\xfd\x00\x00", 3);
	BOOST_CHECK_EXCEPTION(ReadCompactSize(cSs), std::ios_base::failure, isCanonicalException);

	// 0xfc encoded with three bytes:
	cSs.write("\xfd\xfc\x00", 3);
	BOOST_CHECK_EXCEPTION(ReadCompactSize(cSs), std::ios_base::failure, isCanonicalException);

	// 0xfd encoded with three bytes is OK:
	cSs.write("\xfd\xfd\x00", 3);
	n = ReadCompactSize(cSs);
	BOOST_CHECK(n == 0xfd);

	// zero encoded with five bytes:
	cSs.write("\xfe\x00\x00\x00\x00", 5);
	BOOST_CHECK_EXCEPTION(ReadCompactSize(cSs), std::ios_base::failure, isCanonicalException);

	// 0xffff encoded with five bytes:
	cSs.write("\xfe\xff\xff\x00\x00", 5);
	BOOST_CHECK_EXCEPTION(ReadCompactSize(cSs), std::ios_base::failure, isCanonicalException);

	// zero encoded with nine bytes:
	cSs.write("\xff\x00\x00\x00\x00\x00\x00\x00\x00", 9);
	BOOST_CHECK_EXCEPTION(ReadCompactSize(cSs), std::ios_base::failure, isCanonicalException);

	// 0x01ffffff encoded with nine bytes:
	cSs.write("\xff\xff\xff\xff\x01\x00\x00\x00\x00", 9);
	BOOST_CHECK_EXCEPTION(ReadCompactSize(cSs), std::ios_base::failure, isCanonicalException);
}

BOOST_AUTO_TEST_CASE(insert_delete) {
	// Test inserting/deleting bytes.
	CDataStream cSs(SER_DISK, 0);
	BOOST_CHECK_EQUAL(cSs.size(), 0);

	cSs.write("\x00\x01\x02\xff", 4);
	BOOST_CHECK_EQUAL(cSs.size(), 4);

	char c = (char) 11;

	// Inserting at beginning/end/middle:
	cSs.insert(cSs.begin(), c);
	BOOST_CHECK_EQUAL(cSs.size(), 5);
	BOOST_CHECK_EQUAL(cSs[0], c);
	BOOST_CHECK_EQUAL(cSs[1], 0);

	cSs.insert(cSs.end(), c);
	BOOST_CHECK_EQUAL(cSs.size(), 6);
	BOOST_CHECK_EQUAL(cSs[4], (char)0xff);
	BOOST_CHECK_EQUAL(cSs[5], c);

	cSs.insert(cSs.begin() + 2, c);
	BOOST_CHECK_EQUAL(cSs.size(), 7);
	BOOST_CHECK_EQUAL(cSs[2], c);

	// Delete at beginning/end/middle
	cSs.erase(cSs.begin());
	BOOST_CHECK_EQUAL(cSs.size(), 6);
	BOOST_CHECK_EQUAL(cSs[0], 0);

	cSs.erase(cSs.begin() + cSs.size() - 1);
	BOOST_CHECK_EQUAL(cSs.size(), 5);
	BOOST_CHECK_EQUAL(cSs[4], (char)0xff);

	cSs.erase(cSs.begin() + 1);
	BOOST_CHECK_EQUAL(cSs.size(), 4);
	BOOST_CHECK_EQUAL(cSs[0], 0);
	BOOST_CHECK_EQUAL(cSs[1], 1);
	BOOST_CHECK_EQUAL(cSs[2], 2);
	BOOST_CHECK_EQUAL(cSs[3], (char)0xff);

	// Make sure GetAndClear does the right thing:
	CSerializeData d;
	cSs.GetAndClear(d);
	BOOST_CHECK_EQUAL(cSs.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
