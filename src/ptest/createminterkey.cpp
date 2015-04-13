/*
 * createminterkey.cpp
 *
 *  Created on: 2015Äê4ÔÂ13ÈÕ
 *      Author: ranger.shi
 */
#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "init.h"
#include "miner.h"
#include "uint256.h"
#include "util.h"
#include <boost/test/unit_test.hpp>
#include "../rpc/rpcclient.h"
#include "tx.h"
#include "wallet/wallet.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "../vm/script.h"
#include "rpc/rpcserver.h"
#include "noui.h"
#include "ui_interface.h"
#include "../test/systestbase.h"
#include <boost/algorithm/string/predicate.hpp>
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_reader.h"
#include "json/json_spirit_writer.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_stream_reader.h"
#include "../tx.h"
using namespace std;
using namespace boost;


#include "createminterkey.h"

createminterkey::createminterkey() {
	// LEARN Auto-generated constructor stub

}

void createminterkey::CreateMinerKey() {

	std::string TxHash("");
	const int nNewAddrs = 1420;
	string hash = "";
	vector<string> vNewAddress;
	string strAddress("0-8");
	for (int i = 0; i < nNewAddrs; i++) {
		string newaddr;
		BOOST_CHECK(GetNewAddr(newaddr, false));
		vNewAddress.push_back(newaddr);
		uint64_t nMoney = 10000 * COIN;
		if (i == 800) {
			strAddress = "0-7";
		}
		Value value = CreateNormalTx(strAddress, newaddr, nMoney);
		BOOST_CHECK(GetHashFromCreatedTx(value, hash));
	}
	cout << "create new address completed!" << endl;
}

createminterkey::~createminterkey() {
	// LEARN Auto-generated destructor stub
}

BOOST_FIXTURE_TEST_SUITE(CreateMinerKey,createminterkey)

BOOST_FIXTURE_TEST_CASE(create,createminterkey)
{
	CreateMinerKey();
}

BOOST_AUTO_TEST_SUITE_END()



