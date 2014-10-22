// Copyright (c) 2011-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define BOOST_TEST_MODULE Process Test Suite



#include "main.h"
#include "txdb.h"
#include "ui_interface.h"
#include "util.h"
#ifdef ENABLE_WALLET
#include "db.h"
#include "wallet.h"
#endif

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include "chainparams.h"

extern bool fPrintToConsole;
extern void noui_connect();

struct TestingSetup {
	TestingSetup() {
		mapArgs["-datadir"] = "D:\\bitcoin";
		ReadConfigFile(mapArgs, mapMultiArgs);
		if (!SelectParamsFromCommandLine()) {
		       fprintf(stderr, "Error: Invalid combination of -regtest and -testnet.\n");
		}
	}
    ~TestingSetup()
    {

    }
};

BOOST_GLOBAL_FIXTURE(TestingSetup);
