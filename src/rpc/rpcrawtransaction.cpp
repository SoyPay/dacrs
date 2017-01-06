// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"
#include "core.h"
#include "init.h"
#include "keystore.h"
#include "main.h"
#include "net.h"
#include "rpcserver.h"
#include "uint256.h"
#include "../wallet/wallet.h"

#include <stdint.h>

#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"

using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;

//Value getrawtransaction(const Array& params, bool bHelp)
//{
//    if (bHelp || params.size() < 1 || params.size() > 2)
//        throw runtime_error(
//            "getrawtransaction \"txid\" ( verbose )\n"
//            "\nReturn the raw transaction data.\n"
//            "\nIf verbose=0, returns a string that is serialized, hex-encoded data for 'txid'.\n"
//            "If verbose is non-zero, returns an Object with information about 'txid'.\n"
//
//            "\nArguments:\n"
//            "1. \"txid\"      (string, required) The transaction id\n"
//            "2. verbose       (numeric, optional, default=0) If 0, return a string, other return a json object\n"
//
//            "\nResult (if verbose is not set or set to 0):\n"
//            "\"data\"      (string) The serialized, hex-encoded data for 'txid'\n"
//
//            "\nResult (if verbose > 0):\n"
//            "{\n"
//            "  \"hex\" : \"data\",       (string) The serialized, hex-encoded data for 'txid'\n"
//            "  \"txid\" : \"id\",        (string) The transaction id (same as provided)\n"
//            "  \"version\" : n,          (numeric) The version\n"
//            "  \"locktime\" : ttt,       (numeric) The lock time\n"
//            "  \"vin\" : [               (array of json objects)\n"
//            "     {\n"
//            "       \"txid\": \"id\",    (string) The transaction id\n"
//            "       \"vout\": n,         (numeric) \n"
//            "       \"scriptSig\": {     (json object) The script\n"
//            "         \"asm\": \"asm\",  (string) asm\n"
//            "         \"hex\": \"hex\"   (string) hex\n"
//            "       },\n"
//            "       \"sequence\": n      (numeric) The script sequence number\n"
//            "     }\n"
//            "     ,...\n"
//            "  ],\n"
//            "  \"vout\" : [              (array of json objects)\n"
//            "     {\n"
//            "       \"value\" : x.xxx,            (numeric) The value in btc\n"
//            "       \"n\" : n,                    (numeric) index\n"
//            "       \"scriptPubKey\" : {          (json object)\n"
//            "         \"asm\" : \"asm\",          (string) the asm\n"
//            "         \"hex\" : \"hex\",          (string) the hex\n"
//            "         \"reqSigs\" : n,            (numeric) The required sigs\n"
//            "         \"type\" : \"pubkeyhash\",  (string) The type, eg 'pubkeyhash'\n"
//            "         \"addresses\" : [           (json array of string)\n"
//            "           \"Dacrsaddress\"        (string) Dacrs address\n"
//            "           ,...\n"
//            "         ]\n"
//            "       }\n"
//            "     }\n"
//            "     ,...\n"
//            "  ],\n"
//            "  \"blockhash\" : \"hash\",   (string) the block hash\n"
//            "  \"confirmations\" : n,      (numeric) The confirmations\n"
//            "  \"time\" : ttt,             (numeric) The transaction time in seconds since epoch (Jan 1 1970 GMT)\n"
//            "  \"blocktime\" : ttt         (numeric) The block time in seconds since epoch (Jan 1 1970 GMT)\n"
//            "}\n"
//
//            "\nExamples:\n"
//            + HelpExampleCli("getrawtransaction", "\"mytxid\"")
//            + HelpExampleCli("getrawtransaction", "\"mytxid\" 1")
//            + HelpExampleRpc("getrawtransaction", "\"mytxid\", 1")
//        );
//
//    uint256 hash = ParseHashV(params[0], "parameter 1");
//
//    bool fVerbose = false;
//    if (params.size() > 1)
//        fVerbose = (params[1].get_int() != 0);
//
//    CTransaction tx;
//    uint256 hashBlock = 0;
////    if (!GetTransaction(hash, tx, hashBlock, true))
////        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No information available about transaction");
//
//    CDataStream ssTx(SER_NETWORK, g_sProtocolVersion);
//    ssTx << tx;
//    string strHex = HexStr(ssTx.begin(), ssTx.end());
//
//    if (!fVerbose)
//        return strHex;
//
//    Object result;
//    result.push_back(Pair("hex", strHex));
//    TxToJSON(tx, hashBlock, result);
//    return result;
//}


