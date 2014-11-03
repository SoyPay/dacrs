#include "rpcclient.h"
#include "util.h"
#include <boost/test/unit_test.hpp>
#include "txdb.h"
#include "account.h"
#include <iostream>
using namespace std;

void init() {
	 pScriptDB = new CScriptDB(size_t(4<<20), false , Params().IsReindex());
	 pScriptDBTip = new CScriptDBViewCache(*pScriptDB, false);
}


void ReadData() {

}

void WriteData() {
	vector<unsigned char> vScriptId();
}

void TraversalData() {

}




BOOST_AUTO_TEST_SUITE(scriptdb_test)
BOOST_AUTO_TEST_CASE(test)
{
	WriteData();
	ReadData();
	TraversalData();
}
BOOST_AUTO_TEST_SUITE_END()
