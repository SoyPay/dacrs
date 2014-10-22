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
#include "wallet.h"
#include "walletdb.h"

#include "miner.h"

#include <stdint.h>

#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"

using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;

static boost::thread_group sThreadGroup;

string RegIDToAddress(vector<unsigned char> vRegID) {
	if (vRegID.size() == 6) {
		CAccountViewCache view(*pAccountViewTip, true);
		CSecureAccount secureAcc;

		uint64_t balance = 0;
		if (view.GetAccount(vRegID, secureAcc)) {
			CAccountID accid(secureAcc.keyID,vRegID);
			return CBitcoinAddress(accid).ToString();
		}
	} else if (vRegID.size() == 20) {
		CKeyID keyid;
		memcpy(keyid.begin(),&vRegID[0],20);
		return CBitcoinAddress(keyid).ToString();
	}
	return HexStr(vRegID);
}

Object TxToJSON(CBaseTransaction *pTx) {
	Object result;
	result.push_back(Pair("hash", pTx->GetHash().GetHex()));
	switch (pTx->nTxType) {
	case REG_ACCT_TX: {
		CRegisterAccountTx *prtx = (CRegisterAccountTx *) pTx;
		result.push_back(Pair("txtype", "RegisterAccTx"));
		result.push_back(Pair("ver", prtx->nVersion));
		result.push_back(Pair("addr",CBitcoinAddress(prtx->pubKey.GetID()).ToString()));
		result.push_back(Pair("pubkey", HexStr(prtx->pubKey.begin(), prtx->pubKey.end())));
		result.push_back(Pair("fees", prtx->llFees));
		result.push_back(Pair("height", prtx->nValidHeight));
		break;
	}
	case NORMAL_TX: {
		CTransaction *prtx = (CTransaction *) pTx;
		result.push_back(Pair("txtype", "NormalTx"));
		result.push_back(Pair("ver", prtx->nVersion));
		result.push_back(Pair("srcaddr", RegIDToAddress(prtx->srcRegAccountId)));
		result.push_back(Pair("desaddr", RegIDToAddress(prtx->desRegAccountId)));
		result.push_back(Pair("money", prtx->llValues));
		result.push_back(Pair("fees", prtx->llFees));
		result.push_back(Pair("height", prtx->nValidHeight));
		break;
	}
		break;
	case APPEAL_TX: {
		CAppealTransaction *prtx = (CAppealTransaction *) pTx;
		result.push_back(Pair("txtype", "AppealTx"));
		result.push_back(Pair("ver", prtx->nVersion));
//		result.push_back(Pair("addr", RegIDToAddress(prtx->accountId)));
		result.push_back(Pair("securetxhash", prtx->preTxHash.GetHex()));
		result.push_back(Pair("fees", prtx->llFees));
		result.push_back(Pair("contract", HexStr(prtx->vContract)));
		break;
	}
	case SECURE_TX: {
		CSecureTransaction *prtx = (CSecureTransaction *) pTx;
		result.push_back(Pair("txtype", "SecureTx"));
		result.push_back(Pair("ver", prtx->nVersion));
		result.push_back(Pair("script id", HexStr(prtx->regScriptId)));
		{
			Array array;
			for (auto &vid : prtx->vArbitratorRegAccId ) {
				array.push_back(RegIDToAddress(vid));
			}
			result.push_back(Pair("obid", array));
		}
		{
			Array array;
			for (auto &vid : prtx->vRegAccountId ) {
				array.push_back(RegIDToAddress(vid));
			}
			result.push_back(Pair("accountid", array));
		}
		result.push_back(Pair("fees", prtx->llFees));
		result.push_back(Pair("contract", HexStr(prtx->vContract)));
		result.push_back(Pair("height", prtx->nValidHeight));
		break;
	}
	case FREEZE_TX: {
		CFreezeTransaction *prtx = (CFreezeTransaction *) pTx;
		result.push_back(Pair("txtype", "FreezeTx"));
		result.push_back(Pair("ver", prtx->nVersion));
		result.push_back(Pair("addr", RegIDToAddress(prtx->regAccountId)));
		result.push_back(Pair("frozen amt", prtx->llFreezeFunds));
		result.push_back(Pair("fees", prtx->llFees));
		result.push_back(Pair("height", prtx->nValidHeight));
		result.push_back(Pair("free height", prtx->nUnfreezeHeight));
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
		CRegistScriptTx *prtx = (CRegistScriptTx *) pTx;
		result.push_back(Pair("txtype", "RegScriptTx"));
		result.push_back(Pair("ver", prtx->nVersion));
		result.push_back(Pair("addr", RegIDToAddress(prtx->regAccountId)));
		result.push_back(Pair("new", prtx->nFlag));
		result.push_back(Pair("script", HexStr(prtx->script)));
		result.push_back(Pair("fees", prtx->llFees));
		result.push_back(Pair("height", prtx->nValidHeight));
		break;
	}
	default:
		break;
	}
	return result;
}

Value registersecuretx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 3) {
		string msg = "registersecuretx nrequired \"bitcoin addr\" fee height\n"
				     "\nregister secure account\n"
					 "\nArguments:\n"
					 "1.\"bitcoin addr\": (string)\n"
					 "2.fee: (numeric) pay to miner\n"
					 "3.height: (numeric)create height\n"
					 "\nResult:\n"
					 "\"txhash\": (string)\n"
					 "\nExamples:\n" +
					 HelpExampleCli("registersecuretx", "n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj 100000 1")+ "\nAs json rpc call\n" +
					 HelpExampleRpc("registersecuretx", "n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj 100000 1");
		throw runtime_error(msg);
	}

	RPCTypeCheck(params, list_of(str_type)(int_type)(int_type));

	//get addresss
	CBitcoinAddress address(params[0].get_str());
	uint64_t fee = params[1].get_uint64();
	uint32_t nvalidheight = params[2].get_int();

	//get keyid
	CKeyID keyid;
	if (!address.GetKeyID(keyid)) {
		throw runtime_error("in registersecuretx :address err\n");
	}

	CRegisterAccountTx rtx;
	assert(pwalletMain != NULL);
	{
		LOCK2(cs_main, pwalletMain->cs_wallet);
		EnsureWalletIsUnlocked();

		//balance
		CAccountViewCache view(*pAccountViewTip, true);
		CSecureAccount secureAcc;

		uint64_t balance = 0;
		if (view.GetAccount(keyid, secureAcc)) {
			balance = secureAcc.GetBalance(chainActive.Tip()->nHeight);
		}

		if (secureAcc.IsRegister()) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registersecuretx Error: Account is already registered");
		}
		if (balance < fee) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registersecuretx Error: Account balance is insufficient.");
		}

		//pubkey
		CPubKey pubkey;
		if (!pwalletMain->GetPubKey(keyid, pubkey)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registersecuretx Error: not find key.");
		}

		rtx.pubKey = pubkey;
		rtx.llFees = fee;
		rtx.nValidHeight = nvalidheight;

		//sign
		CKey key;
		pwalletMain->GetKey(keyid, key);
		if (!key.Sign(rtx.SignatureHash(), rtx.signature)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registersecuretx Error: Sign failed.");
		}
		if (!pwalletMain->CommitTransaction((CBaseTransaction *) &rtx)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "registersecuretx Error: CommitTransaction failed.");
		}
	}
	return rtx.GetHash().ToString();
}

