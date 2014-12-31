/*
 * CycleTestManger.cpp
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
#include "rpcclient.h"
using namespace std;
using namespace boost;
using namespace json_spirit;
#include "CycleTestBase.h"
#include "CycleTestManger.h"
#include "CycleSesureTrade_tests.h"
class CycleTestManger {

	vector<std::shared_ptr<CycleTestBase> > vTest;

public:
	CycleTestManger(){
		vTest.push_back(std::make_shared<CTestSesureTrade>()) ;
		vTest.push_back(std::make_shared<CTestSesureTrade>()) ;
	};
	void run()
	{
	   while(vTest.size() > 0)
	   {
		 vector<std::shared_ptr<CycleTestBase> >remove;
		 for(auto &it :vTest)
		 {
			 if(it->run()==end_state) {
				 remove.push_back(it);
				 };
			 Sleep(500);
		 }
		 vTest.erase(remove.begin(),remove.end());
	   }
	}
	virtual ~CycleTestManger(){

	};

};


BOOST_FIXTURE_TEST_SUITE(CycleTest,CycleTestManger)

BOOST_FIXTURE_TEST_CASE(Cycle,CycleTestManger)
{
	run();
}

BOOST_AUTO_TEST_SUITE_END()