//Value createrawtransaction(const Array& params, bool bHelp)
//{
//    if (bHelp || params.size() != 2)
//        throw runtime_error(
//            "createrawtransaction [{\"txid\":\"id\",\"vout\":n},...] {\"address\":amount,...}\n"
//            "\nCreate a transaction spending the given inputs and sending to the given addresses.\n"
//            "Returns hex-encoded raw transaction.\n"
//            "Note that the transaction's inputs are not signed, and\n"
//            "it is not stored in the wallet or transmitted to the network.\n"
//
//            "\nArguments:\n"
//            "1. \"transactions\"        (string, required) A json array of json objects\n"
//            "     [\n"
//            "       {\n"
//            "         \"txid\":\"id\",  (string, required) The transaction id\n"
//            "         \"vout\":n        (numeric, required) The output number\n"
//            "       }\n"
//            "       ,...\n"
//            "     ]\n"
//            "2. \"addresses\"           (string, required) a json object with addresses as keys and amounts as values\n"
//            "    {\n"
//            "      \"address\": x.xxx   (numeric, required) The key is the Dacrs address, the value is the btc amount\n"
//            "      ,...\n"
//            "    }\n"
//
//            "\nResult:\n"
//            "\"transaction\"            (string) hex string of the transaction\n"
//
//            "\nExamples\n"
//            + HelpExampleCli("createrawtransaction", "\"[{\\\"txid\\\":\\\"myid\\\",\\\"vout\\\":0}]\" \"{\\\"address\\\":0.01}\"")
//            + HelpExampleRpc("createrawtransaction", "\"[{\\\"txid\\\":\\\"myid\\\",\\\"vout\\\":0}]\", \"{\\\"address\\\":0.01}\"")
//        );
//
//    RPCTypeCheck(params, list_of(array_type)(obj_type));
//
//    Array inputs = params[0].get_array();
//    Object sendTo = params[1].get_obj();
//
//    CTransaction rawtx;
//
//    for (const auto& input : inputs)
//    {
//        const Object& o = input.get_obj();
//
//        uint256 txid = ParseHashO(o, "txid");
//
//        const Value& vout_v = find_value(o, "vout");
//        if (vout_v.type() != int_type)
//            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, missing vout key");
//        int nOutput = vout_v.get_int();
//        if (nOutput < 0)
//            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, vout must be positive");
//
////        CTxIn in(COutPoint(txid, nOutput));
////        rawtx.vin.push_back(in);
//    }
//
//    set<CDacrsAddress> setAddress;
//    for(const auto& s : sendTo)
//    {
////        CDacrsAddress address(s.name_);
////        if (!address.IsValid())
////            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("Invalid Dacrs address: ")+s.name_);
////
////        if (setAddress.count(address))
////            throw JSONRPCError(RPC_INVALID_PARAMETER, string("Invalid parameter, duplicated address: ")+s.name_);
////        setAddress.insert(address);
////
////        CScript scriptPubKey;
////        scriptPubKey.SetDestination(address.Get());
////        int64_t nAmount = AmountFromValue(s.value_);
//
////        CTxOut out(nAmount, scriptPubKey);
////        rawtx.vout.push_back(out);
//    }
//
//    CDataStream ss(SER_NETWORK, g_sProtocolVersion);
//    ss << rawtx;
//    return HexStr(ss.begin(), ss.end());
//}

