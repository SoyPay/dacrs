// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"
#include "init.h"
#include "main.h"
#include "net.h"
#include "netbase.h"
#include "rpcserver.h"
#include "util.h"
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

static bool GetKeyId(string const &strAddr, CKeyID &cKeyId) {
	if (!CRegID::GetKeyID(strAddr, cKeyId)) {
		cKeyId = CKeyID(strAddr);
		if (cKeyId.IsEmpty()) {
			return false;
		}
	}
	return true;
}

Value getbalance(const Array& params, bool bHelp) {
	int nSize = params.size();
	if (bHelp || params.size() > 2) {
		string msg = "getbalance ( \"address\" minconf )\n"
				"\nIf account is not specified, returns the server's total available balance.\n"
				"If account is specified (DEPRECATED), returns the balance in the account.\n"
				"\nArguments:\n"
				"1. \"address\"      (string, optional) DEPRECATED. The selected account or \"*\" for entire wallet.\n"
				"2.  minconf         (numeric, optional, default=1) Only include transactions confirmed\n"
				"\nExamples:\n"
				+ HelpExampleCli("getbalance", "de3nGsPR6i9qpQTNnpC9ASMpFKbKzzFLYF 0")+ "\nAs json rpc call\n"
				+ HelpExampleRpc("getbalance", "de3nGsPR6i9qpQTNnpC9ASMpFKbKzzFLYF 0");
		throw runtime_error(msg);
	}
	Object obj;
	if (nSize == 0) {
		obj.push_back(Pair("balance", ValueFromAmount(g_pwalletMain->GetRawBalance())));
		return std::move(obj);
	} else if (nSize == 1) {
		string strAddr = params[0].get_str();
		if (strAddr == "*") {
			obj.push_back(Pair("balance", ValueFromAmount(g_pwalletMain->GetRawBalance())));
			return std::move(obj);
		} else {
			CKeyID ckeyid;
			if (!GetKeyId(strAddr, ckeyid)) {
				throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid  address");
			}
			if (g_pwalletMain->HaveKey(ckeyid)) {
				CAccount cAccount;
				CAccountViewCache cAccView(*g_pAccountViewTip, true);
				if (cAccView.GetAccount(CUserID(ckeyid), cAccount)) {
					obj.push_back(Pair("balance", ValueFromAmount(cAccount.GetRawBalance())));
					return std::move(obj);
				}
			} else {
				throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "address not inwallet");
			}
		}
	} else if (nSize == 2) {
		string strAddr = params[0].get_str();
		int nConf = params[1].get_int();
		int nMaxConf = SysCfg().GetArg("-maxconf", 30);
		if (nConf > nMaxConf) {
			throw JSONRPCError(RPC_INVALID_PARAMETER, "parameter minconf exceed maxconfed");
		}
		if (strAddr == "*") {
			if (0 != nConf) {
				CBlockIndex *pBlockIndex = g_cChainActive.Tip();
				int64_t llValue(0);
				while (nConf) {
					if (g_pwalletMain->m_mapInBlockTx.count(pBlockIndex->GetBlockHash()) > 0) {
						map<uint256, std::shared_ptr<CBaseTransaction> > mapTx =
								g_pwalletMain->m_mapInBlockTx[pBlockIndex->GetBlockHash()].m_mapAccountTx;
						for (auto &item : mapTx) {
							if (EM_COMMON_TX == item.second->m_chTxType) {
								CTransaction *pTx = (CTransaction *) item.second.get();
								CKeyID cSrcKeyId, cDesKeyId;
								g_pAccountViewTip->GetKeyId(pTx->m_cSrcRegId, cSrcKeyId);
								g_pAccountViewTip->GetKeyId(pTx->m_cDesUserId, cDesKeyId);
								if (!g_pwalletMain->HaveKey(cSrcKeyId) && g_pwalletMain->HaveKey(cDesKeyId)) {
									llValue = pTx->m_ullValues;
								}
							}
						}
					}
					pBlockIndex = pBlockIndex->m_pPrevBlockIndex;
					--nConf;
				}
				obj.push_back(Pair("balance", ValueFromAmount(g_pwalletMain->GetRawBalance() - llValue)));
				return std::move(obj);
			} else {
				obj.push_back(Pair("balance", ValueFromAmount(g_pwalletMain->GetRawBalance(false))));
				return std::move(obj);
			}
		} else {
			CKeyID cKeyID;
			if (!GetKeyId(strAddr, cKeyID)) {
				throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid  address");
			}
			if (g_pwalletMain->HaveKey(cKeyID)) {
				if (0 != nConf) {
					CBlockIndex *pBlockIndex = g_cChainActive.Tip();
					int64_t llValue(0);
					while (nConf) {
						if (g_pwalletMain->m_mapInBlockTx.count(pBlockIndex->GetBlockHash()) > 0) {
							map<uint256, std::shared_ptr<CBaseTransaction> > mapTx =
									g_pwalletMain->m_mapInBlockTx[pBlockIndex->GetBlockHash()].m_mapAccountTx;
							for (auto &item : mapTx) {
								if (EM_COMMON_TX == item.second->m_chTxType) {
									CTransaction *pTx = (CTransaction *) item.second.get();
									CKeyID cSrcKeyId, cDesKeyId;
									g_pAccountViewTip->GetKeyId(pTx->m_cDesUserId, cDesKeyId);
									if (cKeyID == cDesKeyId) {
										llValue = pTx->m_ullValues;
									}
								}
							}
						}
						pBlockIndex = pBlockIndex->m_pPrevBlockIndex;
						--nConf;
					}
					obj.push_back(Pair("balance", ValueFromAmount(g_pAccountViewTip->GetRawBalance(cKeyID) - llValue)));
					return std::move(obj);
				} else {
					obj.push_back(Pair("balance", ValueFromAmount(g_cTxMemPool.m_pAccountViewCache->GetRawBalance(cKeyID))));
					return std::move(obj);
				}
			} else {
				throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "address not inwallet");
			}
		}
	}

	throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid  address");
}

