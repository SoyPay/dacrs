/*
 * rpctx.cpp
 *
 *  Created on: Aug 26, 2014
 *      Author: leo
 */

#include "txdb.h"

#include "base58.h"
#include "rpcserver.h"
#include "init.h"
#include "net.h"
#include "netbase.h"
#include "util.h"
#include "wallet/wallet.h"
#include "wallet/walletdb.h"
#include "syncdatadb.h"
#include "checkpoints.h"
#include "miner.h"
#include "main.h"
#include "vm/script.h"
#include "vm/vmrunevn.h"
#include <stdint.h>

#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_reader.h"

#include "boost/tuple/tuple.hpp"
#define revert(height) ((height<<24) | (height << 8 & 0xff0000) |  (height>>8 & 0xff00) | (height >> 24))

using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;

extern CAccountViewDB *pAccountViewDB;
string RegIDToAddress(CUserID &userId) {
	CKeyID keid;
	if (pAccountViewTip->GetKeyId(userId, keid)) {
		return keid.ToAddress();
	}
	return "can not get address";
}

static bool GetKeyId(string const &addr, CKeyID &KeyId) {
	if (!CRegID::GetKeyID(addr, KeyId)) {
		KeyId = CKeyID(addr);
		if (KeyId.IsEmpty())
			return false;
	}
	return true;
}


Object GetTxDetailJSON(const uint256& txhash) {
	Object obj;
	std::shared_ptr<CBaseTransaction> pBaseTx;
	{
		LOCK(cs_main);
		CBlock genesisblock;
		CBlockIndex* pgenesisblockindex = mapBlockIndex[SysCfg().HashGenesisBlock()];
		ReadBlockFromDisk(genesisblock, pgenesisblockindex);
		assert(genesisblock.hashMerkleRoot == genesisblock.BuildMerkleTree());
		for(unsigned int i=0; i<genesisblock.vptx.size(); ++i) {
			if(txhash == genesisblock.GetTxHash(i)) {
				obj = genesisblock.vptx[i]->ToJSON(*pAccountViewTip);
				obj.push_back(Pair("blockhash", SysCfg().HashGenesisBlock().GetHex()));
				obj.push_back(Pair("confirmHeight", (int) 0));
				obj.push_back(Pair("confirmedtime", (int) genesisblock.nTime));
				CDataStream ds(SER_DISK, CLIENT_VERSION);
				ds << genesisblock.vptx[i];
				obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
				return obj;
			}
		}

		if (SysCfg().IsTxIndex()) {
			CDiskTxPos postx;
			if (pScriptDBTip->ReadTxIndex(txhash, postx)) {
				CAutoFile file(OpenBlockFile(postx, true), SER_DISK, CLIENT_VERSION);
				CBlockHeader header;
				try {
					file >> header;
					fseek(file, postx.nTxOffset, SEEK_CUR);
					file >> pBaseTx;
					obj = pBaseTx->ToJSON(*pAccountViewTip);
					obj.push_back(Pair("blockhash", header.GetHash().GetHex()));
					obj.push_back(Pair("confirmHeight", (int) header.nHeight));
					obj.push_back(Pair("confirmedtime", (int) header.nTime));
					if(pBaseTx->nTxType == CONTRACT_TX) {
						vector<CVmOperate> vOutput;
						pScriptDBTip->ReadTxOutPut(pBaseTx->GetHash(), vOutput);
						Array outputArray;
						for(auto & item : vOutput) {
							outputArray.push_back(item.ToJson());
						}
						obj.push_back(Pair("listOutput", outputArray));
					}
					CDataStream ds(SER_DISK, CLIENT_VERSION);
					ds << pBaseTx;
					obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
				} catch (std::exception &e) {
					throw runtime_error(tfm::format("%s : Deserialize or I/O error - %s", __func__, e.what()).c_str());
				}
				return obj;
			}
		}
		{
			pBaseTx = mempool.lookup(txhash);
			if (pBaseTx.get()) {
				obj = pBaseTx->ToJSON(*pAccountViewTip);
				CDataStream ds(SER_DISK, CLIENT_VERSION);
				ds << pBaseTx;
				obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
				return obj;
			}
		}

	}
	return obj;
}

Value gettxdetail(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
        throw runtime_error(
            "gettxdetail \"txhash\"\n"
			"\nget the transaction detail by given transaction hash.\n"
            "\nArguments:\n"
            "1.txhash   (string,required) The hast of transaction.\n"
        	"\nResult a object about the transaction detail\n"
            "\nResult:\n"
        	"\n\"txhash\"\n"
            "\nExamples:\n"
            + HelpExampleCli("gettxdetail","c5287324b89793fdf7fa97b6203dfd814b8358cfa31114078ea5981916d7a8ac\n")
            + "\nAs json rpc call\n"
            + HelpExampleRpc("gettxdetail","c5287324b89793fdf7fa97b6203dfd814b8358cfa31114078ea5981916d7a8ac\n"));
	}
	uint256 txhash(uint256S(params[0].get_str()));
	return GetTxDetailJSON(txhash);
}

//create a register account tx
Value registaccounttx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 2) {
	       throw runtime_error(
	            "registaccounttx \"addr\" \"fee\"\n"
				"\nregister secure account\n"
				"\nArguments:\n"
				"1.addr: (string, required)\n"
				"2.fee: (numeric, required) pay to miner\n"
	            "\nResult:\n"
	    		"\"txhash\": (string)\n"
	    		"\nExamples:\n"
	            + HelpExampleCli("registaccounttx", "n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj 100000 ")
	            + "\nAs json rpc call\n"
	            + HelpExampleRpc("registaccounttx", "n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj 100000 "));

	}

	string addr = params[0].get_str();
	uint64_t fee = params[1].get_uint64();

	//get keyid
	CKeyID keyid;
	if (!GetKeyId(addr, keyid)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "in registaccounttx :address err");
	}
	CRegisterAccountTx rtx;
	assert(pwalletMain != NULL);
	{
		//	LOCK2(cs_main, pwalletMain->cs_wallet);
		EnsureWalletIsUnlocked();

		//balance
		CAccountViewCache view(*pAccountViewTip, true);
		CAccount account;

		CUserID userId = keyid;
		if (!view.GetAccount(userId, account)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registaccounttx Error: Account balance is insufficient.");
		}

		if (account.IsRegister()) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registaccounttx Error: Account is already registered");
		}
		uint64_t balance = account.GetRawBalance();
		if (balance < fee) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registaccounttx Error: Account balance is insufficient.");
		}

		CPubKey pubkey;
		if (!pwalletMain->GetPubKey(keyid, pubkey)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registaccounttx Error: not find key.");
		}

		CPubKey MinerPKey;
		if (pwalletMain->GetPubKey(keyid, MinerPKey, true)) {
			rtx.minerId = MinerPKey;
		} else {
			CNullID nullId;
			rtx.minerId = nullId;
		}
		rtx.userId = pubkey;
		rtx.llFees = fee;
		rtx.nValidHeight = chainActive.Tip()->nHeight;

		if (!pwalletMain->Sign(keyid, rtx.SignatureHash(), rtx.signature)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registaccounttx Error: Sign failed.");
		}

	}

	std::tuple<bool, string> ret;
	ret = pwalletMain->CommitTransaction((CBaseTransaction *) &rtx);
	if (!std::get<0>(ret)) {
		throw JSONRPCError(RPC_WALLET_ERROR, "registaccounttx Error:" + std::get<1>(ret));
	}
	Object obj;
	obj.push_back(Pair("hash", std::get<1>(ret)));
	return obj;

}

