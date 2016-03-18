/*
 * chainparams_tests.cpp
 *
 *  Created on: Jul 31, 2014
 *      Author: spark.huang
 */
#include "chainparams.h"
#include "util.h"

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>

BOOST_AUTO_TEST_SUITE(chainparam_tests)

static void ResetArgs(const std::string& strArg) {
	std::vector<std::string> vecArg;
	boost::split(vecArg, strArg, boost::is_space(), boost::token_compress_on);

	// Insert dummy executable name:
	vecArg.insert(vecArg.begin(), "testbitcoin");

	// Convert to char*:
	std::vector<const char*> vecChar;
	for (auto& s : vecArg)
		vecChar.push_back(s.c_str());

	CBaseParams::IntialParams(vecChar.size(), &vecChar[0]);
}

BOOST_AUTO_TEST_CASE(boolarg) {
	map<string, string> mapArgs = SysCfg().GetMapArgs();
	map<string, vector<string> > mapMultiArgs = SysCfg().GetMapMultiArgs();

	ResetArgs("-foo");
	BOOST_CHECK(SysCfg().GetBoolArg("-foo", false));
	BOOST_CHECK(SysCfg().GetBoolArg("-foo", true));

	BOOST_CHECK(!SysCfg().GetBoolArg("-fo", false));
	BOOST_CHECK(SysCfg().GetBoolArg("-fo", true));

	BOOST_CHECK(!SysCfg().GetBoolArg("-fooo", false));
	BOOST_CHECK(SysCfg().GetBoolArg("-fooo", true));

	ResetArgs("-foo=0");
	BOOST_CHECK(!SysCfg().GetBoolArg("-foo", false));
	BOOST_CHECK(!SysCfg().GetBoolArg("-foo", true));

	ResetArgs("-foo=1");
	BOOST_CHECK(SysCfg().GetBoolArg("-foo", false));
	BOOST_CHECK(SysCfg().GetBoolArg("-foo", true));

	// New 0.6 feature: auto-map -nosomething to !-something:
	ResetArgs("-nofoo");
	BOOST_CHECK(!SysCfg().GetBoolArg("-foo", false));
	BOOST_CHECK(!SysCfg().GetBoolArg("-foo", true));

	ResetArgs("-nofoo=1");
	BOOST_CHECK(!SysCfg().GetBoolArg("-foo", false));
	BOOST_CHECK(!SysCfg().GetBoolArg("-foo", true));

	ResetArgs("-foo -nofoo");  // -foo should win
	BOOST_CHECK(SysCfg().GetBoolArg("-foo", false));
	BOOST_CHECK(SysCfg().GetBoolArg("-foo", true));

	ResetArgs("-foo=1 -nofoo=1");  // -foo should win
	BOOST_CHECK(SysCfg().GetBoolArg("-foo", false));
	BOOST_CHECK(SysCfg().GetBoolArg("-foo", true));

	ResetArgs("-foo=0 -nofoo=0");  // -foo should win
	BOOST_CHECK(!SysCfg().GetBoolArg("-foo", false));
	BOOST_CHECK(!SysCfg().GetBoolArg("-foo", true));

	// New 0.6 feature: treat -- same as -:
	ResetArgs("--foo=1");
	BOOST_CHECK(SysCfg().GetBoolArg("-foo", false));
	BOOST_CHECK(SysCfg().GetBoolArg("-foo", true));

	ResetArgs("--nofoo=1");
	BOOST_CHECK(!SysCfg().GetBoolArg("-foo", false));
	BOOST_CHECK(!SysCfg().GetBoolArg("-foo", true));

	SysCfg().SetMapArgs(mapArgs);
	SysCfg().SetMultiMapArgs(mapMultiArgs);
}