Value createnormaltx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 5) {
		string msg = "createnormaltx nrequired \"send addr\" \"recv addr\" money fee height\n"
					 "\nsend money from one addr to another addr\n"
					 "\nArguments:\n"
					 "1.\"send addr\": (string)\n"
				     "2.\"recv addr\": (string)\n"
				     "3.money: (numeric) pay to recv addr\n"
					 "4.fee: (numeric) pay to miner\n"
					 "5.height: (numeric)create height\n"
					 "\nResult:\n"
					 "\"txhash\": (string)\n"
					 "\nExamples:\n" +
					 HelpExampleCli("createnormaltx", "5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj 20000000000 100000 1")+
					 "\nAs json rpc call\n" +
					 HelpExampleRpc("createnormaltx", "5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj 20000000000 100000 1");
		throw runtime_error(msg);
	}

	RPCTypeCheck(params, list_of(str_type)(str_type)(int_type)(int_type)(int_type));

	//get addresss
	CBitcoinAddress sendaddr(params[0].get_str());
	CBitcoinAddress recvaddr(params[1].get_str());
	uint64_t money = params[2].get_uint64();
	uint64_t fee = params[3].get_uint64();
	uint32_t nvalidheight = params[4].get_int();

	if (money < CTransaction::nMinTxFee) {
		throw runtime_error("in createnormaltx :money is smaller than nMinTxFee\n");
	}
	//get keyid
	CKeyID keyid;
	if (!sendaddr.GetKeyID(keyid)) {
		throw runtime_error("in createnormaltx :send address err\n");
	}
	CKeyID recvkeyid;
	if (!recvaddr.GetKeyID(recvkeyid)) {
		throw runtime_error("in createnormaltx :recv address err\n");
	}

	assert(pwalletMain != NULL);
	CTransaction tx;
	{
		LOCK2(cs_main, pwalletMain->cs_wallet);
		EnsureWalletIsUnlocked();

		//balance
		CAccountViewCache view(*pAccountViewTip, true);
		CSecureAccount secureAcc;

		uint64_t balance = 0;
		if (view.GetAccount(keyid, secureAcc)) {
			balance = secureAcc.GetBalance(chainActive.Tip()->nHeight);
		}

		if (!secureAcc.IsRegister()) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in createnormaltx Error: Account is not registered.");
		}

		if (!pwalletMain->mapKeyRegID.count(keyid)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in createnormaltx Error: WALLET file is not correct.");
		}

		if (balance < fee || balance < money || balance < (money + fee)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in createnormaltx Error: Account balance is insufficient.");
		}

		vector<unsigned char> vregid;
		if (recvaddr.GetRegID(vregid)) {
			if (!view.GetAccount(vregid, secureAcc)) {
				vregid.clear();
				vregid.insert(vregid.end(), recvkeyid.begin(), recvkeyid.end());
			}
		} else {
			vregid.insert(vregid.end(), recvkeyid.begin(), recvkeyid.end());
		}

		tx.srcRegAccountId = pwalletMain->mapKeyRegID[keyid].vRegID;
		tx.desRegAccountId = vregid;
		tx.llValues = money;
		tx.llFees = fee;
		tx.nValidHeight = nvalidheight;

		CKey key;
		pwalletMain->GetKey(keyid, key);
		if (!key.Sign(tx.SignatureHash(), tx.signature)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "createnormaltx Error: Sign failed.");
		}
		if (!pwalletMain->CommitTransaction((CBaseTransaction *) &tx)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "createnormaltx Error: CommitTransaction failed.");
		}
	}

	return tx.GetHash().ToString();
}

Value createappealtx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 4) {
		string msg = "createappealtx nrequired \"contract addrs index\" \"txhash\" \"contract\" fee\n"
					 "\nappeal contract\n"
					 "\nArguments:\n"
					 "1.\"contract addrs index\": (string) \"0000102\"\n"
					 "2.\"txhash\": (string) pre contract txhash\n"
					 "3.\"contract\":(string) contract content\n"
					 "4.fee: (numeric) pay to miner\n"
					 "\nResult:\n"
					 "\"appeal tx str\": (string)\n"
					 "\nExamples:\n" +
					 HelpExampleCli("createappealtx", "0001 a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a 01020304 100000")+
					 "\nAs json rpc call\n" +
					 HelpExampleRpc("createappealtx", "0001 a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a 01020304 100000");
		throw runtime_error(msg);
	}

	RPCTypeCheck(params, list_of(str_type)(str_type)(str_type)(int_type));

	//get addresss
	vector<unsigned char> vaccindex = ParseHex(params[0].get_str());
	uint256 txhash(params[1].get_str());
	vector<unsigned char> vcontract = ParseHex(params[2].get_str());
	uint64_t fee = params[3].get_uint64();

	if (fee > 0 && fee < CTransaction::nMinTxFee) {
		throw runtime_error("in createappealtx :fee is smaller than nMinTxFee\n");
	}

	assert(pwalletMain != NULL);
	CAppealTransaction tx;
	{
		LOCK2(cs_main, pwalletMain->cs_wallet);
		EnsureWalletIsUnlocked();

		//balance
		CAccountViewCache view(*pAccountViewTip, true);
		CSecureAccount secureAcc;

		tx.vPreAcountIndex = vaccindex;
		tx.preTxHash = txhash;
		tx.vContract = vcontract;
		tx.llFees = fee;
		tx.signature.clear();
	}

	{
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		ds << tx;
		return HexStr(ds.begin(), ds.end());
	}
}

Value signappealtx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		string msg = "signappealtx nrequired \"appeal tx str\"\n"
					 "\nsign \"appeal tx str\"\n"
					 "\nArguments:\n"
					 "1.\"appeal tx str\": (string) \n"
					 "\nResult:\n"
					 "\"appeal tx str\" or \"txhash\": (string) after the last one sign the tx will be commint and return \"txhash\"\n"
					 "\nExamples:\n" +
					 HelpExampleCli("signappealtx", "0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000")+
					 "\nAs json rpc call\n" +
					 HelpExampleRpc("signappealtx", "0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000");
		throw runtime_error(msg);
	}

	vector<unsigned char> vch(ParseHex(params[0].get_str()));
	CDataStream stream(vch, SER_DISK, CLIENT_VERSION);

	CAppealTransaction tx;
	stream >> tx;

	assert(pwalletMain != NULL);
	{
		LOCK2(cs_main, pwalletMain->cs_wallet);
		EnsureWalletIsUnlocked();

		if (tx.vPreAcountIndex.size() <= tx.signature.size() ) {
			throw runtime_error("in signsecuretx :tx data err\n");
		}

		std::shared_ptr<CBaseTransaction> securetx;
		if(pwalletMain->GetTx(tx.preTxHash,securetx))
		{
			if(securetx->nTxType != SECURE_TX)
			{
				throw runtime_error("in signappealtx :pre tx type err tx hash:"+tx.preTxHash.GetHex()+"\n");
			}
		}
		else
		{
			throw runtime_error("in signappealtx :can not find pre tx hash:"+tx.preTxHash.GetHex()+"\n");
		}

		CSecureTransaction *ptx = (CSecureTransaction *)securetx.get();
		for(auto &index:tx.vPreAcountIndex)
		{
			if(index >= (ptx->vArbitratorRegAccId.size()+ptx->vRegAccountId.size()))
			{
				throw runtime_error("in signappealtx :index err\n");
			}
		}

		CAccountViewCache view(*pAccountViewTip, true);
		CSecureAccount secureAcc;

		uint256 signhash = tx.SignatureHash();
		int i = 0;
		for(auto &index:tx.vPreAcountIndex) {
			vector<unsigned char> vregid;
			if(index < ptx->vArbitratorRegAccId.size())
			{
				vregid = ptx->vArbitratorRegAccId[index];
			}
			else
			{
				vregid = ptx->vRegAccountId[index-ptx->vArbitratorRegAccId.size()];
			}

			if (!view.GetAccount(vregid, secureAcc)) {
				throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("unregister RegID: ") + HexStr(vregid));
			}
			if (i < tx.signature.size()) {
				if (!secureAcc.publicKey.Verify(signhash, tx.signature[i])) {
					throw runtime_error("in signappealtx :tx data sign err\n");
				}
			}
			i++;
		}

		i = tx.vPreAcountIndex[tx.signature.size()];
		vector<unsigned char> vregid;
		if (i < ptx->vArbitratorRegAccId.size()) {
			vregid = ptx->vArbitratorRegAccId[i];
		} else {
			vregid = ptx->vRegAccountId[i - ptx->vArbitratorRegAccId.size()];
		}
		view.GetAccount(vregid, secureAcc);
		CKey key;
		LogPrint("INFO","appeal signature key:%s\n", secureAcc.keyID.GetHex());
		if (!pwalletMain->GetKey(secureAcc.keyID, key)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "signappealtx Error: it's not your turn to Sign.");
		}

		vector<unsigned char> vsign;
		if (!key.Sign(signhash, vsign)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "signappealtx Error: Sign failed.");
		}
		LogPrint("INFO","appeal signature :%s\n", HexStr(vsign.begin(), vsign.end()));
		tx.signature.push_back(vsign);

		if (tx.vPreAcountIndex.size() == tx.signature.size()) {
			if (!pwalletMain->CommitTransaction((CBaseTransaction *) &tx)) {
				throw JSONRPCError(RPC_WALLET_ERROR, "signappealtx Error: CommitTransaction failed.");
			}
			return tx.GetHash().ToString();
		}

	}

	{
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		ds << tx;
		return HexStr(ds.begin(), ds.end());
	}
}

