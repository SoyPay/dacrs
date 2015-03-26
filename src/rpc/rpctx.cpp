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
#include "../wallet/wallet.h"
#include "../wallet/walletdb.h"

#include "miner.h"
#include "VmScript/VmScript.h"
#include <stdint.h>

#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"

#include "boost/tuple/tuple.hpp"

using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;

string RegIDToAddress(CUserID &userId) {
	CKeyID keid;
	if(pAccountViewTip->GetKeyId(userId,keid))
		{
		 return keid.ToAddress();
		}
	return "can not get address";
}

static  bool GetKeyId(string const &addr,CKeyID &KeyId) {
	if (!CRegID::GetKeyID(addr, KeyId)) {
		KeyId=CKeyID(addr);
		if (KeyId.IsEmpty())
		return false;
	}
	return true;
};
Object TxToJSON(CBaseTransaction *pTx,bool bPrintScriptContent = true) {
	Object result;
	result.push_back(Pair("hash", pTx->GetHash().GetHex()));
	switch (pTx->nTxType) {
	case REG_ACCT_TX: {
		CRegisterAccountTx *prtx = (CRegisterAccountTx *) pTx;
		result.push_back(Pair("txtype", "RegisterAccTx"));
		result.push_back(Pair("ver", prtx->nVersion));
		result.push_back(Pair("addr", boost::get<CPubKey>(prtx->userId).GetKeyID().ToAddress()));
		CID id(prtx->userId);
		CID minerId(prtx->minerId);
		result.push_back(Pair("pubkey", HexStr(id.GetID())));
		result.push_back(Pair("miner_pubkey", HexStr(minerId.GetID())));
		result.push_back(Pair("fees", prtx->llFees));
		result.push_back(Pair("height", prtx->nValidHeight));
		break;
	}
	case COMMON_TX: {
		CTransaction *prtx = (CTransaction *) pTx;
		result.push_back(Pair("txtype", "NormalTx"));
		result.push_back(Pair("ver", prtx->nVersion));
		result.push_back(Pair("srcaddr", RegIDToAddress(prtx->srcUserId)));
		result.push_back(Pair("desaddr", RegIDToAddress(prtx->desUserId)));
		result.push_back(Pair("money", prtx->llValues));
		result.push_back(Pair("fees", prtx->llFees));
		result.push_back(Pair("height", prtx->nValidHeight));
		break;
	}
	case CONTRACT_TX: {
		CContractTransaction *prtx = (CContractTransaction *) pTx;
		result.push_back(Pair("txtype", "ContractTx"));
		result.push_back(Pair("ver", prtx->nVersion));
		CRegID tep = boost::get<CRegID>(prtx->scriptRegId);
		result.push_back(Pair("appid", tep.ToString()));
		{
			Array array;
			for(auto& item : prtx->vAccountRegId)
			{
				array.push_back(RegIDToAddress(item));
			}
			result.push_back(Pair("accountid", array));
		}
		result.push_back(Pair("fees", prtx->llFees));
		result.push_back(Pair("Contract", HexStr(prtx->vContract)));
		result.push_back(Pair("height", prtx->nValidHeight));
		{
			Array array;
			for(auto& item : prtx->vSignature)
			{
				array.push_back(HexStr(item));
			}
			result.push_back(Pair("Signature", array));
		}
		break;
	}
	case REWARD_TX: {
		CRewardTransaction *prtx = (CRewardTransaction *) pTx;
		result.push_back(Pair("txtype", "RewardTx"));
		result.push_back(Pair("ver", prtx->nVersion));
		result.push_back(Pair("addr", RegIDToAddress(prtx->account)));
		result.push_back(Pair("reward money", prtx->rewardValue));
		result.push_back(Pair("height", prtx->nHeight));
		break;
	}
	case REG_SCRIPT_TX: {
		CRegisterScriptTx *prtx = (CRegisterScriptTx *) pTx;
		result.push_back(Pair("txtype", "RegScriptTx"));
		result.push_back(Pair("ver", prtx->nVersion));
		result.push_back(Pair("addr", RegIDToAddress(prtx->regAccountId)));
		if (!bPrintScriptContent && prtx->script.size() != SCRIPT_ID_SIZE) {
			result.push_back(Pair("script", "script_content"));
		} else {
			result.push_back(Pair("appid", CRegID(prtx->script).ToString()));
		}


		result.push_back(Pair("fees", prtx->llFees));
		result.push_back(Pair("height", prtx->nValidHeight));
		break;
	}
	default:
		assert(0);
		break;
	}
	return result;
}

Object GetTxDetail(const uint256& txhash,bool bPrintScriptContent = true ) {
	Object obj;
	std::shared_ptr<CBaseTransaction> pBaseTx;
	{
		LOCK(cs_main);
		{
			pBaseTx = mempool.lookup(txhash);
			if (pBaseTx.get()) {
				obj = TxToJSON(pBaseTx.get(),bPrintScriptContent);
				return obj;
			}
		}
		if (SysCfg().IsTxIndex()) {
			CDiskTxPos postx;
			if (pblocktree->ReadTxIndex(txhash, postx)) {
				CAutoFile file(OpenBlockFile(postx, true), SER_DISK, CLIENT_VERSION);
				CBlockHeader header;
				try {
					file >> header;
					fseek(file, postx.nTxOffset, SEEK_CUR);
					file >> pBaseTx;
					obj = TxToJSON(pBaseTx.get(),bPrintScriptContent);
					obj.push_back(Pair("blockhash", header.GetHash().GetHex()));
					obj.push_back(Pair("confirmHeight", (int) header.nHeight));
				} catch (std::exception &e) {
					throw runtime_error(tfm::format("%s : Deserialize or I/O error - %s", __func__, e.what()).c_str());
				}
				return obj;
			}
		}
	}
	return obj;
}

Value gettxdetail(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		string msg =
				"gettxdetail \"txhash\"\ngettxdetail\n\nArguments:\n1.\"txhash\":\nResult:\n\"txhash\"\n\nExamples:\n"
						+ HelpExampleCli("gettxdetail",
								"c5287324b89793fdf7fa97b6203dfd814b8358cfa31114078ea5981916d7a8ac\n")
						+ "\nAs json rpc call\n"
						+ HelpExampleRpc("gettxdetail",
								"c5287324b89793fdf7fa97b6203dfd814b8358cfa31114078ea5981916d7a8ac\n");
		throw runtime_error(msg);
	}
	uint256 txhash(params[0].get_str());
//	Object obj;
//	std::shared_ptr<CBaseTransaction> pBaseTx;
//	{
//		LOCK(cs_main);
//		{
//			pBaseTx = mempool.lookup(txhash);
//			if (pBaseTx.get()) {
//				obj = TxToJSON(pBaseTx.get());
//				return obj;
//			}
//		}
//		if (SysCfg().IsTxIndex()) {
//			CDiskTxPos postx;
//			if (pblocktree->ReadTxIndex(txhash, postx)) {
//				CAutoFile file(OpenBlockFile(postx, true), SER_DISK, CLIENT_VERSION);
//				CBlockHeader header;
//				try {
//					file >> header;
//					fseek(file, postx.nTxOffset, SEEK_CUR);
//					file >> pBaseTx;
//					obj = TxToJSON(pBaseTx.get());
//					obj.push_back(Pair("blockhash", header.GetHash().GetHex()));
//					obj.push_back(Pair("confirmHeight", (int)header.nHeight));
//				} catch (std::exception &e) {
//					throw runtime_error(tfm::format("%s : Deserialize or I/O error - %s", __func__, e.what()).c_str());
//				}
//				return obj;
//			}
//		}
//	}
//	return obj;
	return GetTxDetail(txhash);
}


