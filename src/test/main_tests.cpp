// Copyright (c) 2014 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "core.h"
#include "main.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(main_tests)

BOOST_AUTO_TEST_CASE(subsidy_limit_test)
{

	uint64_t nSum = 0;
	if(CBaseParams::MAIN == SysCfg().NetworkID()) {
		for (int nHeight = 0; nHeight < 14000000; nHeight += 1000) {
			uint64_t nSubsidy = GetBlockValue(nHeight, 0);
			BOOST_CHECK(nSubsidy <= 50 * COIN);
			nSum += nSubsidy * 1000;
			BOOST_CHECK(MoneyRange(nSum));
		}
		BOOST_CHECK(nSum == 2099999997690000ULL);
	}
	else
	{
		uint64_t sSum = 0;
		for(int nHeight = 0; nHeight < 10000; nHeight+=150) {
			uint64_t nSubsidy = GetBlockValue(nHeight, 0);
			BOOST_CHECK(nSubsidy <= 50 * COIN);
			nSum += nSubsidy * 150;
			BOOST_CHECK(MoneyRange(nSum));
		}
		BOOST_CHECK(nSum == 1499999998350ULL);
	}
}
BOOST_AUTO_TEST_CASE(coumput_time)
{
	map<vector_unsigned_char,CAuthorizate> mapAuthorizate;
	map<vector_unsigned_char,CAuthorizate> map2;
	CAuthorizate temp;
	vector_unsigned_char it;
	it.push_back(0);
	int64_t runTime = GetTimeMillis();
	mapAuthorizate[it] = temp;
//	for(auto & item : mapAuthorizate) {
//		cout<<HexStr(item.first)<<item.second.ToString(false)<<endl;
//	}
//	cout<<"First:"<<GetTimeMillis()-runTime<<endl;

	runTime = GetTimeMillis();
	for(int i = 1;i <=10;i++){
		it.clear();
		it.push_back(i);
		temp.SetCurMaxMoneyPerDay(1000+i);
		temp.SetMaxMoneyTotal(5000+i);
		mapAuthorizate[it] = temp;
	}
	map2.clear();
	runTime = GetTimeMillis();
	map2 = mapAuthorizate;
	cout<<"10 map2 = mapAuthorizate use time "<<GetTimeMillis()-runTime<< endl;

//	for(auto & item : mapAuthorizate) {
//		cout<<HexStr(item.first)<<item.second.ToString(false)<<endl;
//	}
//	cout<<"Second:"<<GetTime()-runTime<<endl

	for(int i = 11;i <=10000;i++){
		it.clear();
		it.push_back(i);
		temp.SetCurMaxMoneyPerDay(1000+i);
			temp.SetMaxMoneyTotal(5000+i);
		mapAuthorizate[it] = temp;
	}
	runTime = GetTimeMillis();
	map2 = mapAuthorizate;
	cout<< &map2 << "mapAuthorizate" << &mapAuthorizate<<"100 map2 = mapAuthorizate use time "<<GetTimeMillis()-runTime<< endl;

//	for(auto & item : mapAuthorizate) {
//		cout<<HexStr(item.first)<<item.second.ToString(false)<<endl;
//	}
//	cout<<"Third:"<<GetTimeMillis()-runTime<<endl;
}
BOOST_AUTO_TEST_SUITE_END()