Value createfreezetx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 5) {
		string msg = "createfreezetx nrequired \"addr\" frozenmoney fee height freeheight\n"
				     "\nfrozen some money\n"
					 "\nArguments:\n"
					 "1.\"addr\": (string)\n"
					 "2.frozenmoney: (numeric)\n"
					 "3.fee: (numeric) pay to miner\n"
					 "4.height: (numeric)create height\n"
					 "5.freeheight: (numeric)frozenmoney free height\n"
					 "\nResult:\n"
					 "\"txhash\": (string)\n"
					 "\nExamples:\n" +
					 HelpExampleCli("createfreezetx", "5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG 20000000000 100000 1 101")+
					 "\nAs json rpc call\n" +
					 HelpExampleRpc("createfreezetx", "5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG 20000000000 100000 1 101");
		throw runtime_error(msg);
	}

	RPCTypeCheck(params, list_of(str_type)(int_type)(int_type)(int_type)(int_type));

	//get addresss
	CBitcoinAddress addr(params[0].get_str());
	uint64_t frozenmoney = params[1].get_uint64();
	uint64_t fee = params[2].get_uint64();
	uint32_t height = params[3].get_int();
	uint32_t freeheight = params[4].get_int();

	if (frozenmoney < CTransaction::nMinTxFee) {
		throw runtime_error("in createfreezetx :frozenmoney is smaller than nMinTxFee\n");
	}

	if (fee > 0 && fee < CTransaction::nMinTxFee) {
		throw runtime_error("in createfreezetx :fee is smaller than nMinTxFee\n");
	}
	//get keyid
	CKeyID keyid;
	if (!addr.GetKeyID(keyid)) {
		throw runtime_error("in createfreezetx :send address err\n");
	}

	assert(pwalletMain != NULL);
	CFreezeTransaction tx;
	{
		LOCK2(cs_main, pwalletMain->cs_wallet);
		EnsureWalletIsUnlocked();

		if (freeheight < chainActive.Tip()->nHeight + 2) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in createfreezetx Error: freeheight is invalid.");
		}
		//balance
		CAccountViewCache view(*pAccountViewTip, true);
		CSecureAccount secureAcc;

		uint64_t balance = 0;
		if (view.GetAccount(keyid, secureAcc)) {
			balance = secureAcc.GetBalance(chainActive.Tip()->nHeight);
		}

		if (!secureAcc.IsRegister()) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in createfreezetx Error: Account is not registered.");
		}

		if (!pwalletMain->mapKeyRegID.count(keyid)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in createfreezetx Error: WALLET file is not correct.");
		}

		if (balance < fee || balance < frozenmoney || balance < (frozenmoney + fee)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in createfreezetx Error: Account balance is insufficient.");
		}

		tx.regAccountId = pwalletMain->mapKeyRegID[keyid].vRegID;
		tx.llFreezeFunds = frozenmoney;
		tx.llFees = fee;
		tx.nValidHeight = height;
		tx.nUnfreezeHeight = freeheight;

		CKey key;
		pwalletMain->GetKey(keyid, key);
		if (!key.Sign(tx.SignatureHash(), tx.signature)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "createfreezetx Error: Sign failed.");
		}
		if (!pwalletMain->CommitTransaction((CBaseTransaction *) &tx)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "createfreezetx Error: CommitTransaction failed.");
		}
	}

	return tx.GetHash().ToString();
}

Value registerscripttx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 4) {
		string msg = "registerscripttx nrequired \"addr\" \"script\" fee height\n"
					 "\nregister script\n"
					 "\nArguments:\n"
					 "1.\"addr\": (string)\n"
				     "2.\"script\": (string)\n"
					 "3.fee: (numeric) pay to miner\n"
					 "4.height: (numeric)create height\n"
					 "\nResult:\n"
					 "\"txhash\": (string)\n"
					 "\nExamples:\n" +
					 HelpExampleCli("registerscripttx", "5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG 01020304 100000 1")+
					 "\nAs json rpc call\n" +
					 HelpExampleRpc("registerscripttx", "5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG 01020304 100000 1");
		throw runtime_error(msg);
	}

	RPCTypeCheck(params, list_of(str_type)(str_type)(int_type)(int_type));

	//get addresss
	CBitcoinAddress addr(params[0].get_str());
	vector<unsigned char> vscript = ParseHex(params[1].get_str());
	uint64_t fee = params[2].get_uint64();
	uint32_t height = params[3].get_int();

	if (fee > 0 && fee < CTransaction::nMinTxFee) {
		throw runtime_error("in registerscripttx :fee is smaller than nMinTxFee\n");
	}
	//get keyid
	CKeyID keyid;
	if (!addr.GetKeyID(keyid)) {
		throw runtime_error("in registerscripttx :send address err\n");
	}

	assert(pwalletMain != NULL);
	CRegistScriptTx tx;
	{
		LOCK2(cs_main, pwalletMain->cs_wallet);
		EnsureWalletIsUnlocked();

		//balance
		CAccountViewCache view(*pAccountViewTip, true);
		CSecureAccount secureAcc;

		uint64_t balance = 0;
		if (view.GetAccount(keyid, secureAcc)) {
			balance = secureAcc.GetBalance(chainActive.Tip()->nHeight);
		}

		if (!secureAcc.IsRegister()) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registerscripttx Error: Account is not registered.");
		}

		if (!pwalletMain->mapKeyRegID.count(keyid)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registerscripttx Error: WALLET file is not correct.");
		}

		if (balance < fee) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registerscripttx Error: Account balance is insufficient.");
		}

		tx.regAccountId = pwalletMain->mapKeyRegID[keyid].vRegID;
		tx.script = vscript;
		tx.nFlag = 1;
		tx.llFees = fee;
		tx.nValidHeight = height;

		vector<unsigned char> vscriptcontent;
		if (pScriptDB->GetScript(vscript, vscriptcontent)) {
			tx.nFlag = 0;
		}

		CKey key;
		pwalletMain->GetKey(keyid, key);
		if (!key.Sign(tx.SignatureHash(), tx.signature)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "registerscripttx Error: Sign failed.");
		}
		if (!pwalletMain->CommitTransaction((CBaseTransaction *) &tx)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "registerscripttx Error: CommitTransaction failed.");
		}
	}

	return tx.GetHash().ToString();
}

Value createsecuretx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 6) {
		string msg = "createsecuretx nrequired \"scriptid\" [\"obaddr\",...] [\"addr\",...] \"contract\" fee height\n"
					 "\ncreate contract\n"
					 "\nArguments:\n"
					 "1.\"scriptid\": (string)\n"
					 "2.[\"obaddr\",...]: (string list)\n"
					 "3.[\"addr\",...]: (string list)\n"
					 "4.\"contract\": (string)"
					 "5.fee: (numeric) pay to miner\n"
					 "6.height: (numeric)create height\n"
					 "\nResult:\n"
					 "\"contract tx str\": (string)\n"
					 "\nExamples:\n" +
					 HelpExampleCli("createsecuretx", "000000000100 [\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"] [\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\",\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\"] 01020304 100000 1")+
					 "\nAs json rpc call\n" +
					 HelpExampleRpc("createsecuretx", "000000000100 [\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"] [\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\",\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\"] 01020304 100000 1");
		throw runtime_error(msg);
	}

	RPCTypeCheck(params, list_of(str_type)(array_type)(array_type)(str_type)(int_type)(int_type));

	//get addresss
	vector<unsigned char> vscriptid = ParseHex(params[0].get_str());
	Array obaddrs = params[1].get_array();
	Array addrs = params[2].get_array();
	vector<unsigned char> vcontract = ParseHex(params[3].get_str());
	uint64_t fee = params[4].get_uint64();
	uint32_t height = params[5].get_int();

	if (fee > 0 && fee < CTransaction::nMinTxFee) {
		throw runtime_error("in createsecuretx :fee is smaller than nMinTxFee\n");
	}
	//get keyid

	assert(pwalletMain != NULL);
	CSecureTransaction tx;
	{
		LOCK2(cs_main, pwalletMain->cs_wallet);
		EnsureWalletIsUnlocked();

		//balance
		CAccountViewCache view(*pAccountViewTip, true);
		CSecureAccount secureAcc;

		vector<unsigned char> vscript;
		set<string> sob;
		if (!pContractScriptTip->GetScript(HexStr(vscriptid), vscript)) {
			throw runtime_error(tinyformat::format("in createsecuretx :script id %s is not exist\n", HexStr(tx.regScriptId)));
		}

		if (!pContractScriptTip->GetArbitrator(HexStr(vscriptid), sob)) {
			throw runtime_error(tinyformat::format("in createsecuretx :script id %s ob is not exist\n", HexStr(tx.regScriptId)));
		}

		vector<vector_unsigned_char> vvobregid;
		for (auto& obaddr : obaddrs) {
			CBitcoinAddress address(obaddr.get_str());
			vector<unsigned char> vregid;
			if (!address.GetRegID(vregid))
				throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("Invalid address: ") + obaddr.get_str());
			LogPrint("INFO","regid = %s \r\n",HexStr(vregid).c_str());
			if (!view.GetAccount(vregid, secureAcc)) {
				throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("unregister address: ") + obaddr.get_str());
			}

			if (sob.end() != sob.find(HexStr(vregid))) {
				vvobregid.push_back(vregid);
			} else {
				throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,
						string("unregister script ob address: ") + obaddr.get_str());
			}
		}

		vector<vector_unsigned_char> vvregid;
		for (auto& addr : addrs) {
			CBitcoinAddress address(addr.get_str());
			vector<unsigned char> vregid;
			if (!address.GetRegID(vregid))
				throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("Invalid address: ") + addr.get_str());
			LogPrint("INFO","regid = %s \r\n",HexStr(vregid).c_str());
			CKeyID tempkeyid;
			address.GetKeyID(tempkeyid);
			LogPrint("INFO","addr = %s \r\n",tempkeyid.GetHex().c_str());
			LogPrint("INFO","CBitcoinAddress = %s \r\n",CBitcoinAddress(CAccountID(tempkeyid,vregid)).ToString().c_str());
			if (!view.GetAccount(vregid, secureAcc)) {
				throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("unregister address: ") + addr.get_str());
			}
			vvregid.push_back(vregid);
		}

		tx.regScriptId = vscriptid;
		tx.vArbitratorRegAccId = vvobregid;
		tx.vRegAccountId = vvregid;
		tx.vContract = vcontract;
		tx.llFees = fee;
		tx.nValidHeight = height;
		tx.vScripts.clear();
	}
	{
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		ds<<tx;
		return HexStr(ds.begin(),ds.end());
	}
}