//create a contract tx
Value createcontracttx(const Array& params, bool fHelp) {
	if (fHelp || params.size() < 5 || params.size() > 6) {
	   throw runtime_error(
			"createcontracttx \"userregid[\"addr\"]\" \"appid\" \"amount\" \"contract\" \"fee\" (\"height\")\n"
			"\ncreate contract transaction\n"
			"\nArguments:\n"
			"1.\"userregid\": (string, required)\n the address for send"
			"2.\"appid\":(string, required) the appID (for example: Ipo.bin)\n"
			"3.\"amount\":(numeric, required)\n"
			"4.\"contract\": (string, required)\n"
			"5.\"fee\": (numeric, required) pay to miner\n"
			"6.\"height\": (numeric, optional)create height,If not provide use the tip block hegiht in chainActive\n"
			"\nResult:\n"
			"\"contract tx str\": (string)\n"
			"\nExamples:\n"
			+ HelpExampleCli("createcontracttx", "000000000100 [\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"] "
					"\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\""
					"100000 "
					"\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\" "
					"01020304 "
					"1") + "\nAs json rpc call\n"
			+ HelpExampleRpc("createcontracttx", "000000000100 [\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"] "
					"\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\""
					"100000 "
					"\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\" "
					"01020304 "
					"1"));
	}

	RPCTypeCheck(params, list_of(str_type)(str_type)(int_type)(str_type)(int_type)(int_type));

	CRegID userId(params[0].get_str());
	CKeyID srckeyid;
	if (userId.IsEmpty()) {
		if (!GetKeyId(params[0].get_str(), srckeyid)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Source Invalid  address");
		}
		if(!pAccountViewTip->GetRegId(CUserID(srckeyid),userId))
		{
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "address not regist");
		}

	}

	CRegID appId(params[1].get_str());
	uint64_t amount = params[2].get_uint64();
	vector<unsigned char> vcontract = ParseHex(params[3].get_str());
	uint64_t fee = params[4].get_uint64();
	uint32_t height(0);
	if (params.size() > 5)
		height = params[5].get_int();

	if (fee > 0 && fee < CTransaction::nMinTxFee) {
		throw runtime_error("in createcontracttx :fee is smaller than nMinTxFee\n");
	}

	if (appId.IsEmpty()) {
		throw runtime_error("in createcontracttx :addresss is error!\n");
	}
	EnsureWalletIsUnlocked();
	std::shared_ptr<CTransaction> tx = make_shared<CTransaction>();
	{
		//balance
		CAccountViewCache view(*pAccountViewTip, true);
		CAccount secureAcc;

		if (!pScriptDBTip->HaveScript(appId)) {
			throw runtime_error(tinyformat::format("createcontracttx :script id %s is not exist\n", appId.ToString()));
		}
		tx.get()->nTxType = CONTRACT_TX;
		tx.get()->srcRegId = userId;
		tx.get()->desUserId = appId;
		tx.get()->llValues = amount;
		tx.get()->llFees = fee;
		tx.get()->vContract = vcontract;
		if (0 == height) {
			height = chainActive.Tip()->nHeight;
		}
		tx.get()->nValidHeight = height;

		//get keyid by accountid
		CKeyID keyid;
		if (!view.GetKeyId(userId, keyid)) {
			CID id(userId);
			LogPrint("INFO", "vaccountid:%s have no key id\r\n", HexStr(id.GetID()).c_str());
//			assert(0);
			throw JSONRPCError(RPC_WALLET_ERROR, "createcontracttx Error: have no key id.");
		}

		vector<unsigned char> signature;
		if (!pwalletMain->Sign(keyid, tx.get()->SignatureHash(), tx.get()->signature)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "createcontracttx Error: Sign failed.");
		}
	}

	std::tuple<bool, string> ret;
	ret = pwalletMain->CommitTransaction((CBaseTransaction *) tx.get());
	if (!std::get<0>(ret)) {
		throw JSONRPCError(RPC_WALLET_ERROR, "Error:" + std::get<1>(ret));
	}
	Object obj;
	obj.push_back(Pair("hash", std::get<1>(ret)));
	return obj;

}

//create a register script tx
Value registerapptx(const Array& params, bool fHelp) {
	if (fHelp || params.size() < 3 || params.size() > 5) {
		throw runtime_error("registerapptx \"addr\" \"filepath\"\"fee\" (\"height\") (\"scriptdescription\")\n"
				"\ncreate a register script transaction\n"
				"\nArguments:\n"
				"1.\"addr\": (string required)\n"
				"2.\"filepath\": (string required),app's file path\n"
				"3.\"fee\": (numeric required) pay to miner\n"
				"4.\"height\": (numeric optional)valid height,If not provide, use the tip block hegiht in chainActive\n"
				"5.\"scriptdescription\":(string optional) new script description\n"
				"\nResult:\n"
				"\"txhash\": (string)\n"
				"\nExamples:\n"
				+ HelpExampleCli("registerapptx",
						"\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\" \"run.exe\" \"010203040506\" \"100000\" (\"scriptdescription\")") + "\nAs json rpc call\n"
				+ HelpExampleRpc("registerapptx",
						"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG \"run.exe\" \"010203040506\" \"100000\" (\"scriptdescription\")"));
	}

	RPCTypeCheck(params, list_of(str_type)(str_type)(int_type)(int_type)(str_type));
	CVmScript vmScript;
	vector<unsigned char> vscript;

	string path = params[1].get_str();
	FILE* file = fopen(path.c_str(), "rb+");
	if (!file) {
		throw runtime_error("create registerapptx open script file" + path + "error");
	}
	long lSize;
	fseek(file, 0, SEEK_END);
	lSize = ftell(file);
	rewind(file);

	// allocate memory to contain the whole file:
	char *buffer = (char*) malloc(sizeof(char) * lSize);
	if (buffer == NULL) {
		fclose(file); //及时关闭
		throw runtime_error("allocate memory failed");
	}
	if (fread(buffer, 1, lSize, file) != (size_t) lSize) {
		free(buffer);  //及时释放
		fclose(file);  //及时关闭
		throw runtime_error("read script file error");
	}
	else
	{
		fclose(file); //使用完关闭文件
	}

	vmScript.Rom.insert(vmScript.Rom.end(), buffer, buffer + lSize);
    if (buffer)
		free(buffer);

	if (params.size() > 4) {
		string scriptDesc = params[4].get_str();
		vmScript.ScriptExplain.insert(vmScript.ScriptExplain.end(), scriptDesc.begin(), scriptDesc.end());
	}

	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << vmScript;
	vscript.assign(ds.begin(), ds.end());

	uint64_t fee = params[2].get_uint64();
	int height(0);
	if (params.size() > 3)
		height = params[3].get_int();

	if (fee > 0 && fee < CTransaction::nMinTxFee) {
		throw runtime_error("in registerapptx :fee is smaller than nMinTxFee\n");
	}
	//get keyid
	CKeyID keyid;
	if (!GetKeyId(params[0].get_str(), keyid)) {
		throw runtime_error("in registerapptx :send address err\n");
	}

	assert(pwalletMain != NULL);
	CRegisterAppTx tx;
	{
		//	LOCK2(cs_main, pwalletMain->cs_wallet);
		EnsureWalletIsUnlocked();
		//balance
		CAccountViewCache view(*pAccountViewTip, true);
		CAccount account;

		uint64_t balance = 0;
		CUserID userId = keyid;
		if (view.GetAccount(userId, account)) {
			balance = account.GetRawBalance();
		}

		if (!account.IsRegister()) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registerapptx Error: Account is not registered.");
		}

		if (!pwalletMain->HaveKey(keyid)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registerapptx Error: WALLET file is not correct.");
		}

		if (balance < fee) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registerapptx Error: Account balance is insufficient.");
		}

		auto GetUserId =
				[&](CKeyID &mkeyId)
				{
					CAccount acct;
					if (view.GetAccount(CUserID(mkeyId), acct)) {
						return acct.regID;
					}
					throw runtime_error(tinyformat::format("registerapptx :account id %s is not exist\n", mkeyId.ToAddress()));
				};

		tx.regAcctId = GetUserId(keyid);
		tx.script = vscript;
		tx.llFees = fee;
		tx.nRunStep = vscript.size();
		if (0 == height) {
			height = chainActive.Tip()->nHeight;
		}
		tx.nValidHeight = height;

		if (!pwalletMain->Sign(keyid, tx.SignatureHash(), tx.signature)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "registerapptx Error: Sign failed.");
		}
	}

	std::tuple<bool, string> ret;
	ret = pwalletMain->CommitTransaction((CBaseTransaction *) &tx);
	if (!std::get<0>(ret)) {
		throw JSONRPCError(RPC_WALLET_ERROR, "registerapptx Error:" + std::get<1>(ret));
	}
	Object obj;
	obj.push_back(Pair("hash", std::get<1>(ret)));
	return obj;

}

Value listaddr(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0) {
		throw runtime_error(
				 "listaddr\n"
				 "\nreturn Array containing address,balance,haveminerkey,regid information.\n"
				 "\nArguments:\n"
				 "\nResult:\n"
				 "\nExamples:\n"
				 + HelpExampleCli("listaddr", "")
				 + "\nAs json rpc call\n"
                 + HelpExampleRpc("listaddr", ""));
	}
	Array retArry;
	assert(pwalletMain != NULL);
	{
		set<CKeyID> setKeyId;
		pwalletMain->GetKeys(setKeyId);
		if (setKeyId.size() == 0) {
			return retArry;
		}
		CAccountViewCache accView(*pAccountViewTip, true);

		for (const auto &keyId : setKeyId) {
			CUserID userId(keyId);
			CAccount acctInfo;
			accView.GetAccount(userId, acctInfo);
			CKeyCombi keyCombi;
			pwalletMain->GetKeyCombi(keyId, keyCombi);

			Object obj;
			obj.push_back(Pair("addr", keyId.ToAddress()));
			obj.push_back(Pair("balance", (double)acctInfo.GetRawBalance()/ (double) COIN));
			obj.push_back(Pair("haveminerkey", keyCombi.IsContainMinerKey()));
			obj.push_back(Pair("regid",acctInfo.regID.ToString()));
			retArry.push_back(obj);
		}
	}

	return retArry;
}

Value listtx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0) {
		throw runtime_error(
				 "listtx\n"
				 "\nget all confirm transactions and all unconfirm transactions from wallet.\n"
				 "\nArguments:\n"
				 "\nResult:\n"
				 "\nExamples:\n"
				 + HelpExampleCli("listtx", "")
                 + HelpExampleRpc("listtx", ""));
	}

	Object retObj;
	assert(pwalletMain != NULL);
	{
		Object Inblockobj;
		for (auto const &wtx : pwalletMain->mapInBlockTx) {
			for (auto const & item : wtx.second.mapAccountTx) {
				Inblockobj.push_back(Pair("tx", item.first.GetHex()));
			}
		}
		retObj.push_back(Pair("ConfirmTx", Inblockobj));

		CAccountViewCache view(*pAccountViewTip, true);
		Array UnConfirmTxArry;
		for (auto const &wtx : pwalletMain->UnConfirmTx) {
			UnConfirmTxArry.push_back(wtx.first.GetHex());
		}
		retObj.push_back(Pair("UnConfirmTx", UnConfirmTxArry));
	}
	return retObj;
}

