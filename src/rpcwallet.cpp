// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"
#include "rpcserver.h"
#include "init.h"
#include "net.h"
#include "netbase.h"
#include "util.h"
#include "wallet.h"
#include "walletdb.h"

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
    if (pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");
}




Value getnewaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "getnewaddress \n"
            "\nArguments:\n"
        	"1. \"IsMiner\"  (bool)  private key Is used for miner if true will create tow key ,another for miner.\n"
           "\nExamples:\n"
            + HelpExampleCli("getnewaddress", "")
        );

    // Parse the account first so we don't generate a key if there's an error
    string str;


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
            "signmessage \"bitcoinaddress\" \"message\"\n"
            "\nSign a message with the private key of an address"
            + HelpRequiringPassphrase() + "\n"
            "\nArguments:\n"
            "1. \"soypayaddress\"  (string, required) The soypay address to use for the private key.\n"
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

    CSoyPayAddress addr(strAddress);
    if (!addr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

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

Value sendtoaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 4)
        throw runtime_error(
            "sendtoaddress \"bitcoinaddress\" amount "
            "\nSent an amount to a given address. The amount is a real and is rounded to the nearest 0.00000001\n"
            + HelpRequiringPassphrase() +
            "\nArguments:\n"
            "1. \"bitcoinaddress\"  (string, required) The bitcoin address to send to.\n"

            "\nResult:\n"
            "\"transactionid\"  (string) The transaction id.\n"
            "\nExamples:\n"
            + HelpExampleCli("sendtoaddress", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.1")
            + HelpExampleCli("sendtoaddress", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.1 \"donation\" \"seans outpost\"")
            + HelpExampleRpc("sendtoaddress", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\", 0.1, \"donation\", \"seans outpost\"")
        );

    EnsureWalletIsUnlocked();
    CKeyID sendKeyId;
    CKeyID RevKeyId;

    // Amount
    int64_t nAmount = 0;
    //// from address to addreww
    if(params.size() == 3)
    {
    	CSoyPayAddress fromaddress(params[0].get_str());
    	CSoyPayAddress tomaddress(params[1].get_str());
    	nAmount = AmountFromValue(params[2]);

    	fromaddress.GetKeyID(sendKeyId);
    	tomaddress.GetKeyID(RevKeyId);

		if (!fromaddress.IsValid() || !fromaddress.GetKeyID(sendKeyId) )
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM Invalid  address");
		if (!tomaddress.IsValid() || !tomaddress.GetKeyID(RevKeyId) )
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM Invalid  address");

		if (pAccountViewTip->GetBalance(sendKeyId, chainActive.Tip()->nHeight) >= nAmount + nTransactionFee) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "FROM address not enough");
					}
    }else{
    	CSoyPayAddress address(params[0].get_str());
    	 address.GetKeyID(RevKeyId);

		if (!address.IsValid() || !address.GetKeyID(RevKeyId) )
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid  address");

		set<CKeyID> sKeyid;
		for (auto &te : sKeyid) {
			if (pAccountViewTip->GetBalance(te, chainActive.Tip()->nHeight) >= nAmount + nTransactionFee) {
				sendKeyId =te;
				break;
			}
		}

	    if(sendKeyId == 0)
	    {
	    	 throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "not enough moeny");
	    }
    }



    CRegID sendreg;
    CRegID revreg;
    if(!pwalletMain->GetRegId(sendKeyId,sendreg)||!pAccountViewTip->GetRegId(CUserID(RevKeyId),revreg))
    {
    	 throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid  address");
    }
    return  pwalletMain->SendMoney(sendreg,revreg,nAmount);

}




struct tallyitem
{
    int64_t nAmount;
    int nConf;
    vector<uint256> txids;
    tallyitem()
    {
        nAmount = 0;
        nConf = numeric_limits<int>::max();
    }
};



//static void MaybePushAddress(Object & entry, const CTxDestination &dest)
//{
//    CSoyPayAddress addr;
//    if (addr.Set(dest))
//        entry.push_back(Pair("address", addr.ToString()));
//}