//create a register account tx
Value registaccounttx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 3) {
		string msg = "registaccounttx nrequired \"addr\" fee height\n"
				"\nregister secure account\n"
				"\nArguments:\n"
				"1.\"addr\": (string)\n"
				"2.fee: (numeric) pay to miner\n"
				"3.IsMinerID: (bool)create height\n"
				"\nResult:\n"
				"\"txhash\": (string)\n"
				"\nExamples:\n" + HelpExampleCli("registaccounttx", "n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj 100000 true")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("registaccounttx", "n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj 100000 false");
		throw runtime_error(msg);
	}



	string addr = params[0].get_str();
	uint64_t fee = params[1].get_uint64();
	bool IsMiner = params[2].get_bool();

	//get keyid
	CKeyID keyid;
   	if(!GetKeyId(addr,keyid))
	{
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

//		uint64_t balance;
		CUserID userId = keyid;
		if (!view.GetAccount(userId, account)) {
				throw JSONRPCError(RPC_WALLET_ERROR, "in registaccounttx Error: Account balance is insufficient.");
		}

		if (account.IsRegister()) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registaccounttx Error: Account is already registered");
		}
		uint64_t balance = account.GetRawBalance(chainActive.Tip()->nHeight);
		if (balance < fee) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registaccounttx Error: Account balance is insufficient.");
		}

		//pubkey
		CPubKey pubkey;
		if (!pwalletMain->GetPubKey(keyid, pubkey)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registaccounttx Error: not find key.");
		}

		//pubkey
		CPubKey MinerPKey;
		if (IsMiner == true) {
			if (!pwalletMain->GetPubKey(keyid, MinerPKey, true)) {
				throw JSONRPCError(RPC_WALLET_ERROR, " not find Miner key.");
			}
			rtx.minerId= MinerPKey;
		}
		else {
			CNullID nullId;
			rtx.minerId= nullId;
		}
		rtx.userId = pubkey;
		rtx.llFees = fee;
		rtx.nValidHeight = chainActive.Tip()->nHeight;

		//sign
//		CKey key;
//		pwalletMain->GetKey(keyid, key);
		if (!pwalletMain->Sign(keyid,rtx.SignatureHash(), rtx.signature)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registaccounttx Error: Sign failed.");
		}




//		if (!pwalletMain->CommitTransaction((CBaseTransaction *) &rtx)) {
//			throw JSONRPCError(RPC_WALLET_ERROR, "registaccounttx Error: CommitTransaction failed.");
//		}
	}


		std::tuple<bool,string> ret;
		ret = pwalletMain->CommitTransaction((CBaseTransaction *) &rtx);
		if(!std::get<0>(ret))
		{
			throw JSONRPCError(RPC_WALLET_ERROR, "registaccounttx Error:"+ std::get<1>(ret));
		}
		Object obj;
		obj.push_back(Pair("hash", std::get<1>(ret)));
		return obj;

}


//create a contract tx
Value createcontracttx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 5) {
		string msg = "createsecuretx nrequired \"scriptid\" [\"addr\",...] \"fee\" \"contract\" \"height\"\n"
				"\ncreate contract\n"
				"\nArguments:\n"
				"1.\"scriptid\": (string)\n"
				"2.[\"addr\",...]: (string list)\n"
				"3.\"contract\": (string)\n"
				"4.\"fee\": (numeric) pay to miner\n"
				"5.\"height\": (numeric)create height\n"
				"\nResult:\n"
				"\"contract tx str\": (string)\n"
				"\nExamples:\n"
				+ HelpExampleCli("createcontracttx", "000000000100 [\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"] "
						"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\","
						"\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\"] "
						"100000 "
						"01020304 "
						"1") + "\nAs json rpc call\n"
				+ HelpExampleRpc("createcontracttx", "000000000100 [\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"] "
						"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\","
						"\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\"] "
						"100000 "
						"01020304 "
						"1");
		throw runtime_error(msg);
	}


	RPCTypeCheck(params, list_of(str_type)(array_type)(str_type)(int_type)(int_type));

	//get addresss
//	vector<unsigned char> vscriptid = ParseHex(params[0].get_str());
	CRegID vscriptid(params[0].get_str()) ;
	Array addr = params[1].get_array();
	vector<unsigned char> vcontract = ParseHex(params[2].get_str());
	uint64_t fee = params[3].get_uint64();
	uint32_t height = params[4].get_int();

	if (fee > 0 && fee < CTransaction::nMinTxFee) {
		throw runtime_error("in createcontracttx :fee is smaller than nMinTxFee\n");
	}

	if (vscriptid.IsEmpty()) {
		throw runtime_error("in createcontracttx :addresss is error!\n");
	}
	EnsureWalletIsUnlocked();
//	CContractTransaction tx;
	std::shared_ptr<CContractTransaction> tx = make_shared<CContractTransaction>();
	{


		//balance
		CAccountViewCache view(*pAccountViewTip, true);
		CAccount secureAcc;

		if(!pScriptDBTip->HaveScript(vscriptid))
		{
			throw runtime_error(tinyformat::format("createcontracttx :script id %s is not exist\n", vscriptid.ToString()));
		}

		auto GetUserId = [&](CKeyID &keyId)
		{
			CAccount acct;
			if (view.GetAccount(CUserID(keyId), acct)) {
				return acct.regID;
			}
			throw runtime_error(
							tinyformat::format("createcontracttx :account id %s is not exist\n", keyId.GetHex()));
		};

		vector<CUserID > vaccountid;

		for (auto& item : addr) {
			CKeyID keyid;
			if (!GetKeyId(item.get_str(),keyid)) {
				throw runtime_error("in createcontracttx :address err\n");
			}

			vaccountid.push_back(CUserID(GetUserId(keyid)));
		}

		tx.get()->scriptRegId = vscriptid;
		tx.get()->vAccountRegId = vaccountid;
		tx.get()->llFees = fee;
		tx.get()->vContract = vcontract;
		if( 0 == height) {
			height = chainActive.Tip()->nHeight;
		}
		tx.get()->nValidHeight = height;

		//get keyid by accountid
		CKeyID keyid;
		if (!view.GetKeyId(vaccountid.at(0), keyid)) {
			CID id(vaccountid.at(0));
			LogPrint("INFO", "vaccountid:%s have no key id\r\n", HexStr(id.GetID()).c_str());
			assert(0);
		}

		vector<unsigned char> signature;
		if (!pwalletMain->Sign(keyid,tx.get()->SignatureHash(), signature)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "createcontracttx Error: Sign failed.");
		}

		tx.get()->vSignature.push_back(signature);
	}
	if (tx.get()->vSignature.size() == tx.get()->vAccountRegId.size()) {
		std::tuple<bool, string> ret;
		ret = pwalletMain->CommitTransaction((CBaseTransaction *) tx.get());
		if (!std::get<0>(ret)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "Error:" + std::get<1>(ret));
		}
		Object obj;
		obj.push_back(Pair("hash", std::get<1>(ret)));
		return obj;
	} else {
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
		ds << pBaseTx;
		Object obj;
		obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
		return obj;
	}
}

//sign a contract tx
Value signcontracttx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		string msg =
				"signsecuretx nrequired \"contract tx str\"\n"
						"\nsign \"contract tx str\"\n"
						"\nArguments:\n"
						"1.\"contract tx str\": (string) \n"
						"\nResult:\n"
						"\"contract tx str\" or \"txhash\": (string) after the last one sign the tx will be commint and return \"txhash\"\n"
						"\nExamples:\n"
						+ HelpExampleCli("signcontracttx",
								"0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000")
						+ "\nAs json rpc call\n"
						+ HelpExampleRpc("signcontracttx",
								"0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000");
		throw runtime_error(msg);
	}
//	LogPrint("INFO", "signcontracttx enter\r\n");
	EnsureWalletIsUnlocked();
	vector<unsigned char> vch(ParseHex(params[0].get_str()));
	CDataStream stream(vch, SER_DISK, CLIENT_VERSION);

	std::shared_ptr<CBaseTransaction> pBaseTx; //= make_shared<CContractTransaction>();
	stream >> pBaseTx;

	std::shared_ptr<CContractTransaction> tx = make_shared<CContractTransaction>(pBaseTx.get());