BOOST_AUTO_TEST_CASE(stringarg) {
	map<string, string> mapArgs = SysCfg().GetMapArgs();
	map<string, vector<string> > mapMultiArgs = SysCfg().GetMapMultiArgs();

	ResetArgs("");
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-foo", ""), "");
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-foo", "eleven"), "eleven");

	ResetArgs("-foo -bar");
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-foo", ""), "");
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-foo", "eleven"), "");

	ResetArgs("-foo=");
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-foo", ""), "");
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-foo", "eleven"), "");

	ResetArgs("-foo=11");
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-foo", ""), "11");
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-foo", "eleven"), "11");

	ResetArgs("-foo=eleven");
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-foo", ""), "eleven");
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-foo", "eleven"), "eleven");

	SysCfg().SetMapArgs(mapArgs);
	SysCfg().SetMultiMapArgs(mapMultiArgs);

}

BOOST_AUTO_TEST_CASE(intarg) {
	map<string, string> mapArgs = SysCfg().GetMapArgs();
	map<string, vector<string> > mapMultiArgs = SysCfg().GetMapMultiArgs();

	ResetArgs("");
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-foo", 11), 11);
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-foo", 0), 0);

	ResetArgs("-foo -bar");
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-foo", 11), 0);
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-bar", 11), 0);

	ResetArgs("-foo=11 -bar=12");
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-foo", 0), 11);
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-bar", 11), 12);

	ResetArgs("-foo=NaN -bar=NotANumber");
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-foo", 1), 0);
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-bar", 11), 0);

	SysCfg().SetMapArgs(mapArgs);
	SysCfg().SetMultiMapArgs(mapMultiArgs);
}

BOOST_AUTO_TEST_CASE(doubledash) {
	map<string, string> mapArgs = SysCfg().GetMapArgs();
	map<string, vector<string> > mapMultiArgs = SysCfg().GetMapMultiArgs();

	ResetArgs("--foo");
	BOOST_CHECK_EQUAL(SysCfg().GetBoolArg("-foo", false), true);

	ResetArgs("--foo=verbose --bar=1");
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-foo", ""), "verbose");
	BOOST_CHECK_EQUAL(SysCfg().GetArg("-bar", 0), 1);

	SysCfg().SetMapArgs(mapArgs);
	SysCfg().SetMultiMapArgs(mapMultiArgs);
}

BOOST_AUTO_TEST_CASE(boolargno) {
	map<string, string> mapArgs = SysCfg().GetMapArgs();
	map<string, vector<string> > mapMultiArgs = SysCfg().GetMapMultiArgs();

	ResetArgs("-nofoo");
	BOOST_CHECK(!SysCfg().GetBoolArg("-foo", true));
	BOOST_CHECK(!SysCfg().GetBoolArg("-foo", false));

	ResetArgs("-nofoo=1");
	BOOST_CHECK(!SysCfg().GetBoolArg("-foo", true));
	BOOST_CHECK(!SysCfg().GetBoolArg("-foo", false));

	ResetArgs("-nofoo=0");
	BOOST_CHECK(SysCfg().GetBoolArg("-foo", true));
	BOOST_CHECK(SysCfg().GetBoolArg("-foo", false));

	ResetArgs("-foo --nofoo");
	BOOST_CHECK(SysCfg().GetBoolArg("-foo", true));
	BOOST_CHECK(SysCfg().GetBoolArg("-foo", false));

	ResetArgs("-nofoo -foo"); // foo always wins:
	BOOST_CHECK(SysCfg().GetBoolArg("-foo", true));
	BOOST_CHECK(SysCfg().GetBoolArg("-foo", false));

	SysCfg().SetMapArgs(mapArgs);
	SysCfg().SetMultiMapArgs(mapMultiArgs);
}