Value backupwallet(const Array& params, bool fHelp)
{
	  //!@todo  todo work list
	    throw JSONRPCError(RPC_INVALID_PARAMETER, "todo work list ");


//    if (fHelp || params.size() != 1)
//        throw runtime_error(
//            "backupwallet \"destination\"\n"
//            "\nSafely copies wallet.dat to destination, which can be a directory or a path with filename.\n"
//            "\nArguments:\n"
//            "1. \"destination\"   (string) The destination directory or file\n"
//            "\nExamples:\n"
//            + HelpExampleCli("backupwallet", "\"backup.dat\"")
//            + HelpExampleRpc("backupwallet", "\"backup.dat\"")
//        );
//
//    string strDest = params[0].get_str();
//    if (!BackupWallet(*pwalletMain, strDest))
//        throw JSONRPCError(RPC_WALLET_ERROR, "Error: Wallet backup failed!");
//
//    return Value::null;
}


static void LockWallet(CWallet* pWallet)
{
    LOCK(cs_nWalletUnlockTime);
    nWalletUnlockTime = 0;
//    pWallet->Lock();
}

Value walletpassphrase(const Array& params, bool fHelp)
{

	  //!@todo  todo work list
	    throw JSONRPCError(RPC_INVALID_PARAMETER, "todo work list ");


    if (pwalletMain->IsCrypted() && (fHelp || params.size() != 2))
        throw runtime_error(
            "walletpassphrase \"passphrase\" timeout\n"
            "\nStores the wallet decryption key in memory for 'timeout' seconds.\n"
            "This is needed prior to performing transactions related to private keys such as sending soypays\n"
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
				 LogPrint("TODO"," ");
	//assert(0);
    if (strWalletPass.length() > 0)
    {
//        if (!pwalletMain->Unlock(strWalletPass))
//            throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT, "Error: The wallet passphrase entered was incorrect.");
    }
    else
        throw runtime_error(
            "walletpassphrase <passphrase> <timeout>\n"
            "Stores the wallet decryption key in memory for <timeout> seconds.");

//    pwalletMain->TopUpKeyPool();

    int64_t nSleepTime = params[1].get_int64();
    LOCK(cs_nWalletUnlockTime);
    nWalletUnlockTime = GetTime() + nSleepTime;
    RPCRunLater("lockwallet", boost::bind(LockWallet, pwalletMain), nSleepTime);

    return Value::null;
}


Value walletpassphrasechange(const Array& params, bool fHelp)
{

	  //!@todo  todo work list
	    throw JSONRPCError(RPC_INVALID_PARAMETER, "todo work list ");

    if (pwalletMain->IsCrypted() && (fHelp || params.size() != 2))
        throw runtime_error(
            "walletpassphrasechange \"oldpassphrase\" \"newpassphrase\"\n"
            "\nChanges the wallet passphrase from 'oldpassphrase' to 'newpassphrase'.\n"
            "\nArguments:\n"
            "1. \"oldpassphrase\"      (string) The current passphrase\n"
            "2. \"newpassphrase\"      (string) The new passphrase\n"
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

//    if (!pwalletMain->ChangeWalletPassphrase(strOldWalletPass, strNewWalletPass))
//        throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT, "Error: The wallet passphrase entered was incorrect.");

    return Value::null;
}


Value walletlock(const Array& params, bool fHelp)
{
	  //!@todo  todo work list
	    throw JSONRPCError(RPC_INVALID_PARAMETER, "todo work list ");
//    if (pwalletMain->IsCrypted() && (fHelp || params.size() != 0))
//        throw runtime_error(
//            "walletlock\n"
//            "\nRemoves the wallet encryption key from memory, locking the wallet.\n"
//            "After calling this method, you will need to call walletpassphrase again\n"
//            "before being able to call any methods which require the wallet to be unlocked.\n"
//            "\nExamples:\n"
//            "\nSet the passphrase for 2 minutes to perform a transaction\n"
//            + HelpExampleCli("walletpassphrase", "\"my pass phrase\" 120") +
//            "\nPerform a send (requires passphrase set)\n"
//            + HelpExampleCli("sendtoaddress", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 1.0") +
//            "\nClear the passphrase since we are done before 2 minutes is up\n"
//            + HelpExampleCli("walletlock", "") +
//            "\nAs json rpc call\n"
//            + HelpExampleRpc("walletlock", "")
//        );

//    if (fHelp)
//        return true;
//    if (!pwalletMain->IsCrypted())
//        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletlock was called.");
//
//    {
//        LOCK(cs_nWalletUnlockTime);
//        pwalletMain->Lock();
//        nWalletUnlockTime = 0;
//    }

    return Value::null;
}


Value encryptwallet(const Array& params, bool fHelp)
{
  //!@todo  todo work list
    throw JSONRPCError(RPC_INVALID_PARAMETER, "todo work list ");
//    if (!pwalletMain->IsCrypted() && (fHelp || params.size() != 1))
//        throw runtime_error(
//            "encryptwallet \"passphrase\"\n"
//            "\nEncrypts the wallet with 'passphrase'. This is for first time encryption.\n"
//            "After this, any calls that interact with private keys such as sending or signing \n"
//            "will require the passphrase to be set prior the making these calls.\n"
//            "Use the walletpassphrase call for this, and then walletlock call.\n"
//            "If the wallet is already encrypted, use the walletpassphrasechange call.\n"
//            "Note that this will shutdown the server.\n"
//            "\nArguments:\n"
//            "1. \"passphrase\"    (string) The pass phrase to encrypt the wallet with. It must be at least 1 character, but should be long.\n"
//            "\nExamples:\n"
//            "\nEncrypt you wallet\n"
//            + HelpExampleCli("encryptwallet", "\"my pass phrase\"") +
//            "\nNow set the passphrase to use the wallet, such as for signing or sending soypay\n"
//            + HelpExampleCli("walletpassphrase", "\"my pass phrase\"") +
//            "\nNow we can so something like sign\n"
//            + HelpExampleCli("signmessage", "\"soypayaddress\" \"test message\"") +
//            "\nNow lock the wallet again by removing the passphrase\n"
//            + HelpExampleCli("walletlock", "") +
//            "\nAs a json rpc call\n"
//            + HelpExampleRpc("encryptwallet", "\"my pass phrase\"")
//        );
//
//    if (fHelp)
//        return true;
//    if (pwalletMain->IsCrypted())
//        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an encrypted wallet, but encryptwallet was called.");
//
//    // TODO: get rid of this .c_str() by implementing SecureString::operator=(string)
//    // Alternately, find a way to make params[0] mlock()'d to begin with.
//    SecureString strWalletPass;
//    strWalletPass.reserve(100);
//    strWalletPass = params[0].get_str().c_str();
//
//    if (strWalletPass.length() < 1)
//        throw runtime_error(
//            "encryptwallet <passphrase>\n"
//            "Encrypts the wallet with <passphrase>.");
//
//    if (!pwalletMain->EncryptWallet(strWalletPass))
//        throw JSONRPCError(RPC_WALLET_ENCRYPTION_FAILED, "Error: Failed to encrypt the wallet.");

    // BDB seems to have a bad habit of writing old data into
    // slack space in .dat files; that is bad if the old data is
    // unencrypted private keys. So:
    StartShutdown();
    return "wallet encrypted; Bitcoin server stopping, restart to run with encrypted wallet. The keypool has been flushed, you need to make a new backup.";
}

Value settxfee(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 1)
        throw runtime_error(
            "settxfee amount\n"
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
        nAmount = AmountFromValue(params[0]);        // rejects 0.0 amounts

    nTransactionFee = nAmount;
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
            "  \"balance\": xxxxxxx,         (numeric) the total soypay balance of the wallet\n"
            "  \"txcount\": xxxxxxx,         (numeric) the total number of transactions in the wallet\n"
            "  \"keypoololdest\": xxxxxx,    (numeric) the timestamp (seconds since GMT epoch) of the oldest pre-generated key in the key pool\n"
            "  \"keypoolsize\": xxxx,        (numeric) how many new keys are pre-generated\n"
            "  \"unlocked_until\": ttt,      (numeric) the timestamp in seconds since epoch (midnight Jan 1 1970 GMT) that the wallet is unlocked for transfers, or 0 if the wallet is locked\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getwalletinfo", "")
            + HelpExampleRpc("getwalletinfo", "")
        );

    Object obj;
    obj.push_back(Pair("walletversion", pwalletMain->GetVersion()));
    obj.push_back(Pair("balance",       ValueFromAmount(pwalletMain->GetBalance(chainActive.Tip()->nHeight))));
    obj.push_back(Pair("Inblocktx",       (int)pwalletMain->mapInBlockTx.size()));
    obj.push_back(Pair("uncomfirmedtx", (int)pwalletMain->UnConfirmTx.size()));
    if (pwalletMain->IsCrypted())
        obj.push_back(Pair("unlocked_until", nWalletUnlockTime));
    return obj;
}
