// Copyright (c) 2011-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define BOOST_TEST_MODULE SoyPay Test Suite



#include "main.h"
#include "txdb.h"
#include "ui_interface.h"
#include "util.h"


#include "wallet.h"


#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include "SysTestBase.h"
//CWallet* pwalletMain;


extern void noui_connect();

struct TestingSetup {
	TestingSetup() {
			int argc = 2;
			char* argv[] = { "D:\\cppwork\\soypay\\src\\soypayd.exe", "-datadir=d:\\bitcoin" };
			SysTestBase::StartServer(argc, argv);
		}
		~TestingSetup()
		{
			SysTestBase::StopServer();
		}

//        boost::filesystem::remove_all(pathTemp);

};

BOOST_GLOBAL_FIXTURE(TestingSetup);

//void Shutdown(void* parg)
//{
//  exit(0);
//}
//
//void StartShutdown()
//{
//  exit(0);
//}

//bool ShutdownRequested()
//{
//  return false;
//}