Value getaccountinfo(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		  throw runtime_error(
				"getaccountinfo \"addr\"\n"
				"\nget account information\n"
				"\nArguments:\n"
				"1.\"addr\": (string, required)"
				"Returns an object containing various account info.\n"
				"\nResult:\n"
				"\nExamples:\n"
				+ HelpExampleCli("getaccountinfo", "000000000500\n")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("getaccountinfo", "5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\n")
		   );
	}
	RPCTypeCheck(params, list_of(str_type));
	CKeyID keyid;
	CUserID userId;
	string addr = params[0].get_str();
	if (!GetKeyId(addr, keyid)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid  address");
	}

	userId = keyid;
	Object obj;
	{
		CAccount account;

		CAccountViewCache accView(*pAccountViewTip, true);
		if (accView.GetAccount(userId, account)) {
			if (!account.PublicKey.IsValid()) {
				CPubKey pk;
				CPubKey minerpk;
				if (pwalletMain->GetPubKey(keyid, pk)) {
					pwalletMain->GetPubKey(keyid, minerpk, true);
					account.PublicKey = pk;
					account.keyID = std::move(pk.GetKeyID());
					if (pk != minerpk && !account.MinerPKey.IsValid()) {
						account.MinerPKey = minerpk;
					}
				}
			}
			obj = std::move(account.ToJosnObj());
			obj.push_back(Pair("postion", "inblock"));
		} else {
			CPubKey pk;
			CPubKey minerpk;
			if (pwalletMain->GetPubKey(keyid, pk)) {
				pwalletMain->GetPubKey(keyid, minerpk, true);
				account.PublicKey = pk;
				account.keyID = pk.GetKeyID();
				if (minerpk != pk) {
					account.MinerPKey = minerpk;
				}
				obj = std::move(account.ToJosnObj());
				obj.push_back(Pair("postion", "inwallet"));
			}
		}

	}
	return obj;
}

//list unconfirmed transaction of mine
Value listunconfirmedtx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0) {
		 throw runtime_error(
		            "listunconfirmedtx \n"
					"\nget the list  of unconfirmedtx.\n"
		            "\nArguments:\n"
		        	"\nResult a object about the unconfirm transaction\n"
		            "\nResult:\n"
		            "\nExamples:\n"
		            + HelpExampleCli("listunconfirmedtx", "")
		            + "\nAs json rpc call\n"
		            + HelpExampleRpc("listunconfirmedtx", ""));
	}

	Object retObj;
	CAccountViewCache view(*pAccountViewTip, true);
	Array UnConfirmTxArry;
	for (auto const &wtx : pwalletMain->UnConfirmTx) {
		UnConfirmTxArry.push_back(wtx.second.get()->ToString(view));
	}
	retObj.push_back(Pair("UnConfirmTx", UnConfirmTxArry));

	return retObj;
}

//sign
Value sign(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 2) {
		string msg = "sign nrequired \"str\"\n"
				"\nsign \"str\"\n"
				"\nArguments:\n"
				"1.\"str\": (string) \n"
				"\nResult:\n"
				"\"signhash\": (string)\n"
				"\nExamples:\n"
				+ HelpExampleCli("sign",
						"0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("sign",
						"0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000");
		throw runtime_error(msg);
	}

	vector<unsigned char> vch(ParseHex(params[1].get_str()));

	//get keyid
	CKeyID keyid;
	if (!GetKeyId(params[0].get_str(), keyid)) {
		throw runtime_error("in sign :send address err\n");
	}
	vector<unsigned char> vsign;
	{
		LOCK(pwalletMain->cs_wallet);

		CKey key;
		if (!pwalletMain->GetKey(keyid, key)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "sign Error: cannot find key.");
		}

		uint256 hash = Hash(vch.begin(), vch.end());
		if (!key.Sign(hash, vsign)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "sign Error: Sign failed.");
		}
	}
	Object retObj;
	retObj.push_back(Pair("signeddata", HexStr(vsign)));
	return retObj;
}
//
//Value getaccountinfo(const Array& params, bool fHelp) {
//	if (fHelp || params.size() != 1) {
//		throw runtime_error(
//				"getaccountinfo \"address \" dspay address ( \"comment\" \"comment-to\" )\n"
//						"\nGet an account info with dspay address\n" + HelpRequiringPassphrase() + "\nArguments:\n"
//						"1. \"address \"  (string, required) The Dacrs address.\n"
//						"\nResult:\n"
//						"\"account info\"  (string) \n"
//						"\nExamples:\n" + HelpExampleCli("getaccountinfo", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\"")
//						+ HelpExampleCli("getaccountinfo", "\"000000010100\"")
//
//						);
//	}
//	CAccountViewCache view(*pAccountViewTip, true);
//	string strParam = params[0].get_str();
//	CAccount aAccount;
//	if (strParam.length() != 12) {
//		CDacrsAddress address(params[0].get_str());
//		CKeyID keyid;
//		if (!address.GetKeyID(keyid))
//			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Dacrs address");
//
//		CUserID userId = keyid;
//		if (!view.GetAccount(userId, aAccount)) {
//			return "can not get account info by address:" + strParam;
//		}
//	} else {
//		CRegID regId(ParseHex(strParam));
//		if (!view.GetAccount(regId, aAccount)) {
//			return "can not get account info by regid:" + strParam;
//		}
//	}
//	string fundTypeArray[] = {"NULL_FUNDTYPE", "FREEDOM", "REWARD_FUND", "FREEDOM_FUND", "FREEZD_FUND", "SELF_FREEZD_FUND"};
//
//	Object obj;
//	obj.push_back(Pair("keyID:", aAccount.keyID.ToString()));
//	obj.push_back(Pair("publicKey:", aAccount.publicKey.ToString()));
//	obj.push_back(Pair("llValues:", tinyformat::format("%s", aAccount.llValues)));
//	Array array;
//	//string str = ("fundtype  txhash                                  value                        height");
//	string str = tinyformat::format("%-20.20s%-20.20s%-25.8lf%-6.6d", "fundtype", "scriptid", "value", "height");
//	array.push_back(str);
//	for (int i = 0; i < aAccount.vRewardFund.size(); ++i) {
//		CFund fund = aAccount.vRewardFund[i];
//		array.push_back(tinyformat::format("%-20.20s%-20.20s%-25.8lf%-6.6d",fundTypeArray[fund.nFundType], HexStr(fund.scriptID), fund.value, fund.nHeight));
//	}
//
//	for (int i = 0; i < aAccount.vFreedomFund.size(); ++i) {
//		CFund fund = aAccount.vFreedomFund[i];
//		array.push_back(tinyformat::format("%-20.20s%-20.20s%-25.8lf%-6.6d",fundTypeArray[fund.nFundType], HexStr(fund.scriptID), fund.value, fund.nHeight));
//	}
//	for (int i = 0; i < aAccount.vFreeze.size(); ++i) {
//		CFund fund = aAccount.vFreeze[i];
//		array.push_back(tinyformat::format("%-20.20s%-20.20s%-25.8lf%-6.6d",fundTypeArray[fund.nFundType], HexStr(fund.scriptID), fund.value, fund.nHeight));
//	}
//	for (int i = 0; i < aAccount.vSelfFreeze.size(); ++i) {
//		CFund fund = aAccount.vSelfFreeze[i];
//		array.push_back(tinyformat::format("%-20.20s%-20.20s%-25.8lf%-6.6d",fundTypeArray[fund.nFundType], HexStr(fund.scriptID), fund.value, fund.nHeight));
//	}
//	obj.push_back(Pair("detailinfo:", array));
//	return obj;
//}

static Value AccountLogToJson(const CAccountLog &accoutLog) {
	Object obj;
	obj.push_back(Pair("keyid", accoutLog.keyID.ToString()));
	obj.push_back(Pair("llValues", accoutLog.llValues));
	obj.push_back(Pair("nHeight", accoutLog.nHeight));
	obj.push_back(Pair("nCoinDay", accoutLog.nCoinDay));
//	Array array;
//	for(auto const &te: accoutLog.vRewardFund)
//	{
//      Object obj2;
//      obj2.push_back(Pair("value",  te.value));
//      obj2.push_back(Pair("nHeight",  te.nHeight));
//      array.push_back(obj2);
//	}
//	obj.push_back(Pair("vRewardFund", array));
	return obj;
}

Value gettxoperationlog(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		throw runtime_error("gettxoperationlog \"txhash\"\n"
					"\nget transaction operation log\n"
					"\nArguments:\n"
					"1.\"txhash\": (string required) \n"
					"\nResult:\n"
					"\"vOperFund\": (string)\n"
					"\"authorLog\": (string)\n"
					"\nExamples:\n"
					+ HelpExampleCli("gettxoperationlog",
							"\"0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000\"")
					+ "\nAs json rpc call\n"
					+ HelpExampleRpc("gettxoperationlog",
							"\"0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000\""));
	}
	RPCTypeCheck(params, list_of(str_type));
	uint256 txHash(uint256S(params[0].get_str()));
	vector<CAccountLog> vLog;
	Object retobj;
	retobj.push_back(Pair("hash", txHash.GetHex()));
	if (!GetTxOperLog(txHash, vLog))
		throw JSONRPCError(RPC_INVALID_PARAMS, "error hash");
	{
		Array arrayvLog;
		for (auto const &te : vLog) {
			Object obj;
			obj.push_back(Pair("addr", te.keyID.ToAddress()));
			Array array;
			array.push_back(AccountLogToJson(te));
			arrayvLog.push_back(obj);
		}
		retobj.push_back(Pair("AccountOperLog", arrayvLog));

	}
	return retobj;

}