//Value decoderawtransaction(const Array& params, bool bHelp)
//{
//    if (bHelp || params.size() != 1)
//        throw runtime_error(
//            "decoderawtransaction \"hexstring\"\n"
//            "\nReturn a JSON object representing the serialized, hex-encoded transaction.\n"
//
//            "\nArguments:\n"
//            "1. \"hex\"      (string, required) The transaction hex string\n"
//
//            "\nResult:\n"
//            "{\n"
//            "  \"txid\" : \"id\",        (string) The transaction id\n"
//            "  \"version\" : n,          (numeric) The version\n"
//            "  \"locktime\" : ttt,       (numeric) The lock time\n"
//            "  \"vin\" : [               (array of json objects)\n"
//            "     {\n"
//            "       \"txid\": \"id\",    (string) The transaction id\n"
//            "       \"vout\": n,         (numeric) The output number\n"
//            "       \"scriptSig\": {     (json object) The script\n"
//            "         \"asm\": \"asm\",  (string) asm\n"
//            "         \"hex\": \"hex\"   (string) hex\n"
//            "       },\n"
//            "       \"sequence\": n     (numeric) The script sequence number\n"
//            "     }\n"
//            "     ,...\n"
//            "  ],\n"
//            "  \"vout\" : [             (array of json objects)\n"
//            "     {\n"
//            "       \"value\" : x.xxx,            (numeric) The value in btc\n"
//            "       \"n\" : n,                    (numeric) index\n"
//            "       \"scriptPubKey\" : {          (json object)\n"
//            "         \"asm\" : \"asm\",          (string) the asm\n"
//            "         \"hex\" : \"hex\",          (string) the hex\n"
//            "         \"reqSigs\" : n,            (numeric) The required sigs\n"
//            "         \"type\" : \"pubkeyhash\",  (string) The type, eg 'pubkeyhash'\n"
//            "         \"addresses\" : [           (json array of string)\n"
//            "           \"12tvKAXCxZjSmdNbao16dKXC8tRWfcF5oc\"   (string) Dacrs address\n"
//            "           ,...\n"
//            "         ]\n"
//            "       }\n"
//            "     }\n"
//            "     ,...\n"
//            "  ],\n"
//            "}\n"
//
//            "\nExamples:\n"
//            + HelpExampleCli("decoderawtransaction", "\"hexstring\"")
//            + HelpExampleRpc("decoderawtransaction", "\"hexstring\"")
//        );
//
//    vector<unsigned char> txData(ParseHexV(params[0], "argument"));
//    CDataStream ssData(txData, SER_NETWORK, g_sProtocolVersion);
//    CTransaction tx;
//    try {
//        ssData >> tx;
//    }
//    catch (std::exception &e) {
//        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX decode failed");
//    }
//
//    Object result;
//    TxToJSON(tx, 0, result);
//
//    return result;
//}

//Value decodescript(const Array& params, bool bHelp)
//{
//    if (bHelp || params.size() != 1)
//        throw runtime_error(
//            "decodescript \"hex\"\n"
//            "\nDecode a hex-encoded script.\n"
//            "\nArguments:\n"
//            "1. \"hex\"     (string) the hex encoded script\n"
//            "\nResult:\n"
//            "{\n"
//            "  \"asm\":\"asm\",   (string) Script public key\n"
//            "  \"hex\":\"hex\",   (string) hex encoded public key\n"
//            "  \"type\":\"type\", (string) The output type\n"
//            "  \"reqSigs\": n,    (numeric) The required signatures\n"
//            "  \"addresses\": [   (json array of string)\n"
//            "     \"address\"     (string) Dacrs address\n"
//            "     ,...\n"
//            "  ],\n"
//            "  \"p2sh\",\"address\" (string) script address\n"
//            "}\n"
//            "\nExamples:\n"
//            + HelpExampleCli("decodescript", "\"hexstring\"")
//            + HelpExampleRpc("decodescript", "\"hexstring\"")
//        );
//
//    RPCTypeCheck(params, list_of(str_type));
//
//   Object r;
////    CScript script;
////    if (params[0].get_str().size() > 0){
////        vector<unsigned char> scriptData(ParseHexV(params[0], "argument"));
////        script = CScript(scriptData.begin(), scriptData.end());
////    } else {
////        // Empty scripts are valid
////    }
////    ScriptPubKeyToJSON(script, r, false);
////
////    r.push_back(Pair("p2sh", CDacrsAddress(script.GetID()).ToString()));
//    return r;
//}