//	cout << "sig:" << tx.get()->ToString(*pAccountViewTip) << endl;
//	assert(pwalletMain != NULL);
	{



		//balance
		CAccountViewCache view(*pAccountViewTip, true);

		if (tx.get()->vAccountRegId.size() < 1 || tx.get()->vSignature.size() >= tx.get()->vAccountRegId.size()) {
			throw runtime_error("in signsecuretx :tx data err\n");
		}

		vector<unsigned char> vscript;
		if (!pScriptDBTip->GetScript(boost::get<CRegID>(tx.get()->scriptRegId), vscript)) {
			CID id(tx.get()->scriptRegId);
			throw runtime_error(
					tinyformat::format("createcontracttx :script id %s is not exist\n", HexStr(id.GetID())));
		}

		for (auto& item : tx.get()->vAccountRegId) {
			CAccount account;
			if (!pAccountViewTip->GetAccount(item, account)) {
				CID id(item);
				throw runtime_error(
						tinyformat::format("createcontracttx :account id %s is not exist\n", HexStr(id.GetID())));
			}
		}

		CRegID accountid = boost::get<CRegID>(tx.get()->vAccountRegId.at(tx.get()->vSignature.size()));

		if (!pwalletMain->count(accountid.getKeyID(*pAccountViewTip))) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "signcontracttx error: Not to my time!\r\n");
		}

		//verify sig
		for (unsigned int ii = 0; ii < tx.get()->vSignature.size(); ii++) {
			CAccount account;
			if (!view.GetAccount(tx.get()->vAccountRegId.at(ii), account)) {
				CID id(tx.get()->vAccountRegId.at(ii));
				throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("unregister RegID: ") + HexStr(id.GetID()));
			}

			if (!account.PublicKey.Verify(tx.get()->SignatureHash(), tx.get()->vSignature.at(ii))) {
				throw runtime_error("in signsecuretx :tx data sign err\n");
			}
		}

		//sig
		//get keyid by accountid
		CKeyID keyid;
		if (!view.GetKeyId(tx.get()->vAccountRegId.at(tx.get()->vSignature.size()), keyid)) {
			LogPrint("INFO", "vaccountid:%s have no key id\r\n", accountid.ToString());
			assert(0);
		}

//		CKey key;
//		pwalletMain->GetKey(keyid, key);
		vector<unsigned char> signature;
		if (!pwalletMain->Sign(keyid,tx.get()->SignatureHash(), signature)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "createcontracttx Error: Sign failed.");
		}

		tx.get()->vSignature.push_back(signature);
	}

	if (tx.get()->vSignature.size() == tx.get()->vAccountRegId.size()) {

		std::tuple<bool, string> ret;
		ret = pwalletMain->CommitTransaction((CBaseTransaction *) tx.get());
		if (!std::get<0>(ret)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "registerscripttx Error:" + std::get<1>(ret));
		}
		Object obj;
		obj.push_back(Pair("hash", std::get<1>(ret)));
		return obj;
	} else {
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
		ds << pBaseTx;
		Object obj;
		obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
		return obj;
	}

}


//create a register script tx
Value registerscripttx(const Array& params, bool fHelp) {
	if (fHelp || params.size() < 4) {
		string msg = "registerscripttx nrequired \"addr\" \"script\" fee height\n"
				"\nregister script\n"
				"\nArguments:\n"
				"1.\"addr\": (string required)\n"
				"2.\"flag\": (numeric, required)\n"
				"3.\"script or scriptid\": (string required), if flag=0 is script's file path, else if flag=1 scriptid\n"
				"4.\"fee\": (numeric required) pay to miner\n"
				"5.\"height\": (numeric required)valid height\n"
				"6.\"script description\":(string optional) new script description\n"
				"7.\"nAuthorizeTime\": (numeric, optional)\n"
				"8.\"nMaxMoneyPerTime\": (numeric, optional)\n"
				"9.\"nMaxMoneyTotal\": (numeric, optional)\n"
				"10.\"nMaxMoneyPerDay\": (numeric, optional)\n"
				"11.\"nUserDefine\": (string, optional)\n"
				"\nResult:\n"
				"\"txhash\": (string)\n"
				"\nExamples:\n"
				+ HelpExampleCli("registerscripttx",
						"\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\" 010203040506 100000 1") + "\nAs json rpc call\n"
				+ HelpExampleRpc("registerscripttx",
						"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG 010203040506 100000 1");
		throw runtime_error(msg);
	}

	RPCTypeCheck(params, list_of(str_type)(int_type)(str_type)(int_type)(int_type));
	CVmScript vmScript;
	//get addresss
//	CDacrsAddress addr(params[0].get_str());
	vector<unsigned char> vscript;
	int flag = params[1].get_int();
	if (0 == flag) {
		string path = params[2].get_str();
		 FILE* file = fopen(path.c_str(), "rb+");
		 if(!file) {
			 throw runtime_error("create registerscripttx open script file"+path+"error");
		 }
		 long lSize;
//		 size_t nSize = 1;
		 fseek(file , 0 , SEEK_END);
		 lSize = ftell (file);
		 rewind (file);

		 // allocate memory to contain the whole file:
		 char *buffer = (char*) malloc(sizeof(char) * lSize);
		 if (buffer == NULL) {
			throw runtime_error("allocate memory failed");
		 }

		 if(fread(buffer, 1, lSize, file) != (size_t)lSize) {
			 	if(buffer)
			 		free(buffer);
				throw runtime_error("read script file error");
		 }
		 vmScript.Rom.insert(vmScript.Rom.end(), buffer, buffer+lSize);
		 CDataStream ds(SER_DISK, CLIENT_VERSION);
		 ds << vmScript;

		 vscript.assign(ds.begin(), ds.end());

		 if(file)
			 fclose(file);
		 if(buffer)
			 free(buffer);

	} else if (1 == flag) {
		vscript = ParseHex(params[2].get_str());
	}

	uint64_t fee = params[3].get_uint64();
	uint32_t height = params[4].get_int();

	if (params.size() > 5) {
		RPCTypeCheck(params, list_of(str_type)(int_type)(str_type)(int_type)(int_type)(str_type));
		string scriptDesc = params[5].get_str();
		vmScript.ScriptExplain.insert(vmScript.ScriptExplain.end(),scriptDesc.begin(), scriptDesc.end());
	}

	if (fee > 0 && fee < CTransaction::nMinTxFee) {
		throw runtime_error("in registerscripttx :fee is smaller than nMinTxFee\n");
	}
	//get keyid
	CKeyID keyid;
	if (!GetKeyId(params[0].get_str(),keyid)) {
		throw runtime_error("in registerscripttx :send address err\n");
	}

	assert(pwalletMain != NULL);
	CRegisterScriptTx tx;
	{
	//	LOCK2(cs_main, pwalletMain->cs_wallet);
		EnsureWalletIsUnlocked();

		//balance
		CAccountViewCache view(*pAccountViewTip, true);
		CAccount account;

		uint64_t balance = 0;
		CUserID userId = keyid;
		if (view.GetAccount(userId, account)) {
			balance = account.GetRawBalance(chainActive.Tip()->nHeight);
		}

		if (!account.IsRegister()) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registerscripttx Error: Account is not registered.");
		}

		if (!pwalletMain->count(keyid)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registerscripttx Error: WALLET file is not correct.");
		}

		if (balance < fee) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registerscripttx Error: Account balance is insufficient.");
		}

//		if (vscript.size() == SCRIPT_ID_SIZE) {
//			vector<unsigned char> vscriptcontent;
//			CRegID regid(vscript) ;
//			if (!pScriptDBTip->GetScript(CRegID(vscript), vscriptcontent)) {
//				throw JSONRPCError(RPC_WALLET_ERROR, "script id find failed");
//			}
//		}
		if(flag)
		{
			vector<unsigned char> vscriptcontent;
			CRegID regid(params[2].get_str()) ;
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
							tinyformat::format("createcontracttx :account id %s is not exist\n", mkeyId.ToAddress()));
		};

		tx.regAccountId = GetUserId(keyid);
		tx.script = vscript;
		tx.llFees = fee;
		if (0 == height) {
			height = chainActive.Tip()->nHeight;
		}
		tx.nValidHeight = height;


//		vector<unsigned char> vscriptcontent;
//		if (pScriptDBTip->GetScript(vscript, vscriptcontent)) {
//			tx.nFlag = 0;
//		}

//		CKey key;
//		pwalletMain->GetKey(keyid, key);
		if (!pwalletMain->Sign(keyid,tx.SignatureHash(), tx.signature)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "registerscripttx Error: Sign failed.");
		}
//		if (!pwalletMain->CommitTransaction((CBaseTransaction *) &tx)) {
//			throw JSONRPCError(RPC_WALLET_ERROR, "registerscripttx Error: CommitTransaction failed.");
//		}
	}

		std::tuple<bool,string> ret;
		ret = pwalletMain->CommitTransaction((CBaseTransaction *) &tx);
		if(!std::get<0>(ret))
		{
			throw JSONRPCError(RPC_WALLET_ERROR, "registerscripttx Error:"+ std::get<1>(ret));
		}
		Object obj;
		obj.push_back(Pair("hash", std::get<1>(ret)));
		return obj;

}