static Value TestDisconnectBlock(int number) {
//		CBlockIndex* pindex = chainActive.Tip();
	CBlock block;
	Object obj;

	CValidationState state;
	if ((chainActive.Tip()->nHeight - number) < 0) {
		throw JSONRPCError(RPC_INVALID_PARAMS, "restclient Error: number");
	}
	if (number > 0) {
		do {
			// check level 0: read from disk
			CBlockIndex * pTipIndex = chainActive.Tip();
			LogPrint("vm", "current height:%d\n", pTipIndex->nHeight);
			if (!DisconnectBlockFromTip(state))
				return false;
			chainMostWork.SetTip(pTipIndex->pprev);
			if (!EraseBlockIndexFromSet(pTipIndex))
				return false;
			if (!pblocktree->EraseBlockIndex(pTipIndex->GetBlockHash()))
				return false;
			mapBlockIndex.erase(pTipIndex->GetBlockHash());

//			if (!ReadBlockFromDisk(block, pindex))
//				throw ERRORMSG("VerifyDB() : *** ReadBlockFromDisk failed at %d, hash=%s", pindex->nHeight,
//						pindex->GetBlockHash().ToString());
//			bool fClean = true;
//			CTransactionDBCache txCacheTemp(*pTxCacheTip, true);
//			CScriptDBViewCache contractScriptTemp(*pScriptDBTip, true);
//			if (!DisconnectBlock(block, state, view, pindex, txCacheTemp, contractScriptTemp, &fClean))
//				throw ERRORMSG("VerifyDB() : *** irrecoverable inconsistency in block data at %d, hash=%s", pindex->nHeight,
//						pindex->GetBlockHash().ToString());
//			CBlockIndex *pindexDelete = pindex;
//			pindex = pindex->pprev;
//			chainActive.SetTip(pindex);
//
//			assert(view.Flush() &&txCacheTemp.Flush()&& contractScriptTemp.Flush() );
//			txCacheTemp.Clear();
		} while (--number);
	}
//		pTxCacheTip->Flush();

	obj.push_back(Pair("tip", strprintf("hash:%s hight:%s",chainActive.Tip()->GetBlockHash().ToString(),chainActive.Tip()->nHeight)));
	return obj;
}

Value disconnectblock(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		throw runtime_error("disconnectblock \"numbers\" \n"
				"\ndisconnect block\n"
				"\nArguments:\n"
				"1. \"numbers \"  (numeric, required) the block numbers.\n"
				"\nResult:\n"
				"\"disconnect result\"  (bool) \n"
				"\nExamples:\n"
				+ HelpExampleCli("disconnectblock", "\"1\"")
				+ HelpExampleRpc("gettxoperationlog","\"1\""));
	}
	int number = params[0].get_int();

	Value te = TestDisconnectBlock(number);

	return te;
}

Value resetclient(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0) {
		throw runtime_error("resetclient \n"
						"\nreset client\n"
						"\nArguments:\n"
						"\nResult:\n"
						"\nExamples:\n"
						+ HelpExampleCli("resetclient", "")
						+ HelpExampleRpc("resetclient",""));
		}
	Value te = TestDisconnectBlock(chainActive.Tip()->nHeight);

	if (chainActive.Tip()->nHeight == 0) {
		pwalletMain->CleanAll();
		CBlockIndex* te = chainActive.Tip();
		uint256 hash = te->GetBlockHash();
//		auto ret = remove_if( mapBlockIndex.begin(), mapBlockIndex.end(),[&](std::map<uint256, CBlockIndex*>::reference a) {
//			return (a.first == hash);
//		});
//		mapBlockIndex.erase(ret,mapBlockIndex.end());
		for (auto it = mapBlockIndex.begin(), ite = mapBlockIndex.end(); it != ite;) {
			if (it->first != hash)
				it = mapBlockIndex.erase(it);
			else
				++it;
		}
		pAccountViewTip->Flush();
		pScriptDBTip->Flush();
		pTxCacheTip->Flush();

		assert(pAccountViewDB->GetDbCount() == 43);
		assert(pScriptDB->GetDbCount() == 0 || pScriptDB->GetDbCount() == 1);
		assert(pTxCacheTip->GetSize() == 0);

		CBlock firs = SysCfg().GenesisBlock();
		pwalletMain->SyncTransaction(uint256(), NULL, &firs);
		mempool.clear();
	} else {
		throw JSONRPCError(RPC_WALLET_ERROR, "restclient Error: Sign failed.");
	}
	return te;

}

Value listapp(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		throw runtime_error("listapp \"showDetail\" \n"
				"\nget the list register script\n"
				"\nArguments:\n"
	            "1. showDetail  (boolean, required)true to show scriptContent,otherwise to not show it.\n"
				"\nResult an object contain many script data\n"
				"\nResult:\n"
				"\nExamples:\n" + HelpExampleCli("listapp", "true") + HelpExampleRpc("listapp", "true"));
	}
	bool showDetail = false;
	showDetail = params[0].get_bool();
	Object obj;
	Array arrayScript;

//	CAccountViewCache view(*pAccountViewTip, true);
	if (pScriptDBTip != NULL) {
		int nCount(0);
		if (!pScriptDBTip->GetScriptCount(nCount))
			throw JSONRPCError(RPC_DATABASE_ERROR, "get script error: cannot get registered count.");
		CRegID regId;
		vector<unsigned char> vScript;
		Object script;
		if (!pScriptDBTip->GetScript(0, regId, vScript))
			throw JSONRPCError(RPC_DATABASE_ERROR, "get script error: cannot get registered script.");
		script.push_back(Pair("scriptId", regId.ToString()));
		script.push_back(Pair("scriptId2", HexStr(regId.GetVec6())));
		if (showDetail)
			script.push_back(Pair("scriptContent", HexStr(vScript.begin(), vScript.end())));
		arrayScript.push_back(script);
		while (pScriptDBTip->GetScript(1, regId, vScript)) {
			Object obj;
			obj.push_back(Pair("scriptId", regId.ToString()));
			obj.push_back(Pair("scriptId2", HexStr(regId.GetVec6())));
			if (showDetail)
				obj.push_back(Pair("scriptContent", string(vScript.begin(), vScript.end())));
			arrayScript.push_back(obj);
		}
	}

	obj.push_back(Pair("listregedscript", arrayScript));
	return obj;
}

Value getaddrbalance(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		string msg = "getaddrbalance nrequired [\"key\",...] ( \"account\" )\n"
				"\nAdd a nrequired-to-sign multisignature address to the wallet.\n"
				"Each key is a  address or hex-encoded public key.\n" + HelpExampleCli("getaddrbalance", "")
				+ "\nAs json rpc call\n" + HelpExampleRpc("getaddrbalance", "");
		throw runtime_error(msg);
	}

	assert(pwalletMain != NULL);

	CKeyID keyid;
	if (!GetKeyId(params[0].get_str(), keyid))
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid  address");

	double dbalance = 0.0;
	{
		LOCK(cs_main);
		CAccountViewCache accView(*pAccountViewTip, true);
		CAccount secureAcc;
		CUserID userId = keyid;
		if (accView.GetAccount(userId, secureAcc)) {
			dbalance = (double) secureAcc.GetRawBalance() / (double) COIN;
		}
	}
	return dbalance;
}

Value generateblock(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		throw runtime_error("generateblock \"addr\"\n"
				"\ncteate a block with the appointed address\n"
				"\nArguments:\n"
				"1.\"addr\": (string, required)\n"
				"\nResult:\n"
				"\nblockhash\n"
				"\nExamples:\n" +
				HelpExampleCli("generateblock", "\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\"")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("generateblock", "\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\""));
	}
	//get keyid
	CKeyID keyid;

	if (!GetKeyId(params[0].get_str(), keyid)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "in generateblock :address err");
	}

	uint256 hash = CreateBlockWithAppointedAddr(keyid);
	if (hash.IsNull()) {
		throw runtime_error("in generateblock :cannot generate block\n");
	}
	Object obj;
	obj.push_back(Pair("blockhash", hash.GetHex()));
	return obj;
}

Value listtxcache(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0) {
		throw runtime_error("listtxcache\n"
				"\nget all transactions in cahce\n"
				"\nArguments:\n"
				"\nResult:\n"
				"\"txcache\"  (string) \n"
				"\nExamples:\n" + HelpExampleCli("listtxcache", "")+ HelpExampleRpc("listtxcache", ""));
	}
	const map<uint256, vector<uint256> > &mapTxHashByBlockHash = pTxCacheTip->GetTxHashCache();

	Array retTxHashArray;
	for (auto &item : mapTxHashByBlockHash) {
		Object blockObj;
		Array txHashArray;
		blockObj.push_back(Pair("blockhash", item.first.GetHex()));
		for (auto &txHash : item.second)
			txHashArray.push_back(txHash.GetHex());
		blockObj.push_back(Pair("txcache", txHashArray));
		retTxHashArray.push_back(blockObj);
	}

	return retTxHashArray;
}

Value reloadtxcache(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0) {
		throw runtime_error("reloadtxcache \n"
				"\nreload transactions catch\n"
				"\nArguments:\n"
				"\nResult:\n"
				"\nExamples:\n"
				+ HelpExampleCli("reloadtxcache", "")
				+ HelpExampleRpc("reloadtxcache", ""));
	}
	pTxCacheTip->Clear();
	CBlockIndex *pIndex = chainActive.Tip();
	if ((chainActive.Tip()->nHeight - SysCfg().GetTxCacheHeight()) >= 0) {
		pIndex = chainActive[(chainActive.Tip()->nHeight - SysCfg().GetTxCacheHeight())];
	} else {
		pIndex = chainActive.Genesis();
	}
	CBlock block;
	do {
		if (!ReadBlockFromDisk(block, pIndex))
			return ERRORMSG("reloadtxcache() : *** ReadBlockFromDisk failed at %d, hash=%s", pIndex->nHeight,
					pIndex->GetBlockHash().ToString());
		pTxCacheTip->AddBlockToCache(block);
		pIndex = chainActive.Next(pIndex);
	} while (NULL != pIndex);

	Object obj;
	obj.push_back(Pair("info", "reload tx cache succeed"));
	return obj;
}

