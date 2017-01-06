// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"
#include "rpcserver.h"
#include "init.h"
#include "main.h"
#include "sync.h"
#include "../wallet/wallet.h"

#include <fstream>
#include <stdint.h>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "json/json_spirit_value.h"
#include "json/json_spirit_writer_template.h"

#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_reader.h"
#include "json/json_spirit_stream_reader.h"


using namespace json_spirit;
using namespace std;

void EnsureWalletIsUnlocked();

string static EncodeDumpTime(int64_t llTime) {
    return DateTimeStrFormat("%Y-%m-%dT%H:%M:%SZ", llTime);
}

string DecodeDumpString(const string &str) {
	stringstream ret;
	for (unsigned int unPos = 0; unPos < str.length(); unPos++) {
		unsigned char uchChar = str[unPos];
		if (uchChar == '%' && unPos + 2 < str.length()) {
			uchChar = (((str[unPos + 1] >> 6) * 9 + ((str[unPos + 1] - '0') & 15)) << 4)
					| ((str[unPos + 2] >> 6) * 9 + ((str[unPos + 2] - '0') & 15));
			unPos += 2;
		}
		ret << uchChar;
	}
	return ret.str();
}

Value dropprivkey(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		throw runtime_error("dropprivkey \n"
				"\ndrop private key.\n"
				"\nResult:\n"
				"\nExamples:\n" + HelpExampleCli("dropprivkey", "") + HelpExampleRpc("dropprivkey", ""));
	}

	EnsureWalletIsUnlocked();
	if (!g_pwalletMain->IsReadyForCoolMiner(*g_pAccountViewTip)) {
		throw runtime_error("there is no cool miner key  or miner key in on regist to blockchain\n");
	}

	g_pwalletMain->ClearAllCkeyForCoolMiner();
	Object reply2;
	reply2.push_back(Pair("info", "wallet is ready for cool miner"));
	return reply2;
}

Value importprivkey(const Array& params, bool bHelp) {
	if (bHelp || params.size() < 1 || params.size() > 3) {
		throw runtime_error(
				"importprivkey \"Dacrsprivkey\" ( \"label\" rescan )\n"
						"\nAdds a private key (as returned by dumpprivkey) to your wallet.\n"
						"\nArguments:\n"
						"1. \"Dacrsprivkey\"   (string, required) The private key (see dumpprivkey)\n"
						"2. \"label\"            (string, optional) an optional label\n"
						"3. rescan               (boolean, optional, default=true) Rescan the wallet for transactions\n"
						"\nExamples:\n"
						"\nDump a private key\n" + HelpExampleCli("dumpprivkey", "\"myaddress\"")
						+ "\nImport the private key\n" + HelpExampleCli("importprivkey", "\"mykey\"")
						+ "\nImport using a label\n" + HelpExampleCli("importprivkey", "\"mykey\" \"testing\" false")
						+ "\nAs a json rpc call\n" + HelpExampleRpc("importprivkey", "\"mykey\", \"testing\", false"));
	}

	EnsureWalletIsUnlocked();

	string strSecret = params[0].get_str();
	//  string strLabel = "";
	//  if (params.size() > 1)
	//      strLabel = params[1].get_str();
	//
	//	bool fRescan(true);    // Whether to perform rescan after import
	//	if (params.size() > 2) {
	//		fRescan = params[2].get_bool();
	//	}
	CKey cKey;
	if (strSecret.length() == 32) {
		vector<unsigned char> vuchKey = ParseHex(strSecret.c_str());
		CPrivKey privKey(vuchKey.begin(), vuchKey.end());
		if (!cKey.SetPrivKey(privKey, true)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key");
		}
	} else {
		CDacrsSecret cDacrsSecret;
		bool bGood = cDacrsSecret.SetString(strSecret);

		if (!bGood){
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key encoding");
		}
		cKey = cDacrsSecret.GetKey();
		if (!cKey.IsValid()){
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Private key outside allowed range");
		}
	}

	CPubKey cPubkey = cKey.GetPubKey();
	{
		LOCK2(g_cs_main, g_pwalletMain->m_cs_wallet);

		if (!g_pwalletMain->AddKey(cKey)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "Error adding key to wallet");
		}
	}
	Object reply2;
	reply2.push_back(Pair("imorpt key address", cPubkey.GetKeyID().ToAddress()));
	return reply2;
}