Value signsecuretx(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		string msg = "signsecuretx nrequired \"contract tx str\"\n"
					 "\nsign \"contract tx str\"\n"
					 "\nArguments:\n"
					 "1.\"contract tx str\": (string) \n"
					 "\nResult:\n"
					 "\"contract tx str\" or \"txhash\": (string) after the last one sign the tx will be commint and return \"txhash\"\n"
					 "\nExamples:\n" +
					 HelpExampleCli("signsecuretx", "0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000")+
					 "\nAs json rpc call\n" +
					 HelpExampleRpc("signsecuretx", "0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000");
		throw runtime_error(msg);
	}
	LogPrint("INFO","signsecuretx enter\r\n");
	vector<unsigned char> vch(ParseHex(params[0].get_str()));
	CDataStream stream(vch, SER_DISK, CLIENT_VERSION);

	CSecureTransaction tx;
	stream >> tx;

	assert(pwalletMain != NULL);
	{
		LOCK2(cs_main, pwalletMain->cs_wallet);
		EnsureWalletIsUnlocked();

		//balance
		CAccountViewCache view(*pAccountViewTip, true);
		CSecureAccount secureAcc;

		if (tx.vRegAccountId.size() <= tx.vScripts.size() || tx.vRegAccountId.size() < 2) {
			throw runtime_error("in signsecuretx :tx data err\n");
		}

		vector<unsigned char> vscript;
		set<string> sob;
		if (!pContractScriptTip->GetScript(HexStr(tx.regScriptId), vscript)) {
			throw runtime_error(tinyformat::format("in signsecuretx :script id %s is not exist\n", HexStr(tx.regScriptId)));
		}

		if (!pContractScriptTip->GetArbitrator(HexStr(tx.regScriptId), sob)) {
			throw runtime_error(tinyformat::format("in signsecuretx :script id %s ob is not exist\n", HexStr(tx.regScriptId)));
		}

		for (auto& vregid : tx.vArbitratorRegAccId) {
			if (!view.GetAccount(vregid, secureAcc)) {
				throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("unregister RegID: ") + HexStr(vregid));
			}

			if (sob.end() == sob.find(HexStr(vregid))) {
				throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("unregister script ob RegID: ") + HexStr(vregid));
			}
		}

		uint256 signhash = tx.SignatureHash();
		int i = 0;
		for (auto& vregid1 : tx.vRegAccountId) {
			if (!view.GetAccount(vregid1, secureAcc)) {
				throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("unregister RegID: ") + HexStr(vregid1));
			}
			if (i < tx.vScripts.size()) {
				if (!secureAcc.publicKey.Verify(signhash, tx.vScripts[i])) {
					throw runtime_error("in signsecuretx :tx data sign err\n");
				}
			}
			i++;
		}

		i = tx.vScripts.size();
		view.GetAccount(tx.vRegAccountId[i], secureAcc);
		CKey key;
		if (!pwalletMain->GetKey(secureAcc.keyID, key)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "signsecuretx Error: it's not your turn to Sign.");
		}

		vector<unsigned char> vsign;
		if (!key.Sign(signhash, vsign)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "signsecuretx Error: Sign failed.");
		}
		tx.vScripts.push_back(vsign);

		if (tx.vRegAccountId.size() == tx.vScripts.size()) {
			if (!pwalletMain->CommitTransaction((CBaseTransaction *) &tx)) {
				throw JSONRPCError(RPC_WALLET_ERROR, "signsecuretx Error: CommitTransaction failed.");
			}
			LogPrint("INFO","signsecuretx end\r\n");
			return tx.GetHash().ToString();
		}
	}

	{
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		ds << tx;
		LogPrint("INFO","signsecuretx end\r\n");
		return HexStr(ds.begin(), ds.end());
	}
}

Value listaddr(const Array& params, bool fHelp) {
	if (fHelp || params.size() > 0) {
		string msg = "listaddr \n"
					 "\nlistaddr\n"
					 "\nArguments:\n"
					 "\nResult:\n"
					 "addr balance register\n"
					 "\nExamples:\n" +
					 HelpExampleCli("listaddr","\n")+
					 "\nAs json rpc call\n" +
					 HelpExampleRpc("listaddr","\n");
		throw runtime_error(msg);
	}

	assert(pwalletMain != NULL);
	string str;
	{
		LOCK2(cs_main, pwalletMain->cs_wallet);

		set<CKeyID> setKeyID;
		str.clear();

		char buf[128] = { 0 };
		sprintf(buf, "%-45.45s%-25.25s%-10.10s\r\n", "addr", "balance", "register");
		str += buf;

		pwalletMain->GetKeys(setKeyID); //get addrs
		if (setKeyID.empty()) {
			return str;
		}

		CAccountViewCache accView(*pAccountViewTip, true);
		for (const auto &keyid : setKeyID) {
			//find CSecureAccount info by keyid
			bool bReg = false;
			double dbalance = 0.0;
			CSecureAccount secureAcc;
			ostringstream ostr;
			if (accView.GetAccount(keyid, secureAcc)) {
				bReg = secureAcc.IsRegister();
				dbalance = (double) secureAcc.GetBalance(chainActive.Tip()->nHeight+100) / (double) COIN;
			}

			if (bReg && !pwalletMain->mapKeyRegID.count(keyid)) {
				throw JSONRPCError(RPC_WALLET_ERROR, "listaddrinfo Error: WALLET file is not correct.");
			}

			ostr.str("");
			ostr << boolalpha << bReg;

			char buf[128] = { 0 };

			CTxDestination destid;
			if (bReg) {
				destid = CAccountID(keyid, pwalletMain->mapKeyRegID[keyid].vRegID);
			} else {
				destid = keyid;
			}
			sprintf(buf, "%-45.45s%-25.8lf%-10.10s\r\n", CBitcoinAddress(destid).ToString().c_str(), dbalance,
					ostr.rdbuf()->str().c_str());
			str += buf;
		}
	}
	return str;
}