static int getDataFromSriptData(CScriptDBViewCache &cache, const CRegID &regid, int pagesize, int index,
		vector<std::tuple<vector<unsigned char>, vector<unsigned char> > >&ret) {
	int dbsize;
	int height = chainActive.Height();
	cache.GetScriptDataCount(regid, dbsize);
	if (0 == dbsize) {
		throw runtime_error("in getscriptdata :the scirptid database not data!\n");
	}
	vector<unsigned char> value;
	vector<unsigned char> vScriptKey;

	if (!cache.GetScriptData(height, regid, 0, vScriptKey, value)) {
		throw runtime_error("in getscriptdata :the scirptid get data failed!\n");
	}
	if (index == 1) {
		ret.push_back(std::make_tuple(vScriptKey, value));
	}
	int readCount(1);
	while (--dbsize) {
		if (cache.GetScriptData(height, regid, 1, vScriptKey, value)) {
			++readCount;
			if (readCount > pagesize * (index - 1)) {
				ret.push_back(std::make_tuple(vScriptKey, value));
			}
		}
		if (readCount >= pagesize * index) {
			return ret.size();
		}
	}
	return ret.size();
}

Value getscriptdata(const Array& params, bool fHelp) {
	if (fHelp || params.size() < 2 || params.size() > 3) {
		throw runtime_error("getscriptdata \"scriptid\" \"[pagesize or key]\" (\"index\")\n"
				"\nget the script data by given scriptID\n"
				"\nArguments:\n"
				"1.\"scriptid\": (string, required)\n"
				"2.[pagesize or key]: (pagesize int, required),if only two param,it is key,otherwise it is pagesize\n"
				"3.\"index\": (int optional)\n"
				"\nResult:\n"
				"\nExamples:\n"
				+ HelpExampleCli("getscriptdata", "\"123456789012\"")
				+ HelpExampleRpc("getscriptdata", "\"123456789012\""));
	}
	int height = chainActive.Height();
//	//RPCTypeCheck(params, list_of(str_type)(int_type)(int_type));
//	vector<unsigned char> vscriptid = ParseHex(params[0].get_str());
	CRegID regid(params[0].get_str());
	if (regid.IsEmpty() == true) {
		throw runtime_error("in getscriptdata :vscriptid size is error!\n");
	}

	if (!pScriptDBTip->HaveScript(regid)) {
		throw runtime_error("in getscriptdata :vscriptid id is exist!\n");
	}
	Object script;

	CScriptDBViewCache contractScriptTemp(*pScriptDBTip, true);
	if (params.size() == 2) {
		vector<unsigned char> key = ParseHex(params[1].get_str());
		vector<unsigned char> value;
		if (!contractScriptTemp.GetScriptData(height, regid, key, value)) {
			throw runtime_error("in getscriptdata :the key not exist!\n");
		}
		script.push_back(Pair("scritpid", params[0].get_str()));
		script.push_back(Pair("key", HexStr(key)));
		script.push_back(Pair("value", HexStr(value)));
		return script;

	} else {
		int dbsize;
		contractScriptTemp.GetScriptDataCount(regid, dbsize);
		if (0 == dbsize) {
			throw runtime_error("in getscriptdata :the scirptid database not data!\n");
		}
		int pagesize = params[1].get_int();
		int index = params[2].get_int();

		vector<std::tuple<vector<unsigned char>, vector<unsigned char> > > ret;
		getDataFromSriptData(contractScriptTemp, regid, pagesize, index, ret);
		Array retArray;
		for (auto te : ret) {
			vector<unsigned char> key = std::get<0>(te);
			vector<unsigned char> valvue = std::get<1>(te);
			Object firt;
			firt.push_back(Pair("key", HexStr(key)));
			firt.push_back(Pair("value", HexStr(valvue)));
			retArray.push_back(firt);
		}
		return retArray;
	}

	return script;
}

Value getscriptvalidedata(const Array& params, bool fHelp) {
	if (fHelp || (params.size() != 3 && params.size() !=4)) {
		throw runtime_error("getscriptvalidedata \"scriptid\" \"pagesize\" \"index\"\n"
					"\nget script valide data\n"
					"\nArguments:\n"
					"1.\"scriptid\": (string, required)\n"
					"2.\"pagesize\": (int, required)\n"
					"3.\"index\": (int, required )\n"
					"4.\"minconf\":  (numeric, optional, default=1) Only include contract transactions confirmed \n"
				    "\nResult:\n"
				    "\nExamples:\n"
				    + HelpExampleCli("getscriptvalidedata", "\"123456789012\" \"1\"  \"1\"")
				    + HelpExampleRpc("getscriptvalidedata", "\"123456789012\" \"1\"  \"1\""));
	}
	std::shared_ptr<CScriptDBViewCache> pAccountViewCache;
	if(4 == params.size() && 0==params[3].get_int()) {
		pAccountViewCache.reset(new CScriptDBViewCache(*mempool.pScriptDBViewCache, true));
	}else {
		pAccountViewCache.reset(new CScriptDBViewCache(*pScriptDBTip, true));
	}
	int height = chainActive.Height();
	RPCTypeCheck(params, list_of(str_type)(int_type)(int_type));
	CRegID regid(params[0].get_str());
	if (regid.IsEmpty() == true) {
		throw runtime_error("in getscriptdata :vscriptid size is error!\n");
	}

	if (!pAccountViewCache->HaveScript(regid)) {
		throw runtime_error("in getscriptdata :vscriptid id is exist!\n");
	}
	Object obj;
	int pagesize = params[1].get_int();
	int nIndex = params[2].get_int();

	int nKey = revert(height);
	CDataStream ds(SER_NETWORK, PROTOCOL_VERSION);
	ds << nKey;
	std::vector<unsigned char> vScriptKey(ds.begin(), ds.end());
	std::vector<unsigned char> vValue;
	Array retArray;
	int nReadCount = 0;
	while (pAccountViewCache->GetScriptData(height, regid, 1, vScriptKey, vValue)) {
		Object item;
		++nReadCount;
		if (nReadCount > pagesize * (nIndex - 1)) {
			item.push_back(Pair("key", HexStr(vScriptKey)));
			item.push_back(Pair("value", HexStr(vValue)));
			retArray.push_back(item);
		}
		if(nReadCount >= pagesize * nIndex) {
			break;
		}
	}
	return retArray;
}

Value saveblocktofile(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 2) {
		throw runtime_error("saveblocktofile \"blockhash\" \"filepath\"\n"
				"\n save the given block info to the given file\n"
				"\nArguments:\n"
				"1.\"blockhash\": (string, required)\n"
				"2.\"filepath\": (string, required)\n"
				"\nResult:\n"
		        "\nExamples:\n"
				+ HelpExampleCli("saveblocktofile", "\"12345678901211111\" \"block.log\"")
				+ HelpExampleRpc("saveblocktofile", "\"12345678901211111\" \"block.log\""));
	}
	string strblockhash = params[0].get_str();
	uint256 blockHash(uint256S(params[0].get_str()));
	CBlockIndex *pIndex = mapBlockIndex[blockHash];
	CBlock blockInfo;
	if (!pIndex || !ReadBlockFromDisk(blockInfo, pIndex))
		throw runtime_error(_("Failed to read block"));
	assert(strblockhash == blockInfo.GetHash().ToString());
//	CDataStream ds(SER_NETWORK, PROTOCOL_VERSION);
//	ds << blockInfo.GetBlockHeader();
//	cout << "block header:" << HexStr(ds) << endl;
	string file = params[1].get_str();
	try {
		FILE* fp = fopen(file.c_str(), "wb+");
		CAutoFile fileout = CAutoFile(fp, SER_DISK, CLIENT_VERSION);
		if (!fileout)
			throw JSONRPCError(RPC_MISC_ERROR, "open file:" + strblockhash + "failed!");
		if(chainActive.Contains(pIndex))
			fileout << pIndex->nHeight;
		fileout << blockInfo;
		fflush(fileout);
		//fileout object auto free fp point, don't need double free fp point.
//		fclose(fp);
	} catch (std::exception &e) {
		throw JSONRPCError(RPC_MISC_ERROR, "save block to file error");
	}
	return "save succeed";
}

Value getscriptdbsize(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		throw runtime_error("getscriptdbsize \"scriptid\"\n"
							"\nget script data count\n"
							"\nArguments:\n"
							"1.\"scriptid\": (string, required)\n"
							"\nResult:\n"
							"\nExamples:\n"
							+ HelpExampleCli("getscriptdbsize", "\"123456789012\"")
							+ HelpExampleRpc("getscriptdbsize","\"123456789012\""));
	}
	CRegID regid(params[0].get_str());
	if (regid.IsEmpty() == true) {
		throw runtime_error("in getscriptdata :vscriptid is error!\n");
	}

	if (!pScriptDBTip->HaveScript(regid)) {
		throw runtime_error("in getscriptdata :vscriptid id is not exist!\n");
	}
	int nDataCount = 0;
	if (!pScriptDBTip->GetScriptDataCount(regid, nDataCount)) {
		throw runtime_error("GetScriptDataCount error!");
	}
	return nDataCount;
}