Value importwallet(const Array& params, bool bHelp) {
    if (bHelp || params.size() != 1) {
		throw runtime_error(
				"importwallet \"filename\"\n"
						"\nImports keys from a wallet dump file (see dumpwallet).\n"
						"\nArguments:\n"
						"1. \"filename\"    (string, required) The wallet file\n"
						"\nExamples:\n"
						"\nDump the wallet\n" + HelpExampleCli("dumpwallet", "\"test\"") + "\nImport the wallet\n"
						+ HelpExampleCli("importwallet", "\"test\"") + "\nImport using the json rpc call\n"
						+ HelpExampleRpc("importwallet", "\"test\""));
    }


    LOCK2(g_cs_main, g_pwalletMain->m_cs_wallet);

    EnsureWalletIsUnlocked();

    ifstream file;
    file.open(params[0].get_str().c_str(), ios::in | ios::ate);
    if (!file.is_open()){
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open wallet dump file");
    }
    //  int64_t nTimeBegin = g_cChainActive.Tip()->nTime;

    //  bool fGood = true;
    //  int64_t nFilesize = max((int64_t)1, (int64_t)file.tellg());
    file.seekg(0, file.beg);
    int nPort = 0;
    g_pwalletMain->ShowProgress(_("Importing..."), 0); // show progress dialog in GUI
	if (file.good()) {
		Value reply;
		json_spirit::read(file, reply);
		const Value & keyobj = find_value(reply.get_obj(), "key");
		const Array & keyarry = keyobj.get_array();
		for (auto const &keyItem : keyarry) {
			CKeyCombi cKeyCombi;
			const Value &obj = find_value(keyItem.get_obj(), "keyid");
			if (obj.type() == null_type) {
				continue;
			}
			string strKeyId = find_value(keyItem.get_obj(), "keyid").get_str();
			CKeyID cKeyId(uint160(ParseHex(strKeyId)));
			cKeyCombi.UnSersailFromJson(keyItem.get_obj());
			if (!cKeyCombi.IsContainMainKey() && !cKeyCombi.IsContainMinerKey()) {
				continue;
			}
			if (g_pwalletMain->AddKey(cKeyId, cKeyCombi)) {
				nPort++;
			}
		}
	}
    file.close();
    g_pwalletMain->ShowProgress("", 100); // hide progress dialog in GUI
    g_pwalletMain->ScanForWalletTransactions(g_cChainActive.Genesis(), true);

    Object reply2;
    reply2.push_back(Pair("imorpt key size",nPort));
    return reply2;
}

Value dumpprivkey(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		throw runtime_error(
				"dumpprivkey \"Dacrsaddress\"\n"
						"\nReveals the private key corresponding to 'Dacrsaddress'.\n"
						"Then the importprivkey can be used with this output\n"
						"\nArguments:\n"
						"1. \"Dacrsaddress\"   (string, required) The Dacrs address for the private key\n"
						"\nResult:\n"
						"\"key\"                (string) The private key\n"
						"\nExamples:\n" + HelpExampleCli("dumpprivkey", "\"myaddress\"")
						+ HelpExampleCli("importprivkey", "\"mykey\"")
						+ HelpExampleRpc("dumpprivkey", "\"myaddress\""));
	}


	EnsureWalletIsUnlocked();

	string strAddress = params[0].get_str();
	CDacrsAddress cDacrsAddress;
	if (!cDacrsAddress.SetString(strAddress)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Dacrs address");
	}
	CKeyID cKeyID;
	if (!cDacrsAddress.GetKeyID(cKeyID)) {
		throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to a key");
	}
	CKey cSecret;
	if (!g_pwalletMain->GetKey(cKeyID, cSecret)) {
		throw JSONRPCError(RPC_WALLET_ERROR, "Private key for address " + strAddress + " is not known");
	}
	CKey cMinerKey;
	if (!g_pwalletMain->GetKey(cKeyID, cMinerKey, true)) {
		throw JSONRPCError(RPC_WALLET_ERROR, "Private key for address " + strAddress + " is not known");
	}
	Object reply;
	reply.push_back(Pair("privkey", CDacrsSecret(cSecret).ToString()));
	if (cMinerKey.ToString() != cSecret.ToString()) {
		reply.push_back(Pair("minerkey", CDacrsSecret(cMinerKey).ToString()));
	} else {
		reply.push_back(Pair("minerkey", " "));
	}
	return reply;
}

Value dumpwallet(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		throw runtime_error("dumpwallet \"filename\"\n"
				"\nDumps all wallet keys in a human-readable format.\n"
				"\nArguments:\n"
				"1. \"filename\"    (string, required) The filename\n"
				"\nExamples:\n" + HelpExampleCli("dumpwallet", "\"test\"") + HelpExampleRpc("dumpwallet", "\"test\""));
	}

	EnsureWalletIsUnlocked();

	ofstream file;
	file.open(params[0].get_str().c_str());
	if (!file.is_open()) {
		throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open wallet dump file");
	}
	Object reply;
	reply.push_back(Pair("created by Dacrs", g_strClientBuild + g_strClientDate));
	reply.push_back(Pair("Created Time ", EncodeDumpTime(GetTime())));
	reply.push_back(Pair("Best block index hight ", g_cChainActive.Height()));
	reply.push_back(Pair("Best block hash ", g_cChainActive.Tip()->GetBlockHash().ToString()));

	set<CKeyID> setKeyId;
	g_pwalletMain->GetKeys(setKeyId);
	Array key;
	for (auto & keyId : setKeyId) {
		CKeyCombi cKeyCombi;
		g_pwalletMain->GetKeyCombi(keyId, cKeyCombi);
		Object obj = cKeyCombi.ToJsonObj();
		obj.push_back(Pair("keyid", keyId.ToString()));
		key.push_back(obj);
	}
	reply.push_back(Pair("key", key));
	file << write_string(Value(reply), true);
	file.close();
	Object reply2;
	reply2.push_back(Pair("info", "dump ok"));
	reply2.push_back(Pair("key size", (int) setKeyId.size()));
	return reply2;
}