//Value signrawtransaction(const Array& params, bool bHelp)
//{
//    if (bHelp || params.size() < 1 || params.size() > 4)
//        throw runtime_error(
//            "signrawtransaction \"hexstring\" ( [{\"txid\":\"id\",\"vout\":n,\"scriptPubKey\":\"hex\",\"redeemScript\":\"hex\"},...] [\"privatekey1\",...] sighashtype )\n"
//            "\nSign inputs for raw transaction (serialized, hex-encoded).\n"
//            "The second optional argument (may be null) is an array of previous transaction outputs that\n"
//            "this transaction depends on but may not yet be in the block chain.\n"
//            "The third optional argument (may be null) is an array of base58-encoded private\n"
//            "keys that, if given, will be the only keys used to sign the transaction.\n"
//            + HelpRequiringPassphrase() + "\n"
//
//            "\nArguments:\n"
//            "1. \"hexstring\"     (string, required) The transaction hex string\n"
//            "2. \"prevtxs\"       (string, optional) An json array of previous dependent transaction outputs\n"
//            "     [               (json array of json objects, or 'null' if none provided)\n"
//            "       {\n"
//            "         \"txid\":\"id\",             (string, required) The transaction id\n"
//            "         \"vout\":n,                  (numeric, required) The output number\n"
//            "         \"scriptPubKey\": \"hex\",   (string, required) script key\n"
//            "         \"redeemScript\": \"hex\"    (string, required) redeem script\n"
//            "       }\n"
//            "       ,...\n"
//            "    ]\n"
//            "3. \"privatekeys\"     (string, optional) A json array of base58-encoded private keys for signing\n"
//            "    [                  (json array of strings, or 'null' if none provided)\n"
//            "      \"privatekey\"   (string) private key in base58-encoding\n"
//            "      ,...\n"
//            "    ]\n"
//            "4. \"sighashtype\"     (string, optional, default=ALL) The signature hash type. Must be one of\n"
//            "       \"ALL\"\n"
//            "       \"NONE\"\n"
//            "       \"SINGLE\"\n"
//            "       \"ALL|ANYONECANPAY\"\n"
//            "       \"NONE|ANYONECANPAY\"\n"
//            "       \"SINGLE|ANYONECANPAY\"\n"
//
//            "\nResult:\n"
//            "{\n"
//            "  \"hex\": \"value\",   (string) The raw transaction with signature(s) (hex-encoded string)\n"
//            "  \"complete\": n       (numeric) if transaction has a complete set of signature (0 if not)\n"
//            "}\n"
//
//            "\nExamples:\n"
//            + HelpExampleCli("signrawtransaction", "\"myhex\"")
//            + HelpExampleRpc("signrawtransaction", "\"myhex\"")
//        );
//
//    RPCTypeCheck(params, list_of(str_type)(array_type)(array_type)(str_type), true);
//
//    vector<unsigned char> txData(ParseHexV(params[0], "argument 1"));
//    CDataStream ssData(txData, SER_NETWORK, g_sProtocolVersion);
//    vector<CTransaction> txVariants;
//    while (!ssData.empty())
//    {
//        try {
//            CTransaction tx;
//            ssData >> tx;
//            txVariants.push_back(tx);
//        }
//        catch (std::exception &e) {
//            throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX decode failed");
//        }
//    }
//
//    if (txVariants.empty())
//        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Missing transaction");
//
//    // mergedTx will end up with all the signatures; it
//    // starts as a clone of the rawtx:
//    CTransaction mergedTx(txVariants[0]);
//    bool fComplete = true;
//
//    // Fetch previous transactions (inputs):
////    CCoinsView viewDummy;
////    CCoinsViewCache view(viewDummy);
////    {
////        LOCK(g_cTxMemPool.cs);
////        CCoinsViewCache &viewChain = *pcoinsTip;
////        CCoinsViewMemPool viewMempool(viewChain, g_cTxMemPool);
////        view.SetBackend(viewMempool); // temporarily switch cache backend to db+g_cTxMemPool view
////
////        BOOST_FOREACH(const CTxIn& txin, mergedTx.vin) {
////            const uint256& prevHash = txin.prevout.hash;
////            CCoins coins;
////            view.GetCoins(prevHash, coins); // this is certainly allowed to fail
////        }
////
////        view.SetBackend(viewDummy); // switch back to avoid locking g_cTxMemPool for too long
////    }
//
//    bool fGivenKeys = false;
//    CBasicKeyStore tempKeystore;
//    if (params.size() > 2 && params[2].type() != null_type)
//    {
//        fGivenKeys = true;
//        Array keys = params[2].get_array();
//        for (auto k : keys)
//        {
//            CDacrsSecret vchSecret;
//            bool fGood = vchSecret.SetString(k.get_str());
//            if (!fGood)
//                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key");
//            CKey key = vchSecret.GetKey();
//            tempKeystore.AddKey(key);
//        }
//    }
//    else
//        EnsureWalletIsUnlocked();
//
//    // Add previous txouts given in the RPC call:
//    if (params.size() > 1 && params[1].type() != null_type)
//    {
//        Array prevTxs = params[1].get_array();
//        for (auto& p : prevTxs)
//        {
//            if (p.type() != obj_type)
//                throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "expected object with {\"txid'\",\"vout\",\"scriptPubKey\"}");
//
//            Object prevOut = p.get_obj();
//
//            RPCTypeCheck(prevOut, map_list_of("txid", str_type)("vout", int_type)("scriptPubKey", str_type));
//
//            uint256 txid = ParseHashO(prevOut, "txid");
//
//            int nOut = find_value(prevOut, "vout").get_int();
//            if (nOut < 0)
//                throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "vout must be positive");
//
//            vector<unsigned char> pkData(ParseHexO(prevOut, "scriptPubKey"));
//            CScript scriptPubKey(pkData.begin(), pkData.end());
//
////            CCoins coins;
////            if (view.GetCoins(txid, coins)) {
////                if (coins.IsAvailable(nOut) && coins.vout[nOut].scriptPubKey != scriptPubKey) {
////                    string err("Previous output scriptPubKey mismatch:\n");
////                    err = err + coins.vout[nOut].scriptPubKey.ToString() + "\nvs:\n"+
////                        scriptPubKey.ToString();
////                    throw JSONRPCError(RPC_DESERIALIZATION_ERROR, err);
////                }
////                // what todo if txid is known, but the actual output isn't?
////            }
////            if ((unsigned int)nOut >= coins.vout.size())
////                coins.vout.resize(nOut+1);
////            coins.vout[nOut].scriptPubKey = scriptPubKey;
////            coins.vout[nOut].nValue = 0; // we don't know the actual output value
////            view.SetCoins(txid, coins);
//
//            // if redeemScript given and not using the local wallet (private keys
//            // given), add redeemScript to the tempKeystore so it can be signed:
////            if (fGivenKeys && scriptPubKey.IsPayToScriptHash())
////            {
////                RPCTypeCheck(prevOut, map_list_of("txid", str_type)("vout", int_type)("scriptPubKey", str_type)("redeemScript",str_type));
////                Value v = find_value(prevOut, "redeemScript");
////                if (!(v == Value::null))
////                {
////                    vector<unsigned char> rsData(ParseHexV(v, "redeemScript"));
////                    CScript redeemScript(rsData.begin(), rsData.end());
//////                  tempKeystore.AddCScript(redeemScript);
////                }
////            }
//        }
//    }
//