Value listaddrtx(const Array& params, bool fHelp) {
	if (fHelp || params.size() > 2) {
		string msg = "listaddrtx \"addr\" showtxdetail\n"
					 "\listaddrtx\n"
					 "\nArguments:\n"
				     "1.\"addr\": (string)"
					 "2.showtxdetail: (optional,default false)"
					 "\nResult:\n"
					 "\"txhash\"\n"
					 "\nExamples:\n" +
					 HelpExampleCli("listaddrtx","5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\n")+
					 "\nAs json rpc call\n" +
					 HelpExampleRpc("listaddrtx","5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\n");
		throw runtime_error(msg);
	}

	//get addresss
	CBitcoinAddress address(params[0].get_str());

	//get keyid
	CKeyID keyid;
	if (!address.GetKeyID(keyid)) {
		throw runtime_error("in registersecuretx :address err\n");
	}

	bool bshowdetail = 0;
	if (params.size() > 1) {
		bshowdetail = params[1].get_bool();
	}

	Array array;
	assert(pwalletMain != NULL);
	{
		LOCK2(cs_main, pwalletMain->cs_wallet);

		if (!pwalletMain->HaveKey(keyid)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registersecuretx Error: not find key.");
		}

		for (auto&wtx : pwalletMain->mapWalletTx) {
			CAccountTx &acctx = wtx.second;
			for (auto&item : acctx.mapAccountTx) {
				vector<CKeyID> vKey;
				item.second->GetAddress(vKey, *pAccountViewTip);
				if (vKey.end() != find(vKey.begin(), vKey.end(), keyid)) {
					array.push_back(item.first.ToString());
					if (bshowdetail) {
						Object obj;
						obj = TxToJSON(item.second.get());
						obj.push_back(Pair("confirm block", acctx.blockHash.GetHex()));
						array.push_back(obj);
					}
				}
			}
		}
	}
	return array;
}

Value getaddramount(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		string msg = "getaddramount \"addr\"\n"
					 "\getaddramount\n"
					 "\nArguments:\n"
					 "1.\"addr\": (string)"
					 "\nResult:\n"
					 "\"mature amount\":\n"
					 "\"free amount\":\n"
					 "\"frozen amount\":\n"
					 "\nExamples:\n" +
					 HelpExampleCli("getaddramount","5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\n")+
					 "\nAs json rpc call\n" +
					 HelpExampleRpc("getaddramount","5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\n");
		throw runtime_error(msg);
	}

	CBitcoinAddress address(params[0].get_str());

	CKeyID keyid;
	if (!address.GetKeyID(keyid))
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Bitcoin address");

	Object obj;
	{
		LOCK(cs_main);

		CSecureAccount secureAcc;
		CAccountViewCache accView(*pAccountViewTip, true);
		if (accView.GetAccount(keyid, secureAcc))
		{
			int curheight = chainActive.Tip()->nHeight;
			double dbalance = (double) secureAcc.GetBalance(curheight) / (double) COIN;
			double dmature = (double) secureAcc.GetMatureAmount(curheight) / (double) COIN;
			double dfrozen = (double) secureAcc.GetForzenAmount(curheight) / (double) COIN;

			obj.push_back(Pair("mature amount", dmature));
			obj.push_back(Pair("free amount", dbalance));
			obj.push_back(Pair("frozen amount", dfrozen));
		}
	}
	return obj;
}

Value getaddrfrozendetail(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		string msg = "registersecuretx nrequired [\"key\",...] ( \"account\" )\n"
				"\nAdd a nrequired-to-sign multisignature address to the wallet.\n"
				"Each key is a Bitcoin address or hex-encoded public key.\n" + HelpExampleCli("registersecuretx", "")
				+ "\nAs json rpc call\n" + HelpExampleRpc("registersecuretx", "");
		throw runtime_error(msg);
	}

	CBitcoinAddress address(params[0].get_str());

	CKeyID keyid;
	if (!address.GetKeyID(keyid))
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Bitcoin address");

	LOCK(cs_main);

	Array array;

	CSecureAccount secureAcc;
	CAccountViewCache accView(*pAccountViewTip, true);
	if (accView.GetAccount(keyid, secureAcc)) {

		secureAcc.vInputFreeze.insert(secureAcc.vInputFreeze.end(), secureAcc.vOutputFreeze.begin(),
				secureAcc.vOutputFreeze.end());
		secureAcc.vInputFreeze.insert(secureAcc.vInputFreeze.end(), secureAcc.vSelfFreeze.begin(),
				secureAcc.vSelfFreeze.end());

		for (auto &item : secureAcc.vInputFreeze) {
			Object obj;
			obj.clear();
			obj.push_back(Pair("tx hash:", item.uTxHash.ToString()));
			obj.push_back(Pair("frozen amount:", item.value));
			obj.push_back(Pair("height of timeout:", item.nHeight));
			array.push_back(obj);
		}
	}
	return array;
}

//list unconfirmed transaction of mine
Value listunconfirmedtx(const Array& params, bool fHelp) {
	if (fHelp || params.size() > 1) {
		string msg = "listunconfirmedtx  bshowtxdetail\n"
					 "\listunconfirmedtx\n"
					 "\nArguments:\n"
				     "1.bshowtxdetail: default false\n"
					 "\nResult:\n"
					 "\"txhash\"\n"
					 "\nExamples:\n" +
					 HelpExampleCli("listunconfirmedtx","\n")+
					 "\nAs json rpc call\n" +
					 HelpExampleRpc("listunconfirmedtx","\n");
		throw runtime_error(msg);
	}

	bool bshowdetail = 0;
	if (params.size() > 0) {
		bshowdetail = params[0].get_bool();
	}

	Array array;
	{
		LOCK2(cs_main, pwalletMain->cs_wallet);
		CAccountTx &acctx = pwalletMain->mapWalletTx[uint256(0)];

		for (auto& item : acctx.mapAccountTx) {
			array.push_back(item.first.ToString());
			if (bshowdetail) {
				array.push_back(TxToJSON(item.second.get()));
			}
		}
	}
	return array;
}

Value gettxdetail(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		string msg = "gettxdetail \"txhash\"\n"
					 "\gettxdetail\n"
					 "\nArguments:\n"
					 "1.\"txhash\":"
					 "\nResult:\n"
					 "\"txhash\"\n"
					 "\nExamples:\n" +
					 HelpExampleCli("gettxdetail","c5287324b89793fdf7fa97b6203dfd814b8358cfa31114078ea5981916d7a8ac\n")+
					 "\nAs json rpc call\n" +
					 HelpExampleRpc("gettxdetail","c5287324b89793fdf7fa97b6203dfd814b8358cfa31114078ea5981916d7a8ac\n");
		throw runtime_error(msg);
	}

	uint256 txhash(params[0].get_str());

	LOCK(pwalletMain->cs_wallet);
	Object obj;

	for (auto& wtx : pwalletMain->mapWalletTx) {
		CAccountTx &acctx = wtx.second;
		for (auto&item : acctx.mapAccountTx) {
			if (item.first == txhash) {
				obj = TxToJSON(item.second.get());
				return obj;
			}
		}
	}

	return NULL;
}

//list unconfirmed transaction of mine
Value listscriptregid(const Array& params, bool fHelp) {
	if (fHelp || params.size() > 1) {
		string msg = "listscriptregid \n"
					 "\listscriptregid\n"
					 "\nArguments:\n"
					 "\nResult:\n"
					 "\"txhash\":\"scriptid\"\n"
					 "\nExamples:\n" +
					 HelpExampleCli("listscriptregid","\n")+
					 "\nAs json rpc call\n" +
					 HelpExampleRpc("listscriptregid","\n");
		throw runtime_error(msg);
	}

	bool bshowdetail = 0;
	if (params.size() > 0) {
		bshowdetail = params[0].get_bool();
	}

	Object obj;
	{
		LOCK2(cs_main, pwalletMain->cs_wallet);

		for (auto& item : pwalletMain->mapScriptRegID) {
			obj.push_back(Pair(item.first.ToString(),item.second.ToString()));
			if (bshowdetail) {
				//array.push_back(TxToJSON(item.second.get()));
			}
		}
	}
	return obj;
}

//list regid
Value listregid(const Array& params, bool fHelp) {
	if (fHelp || params.size() > 0) {
		string msg = "listregid \n"
					 "\listregid\n"
					 "\nArguments:\n"
					 "\nResult:\n"
					 "\"addr\":\"regid\"\n"
					 "\nExamples:\n" +
					 HelpExampleCli("listregid","\n")+
					 "\nAs json rpc call\n" +
					 HelpExampleRpc("listregid","\n");
		throw runtime_error(msg);
	}

	Array array;
	{
		LOCK2(cs_main, pwalletMain->cs_wallet);

		for (auto& item : pwalletMain->mapKeyRegID) {
			array.push_back(CBitcoinAddress(CAccountID(item.first,item.second.vRegID)).ToString());
			Object obj;
			obj.clear();
			obj.push_back(Pair(CBitcoinAddress(item.first).ToString(),item.second.ToString()));
			array.push_back(obj);
		}
	}
	return array;
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
					 "\nExamples:\n" +
					 HelpExampleCli("sign", "0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000")+
					 "\nAs json rpc call\n" +
					 HelpExampleRpc("sign", "0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000");
		throw runtime_error(msg);
	}

	CBitcoinAddress addr(params[0].get_str());
	vector<unsigned char> vch(ParseHex(params[1].get_str()));

	//get keyid
	CKeyID keyid;
	if (!addr.GetKeyID(keyid)) {
		throw runtime_error("in sign :send address err\n");
	}
	vector<unsigned char> vsign;
	{
		LOCK(pwalletMain->cs_wallet);

		CKey key;
		if(!pwalletMain->GetKey(keyid, key))
		{
			throw JSONRPCError(RPC_WALLET_ERROR, "sign Error: cannot find key.");
		}

		uint256 hash = Hash(vch.begin(),vch.end());
		if (!key.Sign(hash, vsign)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "sign Error: Sign failed.");
		}
	}
	return HexStr(vsign);
}

