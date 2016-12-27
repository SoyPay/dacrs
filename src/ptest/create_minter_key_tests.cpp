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
#include "noui.h"
#include "ui_interface.h"

#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_reader.h"
#include "json/json_spirit_writer.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_stream_reader.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "../test/systestbase.h"
#include "../vm/script.h"
#include "../rpc/rpcclient.h"
#include "../tx.h"
#include "rpc/rpcserver.h"
#include "wallet/wallet.h"
#include "tx.h"
#include "create_minter_key_tests.h"


using namespace boost::unit_test;
using namespace std;
using namespace boost;





const uint64_t kllSendMoney = 10000 * COIN;
bool CCreateMinerkey::SelectAccounts() {
	const char *kszArgv[] = { "rpctest", "listaddr"};
	int nArgc = sizeof(kszArgv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(nArgc, kszArgv, value)) {
		if (value.type() == null_type) {
			return false;
		}
		for(auto & item : value.get_array()) {
			const Value& balance = find_value(item.get_obj(), "balance");
			if(balance.get_real() > 1000000.0) {
				const Value& regId = find_value(item.get_obj(), "regid");
				if("" != regId.get_str() && " "!=regId.get_str()){
					vecAccount.push_back(regId.get_str());
				}
			}
		}
	}
	return true;
}

string CCreateMinerkey::GetOneAccount() {
	for(auto &item : vecAccount) {
		if(GetBalance(item) > 1000000 * COIN) {
			mapSendValue[item] += kllSendMoney;
			if(mapSendValue[item] > 8000000 * COIN){
				continue;
			}
			return item;
		}
	}
	return "";
}

void CCreateMinerkey::CreateAccount() {
//	if(2 == argc){
//		const char* newArgv[] = {argv[0], argv[2] };
//		CBaseParams::IntialParams(2, newArgv);
//	}
	if(!SelectAccounts()){
		return;
	}
	string strTxHash("");
	const int kNewAddrs = 1540;
	string strHash = "";
	vector<string> vstrNewAddress;
//	string strAddress[] = {"0-1","0-2","0-3","0-4","0-5"};

	for (int i = 0; i < kNewAddrs; i++) {
		string strNewaddr;
		BOOST_CHECK(GetNewAddr(strNewaddr, true));
		vstrNewAddress.push_back(strNewaddr);
		string srcAcct = GetOneAccount();
		if("" == srcAcct) {
			cout << "Get source acct failed" << endl;
			return;
		}
		Value value = CreateNormalTx( srcAcct, strNewaddr, kllSendMoney);
		BOOST_CHECK(GetHashFromCreatedTx(value, strHash));
	}
	int nSize = 0 ;
	GenerateOneBlock();

	while (1) {
		if (!GetMemPoolSize(nSize)) {
			cout << "GetMemPoolSize error" << endl;
			return;
		}
		if (nSize > 0) {
			cout << "GetMemPoolSize size :" << nSize << endl;
			MilliSleep(100);
		} else {
			break;
		}

	}

	for(size_t i=0; i < vstrNewAddress.size(); i++) {
		int nFee = GetRandomFee();
		Value value1 = RegistAccountTx(vstrNewAddress[i], nFee);
		BOOST_CHECK(GetHashFromCreatedTx(value1,strHash));
	}

	GenerateOneBlock();
	while (1) {
		if (!GetMemPoolSize(nSize)) {
			cout << "GetMemPoolSize error" << endl;
			return;
		}
		if (nSize > 0) {
			cout << "GetMemPoolSize size :" << nSize << endl;
			MilliSleep(100);
		} else {
			break;
		}

	}

	  cout << "all ok  "  <<  endl;
}

CCreateMinerkey::~CCreateMinerkey() {
	// LEARN Auto-generated destructor stub
}





BOOST_FIXTURE_TEST_SUITE(CreateAccount, CCreateMinerkey)

BOOST_FIXTURE_TEST_CASE(create, CCreateMinerkey){
	CreateAccount();
}

BOOST_AUTO_TEST_SUITE_END()