Value listaddr(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0) {
		string msg = "listaddr \n"
				"\nlistaddr\n"
				"\nArguments:\n"
				"\nResult:\n"
				"addr balance register\n"
				"\nExamples:\n" + HelpExampleCli("listaddr", "") + "\nAs json rpc call\n"
				+ HelpExampleRpc("listaddr", "");
		throw runtime_error(msg);
	}
	Array retArry;
	assert(pwalletMain != NULL);
	{

		map<CKeyID, CKeyStoreValue> pool =  pwalletMain->GetKeyPool();
		set<CKeyID> setKeyID;

		if (pool.size() == 0) {
			return retArry;
		}
		int curheight = chainActive.Tip()->nHeight;
		CAccountViewCache accView(*pAccountViewTip, true);

		for (const auto &tem : pool) {
			//find CAccount info by keyid

//			double dbalance = 0.0;
			CUserID userId = tem.first;


			auto GetDetailInfo = [&] (int curheight){
				Object obj;
				CAccount Lambaacc ;
				accView.GetAccount(userId, Lambaacc);
				obj.push_back(Pair("free  amount", (double)Lambaacc.GetRawBalance(curheight)/ (double) COIN));
				obj.push_back(Pair("Reward amount", (double)Lambaacc.GetRewardAmount(curheight)/ (double) COIN));
				return obj;
			};

			Object obj;
			obj.push_back(Pair("addr",       tem.first.ToAddress()));
			obj.push_back(Pair("balance",    GetDetailInfo(curheight)));
			obj.push_back(Pair("RegID",      tem.second.GetRegID().ToString()));
			if(!tem.second.GetRegID().IsEmpty())
			obj.push_back(Pair("RegID2",     HexStr(tem.second.GetRegID().GetVec6())));
			retArry.push_back(obj);
		}
	}
	return retArry;
}

Value listtx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0) {
		string msg = "listaddrtx \"addr\" showtxdetail\n"
				"\nlistaddrtx\n"
				"\nArguments:\n"
				"\nResult:\n"
				"\"txhash\"\n"
				"\nExamples:\n" + HelpExampleCli("listtx", "")
				+ "\nAs json rpc call\n" + HelpExampleRpc("listtx", "");
		throw runtime_error(msg);
	}

	Object retObj;
	assert(pwalletMain != NULL);
	{
		Object Inblockobj;
		for (auto const &wtx : pwalletMain->mapInBlockTx) {
			for(auto const & item : wtx.second.mapAccountTx) {
				Inblockobj.push_back(Pair("tx",  item.first.GetHex()));
			}
		}
		retObj.push_back(Pair("ConfirmTx" ,Inblockobj));

		CAccountViewCache view(*pAccountViewTip, true);
		Array UnConfirmTxArry;
			for (auto const &wtx : pwalletMain->UnConfirmTx) {
				UnConfirmTxArry.push_back(wtx.first.GetHex());
			}
	  retObj.push_back(Pair("UnConfirmTx" ,UnConfirmTxArry));
	}
	return retObj;
}

Value getaccountinfo(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		string msg = "getaddramount \"addr\"\n"
				"\ngetaddramount\n"
				"\nArguments:\n"
				"1.\"addr\": (string)"
				"\nResult:\n"
				"\"mature amount\":\n"
				"\"free amount\":\n"
				"\"frozen amount\":\n"
				"\"Account Info\":\n"
				"\nExamples:\n" + HelpExampleCli("getaccountinfo", "5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\n")
				+HelpExampleCli("getaccountinfo", "000000000500\n")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("getaccountinfo", "5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\n");
		throw runtime_error(msg);
	}
	RPCTypeCheck(params, list_of(str_type));
//	CUserID userId;
//	if (params[0].get_str().size() == 12) {
//		vector<unsigned char> vscript = ParseHex(params[0].get_str());
//		userId = CRegID(vscript);
//	} else {
//		CDacrsAddress address(params[0].get_str());
//		CKeyID keyid;
//		if (!address.GetKeyID(keyid))
//			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
//		userId = keyid;
//	}
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
			account.CompactAccount(chainActive.Height());
			if(!account.PublicKey.IsValid()){
				CPubKey pk;
				CPubKey minerpk;
				if (pwalletMain->GetPubKey(keyid, pk)) {
					pwalletMain->GetPubKey(keyid, minerpk, true);
					account.PublicKey = pk;
					account.keyID=std::move(pk.GetKeyID());
					if (pk != minerpk && !account.MinerPKey.IsValid()) {
						account.MinerPKey = minerpk;
					}
				}
			}
			obj = std::move(account.ToJosnObj());
			obj.push_back(Pair("postion","inblock"));
		} else {
			CPubKey pk;
			CPubKey minerpk;
			if (pwalletMain->GetPubKey(keyid, pk)) {
				pwalletMain->GetPubKey(keyid, minerpk, true);
				account.PublicKey = pk;
				account.keyID=pk.GetKeyID();
				if (minerpk != pk) {
					account.MinerPKey = minerpk;
				}
				obj = std::move(account.ToJosnObj());
			    obj.push_back(Pair("postion","inwallet"));
			}
		}

	}



	return obj;
}