Value getaccountinfo(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		throw runtime_error(
		            "getaccountinfo \"address \" dspay address ( \"comment\" \"comment-to\" )\n"
		            "\nGet an account info with dspay address\n"
		            + HelpRequiringPassphrase() +
		            "\nArguments:\n"
		            "1. \"address \"  (string, required) The bitcoin address.\n"
		            "\nResult:\n"
		            "\"account info\"  (string) \n"
		            "\nExamples:\n"
		            + HelpExampleCli("getaccountinfo", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\"")
		            + HelpExampleCli("getaccountinfo", "\"000000010100\"")

		        );
	}
	CAccountViewCache view(*pAccountViewTip, true);
	string strParam =  params[0].get_str();
	CSecureAccount aAccount;
	if(strParam.length() != 12) {
		CBitcoinAddress address(params[0].get_str());
		CKeyID keyid;
		if (!address.GetKeyID(keyid))
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Bitcoin address");

		if (!view.GetAccount(keyid, aAccount)) {
			return "can not get account info by address:" + strParam;
		}
	}
	else {
		if(!view.GetAccount(ParseHex(strParam), aAccount)) {
			return "can not get account info by regid:" + strParam;
		}
	}

	string fundTypeArray[]= {"NULL_FUNDTYPE", "FREEDOM", "REWARD_FUND", "FREEDOM_FUND", "IN_FREEZD_FUND", "OUT_FREEZD_FUND", "SELF_FREEZD_FUND"};

	Object obj;
	obj.push_back(Pair("keyID:", HexStr(aAccount.keyID.begin(), aAccount.keyID.end()).c_str()));
	obj.push_back(Pair("publicKey:", HexStr(aAccount.publicKey.begin(), aAccount.publicKey.end()).c_str()));
	obj.push_back(Pair("llValues:", tinyformat::format("%s", aAccount.llValues)));
	Array array;
	//string str = ("fundtype  txhash                                  value                        height");
	string str = tinyformat::format("%-20.20s%-70.70s%-25.8lf%-6.6d","fundtype", "txhash", "value", "height");
	array.push_back(str);
	for (int i = 0; i < aAccount.vRewardFund.size(); ++i) {
		CFund fund = aAccount.vRewardFund[i];
		array.push_back(tinyformat::format("%-20.20s%-70.70s%-25.8lf%-6.6d",fundTypeArray[fund.nFundType], fund.uTxHash.GetHex(), fund.value, fund.nHeight));
	}

	for(int i=0; i<aAccount.vFreedomFund.size(); ++i) {
		CFund fund = aAccount.vFreedomFund[i];
		array.push_back(tinyformat::format("%-20.20s%-70.70s%-25.8lf%-6.6d",fundTypeArray[fund.nFundType], fund.uTxHash.GetHex(), fund.value, fund.nHeight));
	}
	for(int i=0; i< aAccount.vInputFreeze.size(); ++i) {
		CFund fund = aAccount.vInputFreeze[i];
		array.push_back(tinyformat::format("%-20.20s%-70.70s%-25.8lf%-6.6d",fundTypeArray[fund.nFundType], fund.uTxHash.GetHex(), fund.value, fund.nHeight));
	}
	for(int i=0; i< aAccount.vOutputFreeze.size(); ++i) {
		CFund fund = aAccount.vOutputFreeze[i];
		array.push_back(tinyformat::format("%-20.20s%-70.70s%-25.8lf%-6.6d",fundTypeArray[fund.nFundType], fund.uTxHash.GetHex(), fund.value, fund.nHeight));
	}
	for(int i=0; i< aAccount.vSelfFreeze.size(); ++i) {
		CFund fund = aAccount.vSelfFreeze[i];
		array.push_back(tinyformat::format("%-20.20s%-70.70s%-25.8lf%-6.6d",fundTypeArray[fund.nFundType], fund.uTxHash.GetHex(), fund.value, fund.nHeight));
	}
	obj.push_back(Pair("detailinfo:",array));
	return obj;
}

void ThreadCreateNormalTx(const CKeyID keyid,const vector<unsigned char> vregid,int nTimes)
{
	// Make this thread recognisable as the wallet flushing thread
	RenameThread("create-normaltx");

	uint64_t fee = 1000000;
	uint64_t money = 10000000000;
	uint64_t fee_step = 10;//1000000;
	uint64_t money_step = 10000000;//10000000000;
	int count = 0;
	try {
		LogPrint("INFO","ThreadCreateNormalTx nTimes = %d\r\n",nTimes);
		while (true)
		{
			CTransaction tx;
			{
				LOCK2(cs_main, pwalletMain->cs_wallet);
				EnsureWalletIsUnlocked();

				tx.srcRegAccountId = pwalletMain->mapKeyRegID[keyid].vRegID;
				tx.desRegAccountId = vregid;
				tx.llValues = money;
				tx.llFees = fee;
				tx.nValidHeight = chainActive.Tip()->nHeight;

				CKey key;
				pwalletMain->GetKey(keyid, key);
				if (!key.Sign(tx.SignatureHash(), tx.signature)) {
					throw JSONRPCError(RPC_WALLET_ERROR, "createnormaltx Error: Sign failed.");
				}
				if (!pwalletMain->CommitTransaction((CBaseTransaction *) &tx)) {
					throw JSONRPCError(RPC_WALLET_ERROR, "createnormaltx Error: CommitTransaction failed.");
				}
			}

			LogPrint("INFO","tx hash = %s \r\n",tx.GetHash().ToString().c_str());

			boost::this_thread::interruption_point();
			MilliSleep(200);
			money += money_step;
			fee += fee_step;
			if(nTimes > 0)
			{
				count++;
				if(count >= nTimes)
				{
					LogPrint("INFO","ThreadCreateNormalTx exit1\r\n");
					break;
				}
			}
		}
	}
	catch(...)
	{
		LogPrint("INFO","ThreadCreateNormalTx exit2\r\n");
	}
}

Value testnormaltx(const Array& params, bool fHelp) {
	if (fHelp || params.size() <2 ||  params.size() >3) {
		string msg = "testnormaltx\n" +
					 HelpExampleCli("testnormaltx", "")
					 + "\nAs json rpc call\n" +
					 HelpExampleRpc("testnormaltx", "");
		throw runtime_error(msg);
	}

	RPCTypeCheck(params, list_of(str_type)(str_type));
	int times = 0;
	//get addresss
	CBitcoinAddress sendaddr(params[0].get_str());
	CBitcoinAddress recvaddr(params[1].get_str());
	if(params.size() > 2)
	{
		times = params[2].get_int();
		if(times == 0)
		{
			sThreadGroup.interrupt_all();
			sThreadGroup.join_all();
			return string("test normaltx stop");
		}
	}

	//get keyid
	CKeyID keyid;
	if (!sendaddr.GetKeyID(keyid)) {
		throw runtime_error("in createnormaltx :send address err\n");
	}
	CKeyID recvkeyid;
	if (!recvaddr.GetKeyID(recvkeyid)) {
		throw runtime_error("in createnormaltx :recv address err\n");
	}

	CAccountViewCache view(*pAccountViewTip, true);
	CSecureAccount secureAcc;

	vector<unsigned char> vregid;
	if (recvaddr.GetRegID(vregid)) {
		if (!view.GetAccount(vregid, secureAcc)) {
			vregid.clear();
			vregid.insert(vregid.end(), recvkeyid.begin(), recvkeyid.end());
		}
	} else {
		vregid.insert(vregid.end(), recvkeyid.begin(), recvkeyid.end());
	}
	sThreadGroup.create_thread(boost::bind(&ThreadCreateNormalTx, keyid,vregid,times));

	return NULL;
}