Value registaccounttxraw(const Array& params, bool fHelp) {

	if (fHelp || (params.size() < 2  || params.size() > 4)) {
		throw runtime_error("registaccounttxraw \"fee\" \"publickey\" (\"minerpublickey\") (\"height\")\n"
				"\ncreate a register account transaction\n"
				"\nArguments:\n"
				"1.fee: (numeric, required) pay to miner\n"
				"2.publickey: (string, required)\n"
				"3.minerpublickey: (string,optional)\n"
				"4.height: (numeric, optional) pay to miner\n"
				"\nResult:\n"
				"\"txhash\": (string)\n"
				"\nExamples:\n"
				+ HelpExampleCli("registaccounttxraw",  "10000 \"038f679e8b63d6f9935e8ca6b7ce1de5257373ac5461874fc794004a8a00a370ae\" \"026bc0668c767ab38a937cb33151bcf76eeb4034bcb75e1632fd1249d1d0b32aa9\" 10 ")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("registaccounttxraw", " 10000 \"038f679e8b63d6f9935e8ca6b7ce1de5257373ac5461874fc794004a8a00a370ae\" \"026bc0668c767ab38a937cb33151bcf76eeb4034bcb75e1632fd1249d1d0b32aa9\" 10"));
	}
	CUserID ukey;
	CUserID uminerkey = CNullID();

	int64_t Fee = AmountToRawValue(params[0]);

	CKeyID dummy;
	CPubKey pubk = CPubKey(ParseHex(params[1].get_str()));
	if (!pubk.IsCompressed() || !pubk.IsFullyValid()) {
		throw JSONRPCError(RPC_INVALID_PARAMS, "CPubKey err");
	}
	ukey = pubk;
	dummy = pubk.GetKeyID();

	if (params.size() > 2) {
		CPubKey pubk = CPubKey(ParseHex(params[2].get_str()));
		if (!pubk.IsCompressed() || !pubk.IsFullyValid()) {
			throw JSONRPCError(RPC_INVALID_PARAMS, "CPubKey err");
		}
		uminerkey = pubk;
	}

	int hight = chainActive.Tip()->nHeight;
	if (params.size() > 3) {
		hight = params[3].get_int();
	}

	std::shared_ptr<CRegisterAccountTx> tx = make_shared<CRegisterAccountTx>(ukey, uminerkey, Fee, hight);
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
	ds << pBaseTx;
	Object obj;
	obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
	return obj;

}

Value submittx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		throw runtime_error("submittx \"transaction\" \n"
				"\nsubmit transaction\n"
				"\nArguments:\n"
				"1.\"transaction\": (string, required)\n"
				"\nExamples:\n"
				+ HelpExampleCli("submittx", "\"n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj\"")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("submittx", "\"n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj\""));
	}
	EnsureWalletIsUnlocked();
	vector<unsigned char> vch(ParseHex(params[0].get_str()));
	CDataStream stream(vch, SER_DISK, CLIENT_VERSION);

	std::shared_ptr<CBaseTransaction> tx;
	stream >> tx;
	std::tuple<bool, string> ret;
	ret = pwalletMain->CommitTransaction((CBaseTransaction *) tx.get());
	if (!std::get<0>(ret)) {
		throw JSONRPCError(RPC_WALLET_ERROR, "submittx Error:" + std::get<1>(ret));
	}
	Object obj;
	obj.push_back(Pair("hash", std::get<1>(ret)));
	return obj;
}

Value createcontracttxraw(const Array& params, bool fHelp) {
	if (fHelp || params.size() < 5 || params.size() > 6) {
		throw runtime_error("createcontracttxraw \"height\" \"fee\" \"amount\" \"addr\" \"contract\" \n"
				"\ncreate contract\n"
				"\nArguments:\n"
				"1.\"fee\": (numeric, required) pay to miner\n"
				"2.\"amount\": (numeric, required)\n"
				"3.\"addr\": (string, required)\n"
				"4.\"appid\": (string required)"
				"5.\"contract\": (string, required)\n"
				"6.\"height\": (int, optional)create height\n"
				"\nResult:\n"
				"\"contract tx str\": (string)\n"
				"\nExamples:\n"
				+ HelpExampleCli("createcontracttxraw",
						"1000 01020304 000000000100 [\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"] "
								"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\","
								"\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\"] 10") + "\nAs json rpc call\n"
				+ HelpExampleRpc("createcontracttxraw",
						"1000 01020304 000000000100 000000000100 [\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"] "
								"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\","
								"\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\"] 10"));
	}

	RPCTypeCheck(params, list_of(int_type)(real_type)(real_type)(str_type)(str_type)(str_type));


	uint64_t fee = AmountToRawValue(params[0]);
	uint64_t amount = AmountToRawValue(params[1]);
	CRegID userid(params[2].get_str());
	CRegID appid(params[3].get_str());

	vector<unsigned char> vcontract = ParseHex(params[4].get_str());

	if (fee > 0 && fee < CTransaction::nMinTxFee) {
		throw runtime_error("in createcontracttxraw :fee is smaller than nMinTxFee\n");
	}

	if (appid.IsEmpty()) {
		throw runtime_error("in createcontracttxraw :addresss is error!\n");
	}

	CAccountViewCache view(*pAccountViewTip, true);
	CAccount secureAcc;

	if (!pScriptDBTip->HaveScript(appid)) {
		throw runtime_error(tinyformat::format("createcontracttx :app id %s is not exist\n", appid.ToString()));
	}

	CKeyID keyid;
	if (!view.GetKeyId(userid, keyid)) {
		CID id(userid);
		LogPrint("INFO", "vaccountid:%s have no key id\r\n", HexStr(id.GetID()).c_str());
//		assert(0);
		throw runtime_error(tinyformat::format("createcontracttx :vaccountid:%s have no key id\r\n", HexStr(id.GetID()).c_str()));
	}

	int height = chainActive.Tip()->nHeight;
	if (params.size() > 5) {
		height = params[5].get_int();
	}

	std::shared_ptr<CTransaction> tx = make_shared<CTransaction>(userid, appid, fee, amount, height, vcontract);

	CDataStream ds(SER_DISK, CLIENT_VERSION);
	std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
	ds << pBaseTx;
	Object obj;
	obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
	return obj;
}

Value registerscripttxraw(const Array& params, bool fHelp) {
	if (fHelp || params.size() < 4) {
		throw runtime_error("registerscripttxraw \"height\" \"fee\" \"addr\" \"flag\" \"script or scriptid\" (\"script description\")\n"
						"\nregister script\n"
						"\nArguments:\n"
						"1.\"fee\": (numeric required) pay to miner\n"
						"2.\"addr\": (string required)\nfor send"
						"3.\"flag\": (bool, required) 0-1\n"
						"4.\"script or scriptid\": (string required), if flag=0 is script's file path, else if flag=1 scriptid\n"
						"5.\"height\": (int required)valid height\n"
						"6.\"script description\":(string optional) new script description\n"
						"\nResult:\n"
						"\"txhash\": (string)\n"
						"\nExamples:\n"
						+ HelpExampleCli("registerscripttxraw",
								"\"10\" \"10000\" \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\" \"1\" \"010203040506\" ")
						+ "\nAs json rpc call\n"
						+ HelpExampleRpc("registerscripttxraw",
								"\"10\" \"10000\" \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\" \"1\" \"010203040506\" "));
	}

	RPCTypeCheck(params, list_of(real_type)(str_type)(int_type)(str_type)(int_type)(str_type));

	uint64_t fee = AmountToRawValue(params[0]);

	CVmScript vmScript;
	vector<unsigned char> vscript;
	int flag = params[2].get_bool();
	if (0 == flag) {
		string path = params[3].get_str();
		FILE* file = fopen(path.c_str(), "rb+");
		if (!file) {
			throw runtime_error("create registerapptx open script file" + path + "error");
		}
		long lSize;
		fseek(file, 0, SEEK_END);
		lSize = ftell(file);
		rewind(file);

		// allocate memory to contain the whole file:
		char *buffer = (char*) malloc(sizeof(char) * lSize);
		if (buffer == NULL) {
			fclose(file);//及时关闭
			throw runtime_error("allocate memory failed");
		}

		if (fread(buffer, 1, lSize, file) != (size_t) lSize) {
			free(buffer);
			fclose(file);//及时关闭
			throw runtime_error("read script file error");
		}
		else
		{
			fclose(file);
		}
		vmScript.Rom.insert(vmScript.Rom.end(), buffer, buffer + lSize);
		if (buffer)
			free(buffer);
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		ds << vmScript;

		vscript.assign(ds.begin(), ds.end());


	} else if (1 == flag) {
		vscript = ParseHex(params[3].get_str());
	}

	if (params.size() > 5) {
		string scriptDesc = params[5].get_str();
		vmScript.ScriptExplain.insert(vmScript.ScriptExplain.end(), scriptDesc.begin(), scriptDesc.end());
	}

	if (fee > 0 && fee < CTransaction::nMinTxFee) {
		throw runtime_error("in registerapptx :fee is smaller than nMinTxFee\n");
	}
	//get keyid
	CKeyID keyid;
	if (!GetKeyId(params[2].get_str(), keyid)) {
		throw runtime_error("in registerapptx :send address err\n");
	}

	//balance
	CAccountViewCache view(*pAccountViewTip, true);
	CAccount account;

	CUserID userId = keyid;
	if (!view.GetAccount(userId, account)) {
		throw JSONRPCError(RPC_WALLET_ERROR, "in registerscripttxraw Error: Account is not exist.");
	}

	if (!account.IsRegister()) {
		throw JSONRPCError(RPC_WALLET_ERROR, "in registerscripttxraw Error: Account is not registered.");
	}

	if (flag) {
		vector<unsigned char> vscriptcontent;
		CRegID regid(params[2].get_str());
		if (!pScriptDBTip->GetScript(CRegID(vscript), vscriptcontent)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "script id find failed");
		}
	}
	auto GetUserId = [&](CKeyID &mkeyId)
	{
		CAccount acct;
		if (view.GetAccount(CUserID(mkeyId), acct)) {
			return acct.regID;
		}
		throw runtime_error(
				tinyformat::format("registerscripttxraw :account id %s is not exist\n", mkeyId.ToAddress()));
	};
	std::shared_ptr<CRegisterAppTx> tx = make_shared<CRegisterAppTx>();
	tx.get()->regAcctId = GetUserId(keyid);
	tx.get()->script = vscript;
	tx.get()->llFees = fee;

	uint32_t height = chainActive.Tip()->nHeight;
	if (params.size() > 4) {
		height =  params[4].get_int();
	}
	tx.get()->nValidHeight = height;

	CDataStream ds(SER_DISK, CLIENT_VERSION);
	std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
	ds << pBaseTx;
	Object obj;
	obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
	return obj;

}