//    const CKeyStore& keystore = ((fGivenKeys || !pwalletMain) ? tempKeystore : *pwalletMain);

//
//    int nHashType = SIGHASH_ALL;
//    if (params.size() > 3 && params[3].type() != null_type)
//    {
//        static map<string, int> mapSigHashValues =
//            boost::assign::map_list_of
//            (string("ALL"), int(SIGHASH_ALL))
//            (string("ALL|ANYONECANPAY"), int(SIGHASH_ALL|SIGHASH_ANYONECANPAY))
//            (string("NONE"), int(SIGHASH_NONE))
//            (string("NONE|ANYONECANPAY"), int(SIGHASH_NONE|SIGHASH_ANYONECANPAY))
//            (string("SINGLE"), int(SIGHASH_SINGLE))
//            (string("SINGLE|ANYONECANPAY"), int(SIGHASH_SINGLE|SIGHASH_ANYONECANPAY))
//            ;
//        string strHashType = params[3].get_str();
//        if (mapSigHashValues.count(strHashType))
//            nHashType = mapSigHashValues[strHashType];
//        else
//            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid sighash param");
//    }
//
//    bool fHashSingle = ((nHashType & ~SIGHASH_ANYONECANPAY) == SIGHASH_SINGLE);
//
////    // Sign what we can:
////    for (unsigned int i = 0; i < mergedTx.vin.size(); i++)
////    {
////        CTxIn& txin = mergedTx.vin[i];
////        CCoins coins;
////        if (!view.GetCoins(txin.prevout.hash, coins) || !coins.IsAvailable(txin.prevout.n))
////        {
////            fComplete = false;
////            continue;
////        }
////        const CScript& prevPubKey = coins.vout[txin.prevout.n].scriptPubKey;
////
////        txin.scriptSig.clear();
////        // Only sign SIGHASH_SINGLE if there's a corresponding output:
////        if (!fHashSingle || (i < mergedTx.vout.size()))
////            SignSignature(keystore, prevPubKey, mergedTx, i, nHashType);
////
////        // ... and merge in other signatures:
////        BOOST_FOREACH(const CTransaction& txv, txVariants)
////        {
////            txin.scriptSig = CombineSignatures(prevPubKey, mergedTx, i, txin.scriptSig, txv.vin[i].scriptSig);
////        }
////        if (!VerifyScript(txin.scriptSig, prevPubKey, mergedTx, i, SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_STRICTENC, 0))
////            fComplete = false;
////    }
//
//    Object result;
//    CDataStream ssTx(SER_NETWORK, g_sProtocolVersion);
//    ssTx << mergedTx;
//    result.push_back(Pair("hex", HexStr(ssTx.begin(), ssTx.end())));
//    result.push_back(Pair("complete", fComplete));
//
//    return result;
//}
extern void SyncWithWallets(const uint256 &cHash, const CBaseTransaction *pTx, const CBlock *pblock);
//Value sendrawtransaction(const Array& params, bool bHelp)
//{
//    if (bHelp || params.size() < 1 || params.size() > 2)
//        throw runtime_error(
//            "sendrawtransaction \"hexstring\" ( allowhighfees )\n"
//            "\nSubmits raw transaction (serialized, hex-encoded) to local node and network.\n"
//            "\nAlso see createrawtransaction and signrawtransaction calls.\n"
//            "\nArguments:\n"
//            "1. \"hexstring\"    (string, required) The hex string of the raw transaction)\n"
//            "2. allowhighfees    (boolean, optional, default=false) Allow high fees\n"
//            "\nResult:\n"
//            "\"hex\"             (string) The transaction hash in hex\n"
//            "\nExamples:\n"
//            "\nCreate a transaction\n"
//            + HelpExampleCli("createrawtransaction", "\"[{\\\"txid\\\" : \\\"mytxid\\\",\\\"vout\\\":0}]\" \"{\\\"myaddress\\\":0.01}\"") +
//            "Sign the transaction, and get back the hex\n"
//            + HelpExampleCli("signrawtransaction", "\"myhex\"") +
//            "\nSend the transaction (signed hex)\n"
//            + HelpExampleCli("sendrawtransaction", "\"signedhex\"") +
//            "\nAs a json rpc call\n"
//            + HelpExampleRpc("sendrawtransaction", "\"signedhex\"")
//        );
//
//
//    // parse hex string from parameter
////    vector<unsigned char> txData(ParseHexV(params[0], "parameter"));
////    CDataStream ssData(txData, SER_NETWORK, g_sProtocolVersion);
////    CTransaction tx;
////
////    bool fOverrideFees = false;
////    if (params.size() > 1)
////        fOverrideFees = params[1].get_bool();
////
////    // deserialize binary data stream
////    try {
////        ssData >> tx;
////    }
////    catch (exception &e) {
////        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX decode failed");
////    }
////    uint256 hashTx = tx.GetHash();
////
////    CCoinsViewCache &view = *pcoinsTip;
////    CCoins existingCoins;
////    bool fHaveMempool = g_cTxMemPool.exists(hashTx);
////    bool fHaveChain = view.GetCoins(hashTx, existingCoins) && existingCoins.nHeight < 1000000000;
////    if (!fHaveMempool && !fHaveChain) {
////        // push to local node and sync with wallets
////        CValidationState state;
////        if (AcceptToMemoryPool(g_cTxMemPool, state, tx, false, NULL, !fOverrideFees))
////            SyncWithWallets(hashTx, (CBaseTransaction*)&tx, NULL);
////        else {
////            if(state.IsInvalid())
////                throw JSONRPCError(RPC_TRANSACTION_REJECTED, strprintf("%i: %s", state.GetRejectCode(), state.GetRejectReason()));
////            else
////                throw JSONRPCError(RPC_TRANSACTION_ERROR, state.GetRejectReason());
////        }
////    } else if (fHaveChain) {
////        throw JSONRPCError(RPC_TRANSACTION_ALREADY_IN_CHAIN, "transaction already in block chain");
////    }
////    RelayTransaction(tx, hashTx);
//
//    return 0;//hashTx.GetHex();
//}