void ThreadTestMiner(int nTimes)
{
	// Make this thread recognisable as the wallet flushing thread
	RenameThread("create-normaltx");

	uint64_t fee = 1000000;
	uint64_t money = 10000000000;
//	uint64_t fee_step = 10;//1000000;
//	uint64_t money_step = 10000000;//10000000000;
	uint64_t min_money = 100000000000;
	int count = 0;
	try {
		LogPrint("INFO","ThreadTestMiner nTimes = %d\r\n",nTimes);
		while (true)
		{
			{
				LOCK2(cs_main, pwalletMain->cs_wallet);
				EnsureWalletIsUnlocked();

				set<CKeyID> setKeyID;
				pwalletMain->GetKeys(setKeyID); //get addrs
				if (setKeyID.empty()) {
					throw runtime_error("ThreadTestMiner :enpty\n");;
				}
				bool bNormal = false;
				CAccountViewCache accView(*pAccountViewTip, true);
				for (const auto &keyid : setKeyID) {
					//find CSecureAccount info by keyid
					bool bReg = false;
					uint64_t balance = 0;
					CSecureAccount secureAcc;
					if (accView.GetAccount(keyid, secureAcc)) {
						bReg = secureAcc.IsRegister();
						balance = secureAcc.GetBalance(chainActive.Tip()->nHeight + 100);
					}

					if (bReg && !pwalletMain->mapKeyRegID.count(keyid)) {
						throw JSONRPCError(RPC_WALLET_ERROR, "listaddrinfo Error: WALLET file is not correct.");
					}
					if(bReg && balance >= min_money*11)
					{
						if(bNormal)
						{
							continue;
						}
						for(int i = 0; i < 10;i++)
						{
							CTransaction tx;
							CPubKey pubkey = pwalletMain->GenerateNewKey();
							tx.srcRegAccountId = pwalletMain->mapKeyRegID[keyid].vRegID;
							CKeyID deskeyid = pubkey.GetID();
							tx.desRegAccountId.insert(tx.desRegAccountId.end(),deskeyid.begin(),deskeyid.end());
							tx.llValues = min_money;
							tx.llFees = fee*(i+1);
							tx.nValidHeight = chainActive.Tip()->nHeight;

							CKey key;
							pwalletMain->GetKey(keyid, key);
							if (!key.Sign(tx.SignatureHash(), tx.signature)) {
								throw JSONRPCError(RPC_WALLET_ERROR, "createnormaltx Error: Sign failed.");
							}
							LogPrint("INFO","Normal tx destaddr = %s \r\n",CBitcoinAddress(deskeyid).ToString().c_str());
							if (!pwalletMain->CommitTransaction((CBaseTransaction *) &tx)) {
								throw JSONRPCError(RPC_WALLET_ERROR, "createnormaltx Error: CommitTransaction failed.");
							}
							LogPrint("INFO","normal tx hash = %s \r\n",tx.GetHash().ToString().c_str());
							bNormal = true;
						}
						{
							CFreezeTransaction tx;
							tx.regAccountId = pwalletMain->mapKeyRegID[keyid].vRegID;
							tx.llFreezeFunds = min_money / 2;
							tx.llFees = fee;
							tx.nValidHeight = chainActive.Tip()->nHeight;
							tx.nUnfreezeHeight = chainActive.Tip()->nHeight + 100;

							CKey key;
							pwalletMain->GetKey(keyid, key);
							if (!key.Sign(tx.SignatureHash(), tx.signature)) {
								throw JSONRPCError(RPC_WALLET_ERROR, "createfreezetx Error: Sign failed.");
							}
							LogPrint("INFO","Freeze tx addr = %s \r\n",CBitcoinAddress(keyid).ToString().c_str());
							if (!pwalletMain->CommitTransaction((CBaseTransaction *) &tx)) {
								throw JSONRPCError(RPC_WALLET_ERROR, "createfreezetx Error: CommitTransaction failed.");
							}
							LogPrint("INFO","Freeze tx hash = %s \r\n",tx.GetHash().ToString().c_str());
						}
					}
					else if(!bReg && balance > money)
					{
						CRegisterAccountTx rtx;
						pwalletMain->GetPubKey(keyid,rtx.pubKey);
						rtx.llFees = fee;
						rtx.nValidHeight = chainActive.Tip()->nHeight;
						//sign
						CKey key;
						pwalletMain->GetKey(keyid, key);
						if (!key.Sign(rtx.SignatureHash(), rtx.signature)) {
							throw JSONRPCError(RPC_WALLET_ERROR, "in registersecuretx Error: Sign failed.");
						}
						LogPrint("INFO","Reg tx addr = %s \r\n",CBitcoinAddress(keyid).ToString().c_str());
						if (!pwalletMain->CommitTransaction((CBaseTransaction *) &rtx)) {
							throw JSONRPCError(RPC_WALLET_ERROR, "registersecuretx Error: CommitTransaction failed.");
						}
						LogPrint("INFO","Reg tx hash = %s \r\n",rtx.GetHash().ToString().c_str());
					}

				}
			}

			{
				int nHeightEnd = chainActive.Height();;
				int nHeight = chainActive.Height();
				GenerateBitcoins(true, pwalletMain, 1);
				while (nHeight == nHeightEnd)
				{
					MilliSleep(10);
					{   // Don't keep cs_main locked
						LOCK(cs_main);
						nHeight = chainActive.Height();
					}
				 }
				 GenerateBitcoins(false, pwalletMain, 1);
			}

			boost::this_thread::interruption_point();
			MilliSleep(400);
			if(nTimes > 0)
			{
				count++;
				if(count >= nTimes)
				{
					LogPrint("INFO","ThreadCreateNormalTx exit1\r\n");
					break;
				}
			}
		}
	}
	catch(...)
	{
		LogPrint("INFO","ThreadCreateNormalTx exit2\r\n");
	}
}

Value testminer(const Array& params, bool fHelp) {
	if (fHelp || params.size() > 1) {
		string msg = "testminer\n" +
					 HelpExampleCli("testminer", "")
					 + "\nAs json rpc call\n" +
					 HelpExampleRpc("testminer", "");
		throw runtime_error(msg);
	}

	int times = 0;
	//get addresss
	if(params.size() > 0)
	{
		times = params[0].get_int();
		if(times == 0)
		{
			sThreadGroup.interrupt_all();
			sThreadGroup.join_all();
			return string("test testminer stop");
		}
	}
	sThreadGroup.create_thread(boost::bind(&ThreadTestMiner,times));
	return string("testminer start");
}

Value disconnectblock(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		throw runtime_error(
				"disconnectblock \"block numbers \n"
						"\ndisconnect block\n" + HelpRequiringPassphrase() +
						 "\nArguments:\n"
						"1. \"numbers \"  (numeric, required) The bitcoin address.\n"
						"\nResult:\n"
						"\"disconnect result\"  (bool) \n"
						"\nExamples:\n"
						+ HelpExampleCli("disconnectblock", "\"1\"")
						);
	}
	int64_t number = params[0].get_int64();
	CAccountViewCache view(*pAccountViewTip, true);
	CBlockIndex* pindex = chainActive.Tip();
	CBlock block;
	CValidationState state;
	while (number--) {
		// check level 0: read from disk
		if (!ReadBlockFromDisk(block, pindex))
			return ERROR("VerifyDB() : *** ReadBlockFromDisk failed at %d, hash=%s", pindex->nHeight,
					pindex->GetBlockHash().ToString());
		bool fClean = true;
		CTransactionCache txCacheTemp(*pTxCacheTip);
		CContractScriptCache contractScriptTemp(*pContractScriptTip);
		if (!DisconnectBlock(block, state, view, pindex, txCacheTemp, contractScriptTemp, &fClean))
			return ERROR("VerifyDB() : *** irrecoverable inconsistency in block data at %d, hash=%s", pindex->nHeight,
					pindex->GetBlockHash().ToString());
		pindex = pindex->pprev;
		chainActive.SetTip(pindex);
		assert(view.Flush());
	}
	return true;
}

Value listregscript(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0) {
		throw runtime_error("listregscript " + HelpRequiringPassphrase() +
				"\nArguments:\n"
				"\nResult:\n"
				"\"regscript array\"  (bool) \n"
				"\nExamples:\n" + HelpExampleCli("listregscript", ""));
	}
	Object obj;
	Array arrayScript;

	CAccountViewCache view(*pAccountViewTip, true);
	if(pContractScriptTip != NULL) {
		map<string, CContractScript> &mapScript = pContractScriptTip->GetScriptCache();
		map<string, CContractScript>::iterator iterScript = mapScript.begin();
		for(; iterScript != mapScript.end(); ++iterScript) {
			Object script;
			Array arrayArbitrator;
			script.push_back(Pair("RegScriptId", iterScript->first));
			script.push_back(Pair("RegScriptContent", HexStr(iterScript->second.scriptContent.begin(), iterScript->second.scriptContent.end())));

			set<string>::iterator iterArbit =  iterScript->second.setArbitratorAccId.begin();
			for(;iterArbit != iterScript->second.setArbitratorAccId.end(); ++iterArbit) {
				Object arbitrator;
				CKeyID keyId;
				view.GetKeyId(ParseHex(*iterArbit), keyId);
				arbitrator.push_back(Pair("RegId", (*iterArbit).c_str()));
				arbitrator.push_back(Pair("KeyId", keyId.GetHex()));
				arrayArbitrator.push_back(arbitrator);
			}
			script.push_back(Pair("RegisterArbitrator:", arrayArbitrator));
			arrayScript.push_back(script);

		}
	}

	obj.push_back(Pair("ListRegisterScript",arrayScript));
	return obj;
}