Value getinfo(const Array& params, bool bHelp) {
    if (bHelp || params.size() != 0) {
        throw runtime_error(
            "getinfo\n"
            "\nget various state information.\n"
			"\nArguments:\n"
            "Returns an object containing various state info.\n"
            "\nResult:\n"
            "{\n"
            "  \"version\": xxxxx,           (numeric) the server version\n"
        	"  \"fullversion\": xxxxx,       (string) the server fullversion\n"
            "  \"protocolversion\": xxxxx,   (numeric) the protocol version\n"
            "  \"walletversion\": xxxxx,     (numeric) the wallet version\n"
            "  \"balance\": xxxxxxx,         (numeric) the total Dacrs balance of the wallet\n"
            "  \"blocks\": xxxxxx,           (numeric) the current number of blocks processed in the server\n"
            "  \"timeoffset\": xxxxx,        (numeric) the time offset\n"
            "  \"connections\": xxxxx,       (numeric) the number of connections\n"
            "  \"proxy\": \"host:port\",     (string, optional) the proxy used by the server\n"
            "  \"difficulty\": xxxxxx,       (numeric) the current difficulty\n"
        	"  \"nettype\": xxxxx,           (string) the net type\n"
            "  \"chainwork\": xxxxxx,        (string) the  chainwork of the tip block in g_cChainActive\n"
            "  \"tipblocktime\": xxxx,       (numeric) the  nTime of the tip block in g_cChainActive\n"
            "  \"unlocked_until\": ttt,      (numeric) the timestamp in seconds since epoch (midnight Jan 1 1970 GMT) that the wallet is unlocked for transfers, or 0 if the wallet is locked\n"
            "  \"paytxfee\": x.xxxx,         (numeric) the transaction fee set in btc/kb\n"
            "  \"relayfee\": x.xxxx,         (numeric) minimum relay fee for non-free transactions in btc/kb\n"
        	"  \"fuelrate\": xxxxx,          (numeric) the  fuelrate of the tip block in g_cChainActive\n"
        	"  \"fuel\": xxxxx,              (numeric) the  fuel of the tip block in g_cChainActive\n"
        	"  \"data directory\": xxxxx,    (string) the data directory\n"
			"  \"tip block hash\": xxxxx,    (string) the tip block hash\n"
            "  \"errors\": \"...\"           (string) any error messages\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getinfo", "")
            + HelpExampleRpc("getinfo", "")
        );
    }
	proxyType proxy;
	GetProxy(NET_IPV4, proxy);

	Object obj;
	obj.push_back(Pair("version", (int) g_sClientVersion));
	string strFullVersion = strprintf("%s (%s)", FormatFullVersion().c_str(), g_strClientDate.c_str());
	obj.push_back(Pair("fullversion", strFullVersion));
	obj.push_back(Pair("protocolversion", (int) g_sProtocolVersion));

	if (g_pwalletMain) {
		obj.push_back(Pair("walletversion", g_pwalletMain->GetVersion()));
		obj.push_back(Pair("balance", ValueFromAmount(g_pwalletMain->GetRawBalance())));
	}
	static const string strName[] = { "MAIN", "TESTNET", "REGTEST" };

	obj.push_back(Pair("blocks", (int) g_cChainActive.Height()));
	obj.push_back(Pair("timeoffset", GetTimeOffset()));
	obj.push_back(Pair("connections", (int) g_vNodes.size()));
	obj.push_back(Pair("proxy", (proxy.first.IsValid() ? proxy.first.ToStringIPPort() : string())));
	obj.push_back(Pair("difficulty", (double) GetDifficulty()));
	obj.push_back(Pair("nettype", strName[SysCfg().NetworkID()]));
	obj.push_back(Pair("chainwork", g_cChainActive.Tip()->m_cChainWork.GetHex()));
	obj.push_back(Pair("tipblocktime", (int) g_cChainActive.Tip()->m_unTime));
	if (g_pwalletMain && g_pwalletMain->IsCrypted()) {
		obj.push_back(Pair("unlocked_until", g_llWalletUnlockTime));
	}
	obj.push_back(Pair("paytxfee", ValueFromAmount(SysCfg().GetTxFee())));

	obj.push_back(Pair("relayfee", ValueFromAmount(CTransaction::m_sMinRelayTxFee)));
	obj.push_back(Pair("fuelrate", g_cChainActive.Tip()->m_nFuelRate));
	obj.push_back(Pair("fuel", g_cChainActive.Tip()->m_llFuel));
	obj.push_back(Pair("data directory", GetDataDir().string().c_str()));
	//    obj.push_back(Pair("block high",    g_cChainActive.Tip()->nHeight));
	obj.push_back(Pair("tip block hash", g_cChainActive.Tip()->GetBlockHash().ToString()));
	obj.push_back(Pair("errors", GetWarnings("statusbar")));
	return obj;
}

