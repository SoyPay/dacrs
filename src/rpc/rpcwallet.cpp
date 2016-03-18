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

int64_t nWalletUnlockTime;
static CCriticalSection cs_nWalletUnlockTime;

string HelpRequiringPassphrase()
{
    return pwalletMain && pwalletMain->IsCrypted()
        ? "\nRequires wallet passphrase to be set with walletpassphrase call."
        : "";
}

void EnsureWalletIsUnlocked()
{
    if (pwalletMain->IsLocked())
        throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");
}

Value islocked(const Array& params,  bool fHelp)
{
	if(fHelp)
		return true;
	Object obj;
	if(!pwalletMain->IsCrypted()) {        //decrypted
		obj.push_back(Pair("islock", 0));
	}
	else if (!pwalletMain->IsLocked()) {   //encryped and unlocked
		obj.push_back(Pair("islock", 1));
	}else {
		obj.push_back(Pair("islock", 2)); //encryped and locked
	}
	return obj;
}

Value getnewaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "getnewaddress  (\"IsMiner\")\n"
            "\nget a new address\n"
            "\nArguments:\n"
        	"1. \"IsMiner\"  (bool, optional)  private key Is used for miner if true will create tow key ,another for miner.\n"
           "\nExamples:\n"
            + HelpExampleCli("getnewaddress", "")
            + HelpExampleCli("getnewaddress", "true")
        );
	EnsureWalletIsUnlocked();

    CKey  mCkey;
    mCkey.MakeNewKey();

    CKey  Minter;
    bool IsForMiner = false;
	if (params.size() == 1) {
		RPCTypeCheck(params, list_of(bool_type));
		Minter.MakeNewKey();
		IsForMiner = params[0].get_bool();
	}

    CPubKey newKey = mCkey.GetPubKey();

    CKeyID keyID = newKey.GetKeyID();

	if (IsForMiner) {
		if (!pwalletMain->AddKey(mCkey, Minter))
			throw runtime_error("add key failed ");
	}
	else if (!pwalletMain->AddKey(mCkey)) {
		throw runtime_error("add key failed ");
	}
	Object obj;
	obj.push_back(Pair("addr", keyID.ToAddress()));
	obj.push_back(Pair("minerpubkey", IsForMiner?Minter.GetPubKey().ToString(): "no" ));
	return obj;
}

Value signmessage(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 2)
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

    EnsureWalletIsUnlocked();

    string strAddress = params[0].get_str();
    string strMessage = params[1].get_str();


    CKeyID keyID(strAddress);
    if (keyID.IsEmpty())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    vector<unsigned char> vchSig;
    if (!key.SignCompact(ss.GetHash(), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    return EncodeBase64(&vchSig[0], vchSig.size());
}

Value sendtoaddresswithfee(const Array& params, bool fHelp)
 {
	int size = params.size();
	if (fHelp || (!(size == 3 || size == 4)))
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

	EnsureWalletIsUnlocked();
	CKeyID sendKeyId;
	CKeyID RevKeyId;

	auto GetKeyId = [](string const &addr,CKeyID &KeyId) {
		if (!CRegID::GetKeyID(addr, KeyId)) {
			KeyId=CKeyID(addr);
			if (KeyId.IsEmpty())
			return false;
		}
		return true;
	};

	// Amount
	int64_t nAmount = 0;
	int64_t nFee = 0;
	//// from address to addreww
	if (size == 4) {

		if (!GetKeyId(params[0].get_str(), sendKeyId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM Invalid  address");
		}
		if (!GetKeyId(params[1].get_str(), RevKeyId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "to Invalid  address");
		}

		nAmount = AmountToRawValue(params[2]);
		if (pAccountViewTip->GetRawBalance(sendKeyId) <= nAmount + SysCfg().GetTxFee()) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM address not enough coins");
		}
		nFee = AmountToRawValue(params[3]);
	} else {
		if (!GetKeyId(params[0].get_str(), RevKeyId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "to address Invalid  ");
		}

		nAmount = AmountToRawValue(params[1]);
		set<CKeyID> sKeyid;
		sKeyid.clear();
		pwalletMain->GetKeys(sKeyid);
		if (sKeyid.empty()) //get addrs
		{
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No Key In wallet \n");
		}
		nFee = AmountToRawValue(params[2]);
		for (auto &te : sKeyid) {
			if (pAccountViewTip->GetRawBalance(te) >= nAmount + SysCfg().GetTxFee()) {
				sendKeyId = te;
				break;
			}
		}

		if (sendKeyId.IsNull()) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "not find enough moeny account ");
		}
	}

	auto SendMoney = [&](const CRegID &send, const CUserID &rsv, int64_t nValue, int64_t nFee) {
		CTransaction tx;
		tx.srcRegId = send;
		tx.desUserId = rsv;
		tx.llValues = nValue;
		if (0 == nFee) {
			tx.llFees = SysCfg().GetTxFee();
		} else
		tx.llFees = nFee;
		tx.nValidHeight = chainActive.Tip()->nHeight;

		CKeyID keID;
		if(!pAccountViewTip->GetKeyId(send,keID)) {
			return std::make_tuple (false,"key or keID failed");
		}

		if (!pwalletMain->Sign(keID,tx.SignatureHash(), tx.signature)) {
			return std::make_tuple (false,"Sign failed");
		}
		std::tuple<bool,string> ret = pwalletMain->CommitTransaction((CBaseTransaction *) &tx);
		bool falg = std::get<0>(ret);
		string te = std::get<1>(ret);
		if(falg == true)
		te = tx.GetHash().ToString();
		return std::make_tuple (falg,te.c_str());
	};


	CRegID sendreg;
	CRegID revreg;


	if (!pAccountViewTip->GetRegId(CUserID(sendKeyId), sendreg)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid  address");
	}

	std::tuple<bool,string> ret;
	if (pAccountViewTip->GetRegId(CUserID(RevKeyId), revreg)) {
		ret = SendMoney(sendreg, revreg, nAmount, nFee);
	} else {


		ret = SendMoney(sendreg, CUserID(RevKeyId), nAmount, nFee);
	}

	Object obj;
	obj.push_back(Pair(std::get<0>(ret) ? "hash" : "error code", std::get<1>(ret)));
	return obj;
}