//list unconfirmed transaction of mine
Value listunconfirmedtx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0) {
		string msg = "listunconfirmedtx  bshowtxdetail\n"
				"\nlistunconfirmedtx\n"
				"\nArguments:\n"
				"1.bshowtxdetail: default false\n"
				"\nResult:\n"
				"\"txhash\"\n"
				"\nExamples:\n" + HelpExampleCli("listunconfirmedtx", "") + "\nAs json rpc call\n"
				+ HelpExampleRpc("listunconfirmedtx", "");
		throw runtime_error(msg);
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
	if (!GetKeyId(params[0].get_str(),keyid)) {
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
	retObj.push_back(Pair("signeddata",  HexStr(vsign)));
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




static Value COperFundToJson(const COperFund &opfound)
{
	Object obj;
	obj.push_back(Pair("operType",  opfound.operType == 1 ? "ADD_VALUE":"MINUS_VALUE"));
	Array array;

	static const vector<string> strname=
	{
	" ",
	"FREEDOM",
	"REWARD_FUND",
	"FREEDOM_FUND",
	"FREEZD_FUND",
	"SELF_FREEZD_FUND",
	"NULL_FUNDTYPE",
	};
	for(auto const &te: opfound.vFund)
	{
      assert(te.nFundType>=1 &&te.nFundType < strname.size());
      Object obj2;
      obj2.push_back(Pair("FundType",  strname[te.nFundType]));
      obj2.push_back(Pair("sriptID",  HexStr(te.scriptID)));
      obj2.push_back(Pair("value",  te.value));
      obj2.push_back(Pair("nHeight",  te.nHeight));
      array.push_back(obj2);
	}
	obj.push_back(Pair("vFund", array));
	return obj;
}

Value gettxoperationlog(const Array& params, bool fHelp)
{
	if (fHelp || params.size() != 1) {
		if (fHelp || params.size() != 2) {
			string msg = "sign nrequired \"str\"\n"
					"\nsign \"str\"\n"
					"\nArguments:\n"
					"1.\"txhash\": (string required) \n"
					"\nResult:\n"
					"\"vOperFund\": (string)\n"
					"\"authorLog\": (string)\n"
					"\nExamples:\n"
					+ HelpExampleCli("gettxoperationlog",
							"0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000")
					+ "\nAs json rpc call\n"
					+ HelpExampleRpc("gettxoperationlog",
							"0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000");
			throw runtime_error(msg);
		}
	}
	RPCTypeCheck(params, list_of(str_type));
	uint256 txHash(params[0].get_str());
	vector<CAccountOperLog> vLog;
	Object retobj;
	retobj.push_back(Pair("hash",  txHash.GetHex()));
	if(!GetTxOperLog(txHash, vLog))
	   throw JSONRPCError(RPC_INVALID_PARAMS, "error hash");

	{
		Array arrayvLog;
        for(auto const &te :vLog)
        {

			Object obj;
			obj.push_back(Pair("addr",  te.keyID.ToAddress()));
			Array array;
			for(auto const &teOperFund: te.vOperFund)
			{
			  array.push_back(COperFundToJson(teOperFund));
			}
			obj.push_back(Pair("vOperFund",array));
//			obj.push_back(Pair("authorLog",te.authorLog.ToString()));
			arrayvLog.push_back(obj);
        }
        retobj.push_back(Pair("AccountOperLog",  arrayvLog));

	}

	return retobj;

}
static Value TestDisconnectBlock(int number)
{
//		CBlockIndex* pindex = chainActive.Tip();
		CBlock block;
		CValidationState state;
		if((chainActive.Tip()->nHeight - number) < 0)
		{
			throw JSONRPCError(RPC_INVALID_PARAMS, "restclient Error: number");
		}
		while (number--) {
			// check level 0: read from disk
			 CBlockIndex * pTipIndex = chainActive.Tip();
			 LogPrint("vm", "current height:%d\n", pTipIndex->nHeight);
		      if (!DisconnectBlockFromTip(state))
		    	  return false;
		      chainMostWork.SetTip(pTipIndex->pprev);
		      if(!EraseBlockIndexFromSet(pTipIndex))
		    	  return false;
		      if(!pblocktree->EraseBlockIndex(pTipIndex->GetBlockHash()))
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
		}
//		pTxCacheTip->Flush();
		Object obj;
		obj.push_back(Pair("tip", strprintf("hash:%s hight:%s",chainActive.Tip()->GetBlockHash().ToString(),chainActive.Tip()->nHeight)));
		return obj;
}


Value disconnectblock(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		throw runtime_error("disconnectblock \"block numbers \n"
				"\ndisconnect block\n" + HelpRequiringPassphrase() + "\nArguments:\n"
				"1. \"numbers \"  (numeric, required) The Dacrs address.\n"
				"\nResult:\n"
				"\"disconnect result\"  (bool) \n"
				"\nExamples:\n" + HelpExampleCli("disconnectblock", "\"1\""));
	}
	int number = params[0].get_int();

	Value te = TestDisconnectBlock(number);

	return te;
}
extern CAccountViewDB *pAccountViewDB;
Value resetclient(const Array& params, bool fHelp) {
	Value te = TestDisconnectBlock(chainActive.Tip()->nHeight);

	if(chainActive.Tip()->nHeight == 0)
	{
		pwalletMain->CleanAll();
		CBlockIndex* te=chainActive.Tip();
		uint256 hash= te->GetBlockHash();
//		auto ret = remove_if( mapBlockIndex.begin(), mapBlockIndex.end(),[&](std::map<uint256, CBlockIndex*>::reference a) {
//			return (a.first == hash);
//		});
//		mapBlockIndex.erase(ret,mapBlockIndex.end());
		for(auto it = mapBlockIndex.begin(), ite = mapBlockIndex.end(); it != ite;)
		{
		  if(it->first != hash)
		    it = mapBlockIndex.erase(it);
		  else
		    ++it;
		}
		pAccountViewTip->Flush();
		pScriptDBTip->Flush();
		pTxCacheTip->Flush();
//		pTxCacheTip->GetTxHashCache().size()
       if(SysCfg().Network::TESTNET == SysCfg().NetworkID()|| SysCfg().Network::TESTNET==SysCfg().NetworkID()){
       assert(pAccountViewDB->GetDbCount() == 22);
       assert(pScriptDB->GetDbCount() == 0);}

		CBlock firs = SysCfg().GenesisBlock();
		pwalletMain->SyncTransaction(0,NULL,&firs);
		mempool.clear();
	}
	else
	{
		throw JSONRPCError(RPC_WALLET_ERROR, "restclient Error: Sign failed.");
	}
	return te;

}

Value listregscript(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		throw runtime_error("listregscript " + HelpRequiringPassphrase() + "\nArguments:\n"
				"\nResult:\n"
				"\"regscript array\"  (bool) \n"
				"\nExamples:\n" + HelpExampleCli("listregscript", "true"));
	}
	bool showDetail = false;
	showDetail = params[0].get_bool();
	Object obj;
	Array arrayScript;

	CAccountViewCache view(*pAccountViewTip, true);
	if(pScriptDBTip != NULL) {
		int nCount(0);
		if(!pScriptDBTip->GetScriptCount(nCount))
			throw JSONRPCError(RPC_DATABASE_ERROR, "get script error: cannot get registered count.");
		CRegID regId;
		vector<unsigned char> vScript;
		Object script;
		if(!pScriptDBTip->GetScript(0, regId, vScript))
			throw JSONRPCError(RPC_DATABASE_ERROR, "get script error: cannot get registered script.");
		script.push_back(Pair("scriptId", regId.ToString()));
		script.push_back(Pair("scriptId2", HexStr(regId.GetVec6())));
		if(showDetail)
		script.push_back(Pair("scriptContent", HexStr(vScript.begin(), vScript.end())));
		arrayScript.push_back(script);
		while(pScriptDBTip->GetScript(1, regId, vScript)) {
			Object obj;
			obj.push_back(Pair("scriptId",  regId.ToString()));
			obj.push_back(Pair("scriptId2", HexStr(regId.GetVec6())));
			if(showDetail)
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
	if (!GetKeyId(params[0].get_str(),keyid))
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid  address");

	double dbalance = 0.0;
	{
		LOCK(cs_main);
		CAccountViewCache accView(*pAccountViewTip, true);
		CAccount secureAcc;
		CUserID userId = keyid;
		if (accView.GetAccount(userId, secureAcc)) {
			dbalance = (double) secureAcc.GetRawBalance(chainActive.Tip()->nHeight + 100) / (double) COIN;
		}
	}
	return dbalance;
}

Value generateblock(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		string msg = "generateblock nrequired (\"addr\")\n"
				"\ngenerateblock\n"
				"\nArguments:\n"
				"1.\"addr\": (str)\n"
				"\nResult:\n"
				"NULL"
				"\nExamples:\n" + HelpExampleCli("generateblock", "5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("generateblock", "5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9");
		throw runtime_error(msg);
	}
	//get keyid
	CKeyID keyid;

	if (!GetKeyId(params[0].get_str(), keyid)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "in generateblock :address err");
	}

	uint256 hash = CreateBlockWithAppointedAddr(keyid);
	if (hash == 0) {
		throw runtime_error("in generateblock :cannot generate block\n");
	}
	Object obj;
	obj.push_back(Pair("blockhash",hash.GetHex()));
	return obj;
}



Value listtxcache(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0) {
		throw runtime_error("listtxcache \"address \n" + HelpRequiringPassphrase() + "\nArguments:\n"
				"\nResult:\n"
				"\"txcache\"  (string) \n"
				"\nExamples:\n" + HelpExampleCli("listtxcache", ""));
	}
//	const map<uint256, vector<uint256> > &mapTxHashCacheByPrev = pTxCacheTip->GetRelayTx();
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
		throw runtime_error("reloadtxcache" + HelpRequiringPassphrase() + "\nArguments:\n"
				"\nResult:\n"
				"\nExamples:\n" + HelpExampleCli("reloadtxcache", ""));
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
	obj.push_back(Pair("info","reload tx cache succeed"));
	return obj;
//	return string("reload tx cache succeed");
}

