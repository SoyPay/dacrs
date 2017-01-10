// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"
#include "rpcserver.h"
#include "init.h"
#include "net.h"
#include "netbase.h"
#include "util.h"
#include "miner.h"
#include "../wallet/wallet.h"
#include "../wallet/walletdb.h"

#include <stdint.h>

#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"

using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;

int64_t g_llWalletUnlockTime;
static CCriticalSection g_cWalletUnlockTime;

string HelpRequiringPassphrase() {
	return g_pwalletMain && g_pwalletMain->IsCrypted() ?
			"\nRequires wallet passphrase to be set with walletpassphrase call." : "";
}

void EnsureWalletIsUnlocked() {
	if (g_pwalletMain->IsLocked()) {
		throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED,
				"Error: Please enter the wallet passphrase with walletpassphrase first.");
	}
}
/**
 * 钱包是否被锁定
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value islocked(const Array& params, bool bHelp) {
	if (bHelp) {
		return true;
	}
	Object obj;
	if (!g_pwalletMain->IsCrypted()) {        //decrypted
		obj.push_back(Pair("islock", 0));
	} else if (!g_pwalletMain->IsLocked()) {   //encryped and unlocked
		obj.push_back(Pair("islock", 1));
	} else {
		obj.push_back(Pair("islock", 2)); //encryped and locked
	}
	return obj;
}
/**
 * 生成新地址
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value getnewaddress(const Array& params, bool bHelp)
{
    if (bHelp || params.size() > 1) {
        throw runtime_error(
            "getnewaddress  (\"IsMiner\")\n"
            "\nget a new address\n"
            "\nArguments:\n"
        	"1. \"IsMiner\"  (bool, optional)  private key Is used for miner if true will create tow key ,another for miner.\n"
           "\nExamples:\n"
            + HelpExampleCli("getnewaddress", "")
            + HelpExampleCli("getnewaddress", "true")
        );
    }
	EnsureWalletIsUnlocked();

	CKey cCkey;
	cCkey.MakeNewKey();

	CKey cMinter;
	bool bIsForMiner = false;
	if (params.size() == 1) {
		RPCTypeCheck(params, list_of(bool_type));
		cMinter.MakeNewKey();
		bIsForMiner = params[0].get_bool();
	}

	CPubKey cNewKey = cCkey.GetPubKey();

	CKeyID keyID = cNewKey.GetKeyID();

	if (bIsForMiner) {
		if (!g_pwalletMain->AddKey(cCkey, cMinter)) {
			throw runtime_error("add key failed ");
		}
	} else if (!g_pwalletMain->AddKey(cCkey)) {
		throw runtime_error("add key failed ");
	}
	Object obj;
	obj.push_back(Pair("addr", keyID.ToAddress()));
	obj.push_back(Pair("minerpubkey", bIsForMiner ? cMinter.GetPubKey().ToString() : "no"));
	return obj;
}
/**
 * 签名一段信息
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value signmessage(const Array& params, bool bHelp) {
    if (bHelp || params.size() != 2) {
        throw runtime_error(
            "signmessage \"Dacrsaddress\" \"message\"\n"
            "\nSign a message with the private key of an address"
            + HelpRequiringPassphrase() + "\n"
            "\nArguments:\n"
            "1. \"Dacrsaddress\"  (string, required) The Dacrs address to use for the private key.\n"
            "2. \"message\"         (string, required) The message to create a signature of.\n"
            "\nResult:\n"
            "\"signature\"          (string) The signature of the message encoded in base 64\n"
            "\nExamples:\n"
            "\nUnlock the wallet for 30 seconds\n"
            + HelpExampleCli("walletpassphrase", "\"mypassphrase\" 30") +
            "\nCreate the signature\n"
            + HelpExampleCli("signmessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\" \"my message\"") +
            "\nVerify the signature\n"
            + HelpExampleCli("verifymessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\" \"signature\" \"my message\"") +
            "\nAs json rpc\n"
            + HelpExampleRpc("signmessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\", \"my message\"")
        );
    }
	EnsureWalletIsUnlocked();

	string strAddress = params[0].get_str();
	string strMessage = params[1].get_str();

	CKeyID cKeyID(strAddress);
	if (cKeyID.IsEmpty()) {
		throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");
	}
	CKey cKey;
	if (!g_pwalletMain->GetKey(cKeyID, cKey)) {
		throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");
	}
	CHashWriter cSs(SER_GETHASH, 0);
	cSs << g_strMessageMagic;
	cSs << strMessage;

	vector<unsigned char> vuchSig;
	if (!cKey.SignCompact(cSs.GetHash(), vuchSig)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");
	}
	return EncodeBase64(&vuchSig[0], vuchSig.size());
}
/**
 * 指定小费发送总额到指定地址
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value sendtoaddresswithfee(const Array& params, bool bHelp) {
	int nSize = params.size();
	if (bHelp || (!(nSize == 3 || nSize == 4))) {
		throw runtime_error(
						"sendtoaddresswithfee (\"sendaddress\") \"recvaddress\" \"amount\" (fee)\n"
						"\nSend an amount to a given address with fee. The amount is a real and is rounded to the nearest 0.00000001\n"
						"\nArguments:\n"
						"1. \"sendaddress\"  (string, optional) The Dacrs address to send to.\n"
						"2. \"recvaddress\" (string, required) The Dacrs address to receive.\n"
						"3.\"amount\"   (string,required) \n"
						"4.\"fee\"      (string,required) \n"
						"\nResult:\n"
						"\"transactionid\"  (string) The transaction id.\n"
						"\nExamples:\n"
						+ HelpExampleCli("sendtoaddresswithfee", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 10000000 1000")
						+ HelpExampleCli("sendtoaddresswithfee",
						"\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.1 \"donation\" \"seans outpost\"")
						+ HelpExampleRpc("sendtoaddresswithfee",
						"\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\", 0.1, \"donation\", \"seans outpost\""
						+ HelpExampleCli("sendtoaddresswithfee", "\"0-6\" 10 ")
						+ HelpExampleCli("sendtoaddresswithfee", "\"00000000000000000005\" 10 ")
						+ HelpExampleCli("sendtoaddresswithfee", "\"0-6\" \"0-5\" 10 ")
						+ HelpExampleCli("sendtoaddresswithfee", "\"00000000000000000005\" \"0-6\"10 ")));
	}
	EnsureWalletIsUnlocked();
	CKeyID cSendKeyId;
	CKeyID cRevKeyId;

	auto GetKeyId = [](string const &strAddr,CKeyID &cKeyId) {
		if (!CRegID::GetKeyID(strAddr, cKeyId)) {
			cKeyId=CKeyID(strAddr);
			if (cKeyId.IsEmpty())
			return false;
		}
		return true;
	};

	// Amount
	int64_t llAmount = 0;
	int64_t llFee = 0;
	//// from address to addreww
	if (nSize == 4) {
		if (!GetKeyId(params[0].get_str(), cSendKeyId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM Invalid  address");
		}
		if (!GetKeyId(params[1].get_str(), cRevKeyId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "to Invalid  address");
		}
		llAmount = AmountToRawValue(params[2]);
		if (g_pAccountViewTip->GetRawBalance(cSendKeyId) <= llAmount + SysCfg().GetTxFee()) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM address not enough coins");
		}
		llFee = AmountToRawValue(params[3]);
	} else {
		if (!GetKeyId(params[0].get_str(), cRevKeyId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "to address Invalid  ");
		}

		llAmount = AmountToRawValue(params[1]);
		set<CKeyID> sKeyid;
		sKeyid.clear();
		g_pwalletMain->GetKeys(sKeyid);
		if (sKeyid.empty()) {	//get addrs
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No Key In wallet \n");
		}
		llFee = AmountToRawValue(params[2]);
		for (auto &te : sKeyid) {
			if (g_pAccountViewTip->GetRawBalance(te) >= llAmount + SysCfg().GetTxFee()) {
				cSendKeyId = te;
				break;
			}
		}

		if (cSendKeyId.IsNull()) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "not find enough moeny account ");
		}
	}

	auto SendMoney = [&](const CRegID &send, const CUserID &rsv, int64_t nValue, int64_t llFee) {
		CTransaction cTx;
		cTx.m_cSrcRegId = send;
		cTx.m_cDesUserId = rsv;
		cTx.m_ullValues = nValue;
		if (0 == llFee) {
			cTx.m_ullFees = SysCfg().GetTxFee();
		} else {
			cTx.m_ullFees = llFee;
		}
		cTx.m_nValidHeight = g_cChainActive.Tip()->m_nHeight;

		CKeyID cKeyID;
		if(!g_pAccountViewTip->GetKeyId(send,cKeyID)) {
			return std::make_tuple (false,"key or keID failed");
		}
		if (!g_pwalletMain->Sign(cKeyID,cTx.SignatureHash(), cTx.m_vchSignature)) {
			return std::make_tuple (false,"Sign failed");
		}
		std::tuple<bool,string> ret = g_pwalletMain->CommitTransaction((CBaseTransaction *) &cTx);
		bool bFalg = std::get<0>(ret);
		string strTe = std::get<1>(ret);
		if(bFalg == true) {
			strTe = cTx.GetHash().ToString();
		}
		return std::make_tuple (bFalg,strTe.c_str());
	};

	CRegID cSendReg;
	CRegID cRevReg;

	if (!g_pAccountViewTip->GetRegId(CUserID(cSendKeyId), cSendReg)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid  address");
	}

	std::tuple<bool, string> ret;
	if (g_pAccountViewTip->GetRegId(CUserID(cRevKeyId), cRevReg)) {
		ret = SendMoney(cSendReg, cRevReg, llAmount, llFee);
	} else {
		ret = SendMoney(cSendReg, CUserID(cRevKeyId), llAmount, llFee);
	}

	Object obj;
	obj.push_back(Pair(std::get<0>(ret) ? "hash" : "error code", std::get<1>(ret)));
	return obj;
}
/**
 * 由输入信息创建发送交易
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value sendtoaddressraw(const Array& params, bool bHelp) {
	int nSize = params.size();
	if (bHelp || nSize < 5 || nSize > 6 ) {
		throw runtime_error(
						"sendtoaddressraw \"height\" \"fee\" \"amount\" \"srcaddress\" \"recvaddress\"\n"
						"\n create normal transaction by hegiht,fee,amount,srcaddress, recvaddress.\n"
						+ HelpRequiringPassphrase() + "\nArguments:\n"
						"1. \"fee\"     (numeric, required)  \n"
						"2. \"amount\"  (numeric, required)  \n"
						"3. \"srcaddress\"  (string, required) The Dacrs address to send to.\n"
						"4. \"recvaddress\"  (string, required) The Dacrs address to receive.\n"
						"5. \"issign\"  (bool, required) weather needs return signed transaction hex.\n"
						"6. \"height\"  (int, optional) \n"
						"\nResult:\n"
						"\"transactionid\"  (string) The transaction id.\n"
						"\nExamples:\n"
						+ HelpExampleCli("sendtoaddressraw", "100 1000 \"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.1")
						+ HelpExampleCli("sendtoaddressraw",
						"100 1000 \"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.1 \"donation\" \"seans outpost\"")
						+ HelpExampleRpc("sendtoaddressraw",
						"100 1000 \"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\", 0.1, \"donation\", \"seans outpost\""
						+ HelpExampleCli("sendtoaddressraw", "\"0-6\" 10 ")
						+ HelpExampleCli("sendtoaddressraw", "100 1000 \"00000000000000000005\" 10 ")
						+ HelpExampleCli("sendtoaddressraw", "100 1000 \"0-6\" \"0-5\" 10 ")
						+ HelpExampleCli("sendtoaddressraw", "100 1000 \"00000000000000000005\" \"0-6\"10 ")));
	}
	CKeyID cSendKeyId;
	CKeyID cRevKeyId;

	auto GetUserID = [](string const &strAddr,CUserID &userid) {
		CRegID cTe(strAddr);
		if(!cTe.IsEmpty()) {
			userid = cTe;
			return true;
		}
		CKeyID kid(strAddr);
		if(!kid.IsEmpty()) {
			userid = kid;
			return true;
		}
		return false;
	};

	int64_t llFee = AmountToRawValue(params[0]);

	int64_t llAmount = 0;

    llAmount = AmountToRawValue(params[1]);
	if (llAmount == 0) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM Invalid nAmount == 0");
	}

	CUserID cUserSend;
	CUserID cUserRev;
	if (!GetUserID(params[2].get_str(), cUserSend)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM Invalid send address");
	}
	if (!g_pAccountViewTip->GetKeyId(cUserSend, cSendKeyId)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Get CKeyID failed from CUserID");
	}
	if (cUserSend.type() == typeid(CKeyID)) {
		CRegID cRegId;
		if (!g_pAccountViewTip->GetRegId(cUserSend, cRegId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "CKeyID is not registed ");
		}
		cUserSend = cRegId;
	}
	if (!GetUserID(params[3].get_str(), cUserRev)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM Invalid rev address");
	}
	if (cUserRev.type() == typeid(CKeyID)) {
		CRegID cRegId;
		if (g_pAccountViewTip->GetRegId(cUserRev, cRegId)) {
			cUserRev = cRegId;
		}
	}
	bool bIsSign = params[4].get_bool();

	int nHight = g_cChainActive.Tip()->m_nHeight;
	if (params.size() > 5) {
		nHight = params[5].get_int();
	}

	std::shared_ptr<CTransaction> tx = std::make_shared<CTransaction>(cUserSend, cUserRev, llFee, llAmount, nHight);
	if (bIsSign) {
		if (!g_pwalletMain->Sign(cSendKeyId, tx->SignatureHash(), tx->m_vchSignature)) {
			throw JSONRPCError(RPC_INVALID_PARAMETER, "Sign failed");
		}
	}
	CDataStream cDs(SER_DISK, g_sClientVersion);
	std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
	cDs << pBaseTx;
	Object obj;
	obj.push_back(Pair("rawtx", HexStr(cDs.begin(), cDs.end())));
	obj.push_back(Pair("signhash", tx->SignatureHash().GetHex()));
	return obj;
}
/**
 * 归集余额
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value notionalpoolingbalance(const Array& params, bool bHelp) {
	int nSize = params.size();
	if (bHelp || (nSize != 2)) {
		throw runtime_error(
						"notionalpoolingbalance  \"receive address\" \"amount\"\n"
						"\nSend an amount to a given address. The amount is a real and is rounded to the nearest 0.00000001\n"
						+ HelpRequiringPassphrase() + "\nArguments:\n"
						"1. receive address   (string, required) The Koala address to receive\n"
						"2. amount (required)\n"
						"3.\"description\"   (string, required) \n"
						"\nResult:\n"
						"\"transactionid\"  (string) The transaction id.\n"
						"\nExamples:\n"
						+ HelpExampleCli("notionalpoolingbalance", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.1")
						+ HelpExampleRpc("notionalpoolingbalance",
						"\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\", 0.1, \"donation\", \"seans outpost\""));
	}
	EnsureWalletIsUnlocked();
	CKeyID cSendKeyId;
	CKeyID cRevKeyId;

	auto GetKeyId = [](string const &strAddr,CKeyID &cKeyId) {
		if (!CRegID::GetKeyID(strAddr, cKeyId)) {
			cKeyId=CKeyID(strAddr);
			if (cKeyId.IsEmpty())
			return false;
		}
		return true;
	};

	// Amount
	Object retObj;
	int64_t llAmount = 0;
	CRegID cSendReg;
	//// from address to address

	if (!GetKeyId(params[0].get_str(), cRevKeyId)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "to address Invalid  ");
	}
	if (!g_pwalletMain->HaveKey(cRevKeyId)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "to address Invalid  ");
	}
	llAmount = params[1].get_real() * COIN;
	if (llAmount <= SysCfg().GetTxFee()) {
		llAmount = 0.01 * COIN;
	}

	set<CKeyID> sKeyid;
	sKeyid.clear();
	g_pwalletMain->GetKeys(sKeyid); //get addrs
	if (sKeyid.empty()) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No Key In wallet \n");
	}
	set<CKeyID> sResultKeyid;
	for (auto &te : sKeyid) {
		if (te.ToString() == cRevKeyId.ToString()) {
			continue;
		}
		if (g_pAccountViewTip->GetRawBalance(te) >= llAmount) {
			if (g_pAccountViewTip->GetRegId(CUserID(te), cSendReg)) {
				sResultKeyid.insert(te);
			}
		}
	}

	Array arrayTxIds;
	set<CKeyID>::iterator it;

	for (it = sResultKeyid.begin(); it != sResultKeyid.end(); it++) {
		cSendKeyId = *it;
		if (cSendKeyId.IsNull()) {
			continue;
		}

		CRegID cRevReg;
		CUserID cUserRev;

		if (!g_pAccountViewTip->GetRegId(CUserID(cSendKeyId), cSendReg)) {
			continue;
		}
		if (g_pAccountViewTip->GetRegId(CUserID(cRevKeyId), cRevReg)) {
			cUserRev = cRevReg;
		} else {
			cUserRev = cRevKeyId;
		}

		CTransaction cTx(cSendReg, cUserRev, SysCfg().GetTxFee(),
				g_pAccountViewTip->GetRawBalance(cSendReg) - SysCfg().GetTxFee(), g_cChainActive.Height());

		if (!g_pwalletMain->Sign(cSendKeyId, cTx.SignatureHash(), cTx.m_vchSignature)) {
			continue;
		}

		std::tuple<bool, string> ret = g_pwalletMain->CommitTransaction((CBaseTransaction *) &cTx);
		if (!std::get<0>(ret)) {
			continue;
		}
		arrayTxIds.push_back(std::get<1>(ret));
	}

	retObj.push_back(Pair("Tx", arrayTxIds));
	return retObj;
}
/**
 * 分散余额
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value dispersebalance(const Array& params, bool bHelp) {
	int nSize = params.size();
	if (bHelp || (nSize != 2)) {
		throw runtime_error(
						"notionalpoolingbalance  \"send address\" \"amount\"\n"
						"\nSend an amount to a address list. \n"
						+ HelpRequiringPassphrase() + "\nArguments:\n"
						"1. send address   (string, required) The Koala address to receive\n"
						"2. amount (required)\n"
						"3.\"description\"   (string, required) \n"
						"\nResult:\n"
						"\"transactionid\"  (string) The transaction id.\n"
						"\nExamples:\n"
						+ HelpExampleCli("dispersebalance", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.1")
						+ HelpExampleRpc("dispersebalance",
						"\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\", 0.1"));
	}
	EnsureWalletIsUnlocked();

	CKeyID cSendKeyId;

	auto GetKeyId = [](string const &strAddr,CKeyID &cKeyId) {
		if (!CRegID::GetKeyID(strAddr, cKeyId)) {
			cKeyId=CKeyID(strAddr);
			if (cKeyId.IsEmpty())
			return false;
		}
		return true;
	};

	if (!GetKeyId(params[0].get_str(), cSendKeyId)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "send address Invalid  ");
	}
	if (!g_pwalletMain->HaveKey(cSendKeyId)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "send address Invalid  ");
	}
	CRegID cSendReg;
	if (!g_pAccountViewTip->GetRegId(CUserID(cSendKeyId), cSendReg)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "send address not activated  ");
	}

	int64_t llAmount = 0;
	llAmount = params[1].get_real() * COIN;
	if (llAmount <= 0) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "llAmount <= 0  ");
	}
	set<CKeyID> sKeyid;
	g_pwalletMain->GetKeys(sKeyid); //get addrs
	if (sKeyid.empty()) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No Key In wallet \n");
	}

	Array arrayTxIds;
	Object retObj;
	set<CKeyID>::iterator it;
	CKeyID cRevKeyId;

	for (it = sKeyid.begin(); it != sKeyid.end(); it++) {
		cRevKeyId = *it;

		if (cRevKeyId.IsNull()) {
			continue;
		}
		if (cSendKeyId.ToString() == cRevKeyId.ToString()) {
			continue;
		}

		CRegID cRevReg;
		CUserID cUserRev;

		if (g_pAccountViewTip->GetRegId(CUserID(cRevKeyId), cRevReg)) {
			cUserRev = cRevReg;
		} else {
			cUserRev = cRevKeyId;
		}

		if (g_pAccountViewTip->GetRawBalance(cSendReg) < llAmount + SysCfg().GetTxFee()) {
			break;
		}

		CTransaction cTx(cSendReg, cUserRev, SysCfg().GetTxFee(), llAmount, g_cChainActive.Height());

		if (!g_pwalletMain->Sign(cSendKeyId, cTx.SignatureHash(), cTx.m_vchSignature)) {
			continue;
		}

		std::tuple<bool, string> ret = g_pwalletMain->CommitTransaction((CBaseTransaction *) &cTx);
		if (!std::get<0>(ret)) {
			continue;
		}
		arrayTxIds.push_back(std::get<1>(ret));
	}
	retObj.push_back(Pair("Tx", arrayTxIds));
	return retObj;
}

#pragma pack(1)
typedef struct {
	unsigned char nSystype;
	unsigned char uchType;
	unsigned char arruchAddress[34]; //  转账地址

	IMPLEMENT_SERIALIZE
	(
			READWRITE(nSystype);
			READWRITE(uchType);
			for (unsigned int i = 0;i < 34;++i) {
				READWRITE(arruchAddress[i]);
			}
	)
} ST_TRAN_USER;
#pragma pack()
/**
 * 归集资产
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value notionalpoolingasset(const Array& params, bool bHelp) {
	if(bHelp || params.size() < 2) {
		throw runtime_error(
				 "notionalpoolingasset \"scriptid\" \"recvaddress\"\n"
				 "\nThe collection of all assets\n"
				 "\nArguments:\n"
				 "1.\"scriptid\": (string, required)\n"
				 "2.\"recvaddress\"  (string, required) The Popcoin address to receive.\n"
				 "3.\"amount\" (number optional)\n"
				 "\nResult:\n"
				 "\nExamples:\n"
				 + HelpExampleCli("notionalpoolingasset", "11-1 pPKAiv9v4EaKjZGg7yWqnFJbhdZLVLyX8N 10")
				 + HelpExampleRpc("notionalpoolingasset", "11-1 pPKAiv9v4EaKjZGg7yWqnFJbhdZLVLyX8N 10"));
	}
	EnsureWalletIsUnlocked();
	CRegID cRegId(params[0].get_str());
	if (cRegId.IsEmpty() == true) {
		throw runtime_error("in notionalpoolingasset :scriptid size is error!\n");
	}

	if (!g_pScriptDBTip->HaveScript(cRegId)) {
		throw runtime_error("in notionalpoolingasset :scriptid  is not exist!\n");
	}

	int64_t llAmount = 10 * COIN;
	if(3 == params.size()) {
		llAmount = params[2].get_real() * COIN;
	}

	CKeyID cRevKeyId;

	auto GetKeyId = [](string const &strAddr,CKeyID &cKeyId) {
		if (!CRegID::GetKeyID(strAddr, cKeyId)) {
			cKeyId=CKeyID(strAddr);
			if (cKeyId.IsEmpty())
			return false;
		}
		return true;
	};

	Object retObj;
	string strRevAddr = params[1].get_str();
	if (!GetKeyId(strRevAddr, cRevKeyId)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "to address Invalid  ");
	}

	if(!g_pwalletMain->HaveKey(cRevKeyId)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "to address Invalid  ");
	}

	set<CKeyID> sKeyid;
	g_pwalletMain->GetKeys(sKeyid);
	if(sKeyid.empty())
	{
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No Key In wallet \n");
	}
	set<CKeyID> sResultKeyid;
	for (auto &te : sKeyid) {
		if(te.ToString() == cRevKeyId.ToString())
			continue;
		if (g_pAccountViewTip->GetRawBalance(te) >= llAmount)
			sResultKeyid.insert(te);
	}

	set<CKeyID>::iterator it;
	CKeyID cSendKeyId;
	Array arrayTxIds;

	CUserID rev;
	CRegID cRevReg;

	for (it = sResultKeyid.begin(); it != sResultKeyid.end(); it++) {
		cSendKeyId = *it;

		if (cSendKeyId.IsNull()) {
			continue;
		}

		CRegID cSendReg;
		if (!g_pAccountViewTip->GetRegId(CUserID(cSendKeyId), cSendReg)) {
			continue;
		}

		ST_TRAN_USER tTu;
		memset(&tTu, 0, sizeof(ST_TRAN_USER));
		tTu.nSystype = 0xff;
		tTu.uchType = 0x03;
		memcpy(&tTu.arruchAddress, strRevAddr.c_str(), 34);

		CDataStream cScriptData(SER_DISK, g_sClientVersion);
		cScriptData << tTu;
		string sendcontract = HexStr(cScriptData);

		LogPrint("vm", "sendcontract=%s\n", sendcontract.c_str());

		vector_unsigned_char pContract;
		pContract = ParseHex(sendcontract);

		int nFuelRate = GetElementForBurn(g_cChainActive.Tip());
		const int STEP = 645;

		int64_t llFee = (STEP / 100 + 1) * nFuelRate + SysCfg().GetTxFee();

		LogPrint("vm", "nFuelRate=%d, llFee=%lld\n", nFuelRate, llFee);

		CTransaction tx(cSendReg, cRegId, llFee, 0, g_cChainActive.Height(), pContract);

		if (!g_pwalletMain->Sign(cSendKeyId, tx.SignatureHash(), tx.m_vchSignature)) {
			continue;
		}
		std::tuple<bool, string> ret = g_pwalletMain->CommitTransaction((CBaseTransaction *) &tx);
		if (!std::get<0>(ret))
			continue;
		arrayTxIds.push_back(std::get<1>(ret));
	}
	retObj.push_back(Pair("Tx", arrayTxIds));
	return retObj;
}
/**
 * 所有资产的集合
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value getassets(const Array& params, bool bHelp) {
	if (bHelp || params.size() < 1) {
		throw runtime_error(
				 "getassets \"scriptid\"\n"
				 "\nThe collection of all assets\n"
				 "\nArguments:\n"
				 "1.\"scriptid\": (string, required)\n"
				 "\nResult:\n"
				 "\nExamples:\n"
				 + HelpExampleCli("getassets", "11-1")
				 + HelpExampleRpc("getassets", "11-1"));
	}

	CRegID cRegId(params[0].get_str());
	if (cRegId.IsEmpty() == true) {
		throw runtime_error("in getassets :scriptid size is error!\n");
	}

	if (!g_pScriptDBTip->HaveScript(cRegId)) {
		throw runtime_error("in getassets :scriptid  is not exist!\n");
	}

	Object retObj;

	set<CKeyID> sKeyid;
	g_pwalletMain->GetKeys(sKeyid);
	if (sKeyid.empty()) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No Key In wallet \n");
	}

	uint64_t ullTotalAssets = 0;
	Array arrayAssetIds;
	set<CKeyID>::iterator it;
	for (it = sKeyid.begin(); it != sKeyid.end(); it++) {
		CKeyID cKeyId = *it;

		if (cKeyId.IsNull()) {
			continue;
		}

		vector<unsigned char> vchKey;
		string strAddr = cKeyId.ToAddress();
		vchKey.assign(strAddr.c_str(), strAddr.c_str() + strAddr.length());

		std::shared_ptr<CAppUserAccout> temp = std::make_shared<CAppUserAccout>();
		if (!g_pScriptDBTip->GetScriptAcc(cRegId, vchKey, *temp.get())) {
			continue;
		}

		temp.get()->AutoMergeFreezeToFree(cRegId.getHight(), g_cChainActive.Tip()->m_nHeight);
		uint64_t ullFreeValues = temp.get()->getllValues();
		uint64_t ullFreezeValues = temp.get()->GetAllFreezedValues();
		ullTotalAssets += ullFreeValues;
		ullTotalAssets += ullFreezeValues;

		Object result;
		result.push_back(Pair("Address", strAddr));
		result.push_back(Pair("FreeValues", ullFreeValues));
		result.push_back(Pair("FreezedFund", ullFreezeValues));

		arrayAssetIds.push_back(result);
	}

	retObj.push_back(Pair("TotalAssets", ullTotalAssets));
	retObj.push_back(Pair("Lists", arrayAssetIds));

	return retObj;
}
/**
 * 发送到指定地址
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value sendtoaddress(const Array& params, bool bHelp) {
	int nSize = params.size();
	if (bHelp || (!(nSize == 2 || nSize == 3))) {
		throw runtime_error(
								"sendtoaddress (\"Dacrsaddress\") \"receive address\" \"amount\"\n"
								"\nSend an amount to a given address. The amount is a real and is rounded to the nearest 0.00000001\n"
								+ HelpRequiringPassphrase() + "\nArguments:\n"
								"1. \"Dacrsaddress\"  (string, optional) The Dacrs address to send to.\n"
								"2. receive address   (string, required) The Dacrs address to receive\n"
								"3.\"amount\"   (string, required) \n"
								"\nResult:\n"
								"\"transactionid\"  (string) The transaction id.\n"
								"\nExamples:\n"
								+ HelpExampleCli("sendtoaddress", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.1")
								+ HelpExampleCli("sendtoaddress",
								"\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.1 \"donation\" \"seans outpost\"")
								+ HelpExampleRpc("sendtoaddress",
								"\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\", 0.1, \"donation\", \"seans outpost\""
								+ HelpExampleCli("sendtoaddress", "\"0-6\" 10 ")
								+ HelpExampleCli("sendtoaddress", "\"00000000000000000005\" 10 ")
								+ HelpExampleCli("sendtoaddress", "\"0-6\" \"0-5\" 10 ")
								+ HelpExampleCli("sendtoaddress", "\"00000000000000000005\" \"0-6\"10 ")));
	}

	EnsureWalletIsUnlocked();
	CKeyID cSendKeyId;
	CKeyID cRevKeyId;

	auto GetKeyId = [](string const &strAddr,CKeyID &cKeyId) {
		if (!CRegID::GetKeyID(strAddr, cKeyId)) {
			cKeyId=CKeyID(strAddr);
			if (cKeyId.IsEmpty())
			return false;
		}
		return true;
	};

	// Amount
	int64_t llAmount = 0;
	CRegID cSendReg;
	//// from address to addreww
	if (nSize == 3) {
		if (!GetKeyId(params[0].get_str(), cSendKeyId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM Invalid  address");
		}
		if (!GetKeyId(params[1].get_str(), cRevKeyId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "to Invalid  address");
		}
		llAmount = AmountToRawValue(params[2]);
		if (g_pAccountViewTip->GetRawBalance(cSendKeyId) <= llAmount + SysCfg().GetTxFee()) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM address not enough coins");
		}
	} else {
		if (!GetKeyId(params[0].get_str(), cRevKeyId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "to address Invalid  ");
		}
		llAmount = AmountToRawValue(params[1]);
		set<CKeyID> sKeyid;
		sKeyid.clear();
		g_pwalletMain->GetKeys(sKeyid); //get addrs
		if (sKeyid.empty()) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No Key In wallet \n");
		}
		for (auto &te : sKeyid) {
			if (g_pAccountViewTip->GetRawBalance(te) >= llAmount + SysCfg().GetTxFee()) {
				if (g_pAccountViewTip->GetRegId(CUserID(te), cSendReg)) {
					cSendKeyId = te;
					break;
				}
			}
		}
		if (cSendKeyId.IsNull()) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "not find enough moeny account ");
		}
	}

	CRegID cRevReg;
	CUserID cUserRev;

	if (!g_pAccountViewTip->GetRegId(CUserID(cSendKeyId), cSendReg)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid  address");
	}
	if (g_pAccountViewTip->GetRegId(CUserID(cRevKeyId), cRevReg)) {
		cUserRev = cRevReg;
	} else {
		cUserRev = cRevKeyId;
	}

	CTransaction cTx(cSendReg, cUserRev, SysCfg().GetTxFee(), llAmount, g_cChainActive.Height());
	if (!g_pwalletMain->Sign(cSendKeyId, cTx.SignatureHash(), cTx.m_vchSignature)) {
		throw JSONRPCError(RPC_INVALID_PARAMETER, "Sign failed");
	}

	std::tuple<bool, string> ret = g_pwalletMain->CommitTransaction((CBaseTransaction *) &cTx);
	if (!std::get<0>(ret)) {
		throw JSONRPCError(RPC_INVALID_PARAMETER, std::get<1>(ret));
	}

	Object obj;
	obj.push_back(Pair("hash", std::get<1>(ret)));
	return obj;
}
/**
 * 备份钱包
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value backupwallet(const Array& params, bool bHelp) {
    if (bHelp || params.size() != 1) {
        throw runtime_error(
            "backupwallet \"destination\"\n"
            "\nSafely copies wallet.dat to destination, which can be a directory or a path with filename.\n"
            "\nArguments:\n"
            "1. \"destination\"   (string, required) The destination directory or file\n"
            "\nExamples:\n"
            + HelpExampleCli("backupwallet", "\"backup.dat\"")
            + HelpExampleRpc("backupwallet", "\"backup.dat\"")
        );
    }
	string strDest = params[0].get_str();
	if (!BackupWallet(*g_pwalletMain, strDest)) {
		throw JSONRPCError(RPC_WALLET_ERROR, "Error: Wallet backup failed!");
	}
	return Value::null;
}

static void LockWallet(CWallet* pWallet) {
	LOCK(g_cWalletUnlockTime);
	g_llWalletUnlockTime = 0;
	pWallet->Lock();
}
/**
 * 定时解锁钱包
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value walletpassphrase(const Array& params, bool bHelp) {
    if (g_pwalletMain->IsCrypted() && (bHelp || params.size() != 2)) {
        throw runtime_error(
            "walletpassphrase \"passphrase\" timeout\n"
            "\nStores the wallet decryption key in memory for 'timeout' seconds.\n"
            "This is needed prior to performing transactions related to private keys such as sending Dacrss\n"
            "\nArguments:\n"
            "1. \"passphrase\"     (string, required) The wallet passphrase\n"
            "2. timeout            (numeric, required) The time to keep the decryption key in seconds.\n"
            "\nNote:\n"
            "Issuing the walletpassphrase command while the wallet is already unlocked will set a new unlock\n"
            "time that overrides the old one.\n"
            "\nExamples:\n"
            "\nunlock the wallet for 60 seconds\n"
            + HelpExampleCli("walletpassphrase", "\"my pass phrase\" 60") +
            "\nLock the wallet again (before 60 seconds)\n"
            + HelpExampleCli("walletlock", "") +
            "\nAs json rpc call\n"
            + HelpExampleRpc("walletpassphrase", "\"my pass phrase\", 60")
        );
    }
	LOCK2(g_cs_main, g_pwalletMain->m_cs_wallet);

	if (bHelp) {
		return true;
	}
	if (!g_pwalletMain->IsCrypted()) {
		throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE,
				"Error: running with an unencrypted wallet, but walletpassphrase was called.");
	}
	// Note that the walletpassphrase is stored in params[0] which is not mlock()ed
	SecureString strWalletPass;
	strWalletPass.reserve(100);
	// TODO: get rid of this .c_str() by implementing SecureString::operator=(string)
	// Alternately, find a way to make params[0] mlock()'d to begin with.
	strWalletPass = params[0].get_str().c_str();
	//assert(0);
	if (strWalletPass.length() > 0) {
		if (!g_pwalletMain->Unlock(strWalletPass)) {
			throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT, "Error: The wallet passphrase entered was incorrect.");
		}
	} else {
		throw runtime_error("walletpassphrase <passphrase> <timeout>\n"
				"Stores the wallet decryption key in memory for <timeout> seconds.");
	}
    int64_t llSleepTime = params[1].get_int64();
    LOCK(g_cWalletUnlockTime);
    g_llWalletUnlockTime = GetTime() + llSleepTime;
    RPCRunLater("lockwallet", boost::bind(LockWallet, g_pwalletMain), llSleepTime);
    Object retObj;
    retObj.push_back(Pair("passphrase", true));
    return retObj;
}
/**
 * 修改钱包密码
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value walletpassphrasechange(const Array& params, bool bHelp) {
    if (g_pwalletMain->IsCrypted() && (bHelp || params.size() != 2)) {
        throw runtime_error(
            "walletpassphrasechange \"oldpassphrase\" \"newpassphrase\"\n"
            "\nChanges the wallet passphrase from 'oldpassphrase' to 'newpassphrase'.\n"
            "\nArguments:\n"
            "1. \"oldpassphrase\"      (string, required) The current passphrase\n"
            "2. \"newpassphrase\"      (string, required) The new passphrase\n"
            "\nExamples:\n"
            + HelpExampleCli("walletpassphrasechange", "\"old one\" \"new one\"")
            + HelpExampleRpc("walletpassphrasechange", "\"old one\", \"new one\"")
        );
    }
	if (bHelp) {
		return true;
	}
	if (!g_pwalletMain->IsCrypted()) {
		throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE,
				"Error: running with an unencrypted wallet, but walletpassphrasechange was called.");
	}
    // TODO: get rid of these .c_str() calls by implementing SecureString::operator=(string)
    // Alternately, find a way to make params[0] mlock()'d to begin with.
    SecureString strOldWalletPass;
    strOldWalletPass.reserve(100);
    strOldWalletPass = params[0].get_str().c_str();

    SecureString strNewWalletPass;
    strNewWalletPass.reserve(100);
    strNewWalletPass = params[1].get_str().c_str();

	if (strOldWalletPass.length() < 1 || strNewWalletPass.length() < 1) {
		throw runtime_error("walletpassphrasechange <oldpassphrase> <newpassphrase>\n"
				"Changes the wallet passphrase from <oldpassphrase> to <newpassphrase>.");
	}
	if (!g_pwalletMain->ChangeWalletPassphrase(strOldWalletPass, strNewWalletPass)) {
		throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT, "Error: The wallet passphrase entered was incorrect.");
	}
	Object retObj;
	retObj.push_back(Pair("chgpwd", true));
	return retObj;
}
/**
 * 锁钱包
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value walletlock(const Array& params, bool bHelp) {
    if (g_pwalletMain->IsCrypted() && (bHelp || params.size() != 0)) {
        throw runtime_error(
            "walletlock\n"
            "\nRemoves the wallet encryption key from memory, locking the wallet.\n"
            "After calling this method, you will need to call walletpassphrase again\n"
            "before being able to call any methods which require the wallet to be unlocked.\n"
            "\nExamples:\n"
            "\nSet the passphrase for 2 minutes to perform a transaction\n"
            + HelpExampleCli("walletpassphrase", "\"my pass phrase\" 120") +
            "\nPerform a send (requires passphrase set)\n"
            + HelpExampleCli("sendtoaddress", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 1.0") +
            "\nClear the passphrase since we are done before 2 minutes is up\n"
            + HelpExampleCli("walletlock", "") +
            "\nAs json rpc call\n"
            + HelpExampleRpc("walletlock", "")
        );
    }
	if (bHelp) {
		return true;
	}
	if (!g_pwalletMain->IsCrypted()) {
		throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE,
				"Error: running with an unencrypted wallet, but walletlock was called.");
	}
    {
        LOCK(g_cWalletUnlockTime);
        g_pwalletMain->Lock();
        g_llWalletUnlockTime = 0;
    }
    Object retObj;
    retObj.push_back(Pair("walletlock", true));
    return retObj;
}
/**
 * 加密钱包
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value encryptwallet(const Array& params, bool bHelp) {
    if (!g_pwalletMain->IsCrypted() && (bHelp || params.size() != 1)) {
        throw runtime_error(
            "encryptwallet \"passphrase\"\n"
            "\nEncrypts the wallet with 'passphrase'. This is for first time encryption.\n"
            "After this, any calls that interact with private keys such as sending or signing \n"
            "will require the passphrase to be set prior the making these calls.\n"
            "Use the walletpassphrase call for this, and then walletlock call.\n"
            "If the wallet is already encrypted, use the walletpassphrasechange call.\n"
            "Note that this will shutdown the server.\n"
            "\nArguments:\n"
            "1. \"passphrase\"    (string, required) The pass phrase to encrypt the wallet with. It must be at least 1 character, but should be long.\n"
            "\nExamples:\n"
            "\nEncrypt you wallet\n"
            + HelpExampleCli("encryptwallet", "\"my pass phrase\"") +
            "\nNow set the passphrase to use the wallet, such as for signing or sending Dacrs\n"
            + HelpExampleCli("walletpassphrase", "\"my pass phrase\"") +
            "\nNow we can so something like sign\n"
            + HelpExampleCli("signmessage", "\"Dacrsaddress\" \"test message\"") +
            "\nNow lock the wallet again by removing the passphrase\n"
            + HelpExampleCli("walletlock", "") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("encryptwallet", "\"my pass phrase\"")
        );
    }
	LOCK2(g_cs_main, g_pwalletMain->m_cs_wallet);

	if (bHelp) {
		return true;
	}
	if (g_pwalletMain->IsCrypted()) {
		throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE,
				"Error: running with an encrypted wallet, but encryptwallet was called.");
	}
	// TODO: get rid of this .c_str() by implementing SecureString::operator=(string)
	// Alternately, find a way to make params[0] mlock()'d to begin with.
	SecureString strWalletPass;
	strWalletPass.reserve(100);
	strWalletPass = params[0].get_str().c_str();

	if (strWalletPass.length() < 1) {
		throw runtime_error("encryptwallet <passphrase>\n"
				"Encrypts the wallet with <passphrase>.");
	}
	if (!g_pwalletMain->EncryptWallet(strWalletPass)) {
		throw JSONRPCError(RPC_WALLET_ENCRYPTION_FAILED, "Error: Failed to encrypt the wallet.");
	}

    StartShutdown();

    Object retObj;
    retObj.push_back(Pair("encrypt", true));
    return retObj;
    //return "wallet encrypted; Dacrs server stopping, restart to run with encrypted wallet. The keypool has been flushed, you need to make a new backup.";
}
/**
 * 设置交易小费
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value settxfee(const Array& params, bool bHelp) {
    if (bHelp || params.size() < 1 || params.size() > 1) {
        throw runtime_error(
            "settxfee \"amount\"\n"
            "\nSet the transaction fee per kB.\n"
            "\nArguments:\n"
            "1. amount         (numeric, required) The transaction fee in BTC/kB rounded to the nearest 0.00000001\n"
            "\nResult\n"
            "true|false        (boolean) Returns true if successful\n"
            "\nExamples:\n"
            + HelpExampleCli("settxfee", "0.00001")
            + HelpExampleRpc("settxfee", "0.00001")
        );
    }
    // Amount
	int64_t llAmount = 0;
	if (params[0].get_real() != 0.0) {
		llAmount = AmountToRawValue(params[0]);        // rejects 0.0 amounts
		SysCfg().SetDeflautTxFee(llAmount);
	}

	return true;
}
/**
 * 获取钱包信息
 * @param params 输入参数
 * @param bHelp 输出帮助信息。
 * @return
 */
