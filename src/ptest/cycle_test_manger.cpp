/*
 * cycle_test_manger.cpp
 *
 *  Created on: 2014Äê12ÔÂ30ÈÕ
 *      Author: ranger.shi
 */

#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "miner.h"
#include "uint256.h"
#include "util.h"
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "json/json_spirit_writer_template.h"
#include "black_halo_tests.h"
#include "./rpc/rpcclient.h"

#include "cycle_p2p_bet_tests.h"

using namespace std;
using namespace boost;
using namespace json_spirit;

#include "cycle_test_manger.h"
#include "cycle_sesure_trade_tests.h"
#include "create_tx_tests.h"

void CycleTestManger::Initialize() {
	// vTest.push_back(std::make_shared<CTestSesureTrade>());
	// vTest.push_back(std::make_shared<CTestSesureTrade>());
	// vTest.push_back(std::make_shared<CTestSesureTrade>());
	for (int i = 0; i < 100; i++) {
		m_vcTest.push_back(std::make_shared<CBlackHalo>());
	}
	for (int i = 0; i < 100; i++) {
		m_vcTest.push_back(std::make_shared<CTestBetTx>());
	}
	// for (int i = 0; i < 300; i++)
	//		vTest.push_back(std::make_shared<CCreateTxTest>());
}

void CycleTestManger::Initialize(vector<std::shared_ptr<CycleTestBase> > &refvTestIn) {
	m_vcTest = refvTestIn;
}

void CycleTestManger::Run() {
	while (m_vcTest.size() > 0) {
		for (auto it = m_vcTest.begin(); it != m_vcTest.end();) {
			bool bFlag = false;
			try {
				if (it->get()->Run() == EM_END_STATE) {
					bFlag = true;
				};
			} catch (...) {
				bFlag = true;
			}

			if (bFlag) {
				it = m_vcTest.erase(it);
			} else {
				++it;
			}

		}
		MilliSleep(1000);
	}
}

BOOST_FIXTURE_TEST_SUITE(CycleTest,CycleTestManger)

BOOST_FIXTURE_TEST_CASE(Cycle,CycleTestManger) {
	Initialize();
	Run();
}

BOOST_AUTO_TEST_SUITE_END()