Value sigstr(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 2) {
		throw runtime_error("sigstr \"str\" \"addr\"\n"
				"\nsignature transaction\n"
				"\nArguments:\n"
				"1.\"str\": (string, required) sig str\n"
				"2.\"addr\": (string, required)\n"
				"\nExamples:\n"
				+ HelpExampleCli("sigstr", "\"1010000010203040506\" \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\" ")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("sigstr", "\"1010000010203040506\" \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG 010203040506\" "));
	}
	vector<unsigned char> vch(ParseHex(params[0].get_str()));
	string addr = params[1].get_str();
	CKeyID keyid;
	if (!GetKeyId(params[1].get_str(), keyid)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "to address Invalid  ");
	}
	CDataStream stream(vch, SER_DISK, CLIENT_VERSION);
	std::shared_ptr<CBaseTransaction> pBaseTx;
	stream >> pBaseTx;
	CAccountViewCache view(*pAccountViewTip, true);
	Object obj;
	switch (pBaseTx.get()->nTxType) {
	case COMMON_TX: {
		std::shared_ptr<CTransaction> tx = make_shared<CTransaction>(pBaseTx.get());
		CKeyID keyid;
		if (!view.GetKeyId(tx.get()->srcRegId, keyid)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "vaccountid have no key id");
		}
		if (!pwalletMain->Sign(keyid, tx.get()->SignatureHash(), tx.get()->signature)) {
			throw JSONRPCError(RPC_INVALID_PARAMETER, "Sign failed");
		}
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
		ds << pBaseTx;
		obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
	}
		break;
	case REG_ACCT_TX: {
		std::shared_ptr<CRegisterAccountTx> tx = make_shared<CRegisterAccountTx>(pBaseTx.get());
		if (!pwalletMain->Sign(keyid, tx.get()->SignatureHash(), tx.get()->signature)) {
			throw JSONRPCError(RPC_INVALID_PARAMETER, "Sign failed");
		}
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
		ds << pBaseTx;
		obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
	}
		break;
	case CONTRACT_TX: {
		std::shared_ptr<CTransaction> tx = make_shared<CTransaction>(pBaseTx.get());
		if (!pwalletMain->Sign(keyid, tx.get()->SignatureHash(), tx.get()->signature)) {
			throw JSONRPCError(RPC_INVALID_PARAMETER, "Sign failed");
		}
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
		ds << pBaseTx;
		obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
	}
		break;
	case REWARD_TX:
		break;
	case REG_APP_TX: {
		std::shared_ptr<CRegisterAppTx> tx = make_shared<CRegisterAppTx>(pBaseTx.get());
		if (!pwalletMain->Sign(keyid, tx.get()->SignatureHash(), tx.get()->signature)) {
			throw JSONRPCError(RPC_INVALID_PARAMETER, "Sign failed");
		}
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
		ds << pBaseTx;
		obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
	}
		break;
	default:
//		assert(0);
		break;
	}
	return obj;
}

Value getalltxinfo(const Array& params, bool fHelp) {
	if (fHelp || (params.size() != 0 && params.size() != 1)) {
		throw runtime_error("getalltxinfo \n"
				"\nget all transaction info\n"
				"\nArguments:\n"
				"1.\"nlimitCount\": (numeric, optional, default=0) 0 return all tx, else return number of nlimitCount txs \n"
				"\nResult:\n"
				"\nExamples:\n" + HelpExampleCli("getalltxinfo", "") + "\nAs json rpc call\n"
				+ HelpExampleRpc("getalltxinfo", ""));
	}

	Object retObj;
	int nLimitCount(0);
	if(params.size() == 1)
		nLimitCount = params[0].get_int();
	assert(pwalletMain != NULL);
	if(nLimitCount <=0 ) {
		Array ComfirmTx;
		for (auto const &wtx : pwalletMain->mapInBlockTx) {
			for (auto const & item : wtx.second.mapAccountTx) {
				Object objtx = GetTxDetailJSON(item.first);
				ComfirmTx.push_back(objtx);
			}
		}
		retObj.push_back(Pair("Confirmed", ComfirmTx));

		Array UnComfirmTx;
		CAccountViewCache view(*pAccountViewTip, true);
		for (auto const &wtx : pwalletMain->UnConfirmTx) {
			Object objtx = GetTxDetailJSON(wtx.first);
			UnComfirmTx.push_back(objtx);
		}
		retObj.push_back(Pair("UnConfirmed", UnComfirmTx));
	}else {
		Array ComfirmTx;
		multimap<int, Object, std::greater<int> > mapTx;
		for (auto const &wtx : pwalletMain->mapInBlockTx) {
			for (auto const & item : wtx.second.mapAccountTx) {
				Object objtx = GetTxDetailJSON(item.first);
				int nConfHeight = find_value(objtx, "confirmHeight").get_int();
				mapTx.insert(pair<int, Object>(nConfHeight, objtx));
			}
		}
		int nSize(0);
		for(auto & txItem : mapTx) {
			if(++nSize > nLimitCount)
				break;
			ComfirmTx.push_back(txItem.second);
		}
		retObj.push_back(Pair("Confirmed", ComfirmTx));
	}

	return retObj;
}

Value printblokdbinfo(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0) {
		throw runtime_error("printblokdbinfo \n"
				"\nprint block log\n"
				"\nArguments:\n"
				"\nExamples:\n" + HelpExampleCli("printblokdbinfo", "")
				+ HelpExampleRpc("printblokdbinfo", ""));
	}

	if (!pAccountViewTip->Flush())
		throw runtime_error("Failed to write to account database\n");
	if (!pScriptDBTip->Flush())
		throw runtime_error("Failed to write to account database\n");
	WriteBlockLog(false, "");
	return Value::null;
}

Value getappaccinfo(const Array& params, bool fHelp) {
	if (fHelp || (params.size() != 2 && params.size() != 3)) {
		throw runtime_error("getappaccinfo  \"scriptid\" \"address\""
				"\nget appaccount info\n"
				"\nArguments:\n"
				"1.\"scriptid\":(string, required) \n"
				"2.\"address\": (string, required) \n"
				"3.\"minconf\"  (numeric, optional, default=1) Only include contract transactions confirmed \n"
				"\nExamples:\n"
				+ HelpExampleCli("getappaccinfo", "\"000000100000\" \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("getappaccinfo", "\"000000100000\" \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\""));
	}


	CRegID script(params[0].get_str());
	vector<unsigned char> key;

	if (CRegID::IsSimpleRegIdStr(params[1].get_str())) {
		CRegID reg(params[1].get_str());
		key.insert(key.begin(), reg.GetVec6().begin(), reg.GetVec6().end());
	} else {
		string addr = params[1].get_str();
		key.assign(addr.c_str(), addr.c_str() + addr.length());
	}
	std::shared_ptr<CAppUserAccout> tem = make_shared<CAppUserAccout>();
	if(params.size() == 3 && 0 == params[2].get_int())
	{

		CScriptDBViewCache contractScriptTemp(*mempool.pScriptDBViewCache, true);
		if (!contractScriptTemp.GetScriptAcc(script, key, *tem.get())) {
			tem = make_shared<CAppUserAccout>(key);
		}
	}else {
		CScriptDBViewCache contractScriptTemp(*pScriptDBTip, true);
		if (!contractScriptTemp.GetScriptAcc(script, key, *tem.get())) {
			tem = make_shared<CAppUserAccout>(key);
		}
	}

	tem.get()->AutoMergeFreezeToFree(chainActive.Tip()->nHeight);
	return Value(tem.get()->toJSON());
}

Value gethash(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		throw runtime_error("gethash  \"str\"\n"
					"\nget the hash of given str\n"
					"\nArguments:\n"
					"1.\"str\": (string, required) \n"
				    "\nresult an object \n"
					"\nExamples:\n"
					+ HelpExampleCli("gethash", "\"0000001000005zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"")
					+ "\nAs json rpc call\n"
					+ HelpExampleRpc("gethash", "\"0000001000005zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\""));
	}

	string str = params[0].get_str();
	vector<unsigned char> vTemp;
	vTemp.assign(str.c_str(), str.c_str() + str.length());
	uint256 strhash = Hash(vTemp.begin(), vTemp.end());
	Object obj;
	obj.push_back(Pair("hash", strhash.ToString()));
	return obj;

}

