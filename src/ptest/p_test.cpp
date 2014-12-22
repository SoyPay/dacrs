// Copyright (c) 2011-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define BOOST_TEST_MODULE Process Test Suite



#include "main.h"
#include "txdb.h"
#include "ui_interface.h"
#include "util.h"


#include "wallet.h"


#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include "chainparams.h"


//extern void noui_connect();

struct TestingSetup {
	TestingSetup() {
		char *argv[] = {"progname", "-datadir=D:\\bitcoin"};
		int argc = sizeof(argv) / sizeof(char*);
		CBaseParams::IntialParams(argc, argv);
	}
    ~TestingSetup()
    {

    }
};

BOOST_GLOBAL_FIXTURE(TestingSetup);
