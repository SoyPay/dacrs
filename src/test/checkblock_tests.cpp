// Copyright (c) 2013-2014 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//
// Unit tests for block.CheckBlock()
//



#include "main.h"

#include <cstdio>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_SUITE(CheckBlock_tests)

bool read_block(const std::string& filename, CBlock& block) {
	namespace fs = boost::filesystem;
	fs::path testFile = fs::current_path() / "data" / filename;
#ifdef TEST_DATA_DIR
	if (!fs::exists(testFile)) {
		testFile = fs::path(BOOST_PP_STRINGIZE(TEST_DATA_DIR)) / filename;
	}
#endif
	FILE* pFile = fopen(testFile.string().c_str(), "rb");
	if (!pFile) {
		return false;
	}
	fseek(pFile, 8, SEEK_SET); // skip msgheader/size
	CAutoFile cFilein = CAutoFile(pFile, SER_DISK, g_sClientVersion);
	if (!cFilein) {
		return false;
	}
	cFilein >> block;
	return true;
}

BOOST_AUTO_TEST_CASE(May15) {
	// Putting a 1MB binary file in the git repository is not a great
	// idea, so this test is only run if you manually download
	// test/data/Mar12Fork.dat from
	// http://sourceforge.net/projects/bitcoin/files/Bitcoin/blockchain/Mar12Fork.dat/download
	unsigned int unMay15 = 1368576000;
	SetMockTime(unMay15); // Test as if it was right at May 15
	CAccountViewCache cView(*g_pAccountViewTip, true);
	CScriptDBViewCache cScriptDBCache(*g_pScriptDBTip, true);
	CBlock cForkingBlock;
	if (read_block("Mar12Fork.dat", cForkingBlock)) {
		CValidationState cState;
		// After May 15'th, big blocks are OK:
		cForkingBlock.SetTime(unMay15); // Invalidates PoW
		BOOST_CHECK(CheckBlock(cForkingBlock, cState, cView, cScriptDBCache, false, false));
	}

	SetMockTime(0);
}

BOOST_AUTO_TEST_SUITE_END()
