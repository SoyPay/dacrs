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
	string strAddress[] = {"0-1","0-2","0-3","0-4","0-5"};
   int index = 0 ;
	for (int i = 0; i < nNewAddrs; i++) {
		string newaddr;
		BOOST_CHECK(GetNewAddr(newaddr, true));
		vNewAddress.push_back(newaddr);
		if (i == 800) {
			++index;
		}
		Value value = CreateNormalTx(strAddress[index], newaddr, 10000 * COIN);
		BOOST_CHECK(GetHashFromCreatedTx(value, hash));
	}
	int size = 0 ;
	GenerateOneBlock();

	while (1) {
		if (!GetMemPoolSize(size)) {
			cout << "GetMemPoolSize error" << endl;
		}
		if (size > 0) {
			cout << "GetMemPoolSize size :" << size << endl;
		} else {
			break;
		}

	}

	for(size_t i=0; i < vNewAddress.size(); i++) {
		int nfee = GetRandomFee();
		Value value1 = registaccounttx(vNewAddress[i], nfee);
		BOOST_CHECK(GetHashFromCreatedTx(value1,hash));
	}

	GenerateOneBlock();
	while (1) {
		if (!GetMemPoolSize(size)) {
			cout << "GetMemPoolSize error" << endl;
		}
		if (size > 0) {
			cout << "GetMemPoolSize size :" << size << endl;
		} else {
			break;
		}

	}

	  cout << "all ok  "  <<  endl;
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