Value getoneaddr(const Array& params, bool fHelp) {
	if (fHelp || params.size() > 2) {
		string msg = "getoneaddr nrequired (minmoney reg)\n"
					 "\ngetoneaddr\n"
					 "\nArguments:\n"
					 "1.minmoney: (numeric)\n"
					 "2.reg: (numeric)\n"
					 "\nResult:\n"
					 "\"addr\": (string)\n"
					 "\nExamples:\n" +
					 HelpExampleCli("getoneaddr", "100000 true")+ "\nAs json rpc call\n" +
					 HelpExampleRpc("getoneaddr", "100000 true");
				throw runtime_error(msg);
	}

	assert(pwalletMain != NULL);
	bool bneedReg = true;
	uint64_t min_money = 100000;

	if(params.size() > 0)
	{
		min_money = params[0].get_uint64();
	}
	if(params.size() > 1)
	{
		bneedReg = params[1].get_bool();
	}
	string str = "";
	{
		LOCK2(cs_main, pwalletMain->cs_wallet);

		set<CKeyID> setKeyID;

		pwalletMain->GetKeys(setKeyID); //get addrs
		if (setKeyID.empty()) {
			return str;
		}

		CAccountViewCache accView(*pAccountViewTip, true);
		for (const auto &keyid : setKeyID) {
			//find CSecureAccount info by keyid
			bool bReg = false;
			uint64_t balance = 0;
			CSecureAccount secureAcc;

			if (accView.GetAccount(keyid, secureAcc)) {
				bReg = secureAcc.IsRegister();
				balance = secureAcc.GetBalance(chainActive.Tip()->nHeight+100);
			}

			if (bReg && !pwalletMain->mapKeyRegID.count(keyid)) {
				throw JSONRPCError(RPC_WALLET_ERROR, "getoneaddr Error: WALLET file is not correct.");
			}

			if(balance >= min_money && bReg == bneedReg)
			{

				CTxDestination destid;
				if (bReg) {
					destid = CAccountID(keyid, pwalletMain->mapKeyRegID[keyid].vRegID);
				} else {
					destid = keyid;
				}
				str = CBitcoinAddress(destid).ToString();
				break;
			}
		}
	}
	return str;
}

Value getaddrbalance(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		string msg = "getaddrbalance nrequired [\"key\",...] ( \"account\" )\n"
				"\nAdd a nrequired-to-sign multisignature address to the wallet.\n"
				"Each key is a Bitcoin address or hex-encoded public key.\n" +
				HelpExampleCli("getaddrbalance", "")
				+ "\nAs json rpc call\n" +
				HelpExampleRpc("getaddrbalance", "");
		throw runtime_error(msg);
	}

	assert(pwalletMain != NULL);
	CBitcoinAddress addr(params[0].get_str());

	CKeyID keyid;
	if (!addr.GetKeyID(keyid))
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Bitcoin address");

	double dbalance = 0.0;
	{
		LOCK(cs_main);
		CAccountViewCache accView(*pAccountViewTip, true);
		CSecureAccount secureAcc;
		if (accView.GetAccount(keyid, secureAcc)) {
			dbalance = (double) secureAcc.GetBalance(chainActive.Tip()->nHeight + 100) / (double) COIN;
		}
	}
	return dbalance;
}

Value generateblock(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		string msg = "generateblock nrequired (\"addr\")\n"
					 "\generateblock\n"
					 "\nArguments:\n"
					 "1.\"addr\": (str)\n"
					 "\nResult:\n"
					 "NULL"
					 "\nExamples:\n" +
					 HelpExampleCli("generateblock", "5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9")+ "\nAs json rpc call\n" +
					 HelpExampleRpc("generateblock", "5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9");
		throw runtime_error(msg);
	}

	CBitcoinAddress addr(params[0].get_str());

	//get keyid
	CKeyID keyid;
	if (!addr.GetKeyID(keyid)) {
		throw runtime_error("in generateblock :address err \n");
	}

	{
		LOCK2(cs_main, pwalletMain->cs_wallet);
		if(!CreateBlockWithAppointedAddr(keyid))
		{
			throw runtime_error("in generateblock :cannot generate block\n");
		}

	}
	return NULL;
}

Value getpublickey(const Array& params, bool fHelp) {
	if (fHelp || params.size() !=1) {
		throw runtime_error("getpublickey \"address \n" +
				HelpRequiringPassphrase() +
				"\nArguments:\n"
				"1. \"address \"  (string, required) The bitcoin address.\n"
				"\nResult:\n"
				"\"publickey\"  (string) \n"
				"\nExamples:\n" + HelpExampleCli("getpublickey", "mpif58ohTASDZNNXreFMFuNAHBfuDUXtjP"));
	}
	CBitcoinAddress address(params[0].get_str());

	CKeyID keyid;
	if (!address.GetKeyID(keyid))
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Bitcoin address");

	CPubKey pubkey;
	{
		LOCK2(cs_main, pwalletMain->cs_wallet);
		if(!pwalletMain->GetPubKey(keyid, pubkey))
			throw JSONRPCError(RPC_MISC_ERROR, tinyformat::format("Wallet do not contain address %s", params[0].get_str()));
	}
	return HexStr(pubkey.begin(), pubkey.end());
}

Value listtxcache(const Array& params, bool fHelp) {
	if (fHelp || params.size() !=0) {
		throw runtime_error("listtxcache \"address \n" +
				HelpRequiringPassphrase() +
				"\nArguments:\n"
				"\nResult:\n"
				"\"txcache\"  (string) \n"
				"\nExamples:\n" + HelpExampleCli("listtxcache", ""));
	}
	const map<uint256, vector<uint256> > &mapTxHashCacheByPrev = pTxCacheTip->GetRelayTx();
	const map<uint256, vector<uint256> > &mapTxHashByBlockHash = pTxCacheTip->GetTxHashCache();

	Array retTxHashArray;
	for(auto &item : mapTxHashByBlockHash) {
		Object blockObj;
		Array txHashArray;
		blockObj.push_back(Pair("blockhash", item.first.GetHex()));
		for(auto &txHash : item.second)
			txHashArray.push_back(txHash.GetHex());
		blockObj.push_back(Pair("txcache", txHashArray));
		retTxHashArray.push_back(blockObj);
	}
	for(auto &item : mapTxHashCacheByPrev) {
		Object blockObj;
		Array txHashPreArray;
		blockObj.push_back(Pair("prehash", item.first.GetHex()));
		for(auto &relayTx : item.second)
			txHashPreArray.push_back(relayTx.GetHex());
		blockObj.push_back(Pair("relayhash", txHashPreArray));
		retTxHashArray.push_back(blockObj);
	}
	return retTxHashArray;
}


Value reloadtxcache(const Array& params, bool fHelp) {
	if (fHelp || params.size() !=0) {
		throw runtime_error("reloadtxcache" +
				HelpRequiringPassphrase() +
				"\nArguments:\n"
				"\nResult:\n"
				"\nExamples:\n" + HelpExampleCli("reloadtxcache", ""));
	}
	pTxCacheTip->Clear();
	CBlockIndex *pIndex = chainActive.Tip();
	if ((chainActive.Tip()->nHeight - Params().GetTxCacheHeight()) >= 0) {
		pIndex = chainActive[(chainActive.Tip()->nHeight - Params().GetTxCacheHeight())];
	} else {
		pIndex = chainActive.Genesis();
	}
	CBlock block;
	do {
		if (!ReadBlockFromDisk(block, pIndex))
			return ERROR("reloadtxcache() : *** ReadBlockFromDisk failed at %d, hash=%s", pIndex->nHeight,
							pIndex->GetBlockHash().ToString());
		pTxCacheTip->AddBlockToCache(block);
		pIndex = chainActive.Next(pIndex);
	}
	while (NULL != pIndex);
	return string("reload tx cache succeed");
}