enum BETSTATUS {
	BETSEND,  //!< SEND
	BETACCEPT,//!< ACCEPT
	BETOPEN   //!< OPEN
};
typedef struct {



	unsigned char status;
	unsigned char sendid[6];//send bet account id
	unsigned char acceptid[6];//accept bet account id
	uint64_t money;//bet amount
	int hight;//�ϴζ���߶�
	int delyhight;
	uint256 shash;//send data hash
//	uchar ahash[32];//accept data hash
	unsigned char sdata[5];//send data
	unsigned char adata[5];//accept data

	  IMPLEMENT_SERIALIZE
	    (
	        READWRITE(status);
	  for(unsigned int i = 0 ;i < sizeof(sendid);i++)
	        READWRITE(sendid[i]);
	  for(unsigned int i = 0 ;i < sizeof(acceptid);i++)
	        READWRITE(acceptid[i]);
	  READWRITE(money);
	  READWRITE(hight);
	  READWRITE(delyhight);
	  READWRITE(shash);
	  for(unsigned int i = 0 ;i < sizeof(sdata);i++)
		        READWRITE(sdata[i]);
	  for(unsigned int i = 0 ;i < sizeof(adata);i++)
		        READWRITE(adata[i]);
	    )

	Object toJson()
	  {
		  string temp[3] ={"BET_SENDED","BET_ACCEPTED","BET_OPENED"};
			Object obj;
			obj.push_back(Pair("status",temp[status]));
			CRegID sid(vector<unsigned char>(sendid,sendid+sizeof(sendid)));
			obj.push_back(Pair("sendid",sid.ToString()));
			if(status >= BETACCEPT){
			CRegID rid(vector<unsigned char>(acceptid,acceptid+sizeof(acceptid)));
			obj.push_back(Pair("acceptid",rid.ToString()));
			}
			obj.push_back(Pair("money",ValueFromAmount(money)));
			obj.push_back(Pair("timeouthigh",hight));
			obj.push_back(Pair("delyhight",delyhight));
			obj.push_back(Pair("shash",shash.ToString()));
			if(BETOPEN == status)
			 obj.push_back(Pair("senddata",strprintf("%d-%d-%d-%d-%d",sdata[0],sdata[1],sdata[2],sdata[3],sdata[4])));
			if(status >= BETACCEPT)
		       obj.push_back(Pair("acceptdata",strprintf("%d-%d-%d-%d-%d",adata[0],adata[1],adata[2],adata[3],adata[4])));
			return obj;
	  }

}P2P_BET_DATA;

static int getDataFromSriptData(CScriptDBViewCache &cache, const CRegID &regid,
		int pagesize, int index,
		vector<
				std::tuple<vector<unsigned char>,
				vector<unsigned char>,
				int> >&ret
				) {
	int dbsize;
	int nHeight = 0;
	int height = chainActive.Height();
	cache.GetScriptDataCount(regid, dbsize);
	if (0 == dbsize) {
		throw runtime_error(
				"in getscriptdata :the scirptid database not data!\n");
	}
	vector<unsigned char> value;
	vector<unsigned char> vScriptKey;

	set<CScriptDBOperLog> dumy;
	if (!cache.GetScriptData(height, regid, 0, vScriptKey, value, nHeight,
			dumy)) {
		throw runtime_error(
				"in getscriptdata :the scirptid get data failed!\n");
	}
	if (index == 1) {
		ret.push_back(std::make_tuple(vScriptKey, value, nHeight));
	}
	int readCount(1);
	while (--dbsize) {
		dumy.clear();
		if (cache.GetScriptData(height, regid, 1, vScriptKey, value, nHeight,
				dumy)) {
			++readCount;
			if (readCount > pagesize * (index - 1)) {
				ret.push_back(std::make_tuple(vScriptKey, value, nHeight));
			}
		}
		if (readCount >= pagesize * index) {
			return ret.size();
		}
	}
	return ret.size();
}
Value getp2pbetdata(const Array& params, bool fHelp){
	if (fHelp || params.size() != 3) {
		string msg = "getscriptdata nrequired \"scriptid\" \"\n"
				"\ncreate contract\n"
				"\nArguments:\n"
				"1.\"appid\": (string)\n"
				"2.[pagesize]: (pagesize int)\n"
				"3.\"index\": (int )\n";
		throw runtime_error(msg);
	}
	CRegID regid(params[0].get_str());
	if (regid.IsEmpty() == true) {
		throw runtime_error("in getscriptdata :vscriptid size is error!\n");
	}
	int pagesize = params[1].get_int();
	int index = params[2].get_int();
	int dbsize;
	CScriptDBViewCache cacheTemp(*pScriptDBTip, true);
	cacheTemp.GetScriptDataCount(regid, dbsize);
	if (0 == dbsize) {
		throw runtime_error(
				"in getscriptdata :the scirptid database not data!\n");
	}
	vector<std::tuple<vector<unsigned char>, vector<unsigned char>, int> > ret;
	getDataFromSriptData(cacheTemp, regid, pagesize, index, ret);
	Object sendobj;
	Object accseptobj;
	Object openobj;
	for (auto te : ret) {
		vector<unsigned char> key = std::get<0>(te);
		uint256 hash(key);
		P2P_BET_DATA tem;
		vector<unsigned char> valvue = std::get<1>(te);
		CDataStream stream(valvue, SER_DISK, CLIENT_VERSION);

		try { // because of bug !
			stream >> tem;
		} catch (...) {
			ERRORMSG("unseraile err!");
			continue;
		}
		Object temjosn;
		temjosn = tem.toJson();
		temjosn.push_back(Pair("key",hash.ToString()));
		switch (tem.status) {
		case BETSEND:
			sendobj.push_back(Pair(hash.ToString(), temjosn));
			break;
		case BETACCEPT:
			accseptobj.push_back(Pair(hash.ToString(), temjosn));
			break;
		case BETOPEN:
			openobj.push_back(Pair(hash.ToString(), temjosn));
			break;
		default:
			break;
		}
	}
	Object retd;
	retd.push_back(Pair("first", sendobj));
	retd.push_back(Pair("second", accseptobj));
	retd.push_back(Pair("third", openobj));
	return retd;
}

Value getscriptdata(const Array& params, bool fHelp) {
	if (fHelp || params.size() < 2) {
		string msg = "getscriptdata nrequired \"scriptid\" \"\n"
				"\ncreate contract\n"
				"\nArguments:\n"
				"1.\"scriptid\": (string)\n"
				"2.[pagesize or key]: (pagesize int)\n"
				"3.\"index\": (int )\n"
				"\"contract tx str\": (string)\n";
		throw runtime_error(msg);
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
		int nHeight = 0;
		CScriptDBOperLog operLog;
		if (!contractScriptTemp.GetScriptData(height,regid, key, value, nHeight, operLog)) {
			throw runtime_error("in getscriptdata :the key not exist!\n");
		}
		script.push_back(Pair("scritpid", params[0].get_str()));
		script.push_back(Pair("key", HexStr(key)));
		script.push_back(Pair("value", HexStr(value)));
		script.push_back(Pair("height", nHeight));
		return script;

	} else {
		int dbsize;
		contractScriptTemp.GetScriptDataCount(regid, dbsize);
		if (0 == dbsize) {
			throw runtime_error("in getscriptdata :the scirptid database not data!\n");
		}
		int pagesize = params[1].get_int();
		int index = params[2].get_int();

		vector<std::tuple<vector<unsigned char>,vector<unsigned char>,int> >ret;
		getDataFromSriptData(contractScriptTemp,regid,pagesize,index,ret);
		Array retArray;
		for(auto te:ret)
		{
			vector<unsigned char>key =  std::get<0>(te);
			vector<unsigned char>valvue =  std::get<1>(te);
			int high =  std::get<2>(te);
			Object firt;
			firt.push_back(Pair("key", HexStr(key)));
			firt.push_back(Pair("value", HexStr(valvue)));
			firt.push_back(Pair("height", high));
			retArray.push_back(firt);
		}
		return retArray;
	}

	return script;
}

Value saveblocktofile(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 2) {
			string msg = "saveblocktofile nrequired"
					"\nArguments:\n"
					"1.\"blockhash\": (string, required)\n"
					"2.\"filepath\": (string, required)\n";
			throw runtime_error(msg);
		}
	string strblockhash = params[0].get_str();
	uint256 blockHash(params[0].get_str());
	CBlockIndex *pIndex = mapBlockIndex[blockHash];
	CBlock blockInfo;
	if (!ReadBlockFromDisk(blockInfo, pIndex))
		throw runtime_error(_("Failed to read block"));
	string file = params[1].get_str();
	try {
		FILE* fp = fopen(file.c_str(), "a+");
		CAutoFile fileout = CAutoFile(fp, SER_DISK, CLIENT_VERSION);
		if (!fileout)
			throw JSONRPCError(RPC_MISC_ERROR, "open file:"+strblockhash+"failed!");
		fileout << blockInfo;
		fflush(fileout);
		fclose(fp);
	}catch(std::exception &e) {
		throw JSONRPCError(RPC_MISC_ERROR, "save block to file error");
	}
	return "save succeed";
}