class DescribeAddressVisitor: public boost::static_visitor<Object> {
 public:
	Object operator()(const CNoDestination &dest) const {
		return Object();
	}

	Object operator()(const CKeyID &keyID) const {
		Object obj;
		CPubKey cPubKey;
		g_pwalletMain->GetPubKey(keyID, cPubKey);
		obj.push_back(Pair("isscript", false));
		obj.push_back(Pair("pubkey", HexStr(cPubKey)));
		obj.push_back(Pair("iscompressed", cPubKey.IsCompressed()));
		return obj;
	}
};

Value verifymessage(const Array& params, bool bHelp) {
    if (bHelp || params.size() != 3) {
        throw runtime_error(
            "verifymessage \"Dacrsaddress\" \"signature\" \"message\"\n"
            "\nVerify a signed message\n"
            "\nArguments:\n"
            "1. \"Dacrsaddress or pubkey\"  (string, required) The Dacrs address to use for the signature.\n"
            "2. \"signature\"       (string, required) The signature provided by the signer in base 64 encoding (see signmessage).\n"
            "3. \"message\"         (string, required) The message that was signed.\n"
            "\nResult:\n"
            "true|false   (boolean) If the signature is verified or not.\n"
            "\nExamples:\n"
            "\nUnlock the wallet for 30 seconds\n"
            + HelpExampleCli("walletpassphrase", "\"mypassphrase\" 30") +
            "\nCreate the signature\n"
            + HelpExampleCli("signmessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\" \"my message\"") +
            "\nVerify the signature\n"
            + HelpExampleCli("verifymessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\" \"signature\" \"my message\"") +
            "\nAs json rpc\n"
            + HelpExampleRpc("verifymessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\", \"signature\", \"my message\"")
        );
    }
	string strAddress = params[0].get_str();
	string strSign = params[1].get_str();
	string strMessage = params[2].get_str();
	CKeyID ckeyID;
	if (strAddress.length() == 66) {   //pubkey
		vector<unsigned char> vuchPubKey = ParseHex(strAddress);
		CPubKey cPubkeyIn(vuchPubKey.begin(), vuchPubKey.end());
		ckeyID = cPubkeyIn.GetKeyID();
	} else {
		if (!GetKeyId(strAddress, ckeyID)) {
			throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");
		}
	}
	bool bInvalid = false;
	vector<unsigned char> vuchSig = DecodeBase64(strSign.c_str(), &bInvalid);

	if (bInvalid) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");
	}
	CHashWriter cHashWriter(SER_GETHASH, 0);
	cHashWriter << g_strMessageMagic;
	cHashWriter << strMessage;

	CPubKey cPubkey;
	if (!cPubkey.RecoverCompact(cHashWriter.GetHash(), vuchSig)) {
		return false;
	}
	return (cPubkey.GetKeyID() == ckeyID);
}