BOOST_AUTO_TEST_CASE(chain_main) {
//	BOOST_CHECK(
//			SysParamsMain().HashGenesisBlock()
//					== uint256("30090f75e06a2edb5faf635626dbdbebc3fdc8d8922382b0c0d261aa561666f9"));

	BOOST_CHECK(SysParamsMain().MessageStart()[0] == 0xff);
	BOOST_CHECK(SysParamsMain().MessageStart()[1] == 0xfe);
	BOOST_CHECK(SysParamsMain().MessageStart()[2] == 0x1d);
	BOOST_CHECK(SysParamsMain().MessageStart()[3] == 0x20);
	BOOST_CHECK(SysParamsMain().AlertKey() == ParseHex("02d99681b6287b3765dfbb930e6caa10d1f8ac19e02b88f52362ce6eb43c0ec71e"));
	BOOST_CHECK(SysParamsMain().GetDefaultPort() == 8668);
//	BOOST_CHECK(SysParamsMain().ProofOfWorkLimit() == (~arith_uint256(0) >> 10));
	BOOST_CHECK(SysParamsMain().RequireRPCPassword() == true);
	BOOST_CHECK(SysParamsMain().DataDir() == "main");
	BOOST_CHECK(SysParamsMain().NetworkID() == CBaseParams::MAIN);
	BOOST_CHECK(SysParamsMain().RPCPort() == 8669);
}

BOOST_AUTO_TEST_CASE(chain_test) {
//	BOOST_CHECK(
//			SysParamsTest().HashGenesisBlock()
//					== uint256("379789424d6706afff7d19410c857601e483a8a70e8e83bea4d72bbe71ea1781"));
	BOOST_CHECK(SysParamsTest().MessageStart()[0] == 0xfe);
	BOOST_CHECK(SysParamsTest().MessageStart()[1] == 0x2d);
	BOOST_CHECK(SysParamsTest().MessageStart()[2] == 0x1c);
	BOOST_CHECK(SysParamsTest().MessageStart()[3] == 0x0d);
	BOOST_CHECK(SysParamsTest().AlertKey() == ParseHex("036e15523feb9e329b4fdf53c227fc89ea45a1a36342e7e38fad7fe6e3777243af"));
	BOOST_CHECK(SysParamsTest().GetDefaultPort() == 18668);
//	BOOST_CHECK(SysParamsTest().ProofOfWorkLimit() == (~arith_uint256(0) >> 10));

	BOOST_CHECK(SysParamsTest().RequireRPCPassword() == true);
	BOOST_CHECK(SysParamsTest().DataDir() == "testnet");
	BOOST_CHECK(SysParamsTest().NetworkID() == CBaseParams::TESTNET);
	BOOST_CHECK(SysParamsTest().RPCPort() == 18669);
}

BOOST_AUTO_TEST_CASE(chain_regtest) {
//	BOOST_CHECK(
//			SysParamsReg().HashGenesisBlock()
//					== uint256("b3c285730fee7d92791cd71fa0f7bb9dbb8d5f0da39029f0d284935f39afacae"));
	BOOST_CHECK(SysParamsReg().MessageStart()[0] == 0xfc);
	BOOST_CHECK(SysParamsReg().MessageStart()[1] == 0x1d);
	BOOST_CHECK(SysParamsReg().MessageStart()[2] == 0x2d);
	BOOST_CHECK(SysParamsReg().MessageStart()[3] == 0x3d);
	BOOST_CHECK(SysParamsReg().AlertKey() == ParseHex("036e15523feb9e329b4fdf53c227fc89ea45a1a36342e7e38fad7fe6e3777243af"));
	BOOST_CHECK(SysParamsReg().GetDefaultPort() == 18666);
//	BOOST_CHECK(SysParamsReg().ProofOfWorkLimit() == (~arith_uint256(0) >> 6));
	BOOST_CHECK(SysParamsReg().SubsidyHalvingInterval() == 150);
	BOOST_CHECK(SysParamsReg().RequireRPCPassword() == false);
	BOOST_CHECK(SysParamsReg().DataDir() == "regtest");
	BOOST_CHECK(SysParamsReg().NetworkID() == CBaseParams::REGTEST);
	BOOST_CHECK(SysParamsReg().RPCPort() == 18669);
}

BOOST_AUTO_TEST_SUITE_END()