Value sendtoaddressraw(const Array& params, bool fHelp)
{
	int size = params.size();
	if (fHelp || size < 4 || size > 5 )
		throw runtime_error(
						"sendtoaddressraw \"height\" \"fee\" \"amount\" \"srcaddress\" \"recvaddress\"\n"
						"\n create normal transaction by hegiht,fee,amount,srcaddress, recvaddress.\n"
						+ HelpRequiringPassphrase() + "\nArguments:\n"
						"1. \"fee\"     (numeric, required)  \n"
						"2. \"amount\"  (numeric, required)  \n"
						"3. \"srcaddress\"  (string, required) The Dacrs address to send to.\n"
						"4. \"recvaddress\"  (string, required) The Dacrs address to receive.\n"
						"5. \"height\"  (int, optional) \n"
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

	CKeyID sendKeyId;
	CKeyID RevKeyId;

	auto GetUserID = [](string const &addr,CUserID &userid) {
		CRegID te(addr);
		if(!te.IsEmpty()) {
			userid = te;
			return true;
		}
		CKeyID kid(addr);
		if(!kid.IsEmpty()) {
			userid = kid;
			return true;
		}
		return false;
	};

	int64_t Fee = AmountToRawValue(params[0]);


	int64_t nAmount = 0;

    nAmount = AmountToRawValue(params[1]);
	if(nAmount == 0){
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM Invalid nAmount == 0");
	}

	CUserID  send;
	CUserID  rev;
	if(!GetUserID(params[2].get_str(),send)){
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM Invalid send address");
	}
	if(!pAccountViewTip->GetKeyId(send, sendKeyId)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Get CKeyID failed from CUserID");
	}
	if(send.type() == typeid(CKeyID)){
		CRegID regId;
		if(!pAccountViewTip->GetRegId(send,regId)){
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "CKeyID is not registed ");
		}
		send = regId;
	}

	if(!GetUserID(params[3].get_str(),rev)){
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM Invalid rev address");
	}

	if(rev.type() == typeid(CKeyID)){
		CRegID regId;
		if(pAccountViewTip->GetRegId(rev,regId)){
			rev = regId;
		}
	}

	int hight = chainActive.Tip()->nHeight;
	if(params.size() > 4) {
		hight = params[4].get_int();
	}

	std::shared_ptr<CTransaction> tx = make_shared<CTransaction>(send,rev,Fee, nAmount,hight);
	if (!pwalletMain->Sign(sendKeyId, tx->SignatureHash(), tx->signature)) {
				throw JSONRPCError(RPC_INVALID_PARAMETER,  "Sign failed");
	}
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
	ds << pBaseTx;
	Object obj;
	obj.push_back(Pair("rawtx", HexStr(ds.begin(), ds.end())));
	return obj;

}