Value getscriptdbsize(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
				string msg = "getscriptdbsize nrequired"
						"\nArguments:\n"
						"1.\"scriptid\": (string, required)\n";
				throw runtime_error(msg);
			}
	CRegID regid(params[0].get_str());
	if (regid.IsEmpty() == true) {
		throw runtime_error("in getscriptdata :vscriptid is error!\n");
	}

	if (!pScriptDBTip->HaveScript(regid)) {
		throw runtime_error("in getscriptdata :vscriptid id is not exist!\n");
	}
	int nDataCount = 0;
	if(!pScriptDBTip->GetScriptDataCount(regid, nDataCount)) {
		throw runtime_error("GetScriptDataCount error!");
	}
	return nDataCount;
}

Value registaccounttxraw(const Array& params, bool fHelp) {

	if (fHelp || !(params.size() == 4 || params.size() == 3)) {
		string msg = "registaccounttx nrequired \"addr\" fee height\n"
				"\nregister secure account\n"
				"\nArguments:\n"
				"1.height: (numeric) pay to miner\n"
				"2.fee: (numeric) pay to miner\n"
				"3.\"publick\": (string)\n"
				"4.minerpublickey: (string)create height\n"
				"\nResult:\n"
				"\"txhash\": (string)\n"
				"\nExamples:\n" + HelpExampleCli("registaccounttxraw", "10 10000 n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj true")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("registaccounttxraw", "10 1000000 n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj false");
		throw runtime_error(msg);
	}
    CUserID ukey;
    CUserID uminerkey = CNullID();

	int hight = params[0].get_int();

    int64_t Fee = AmountToRawValue(params[1]);

    CKeyID dummy;
    CPubKey pubk =  CPubKey(ParseHex(params[2].get_str()));
    if(!pubk.IsCompressed()||!pubk.IsFullyValid())
    {
    	throw JSONRPCError(RPC_INVALID_PARAMS, "CPubKey err");
    }
    ukey = pubk;
    dummy =pubk.GetKeyID();

	if (params.size() == 4) {
		CPubKey pubk = CPubKey(ParseHex(params[3].get_str()));
		if (!pubk.IsCompressed() || !pubk.IsFullyValid()) {
			throw JSONRPCError(RPC_INVALID_PARAMS, "CPubKey err");
		}
		uminerkey = pubk;
	}

	std::shared_ptr<CRegisterAccountTx> tx = make_shared<CRegisterAccountTx>(ukey,uminerkey,Fee,hight);
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
	ds << pBaseTx;
	Object obj;
	obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
	return obj;



	}
Value submittx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		string msg = "registaccounttx nrequired \"addr\" fee height\n"
				"\nregister secure account\n"
				"\nArguments:\n"
				"1.\"raw\": (string)\n"
				"\nExamples:\n" + HelpExampleCli("submittx", "n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj");
		throw runtime_error(msg);
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
	if (fHelp || params.size() != 5) {
		string msg = "createsecuretx nrequired \"scriptid\" [\"addr\",...] \"fee\" \"contract\" \"height\"\n"
				"\ncreate contract\n"
				"\nArguments:\n"
				"1.\"height\": (numeric)create height\n"
				"2.\"fee\": (numeric) pay to miner\n"
				"3.\"scriptid\": (string)\n"
				"4.[\"addr\",...]: (string list)\n"
				"5.\"contract\": (string)\n"
				"\nResult:\n"
				"\"contract tx str\": (string)\n"
				"\nExamples:\n"
				+ HelpExampleCli("createcontracttxraw", "10 1000 01020304 000000000100 [\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"] "
						"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\","
						"\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\"] ") + "\nAs json rpc call\n"
				+ HelpExampleRpc("createcontracttxraw", "10 1000 01020304 000000000100 000000000100 [\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"] "
						"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\","
						"\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\"] ");
		throw runtime_error(msg);
	}


	RPCTypeCheck(params, list_of(int_type)(real_type)(str_type)(array_type)(str_type));

	int hight = params[0].get_int();
	uint64_t fee = AmountToRawValue(params[1]);
	CRegID vscriptid(params[2].get_str()) ;
	Array addr = params[3].get_array();
	vector<unsigned char> vcontract = ParseHex(params[4].get_str());


	if (fee > 0 && fee < CTransaction::nMinTxFee) {
		throw runtime_error("in createcontracttxraw :fee is smaller than nMinTxFee\n");
	}

	if (vscriptid.IsEmpty()) {
		throw runtime_error("in createcontracttxraw :addresss is error!\n");
	}

	CAccountViewCache view(*pAccountViewTip, true);
	CAccount secureAcc;

	if(!pScriptDBTip->HaveScript(vscriptid))
	{
		throw runtime_error(tinyformat::format("createcontracttx :script id %s is not exist\n", vscriptid.ToString()));
	}

	auto GetUserId = [&](CKeyID &keyId)
	{
		CAccount acct;
		if (view.GetAccount(CUserID(keyId), acct)) {
			return acct.regID;
		}
		throw runtime_error(
						tinyformat::format("createcontracttx :account id %s is not exist\n", keyId.GetHex()));
	};

	vector<CUserID > vaccountid;

	for (auto& item : addr) {
		CKeyID keyid;
		if (!GetKeyId(item.get_str(),keyid)) {
			throw runtime_error("in createcontracttx :address err\n");
		}
		vaccountid.push_back(CUserID(GetUserId(keyid)));
	}

	CKeyID keyid;
	if (!view.GetKeyId(vaccountid.at(0), keyid)) {
		CID id(vaccountid.at(0));
		LogPrint("INFO", "vaccountid:%s have no key id\r\n", HexStr(id.GetID()).c_str());
		assert(0);
	}
	std::shared_ptr<CContractTransaction> tx = make_shared<CContractTransaction>(vscriptid,vcontract,vaccountid,hight,fee);

	CDataStream ds(SER_DISK, CLIENT_VERSION);
	std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
	ds << pBaseTx;
	Object obj;
	obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
	return obj;
}