Value getappkeyvalue(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 2) {
		throw runtime_error("getappkeyvalue  \"scriptid\" \"array\""
						"\nget application key value\n"
						"\nArguments:\n"
						"1.\"scriptid\": (string, required) \n"
						"2.\"array\": (string, required) \n"
						"\nExamples:\n"
						+ HelpExampleCli("getappkeyvalue", "\"000000100000\" \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"")
						+ "\nAs json rpc call\n"
						+ HelpExampleRpc("getappkeyvalue", "\"000000100000\" \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\""));
	}

	CRegID scriptid(params[0].get_str());
	Array array = params[1].get_array();

	int height = chainActive.Height();

	if (scriptid.IsEmpty() == true) {
		throw runtime_error("in getappkeyvalue :vscriptid size is error!\n");
	}

	if (!pScriptDBTip->HaveScript(scriptid)) {
		throw runtime_error("in getappkeyvalue :vscriptid id is exist!\n");
	}

	Array retArry;
	CScriptDBViewCache contractScriptTemp(*pScriptDBTip, true);

	for (size_t i = 0; i < array.size(); i++) {
		uint256 txhash(uint256S(array[i].get_str()));
		vector<unsigned char> key;	// = ParseHex(array[i].get_str());
		key.insert(key.begin(), txhash.begin(), txhash.end());
		vector<unsigned char> value;
		Object obj;
		if (!contractScriptTemp.GetScriptData(height, scriptid, key, value)) {
			obj.push_back(Pair("key", array[i].get_str()));
			obj.push_back(Pair("value", HexStr(value)));
		} else {
			obj.push_back(Pair("key", array[i].get_str()));
			obj.push_back(Pair("value", HexStr(value)));
		}

		std::shared_ptr<CBaseTransaction> pBaseTx;
		int time = 0;
		int height = 0;
		if (SysCfg().IsTxIndex()) {
			CDiskTxPos postx;
			if (pScriptDBTip->ReadTxIndex(txhash, postx)) {
				CAutoFile file(OpenBlockFile(postx, true), SER_DISK, CLIENT_VERSION);
				CBlockHeader header;
				try {
					file >> header;
					fseek(file, postx.nTxOffset, SEEK_CUR);
					file >> pBaseTx;
					height = header.nHeight;
					time = header.nTime;
				} catch (std::exception &e) {
					throw runtime_error(tfm::format("%s : Deserialize or I/O error - %s", __func__, e.what()).c_str());
				}
			}
		}

		obj.push_back(Pair("confirmHeight", (int) height));
		obj.push_back(Pair("confirmedtime", (int) time));
		retArry.push_back(obj);
	}

	return retArry;
}

Value gencheckpoint(const Array& params, bool fHelp)
{
	if(fHelp || params.size() != 2)
	{
		throw runtime_error(
				 "gencheckpoint \"privatekey\" \"filepath\"\n"
				 "\ngenerate checkpoint by Private key signature block.\n"
				 "\nArguments:\n"
				 "1. \"privatekey\"  (string, required) the private key\n"
				 "2. \"filepath\"  (string, required) check point block path\n"
				"\nResult:\n"
				"\nExamples:\n"
				 + HelpExampleCli("gencheckpoint", "\"privatekey\" \"filepath\"")
                 + HelpExampleRpc("gencheckpoint", "\"privatekey\" \"filepath\""));
	}
	std::string strSecret = params[0].get_str();
	CDacrsSecret vchSecret;
	bool fGood = vchSecret.SetString(strSecret);

	if (!fGood) throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key encoding");

	CKey key = vchSecret.GetKey();
	if (!key.IsValid()) throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Private key outside allowed range");

	string file = params[1].get_str();
	int nHeight(0);
	CBlock block;
	try {
		FILE* fp = fopen(file.c_str(), "rb+");
		CAutoFile fileout = CAutoFile(fp, SER_DISK, CLIENT_VERSION);
		if (!fileout)
			throw JSONRPCError(RPC_MISC_ERROR, "open file:" + file + "failed!");
		fileout >> nHeight;
		fileout >> block;
	} catch (std::exception &e) {

		throw JSONRPCError(RPC_MISC_ERROR, strprintf("read block to file error:%s", e.what()).c_str());
	}

	SyncData::CSyncData data;
	SyncData::CSyncCheckPoint point;
	CDataStream sstream(SER_NETWORK, PROTOCOL_VERSION);
	point.m_height = nHeight;
	point.m_hashCheckpoint = block.GetHash();//chainActive[intTemp]->GetBlockHash();
	LogPrint("CHECKPOINT","send hash = %s\n",block.GetHash().ToString());
	sstream << point;
	Object obj;
	if (data.Sign(key, std::vector<unsigned char>(sstream.begin(), sstream.end()))
		&& data.CheckSignature(SysCfg().GetPublicKey()))
	{
		obj.push_back(Pair("chenkpoint", data.ToJsonObj()));
		return obj;
	}
	return obj;
}

Value setcheckpoint(const Array& params, bool fHelp)
{
	if(fHelp || params.size() != 1)
	{
		throw runtime_error(
				 "setcheckpoint \"filepath\"\n"
				 "\nadd new checkpoint and send it out.\n"
				 "\nArguments:\n"
				 "1. \"filepath\"  (string, required) check point block path\n"
				 "\nResult:\n"
				 "\nExamples:\n"
		         + HelpExampleCli("setcheckpoint", "\"filepath\"")
		         + HelpExampleRpc("setcheckpoint", "\"filepath\""));
	}
	SyncData::CSyncData data;
	ifstream file;
	file.open(params[0].get_str().c_str(), ios::in | ios::ate);
	if (!file.is_open())
	      throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open check point dump file");
	file.seekg(0, file.beg);
    if (file.good()){
    	Value reply;
    	json_spirit::read(file,reply);
    	const Value & checkpoint = find_value(reply.get_obj(),"chenkpoint");
    	if(checkpoint.type() ==  json_spirit::null_type) {
    		throw JSONRPCError(RPC_INVALID_PARAMETER, "read check point failed");
    	}
    	const Value & msg = find_value(checkpoint.get_obj(), "msg");
    	const Value & sig = find_value(checkpoint.get_obj(), "sig");
    	if(msg.type() == json_spirit::null_type || sig.type() == json_spirit::null_type) {
    		throw JSONRPCError(RPC_INVALID_PARAMETER, "read msg or sig failed");
    	}
    	data.m_vchMsg = ParseHex(msg.get_str());
    	data.m_vchSig = ParseHex(sig.get_str());
    }
    file.close();
    if(!data.CheckSignature(SysCfg().GetPublicKey())) {
    	throw JSONRPCError(RPC_INVALID_PARAMETER, "check signature failed");
    }
	SyncData::CSyncDataDb db;
	std::vector<SyncData::CSyncData> vdata;
	SyncData::CSyncCheckPoint point;
	CDataStream sstream(data.m_vchMsg, SER_NETWORK, PROTOCOL_VERSION);
	sstream >> point;
	db.WriteCheckpoint(point.m_height, data);
	Checkpoints::AddCheckpoint(point.m_height, point.m_hashCheckpoint);
	CheckActiveChain(point.m_height, point.m_hashCheckpoint);
	vdata.push_back(data);
	LOCK(cs_vNodes);
	BOOST_FOREACH(CNode* pnode, vNodes)
	{
		if (pnode->setcheckPointKnown.count(point.m_height) == 0)
		{
			pnode->setcheckPointKnown.insert(point.m_height);
			pnode->PushMessage("checkpoint", vdata);
		}
	}
	return tfm::format("sendcheckpoint :%d\n", point.m_height);
}

Value validateaddress(const Array& params, bool fHelp)
{
	if(fHelp || params.size() != 1)
		{
			throw runtime_error(
					 "isvalideaddess \"dacrs address\"\n"
					 "\ncheck address is valide\n"
					 "\nArguments:\n"
					 "1. \"dacrs address\"  (string, required) dacrs address\n"
					 "\nResult:\n"
					 "\nExamples:\n"
			         + HelpExampleCli("isvalideaddress", "\"De5nZAbhMikMPGHzxvSGqHTgEuf3eNUiZ7\"")
			         + HelpExampleRpc("isvalideaddress", "\"De5nZAbhMikMPGHzxvSGqHTgEuf3eNUiZ7\""));
		}
	{
		Object obj;
		CKeyID keyid;
		string addr = params[0].get_str();
		if (!GetKeyId(addr, keyid)) {
			obj.push_back(Pair("ret" , false));
		}else {
			obj.push_back(Pair("ret" , true));
		}
		return obj;
	}
}

Value gettotalcoin(const Array& params, bool fHelp) {
	if(fHelp || params.size() != 0)
	{
		throw runtime_error(
				 "gettotalcoin \n"
				 "\nget all coin amount\n"
				 "\nArguments:\n"
				 "\nResult:\n"
				 "\nExamples:\n"
				 + HelpExampleCli("gettotalcoin", "")
				 + HelpExampleRpc("gettotalcoin", ""));
	}
		Object obj;
		{
			CAccountViewCache view(*pAccountViewTip, true);
			uint64_t totalcoin = view.TraverseAccount();
			obj.push_back(Pair("TotalCoin", ValueFromAmount(totalcoin)));
		}
		return obj;
}