Value sendtoaddress(const Array& params, bool fHelp)
 {
	int size = params.size();
	if (fHelp || (!(size == 2 || size == 3)))
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

	EnsureWalletIsUnlocked();
	CKeyID sendKeyId;
	CKeyID RevKeyId;

	auto GetKeyId = [](string const &addr,CKeyID &KeyId) {
		if (!CRegID::GetKeyID(addr, KeyId)) {
			KeyId=CKeyID(addr);
			if (KeyId.IsEmpty())
			return false;
		}
		return true;
	};

	// Amount
	int64_t nAmount = 0;
	CRegID sendreg;
	//// from address to addreww
	if (size == 3) {

		if (!GetKeyId(params[0].get_str(), sendKeyId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM Invalid  address");
		}
		if (!GetKeyId(params[1].get_str(), RevKeyId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "to Invalid  address");
		}

		nAmount = AmountToRawValue(params[2]);
		if (pAccountViewTip->GetRawBalance(sendKeyId) <= nAmount + SysCfg().GetTxFee()) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM address not enough coins");
		}
	} else {
		if (!GetKeyId(params[0].get_str(), RevKeyId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "to address Invalid  ");
		}

		nAmount = AmountToRawValue(params[1]);
		set<CKeyID> sKeyid;
		sKeyid.clear();
		pwalletMain->GetKeys(sKeyid); //get addrs
		if(sKeyid.empty())
		{
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No Key In wallet \n");
		}
		for (auto &te : sKeyid) {
			if (pAccountViewTip->GetRawBalance(te) >= nAmount + SysCfg().GetTxFee()) {
				if (pAccountViewTip->GetRegId(CUserID(te), sendreg)) {
					sendKeyId = te;
					break;
				}
			}
		}

		if (sendKeyId.IsNull()) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "not find enough moeny account ");
		}
	}

	CRegID revreg;
	CUserID rev;

	if (!pAccountViewTip->GetRegId(CUserID(sendKeyId), sendreg)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid  address");
	}

	if (pAccountViewTip->GetRegId(CUserID(RevKeyId), revreg)) {
		rev = revreg;
	} else {
		rev = RevKeyId;
	}

	CTransaction tx(sendreg, rev, SysCfg().GetTxFee(), nAmount, chainActive.Height());

	if (!pwalletMain->Sign(sendKeyId, tx.SignatureHash(), tx.signature)) {
		throw JSONRPCError(RPC_INVALID_PARAMETER,  "Sign failed");
	}

	std::tuple<bool,string> ret = pwalletMain->CommitTransaction((CBaseTransaction *) &tx);
	if(!std::get<0>(ret))
		 throw JSONRPCError(RPC_INVALID_PARAMETER,  std::get<1>(ret));

	Object obj;
	obj.push_back(Pair("hash" , std::get<1>(ret)));
	return obj;
}

Value backupwallet(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "backupwallet \"destination\"\n"
            "\nSafely copies wallet.dat to destination, which can be a directory or a path with filename.\n"
            "\nArguments:\n"
            "1. \"destination\"   (string, required) The destination directory or file\n"
            "\nExamples:\n"
            + HelpExampleCli("backupwallet", "\"backup.dat\"")
            + HelpExampleRpc("backupwallet", "\"backup.dat\"")
        );

    string strDest = params[0].get_str();
    if (!BackupWallet(*pwalletMain, strDest))
        throw JSONRPCError(RPC_WALLET_ERROR, "Error: Wallet backup failed!");

    return Value::null;
}

static void LockWallet(CWallet* pWallet)
{
    LOCK(cs_nWalletUnlockTime);
    nWalletUnlockTime = 0;
    pWallet->Lock();
}

Value walletpassphrase(const Array& params, bool fHelp)
{
    if (pwalletMain->IsCrypted() && (fHelp || params.size() != 2))
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

    LOCK2(cs_main, pwalletMain->cs_wallet);

    if (fHelp)
        return true;
    if (!pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletpassphrase was called.");

    // Note that the walletpassphrase is stored in params[0] which is not mlock()ed
    SecureString strWalletPass;
    strWalletPass.reserve(100);
    // TODO: get rid of this .c_str() by implementing SecureString::operator=(string)
    // Alternately, find a way to make params[0] mlock()'d to begin with.
    strWalletPass = params[0].get_str().c_str();
	//assert(0);
    if (strWalletPass.length() > 0)
    {
        if (!pwalletMain->Unlock(strWalletPass))
            throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT, "Error: The wallet passphrase entered was incorrect.");
    }
    else
        throw runtime_error(
            "walletpassphrase <passphrase> <timeout>\n"
            "Stores the wallet decryption key in memory for <timeout> seconds.");

    int64_t nSleepTime = params[1].get_int64();
    LOCK(cs_nWalletUnlockTime);
    nWalletUnlockTime = GetTime() + nSleepTime;
    RPCRunLater("lockwallet", boost::bind(LockWallet, pwalletMain), nSleepTime);
    Object retObj;
    retObj.push_back(Pair("passphrase", true));
    return retObj;
}