Value registerscripttxraw(const Array& params, bool fHelp) {
	if (fHelp || params.size() < 4) {
		string msg = "registerscripttx nrequired \"addr\" \"script\" fee height\n"
				"\nregister script\n"
				"\nArguments:\n"
				"1.\"height\": (numeric required)valid height\n"
				"2.\"fee\": (numeric required) pay to miner\n"
				"3.\"addr\": (string required)\n"
				"4.\"flag\": (numeric, required)\n"
				"5.\"script or scriptid\": (string required), if flag=0 is script's file path, else if flag=1 scriptid\n"
				"6.\"script description\":(string optional) new script description\n"
				"7.\"nAuthorizeTime\": (numeric, optional)\n"
				"8.\"nMaxMoneyPerTime\": (numeric, optional)\n"
				"9.\"nMaxMoneyTotal\": (numeric, optional)\n"
				"10.\"nMaxMoneyPerDay\": (numeric, optional)\n"
				"11.\"nUserDefine\": (string, optional)\n"
				"\nResult:\n"
				"\"txhash\": (string)\n"
				"\nExamples:\n"
				+ HelpExampleCli("registerscripttxraw",
						"10 10000 \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\" 010203040506 ") + "\nAs json rpc call\n"
				+ HelpExampleRpc("registerscripttxraw",
						"10 10000 5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG 010203040506 ");
		throw runtime_error(msg);
	}


	RPCTypeCheck(params, list_of(int_type)(real_type)(str_type)(bool_type)(str_type));

	uint64_t fee = AmountToRawValue(params[1]);;
	uint32_t height = params[0].get_int();

	CVmScript vmScript;
	vector<unsigned char> vscript;
	int flag = params[3].get_bool();
	if (0 == flag) {
		string path = params[4].get_str();
		 FILE* file = fopen(path.c_str(), "rb+");
		 if(!file) {
			 throw runtime_error("create registerscripttx open script file"+path+"error");
		 }
		 long lSize;
//		 size_t nSize = 1;
		 fseek(file , 0 , SEEK_END);
		 lSize = ftell (file);
		 rewind (file);

		 // allocate memory to contain the whole file:
		 char *buffer = (char*) malloc(sizeof(char) * lSize);
		 if (buffer == NULL) {
			throw runtime_error("allocate memory failed");
		 }

		 if(fread(buffer, 1, lSize, file) != (size_t)lSize) {
			 	if(buffer)
			 		free(buffer);
				throw runtime_error("read script file error");
		 }
		 vmScript.Rom.insert(vmScript.Rom.end(), buffer, buffer+lSize);
		 CDataStream ds(SER_DISK, CLIENT_VERSION);
		 ds << vmScript;

		 vscript.assign(ds.begin(), ds.end());

		 if(file)
			 fclose(file);
		 if(buffer)
			 free(buffer);

	} else if (1 == flag) {
		vscript = ParseHex(params[4].get_str());
	}

	if (params.size() > 5) {
		RPCTypeCheck(params, list_of(int_type)(real_type)(str_type)(bool_type)(str_type)(str_type));
		string scriptDesc = params[5].get_str();
		vmScript.ScriptExplain.insert(vmScript.ScriptExplain.end(),scriptDesc.begin(), scriptDesc.end());
	}

	if (fee > 0 && fee < CTransaction::nMinTxFee) {
		throw runtime_error("in registerscripttx :fee is smaller than nMinTxFee\n");
	}
	//get keyid
	CKeyID keyid;
	if (!GetKeyId(params[2].get_str(),keyid)) {
		throw runtime_error("in registerscripttx :send address err\n");
	}

	//balance
	CAccountViewCache view(*pAccountViewTip, true);
	CAccount account;

//	uint64_t balance = 0;
	CUserID userId = keyid;
	if (!view.GetAccount(userId, account)) {
		throw JSONRPCError(RPC_WALLET_ERROR, "in registerscripttxraw Error: Account is not exist.");
	}

	if (!account.IsRegister()) {
		throw JSONRPCError(RPC_WALLET_ERROR, "in registerscripttxraw Error: Account is not registered.");
	}

	if(flag)
	{
		vector<unsigned char> vscriptcontent;
		CRegID regid(params[2].get_str()) ;
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
	std::shared_ptr<CRegisterScriptTx> tx =  make_shared<CRegisterScriptTx>();
	tx.get()->regAccountId = GetUserId(keyid);
	tx.get()->script = vscript;
	tx.get()->llFees = fee;
	tx.get()->nValidHeight = height;
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
	ds << pBaseTx;
	Object obj;
	obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
	return obj;

}

Value sigstr(const Array& params, bool fHelp)
{
	if (fHelp || params.size() != 2) {
		string msg = "registerscripttx nrequired \"addr\" \"script\" fee height\n"
				"\nregister script\n"
				"\nArguments:\n"
				"1.\"str\": (str) sig str\n"
				"2.\"addr\": (str)\n"
				"\nExamples:\n"
				+ HelpExampleCli("sigstr",
						"1010000010203040506 \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\" ") + "\nAs json rpc call\n"
				+ HelpExampleRpc("sigstr",
						"1010000010203040506 5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG 010203040506 ");
		throw runtime_error(msg);
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
	switch(pBaseTx.get()->nTxType){
	case COMMON_TX:{
		std::shared_ptr<CTransaction> tx = make_shared<CTransaction>(pBaseTx.get());
		CKeyID keyid;
		if (!view.GetKeyId(tx.get()->srcUserId, keyid)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "vaccountid have no key id");
		}
		if (!pwalletMain->Sign(keyid,tx.get()->SignatureHash(), tx.get()->signature)) {
			throw JSONRPCError(RPC_INVALID_PARAMETER,  "Sign failed");
		}
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
		ds << pBaseTx;
		obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
	}
		break;
	case REG_ACCT_TX:
	{
			std::shared_ptr<CRegisterAccountTx> tx = make_shared<CRegisterAccountTx>(pBaseTx.get());
			if (!pwalletMain->Sign(keyid,tx.get()->SignatureHash(), tx.get()->signature)) {
				throw JSONRPCError(RPC_INVALID_PARAMETER,  "Sign failed");
			}
			CDataStream ds(SER_DISK, CLIENT_VERSION);
			std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
			ds << pBaseTx;
			obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
		}
			break;
	case CONTRACT_TX:{
		std::shared_ptr<CContractTransaction> tx = make_shared<CContractTransaction>(pBaseTx.get());
		if (!pwalletMain->Sign(keyid,tx.get()->SignatureHash(),tx.get()->vSignature[0])) {
			throw JSONRPCError(RPC_INVALID_PARAMETER,  "Sign failed");
		}
		CDataStream ds(SER_DISK, CLIENT_VERSION);
			std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
			ds << pBaseTx;
			obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
	}
	break;
	case REWARD_TX:
		break;
	case REG_SCRIPT_TX:	{
		std::shared_ptr<CRegisterScriptTx> tx = make_shared<CRegisterScriptTx>(pBaseTx.get());
		if (!pwalletMain->Sign(keyid,tx.get()->SignatureHash(), tx.get()->signature)) {
			throw JSONRPCError(RPC_INVALID_PARAMETER,  "Sign failed");
		}
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
		ds << pBaseTx;
		obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
	}
		break;
	default:
		assert(0);
		break;
	}
	return obj;
}

Value getalltxinfo(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0) {
		string msg = "getalltxinfo \"addr\" showtxdetail\n"
				"\nlistaddrtx\n"
				"\nArguments:\n"
				"\nResult:\n"
				"\"txhash\"\n"
				"\nExamples:\n" + HelpExampleCli("getalltxinfo", "") + "\nAs json rpc call\n"
				+ HelpExampleRpc("getalltxinfo", "");
		throw runtime_error(msg);
	}

	Object retObj;

	assert(pwalletMain != NULL);
	{
		Array ComfirmTx;
		for (auto const &wtx : pwalletMain->mapInBlockTx) {
			for (auto const & item : wtx.second.mapAccountTx) {
				Object objtx = GetTxDetail(item.first, false);
				ComfirmTx.push_back(objtx);
			}
		}
		retObj.push_back(Pair("Confirmed", ComfirmTx));

		Array UnComfirmTx;
		CAccountViewCache view(*pAccountViewTip, true);
		for (auto const &wtx : pwalletMain->UnConfirmTx) {
			Object objtx = GetTxDetail(wtx.first, false);
			UnComfirmTx.push_back(objtx);
		}
		retObj.push_back(Pair("UnConfirmed", UnComfirmTx));
	}

	return retObj;
}

Value getbetrandomdata(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0) {
		string msg = "getbetrandomdata \"addr\" showtxdetail\n"
				"\nlistaddrtx\n"
				"\nArguments:\n"
				"\nResult:\n"
				"\"txhash\"\n"
				"\nExamples:\n" + HelpExampleCli("getbetrandomdata", "") + "\nAs json rpc call\n"
				+ HelpExampleRpc("getbetrandomdata", "");
		throw runtime_error(msg);
	}

	unsigned char szDataA[5] = {0};
	unsigned char szDataB[5] = {0};
	size_t nSize = sizeof(szDataA)/sizeof(szDataA[0]);
	RAND_bytes(szDataA, nSize);
	RAND_bytes(szDataB, nSize);
	uint256 hashA = Hash(szDataA,szDataA+nSize);
	uint256 hashB = Hash(szDataB,szDataB+nSize);

	Object retObj;
	string strDataA = strprintf("%d,%d,%d,%d,%d",static_cast<int>(szDataA[0]),static_cast<int>(szDataA[1]),
			static_cast<int>(szDataA[2]),static_cast<int>(szDataA[3]),static_cast<int>(szDataA[4]));
	string strDataB = strprintf("%d,%d,%d,%d,%d",static_cast<int>(szDataB[0]),static_cast<int>(szDataB[1]),
				static_cast<int>(szDataB[2]),static_cast<int>(szDataB[3]),static_cast<int>(szDataB[4]));
	retObj.push_back(Pair("dataA", strDataA));
	retObj.push_back(Pair("HashA", hashA.ToString()));
	retObj.push_back(Pair("dataB", strDataB));
	retObj.push_back(Pair("HashB", hashB.ToString()));
	return retObj;
}

Value printblokdbinfo(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0) {
		string msg = "registerscripttx nrequired \"addr\" \"script\" fee height\n"
				"\nregister script\n"
				"\nArguments:\n"
				"\nExamples:\n" + HelpExampleCli("printblokdbinfo", "") + "\nAs json rpc call\n"
				+ HelpExampleRpc("printblokdbinfo", "");
		throw runtime_error(msg);
	}

	if (!pAccountViewTip->Flush())
		throw runtime_error("Failed to write to account database\n");
	if (!pScriptDBTip->Flush())
		throw runtime_error("Failed to write to account database\n");
	WriteBlockLog(false);
	return Value::null;
}