Value getwalletinfo(const Array& params, bool bHelp) {
    if (bHelp || params.size() != 0) {
        throw runtime_error(
            "getwalletinfo\n"
            "Returns an object containing various wallet state info.\n"
            "\nResult:\n"
            "{\n"
            "  \"walletversion\": xxxxx,     (numeric) the wallet version\n"
            "  \"balance\": xxxxxxx,         (numeric) the total Dacrs balance of the wallet\n"
            "  \"Inblocktx\": xxxxxxx,       (numeric) the size of transactions in the wallet\n"
            "  \"uncomfirmedtx\": xxxxxx,    (numeric) the size of unconfirmtx transactions in the wallet\n"
            "  \"unlocked_until\": ttt,      (numeric) the timestamp in seconds since epoch (midnight Jan 1 1970 GMT) that the wallet is unlocked for transfers, or 0 if the wallet is locked\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getwalletinfo", "")
            + HelpExampleRpc("getwalletinfo", "")
        );
    }
	Object obj;
	obj.push_back(Pair("walletversion", g_pwalletMain->GetVersion()));
	obj.push_back(Pair("balance", ValueFromAmount(g_pwalletMain->GetRawBalance())));
	obj.push_back(Pair("Inblocktx", (int) g_pwalletMain->m_mapInBlockTx.size()));
	obj.push_back(Pair("unconfirmtx", (int) g_pwalletMain->m_mapUnConfirmTx.size()));
	if (g_pwalletMain->IsCrypted()) {
		obj.push_back(Pair("unlocked_until", g_llWalletUnlockTime));
	}
	return obj;
}