Value walletpassphrasechange(const Array& params, bool fHelp)
{
    if (pwalletMain->IsCrypted() && (fHelp || params.size() != 2))
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

    if (fHelp)
        return true;
    if (!pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletpassphrasechange was called.");

    // TODO: get rid of these .c_str() calls by implementing SecureString::operator=(string)
    // Alternately, find a way to make params[0] mlock()'d to begin with.
    SecureString strOldWalletPass;
    strOldWalletPass.reserve(100);
    strOldWalletPass = params[0].get_str().c_str();

    SecureString strNewWalletPass;
    strNewWalletPass.reserve(100);
    strNewWalletPass = params[1].get_str().c_str();

    if (strOldWalletPass.length() < 1 || strNewWalletPass.length() < 1)
        throw runtime_error(
            "walletpassphrasechange <oldpassphrase> <newpassphrase>\n"
            "Changes the wallet passphrase from <oldpassphrase> to <newpassphrase>.");

    if (!pwalletMain->ChangeWalletPassphrase(strOldWalletPass, strNewWalletPass))
        throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT, "Error: The wallet passphrase entered was incorrect.");
    Object retObj;
    retObj.push_back(Pair("chgpwd", true));
    return retObj;
}

Value walletlock(const Array& params, bool fHelp)
{
    if (pwalletMain->IsCrypted() && (fHelp || params.size() != 0))
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

    if (fHelp)
        return true;
    if (!pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletlock was called.");
    {
        LOCK(cs_nWalletUnlockTime);
        pwalletMain->Lock();
        nWalletUnlockTime = 0;
    }
    Object retObj;
    retObj.push_back(Pair("walletlock", true));
    return retObj;
}

Value encryptwallet(const Array& params, bool fHelp)
{
    if (!pwalletMain->IsCrypted() && (fHelp || params.size() != 1))
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
    LOCK2(cs_main, pwalletMain->cs_wallet);

    if (fHelp)
        return true;
    if (pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an encrypted wallet, but encryptwallet was called.");

    // TODO: get rid of this .c_str() by implementing SecureString::operator=(string)
    // Alternately, find a way to make params[0] mlock()'d to begin with.
    SecureString strWalletPass;
    strWalletPass.reserve(100);
    strWalletPass = params[0].get_str().c_str();

    if (strWalletPass.length() < 1)
        throw runtime_error(
            "encryptwallet <passphrase>\n"
            "Encrypts the wallet with <passphrase>.");

    if (!pwalletMain->EncryptWallet(strWalletPass))
        throw JSONRPCError(RPC_WALLET_ENCRYPTION_FAILED, "Error: Failed to encrypt the wallet.");

    //BDB seems to have a bad habit of writing old data into
    //slack space in .dat files; that is bad if the old data is
    //unencrypted private keys. So:
    StartShutdown();

//    string defaultFilename = SysCfg().GetArg("-wallet", "wallet.dat");
//    string strFileCopy = defaultFilename + ".rewrite";
//
//    boost::filesystem::remove(GetDataDir() / defaultFilename);
//    boost::filesystem::rename(GetDataDir() / strFileCopy, GetDataDir() / defaultFilename);

    Object retObj;
    retObj.push_back(Pair("encrypt", true));
    return retObj;
    //return "wallet encrypted; Dacrs server stopping, restart to run with encrypted wallet. The keypool has been flushed, you need to make a new backup.";
}

Value settxfee(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 1)
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

    // Amount
    int64_t nAmount = 0;
    if (params[0].get_real() != 0.0)
    {
       nAmount = AmountToRawValue(params[0]);        // rejects 0.0 amounts
       SysCfg().SetDeflautTxFee(nAmount);
    }

    return true;
}

Value getwalletinfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
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

    Object obj;
    obj.push_back(Pair("walletversion", pwalletMain->GetVersion()));
    obj.push_back(Pair("balance",       ValueFromAmount(pwalletMain->GetRawBalance())));
    obj.push_back(Pair("Inblocktx",       (int)pwalletMain->mapInBlockTx.size()));
    obj.push_back(Pair("unconfirmtx", (int)pwalletMain->UnConfirmTx.size()));
    if (pwalletMain->IsCrypted())
        obj.push_back(Pair("unlocked_until", nWalletUnlockTime));
    return obj;
}
